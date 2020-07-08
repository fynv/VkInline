namespace VkInline
{
	namespace Internal
	{
		AS::AS() {}

		AS::~AS()
		{
			const Context* ctx = Context::get_context();
			vkDestroyAccelerationStructureKHR(ctx->device(), m_structure, nullptr);
		}


		BaseLevelAS::BaseLevelAS(uint32_t geometryCount, const VkAccelerationStructureCreateGeometryTypeInfoKHR* geoTypeInfo, const VkAccelerationStructureGeometryKHR* pGeometries, const VkAccelerationStructureBuildOffsetInfoKHR* offsets)
		{
			const Context* ctx = Context::get_context();

			VkAccelerationStructureCreateInfoKHR acceleration_create_info = {};
			acceleration_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
			acceleration_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			acceleration_create_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
			acceleration_create_info.maxGeometryCount = geometryCount;
			acceleration_create_info.pGeometryInfos = geoTypeInfo;
			vkCreateAccelerationStructureKHR(ctx->device(), &acceleration_create_info, nullptr, &m_structure);

			VkDeviceSize scratchSizeInBytes = 0;
			VkDeviceSize resultSizeInBytes = 0;
			{

				VkAccelerationStructureMemoryRequirementsInfoKHR  memoryRequirementsInfo = {};
				memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR;
				memoryRequirementsInfo.buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR;
				memoryRequirementsInfo.accelerationStructure = m_structure;

				VkMemoryRequirements2 memoryRequirements = {};
				memoryRequirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
				memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_KHR;
				vkGetAccelerationStructureMemoryRequirementsKHR(ctx->device(), &memoryRequirementsInfo, &memoryRequirements);
				resultSizeInBytes = memoryRequirements.memoryRequirements.size;
				memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_KHR;
				vkGetAccelerationStructureMemoryRequirementsKHR(ctx->device(), &memoryRequirementsInfo, &memoryRequirements);
				scratchSizeInBytes = memoryRequirements.memoryRequirements.size;
				memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_KHR;
				vkGetAccelerationStructureMemoryRequirementsKHR(ctx->device(), &memoryRequirementsInfo, &memoryRequirements);
				if (memoryRequirements.memoryRequirements.size > scratchSizeInBytes) scratchSizeInBytes = memoryRequirements.memoryRequirements.size;
			}

			m_scratchBuffer = new DeviceBuffer(scratchSizeInBytes, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR);
			m_resultBuffer = new DeviceBuffer(resultSizeInBytes, VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR);

			VkBindAccelerationStructureMemoryInfoKHR bind_acceleration_memory_info = {};
			bind_acceleration_memory_info.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_KHR;
			bind_acceleration_memory_info.accelerationStructure = m_structure;
			bind_acceleration_memory_info.memory = m_resultBuffer->memory();
			vkBindAccelerationStructureMemoryKHR(ctx->device(), 1, &bind_acceleration_memory_info);

			VkAccelerationStructureBuildGeometryInfoKHR acceleration_build_geometry_info = {};
			acceleration_build_geometry_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
			acceleration_build_geometry_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			acceleration_build_geometry_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
			acceleration_build_geometry_info.update = VK_FALSE;
			acceleration_build_geometry_info.dstAccelerationStructure = m_structure;
			acceleration_build_geometry_info.geometryArrayOfPointers = VK_FALSE;
			acceleration_build_geometry_info.geometryCount = geometryCount;
			acceleration_build_geometry_info.ppGeometries = &pGeometries;
			acceleration_build_geometry_info.scratchData.deviceAddress = m_scratchBuffer->get_device_address();

			auto cmdBuf = new AutoCommandBuffer;
			vkCmdBuildAccelerationStructureKHR(cmdBuf->buf(), 1, &acceleration_build_geometry_info, &offsets);
			ctx->SubmitCommandBuffer(cmdBuf);
		}

		BaseLevelAS::~BaseLevelAS()
		{
			delete m_resultBuffer;
			delete m_scratchBuffer;
		}

		TopLevelAS::TopLevelAS(size_t num_hitgroups, size_t* num_instances, const VkAccelerationStructureKHR** pblases, const float*** ptransforms)
		{
			const Context* ctx = Context::get_context();

			size_t total = 0;
			for (int i = 0; i < num_hitgroups; i++)
				total += num_instances[i];

			VkAccelerationStructureCreateGeometryTypeInfoKHR acceleration_create_geometry_info = {};
			acceleration_create_geometry_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR;
			acceleration_create_geometry_info.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
			acceleration_create_geometry_info.maxPrimitiveCount = (unsigned)total;

			VkAccelerationStructureCreateInfoKHR acceleration_create_info = {};
			acceleration_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
			acceleration_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
			acceleration_create_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
			acceleration_create_info.maxGeometryCount = 1;
			acceleration_create_info.pGeometryInfos = &acceleration_create_geometry_info;
			vkCreateAccelerationStructureKHR(ctx->device(), &acceleration_create_info, nullptr, &m_structure);

			VkDeviceSize scratchSizeInBytes, resultSizeInBytes, instanceDescsSizeInBytes;

			{
				VkAccelerationStructureMemoryRequirementsInfoKHR  memoryRequirementsInfo = {};
				memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR;
				memoryRequirementsInfo.buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR;
				memoryRequirementsInfo.accelerationStructure = m_structure;

				VkMemoryRequirements2 memoryRequirements = {};
				memoryRequirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
				memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_KHR;
				vkGetAccelerationStructureMemoryRequirementsKHR(ctx->device(), &memoryRequirementsInfo, &memoryRequirements);
				resultSizeInBytes = memoryRequirements.memoryRequirements.size;

				memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_KHR;
				vkGetAccelerationStructureMemoryRequirementsKHR(ctx->device(), &memoryRequirementsInfo, &memoryRequirements);
				scratchSizeInBytes = memoryRequirements.memoryRequirements.size;

				memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_KHR;
				vkGetAccelerationStructureMemoryRequirementsKHR(ctx->device(), &memoryRequirementsInfo, &memoryRequirements);
				if (memoryRequirements.memoryRequirements.size > scratchSizeInBytes) scratchSizeInBytes = memoryRequirements.memoryRequirements.size;

				instanceDescsSizeInBytes = total * sizeof(VkAccelerationStructureInstanceKHR);
			}

			m_scratchBuffer = new DeviceBuffer(scratchSizeInBytes, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR);
			m_resultBuffer = new DeviceBuffer(resultSizeInBytes, VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR);
			m_instancesBuffer = new DeviceBuffer(instanceDescsSizeInBytes, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR);

			VkBindAccelerationStructureMemoryInfoKHR bind_acceleration_memory_info = {};
			bind_acceleration_memory_info.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_KHR;
			bind_acceleration_memory_info.accelerationStructure = m_structure;
			bind_acceleration_memory_info.memory = m_resultBuffer->memory();
			vkBindAccelerationStructureMemoryKHR(ctx->device(), 1, &bind_acceleration_memory_info);


			std::vector<VkAccelerationStructureInstanceKHR> geometryInstances(total);
			unsigned k = 0;
			for (int i = 0; i < num_hitgroups; i++)
			{
				for (int j = 0; j < num_instances[i]; j++, k++)
				{
					VkAccelerationStructureInstanceKHR& gInst = geometryInstances[k];
					const float *ptrans = ptransforms[i][j];

					gInst.transform.matrix[0][0] = ptrans[0];
					gInst.transform.matrix[1][0] = ptrans[1];
					gInst.transform.matrix[2][0] = ptrans[2];
					gInst.transform.matrix[0][1] = ptrans[4];
					gInst.transform.matrix[1][1] = ptrans[5];
					gInst.transform.matrix[2][1] = ptrans[6];
					gInst.transform.matrix[0][2] = ptrans[8];
					gInst.transform.matrix[1][2] = ptrans[9];
					gInst.transform.matrix[2][2] = ptrans[10];
					gInst.transform.matrix[0][3] = ptrans[12];
					gInst.transform.matrix[1][3] = ptrans[13];
					gInst.transform.matrix[2][3] = ptrans[14];
					
					gInst.instanceCustomIndex = j;
					gInst.mask = 0xFF;
					gInst.instanceShaderBindingTableRecordOffset = i;
					gInst.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;

					VkAccelerationStructureDeviceAddressInfoKHR acceleration_device_address_info{};
					acceleration_device_address_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
					acceleration_device_address_info.accelerationStructure = pblases[i][j];

					uint64_t accelerationStructureHandle = vkGetAccelerationStructureDeviceAddressKHR(ctx->device(), &acceleration_device_address_info);
					gInst.accelerationStructureReference = accelerationStructureHandle;

				}
			}

			m_instancesBuffer->upload(geometryInstances.data());

			VkAccelerationStructureGeometryKHR geo_instance = {};
			geo_instance.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
			geo_instance.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
			geo_instance.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
			geo_instance.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
			geo_instance.geometry.instances.arrayOfPointers = VK_FALSE;
			geo_instance.geometry.instances.data.deviceAddress = m_instancesBuffer->get_device_address();

			VkAccelerationStructureGeometryKHR* p_geo_instance = &geo_instance;

			VkAccelerationStructureBuildGeometryInfoKHR acceleration_build_geometry_info = {};
			acceleration_build_geometry_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
			acceleration_build_geometry_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
			acceleration_build_geometry_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
			acceleration_build_geometry_info.update = VK_FALSE;
			acceleration_build_geometry_info.srcAccelerationStructure = VK_NULL_HANDLE;
			acceleration_build_geometry_info.dstAccelerationStructure = m_structure;
			acceleration_build_geometry_info.geometryArrayOfPointers = VK_FALSE;
			acceleration_build_geometry_info.geometryCount = 1;
			acceleration_build_geometry_info.ppGeometries = &p_geo_instance;
			acceleration_build_geometry_info.scratchData.deviceAddress = m_scratchBuffer->get_device_address();

			VkAccelerationStructureBuildOffsetInfoKHR acceleration_build_offset_info = {};
			acceleration_build_offset_info.primitiveCount = (unsigned)total;
			acceleration_build_offset_info.primitiveOffset = 0x0;
			acceleration_build_offset_info.firstVertex = 0;
			acceleration_build_offset_info.transformOffset = 0x0;

			VkAccelerationStructureBuildOffsetInfoKHR* p_acceleration_build_offset_info = &acceleration_build_offset_info;

			auto cmdBuf = new AutoCommandBuffer;
			vkCmdBuildAccelerationStructureKHR(cmdBuf->buf(), 1, &acceleration_build_geometry_info, &p_acceleration_build_offset_info);			
			ctx->SubmitCommandBuffer(cmdBuf);

		}

		TopLevelAS::~TopLevelAS()
		{
			delete m_instancesBuffer;
			delete m_resultBuffer;
			delete m_scratchBuffer;
		}

	}
}
