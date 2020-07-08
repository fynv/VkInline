namespace VkInline
{
	BaseLevelAS::BaseLevelAS(SVBuffer* indBuf, SVBuffer* posBuf)
	{
		VkAccelerationStructureCreateGeometryTypeInfoKHR acceleration_create_geometry_info = {};
		acceleration_create_geometry_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR;
		acceleration_create_geometry_info.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		acceleration_create_geometry_info.maxPrimitiveCount = (unsigned)(indBuf == nullptr ? posBuf->size() / 3: indBuf->size()/3);
		acceleration_create_geometry_info.indexType = VK_INDEX_TYPE_NONE_KHR;
		if (indBuf != nullptr)
			acceleration_create_geometry_info.indexType = indBuf->elem_size() > 2? VK_INDEX_TYPE_UINT32: VK_INDEX_TYPE_UINT16;
		acceleration_create_geometry_info.maxVertexCount = (unsigned)posBuf->size();

		if (posBuf->name_elem_type() == "vec2")
			acceleration_create_geometry_info.vertexFormat = VK_FORMAT_R32G32_SFLOAT;
		else if(posBuf->name_elem_type() == "vec3")
			acceleration_create_geometry_info.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;

		VkAccelerationStructureGeometryKHR acceleration_geometry = {};
		acceleration_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		acceleration_geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		acceleration_geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		acceleration_geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		acceleration_geometry.geometry.triangles.vertexFormat = acceleration_create_geometry_info.vertexFormat;
		acceleration_geometry.geometry.triangles.vertexData.deviceAddress = posBuf->internal()->get_device_address();
		acceleration_geometry.geometry.triangles.vertexStride = posBuf->elem_size();
		acceleration_geometry.geometry.triangles.indexType = acceleration_create_geometry_info.indexType;
		acceleration_geometry.geometry.triangles.indexData.deviceAddress = indBuf==nullptr? 0: indBuf->internal()->get_device_address();

		VkAccelerationStructureBuildOffsetInfoKHR acceleration_build_offset_info = {};
		acceleration_build_offset_info.primitiveCount = (unsigned)(indBuf == nullptr ? posBuf->size() / 3 : indBuf->size() / 3);
		acceleration_build_offset_info.primitiveOffset = 0x0;
		acceleration_build_offset_info.firstVertex = 0;
		acceleration_build_offset_info.transformOffset = 0x0;

		m_blas = new Internal::BaseLevelAS(1, &acceleration_create_geometry_info, &acceleration_geometry, &acceleration_build_offset_info);
	}

	BaseLevelAS::BaseLevelAS(SVBuffer* aabbBuf)
	{
		VkAccelerationStructureCreateGeometryTypeInfoKHR acceleration_create_geometry_info = {};
		acceleration_create_geometry_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR;
		acceleration_create_geometry_info.geometryType = VK_GEOMETRY_TYPE_AABBS_KHR;
		acceleration_create_geometry_info.maxPrimitiveCount = 1;

		VkAccelerationStructureGeometryKHR acceleration_geometry = {};
		acceleration_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		acceleration_geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		acceleration_geometry.geometryType = VK_GEOMETRY_TYPE_AABBS_KHR;
		acceleration_geometry.geometry.aabbs.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
		acceleration_geometry.geometry.aabbs.data.deviceAddress = aabbBuf->internal()->get_device_address();
		acceleration_geometry.geometry.aabbs.stride = 0;

		VkAccelerationStructureBuildOffsetInfoKHR acceleration_build_offset_info{};
		acceleration_build_offset_info.primitiveCount = 1;
		acceleration_build_offset_info.primitiveOffset = 0x0;
		acceleration_build_offset_info.firstVertex = 0;
		acceleration_build_offset_info.transformOffset = 0x0;

		m_blas = new Internal::BaseLevelAS(1, &acceleration_create_geometry_info, &acceleration_geometry, &acceleration_build_offset_info);
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

}
