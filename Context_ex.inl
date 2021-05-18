namespace VkInline
{
	BaseLevelAS::BaseLevelAS(SVBuffer* indBuf, SVBuffer* posBuf)
	{
		VkAccelerationStructureGeometryKHR acceleration_geometry = {};
		acceleration_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		acceleration_geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		acceleration_geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		acceleration_geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		acceleration_geometry.geometry.triangles.vertexData.deviceAddress = posBuf->internal()->get_device_address();		
		acceleration_geometry.geometry.triangles.maxVertex = (unsigned)posBuf->size();
		acceleration_geometry.geometry.triangles.vertexStride = posBuf->elem_size();		
		acceleration_geometry.geometry.triangles.indexData.deviceAddress = indBuf == nullptr ? 0 : indBuf->internal()->get_device_address();

		if (posBuf->name_elem_type() == "vec2")
			acceleration_geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32_SFLOAT;
		else if (posBuf->name_elem_type() == "vec3")
			acceleration_geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;

		acceleration_geometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_KHR;
		if (indBuf != nullptr)
			acceleration_geometry.geometry.triangles.indexType = indBuf->elem_size() > 2 ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16;

		VkAccelerationStructureBuildGeometryInfoKHR geoBuildInfo{};
		geoBuildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		geoBuildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		geoBuildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		geoBuildInfo.geometryCount = 1;
		geoBuildInfo.pGeometries = &acceleration_geometry;
		
		VkAccelerationStructureBuildRangeInfoKHR range{};
		range.primitiveCount = (unsigned)(indBuf == nullptr ? posBuf->size() / 3 : indBuf->size() / 3);
		range.primitiveOffset = 0;
		range.firstVertex = 0;
		range.transformOffset = 0;
		const VkAccelerationStructureBuildRangeInfoKHR* ranges = &range;		

		m_blas = new Internal::BaseLevelAS(geoBuildInfo, &acceleration_geometry, &ranges);
	}

	BaseLevelAS::BaseLevelAS(SVBuffer* aabbBuf)
	{
		VkAccelerationStructureGeometryKHR acceleration_geometry = {};
		acceleration_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		acceleration_geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		acceleration_geometry.geometryType = VK_GEOMETRY_TYPE_AABBS_KHR;
		acceleration_geometry.geometry.aabbs.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
		acceleration_geometry.geometry.aabbs.data.deviceAddress = aabbBuf->internal()->get_device_address();
		acceleration_geometry.geometry.aabbs.stride = 0;

		VkAccelerationStructureBuildGeometryInfoKHR geoBuildInfo{};
		geoBuildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		geoBuildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		geoBuildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		geoBuildInfo.geometryCount = 1;
		geoBuildInfo.pGeometries = &acceleration_geometry;

		VkAccelerationStructureBuildRangeInfoKHR range{};
		range.primitiveCount = 1;
		range.primitiveOffset = 0;
		range.firstVertex = 0;
		range.transformOffset = 0;
		const VkAccelerationStructureBuildRangeInfoKHR* ranges = &range;

		m_blas = new Internal::BaseLevelAS(geoBuildInfo, &acceleration_geometry, &ranges);
	}

	BaseLevelAS::~BaseLevelAS()
	{
		delete m_blas;
	}

	Mat4::Mat4(const float* data)
	{
		memcpy(m_data, data, sizeof(float) * 16);
	}

	TopLevelAS::TopLevelAS(const std::vector<std::vector<BLAS_EX>>& blases)
	{
		size_t num_hitgroups = blases.size();
		std::vector<size_t> num_instances(num_hitgroups);
		std::vector<std::vector<VkAccelerationStructureKHR>> vblases(num_hitgroups);
		std::vector<const VkAccelerationStructureKHR*> pblases(num_hitgroups);
		std::vector<std::vector<const float*>> vtrans(num_hitgroups);
		std::vector<const float**> ptrans(num_hitgroups);

		for (size_t i = 0; i < num_hitgroups; i++)
		{
			size_t num_blases = blases[i].size();
			num_instances[i] = num_blases;
			vblases[i].resize(num_blases);
			vtrans[i].resize(num_blases);
			for (size_t j = 0; j < num_blases; j++)
			{
				vblases[i][j] = blases[i][j].blas->internal()->structure();
				vtrans[i][j] = blases[i][j].trans->data();
			}
			pblases[i] = vblases[i].data();
			ptrans[i] = vtrans[i].data();
		}

		m_tlas = new Internal::TopLevelAS(num_hitgroups, num_instances.data(), pblases.data(), ptrans.data());
	}

	TopLevelAS::~TopLevelAS()
	{
		delete m_tlas;
	}

	BodyHitShaders::BodyHitShaders(const char* body_closest_hit, const char* body_intersection)
	{
		m_body_closest_hit = body_closest_hit;
		if (body_intersection!=nullptr)
			m_body_intersection = body_intersection;
	}

	const char* BodyHitShaders::body_closest_hit() const
	{
		return m_body_closest_hit.c_str();
	}

	const char* BodyHitShaders::body_intersection() const
	{
		if (m_body_intersection.empty()) return nullptr;
		return m_body_intersection.c_str();
	}

	RayTracer::RayTracer(const std::vector<const char*>& param_names, const char* body_raygen, const std::vector<const char*>& body_miss, const std::vector<const BodyHitShaders*>& body_hit, unsigned maxRecursionDepth, bool type_locked)
		: m_param_names(param_names.size()), m_body_raygen(body_raygen), m_body_miss(body_miss.size()), m_body_hit(body_hit), m_maxRecursionDepth(maxRecursionDepth), m_type_locked(type_locked)
	{
		for (size_t i = 0; i < param_names.size(); i++)
			m_param_names[i] = param_names[i];

		for (size_t i = 0; i < body_miss.size(); i++)
			m_body_miss[i] = body_miss[i];	

		m_kid = (unsigned)(-1);
	}

	bool RayTracer::launch(dim_type glbDim, const ShaderViewable** args, 
		const std::vector<TopLevelAS*>& arr_tlas, const std::vector<Texture2D*>& tex2ds, const std::vector<Texture3D*>& tex3ds, const std::vector<Cubemap*>& cubemaps, size_t times_submission)
	{
		Context& ctx = Context::get_context();
		if (!m_type_locked)
		{
			std::vector<CapturedShaderViewable> arg_map(m_param_names.size());
			for (size_t i = 0; i < m_param_names.size(); i++)
			{
				arg_map[i].obj_name = m_param_names[i].c_str();
				arg_map[i].obj = args[i];
			}

			std::vector<const char*> p_body_miss(m_body_miss.size());
			for (size_t i = 0; i < m_body_miss.size(); i++)
				p_body_miss[i] = m_body_miss[i].c_str();

			return ctx.launch_raytrace(glbDim, arg_map, m_maxRecursionDepth, 
				arr_tlas, tex2ds, tex3ds, cubemaps,	m_body_raygen.c_str(), p_body_miss, m_body_hit, times_submission);
		}
		else
		{
			std::unique_lock<std::mutex> locker(m_mu_type_lock);
			if (m_kid == (unsigned)(-1))
			{
				std::vector<CapturedShaderViewable> arg_map(m_param_names.size());
				for (size_t i = 0; i < m_param_names.size(); i++)
				{
					arg_map[i].obj_name = m_param_names[i].c_str();
					arg_map[i].obj = args[i];
				}
				std::vector<const char*> p_body_miss(m_body_miss.size());
				for (size_t i = 0; i < m_body_miss.size(); i++)
					p_body_miss[i] = m_body_miss[i].c_str();

				m_offsets.resize(m_param_names.size() + 1);
				return ctx.launch_raytrace(glbDim, arg_map, m_maxRecursionDepth, 
					arr_tlas, tex2ds, tex3ds, cubemaps,	m_body_raygen.c_str(), p_body_miss, m_body_hit, m_kid, m_offsets.data(), times_submission);
			}
			else
			{
				locker.unlock();
				return ctx.launch_raytrace(glbDim, m_param_names.size(), args, 
					arr_tlas.data(), tex2ds.data(), tex3ds.data(), cubemaps.data(), m_kid, m_offsets.data(), times_submission);
			}
		}
	}

}
