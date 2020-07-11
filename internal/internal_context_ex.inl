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

		RayTracePipeline::RayTracePipeline(const std::vector<unsigned>& spv_raygen,
			const std::vector<const std::vector<unsigned>*>& spv_miss,
			const std::vector<HitShaders>& spv_hit,
			unsigned maxRecursionDepth, size_t num_tlas, size_t num_tex2d, size_t num_tex3d, size_t num_cubemap)
		{
			m_sampler = nullptr;
			const Context* ctx = Context::get_context();

			static const char s_main[] = "main";

			std::vector<VkPipelineShaderStageCreateInfo> stages;
			std::vector<VkRayTracingShaderGroupCreateInfoKHR> groups;

			VkShaderModule shader_module_raygen;
			std::vector<VkShaderModule> shader_modules_miss(spv_miss.size());
			std::vector<VkShaderModule> shader_modules_closest_hit(spv_hit.size());
			std::vector<VkShaderModule> shader_modules_intersection(spv_hit.size(), nullptr);

			{
				VkShaderModuleCreateInfo createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				createInfo.codeSize = spv_raygen.size() * sizeof(unsigned);
				createInfo.pCode = reinterpret_cast<const uint32_t*>(spv_raygen.data());
				vkCreateShaderModule(ctx->device(), &createInfo, nullptr, &shader_module_raygen);

				VkRayTracingShaderGroupCreateInfoKHR group = {};
				group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
				group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
				group.generalShader = (unsigned)stages.size();
				group.closestHitShader = VK_SHADER_UNUSED_KHR;
				group.anyHitShader = VK_SHADER_UNUSED_KHR;
				group.intersectionShader = VK_SHADER_UNUSED_KHR;

				VkPipelineShaderStageCreateInfo stage = {};
				stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				stage.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
				stage.module = shader_module_raygen;
				stage.pName = s_main;
				stages.push_back(stage);			
				groups.push_back(group);
			}

			for (size_t i = 0; i < spv_miss.size(); i++)
			{
				VkShaderModuleCreateInfo createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				createInfo.codeSize = spv_miss[i]->size() * sizeof(unsigned);
				createInfo.pCode = reinterpret_cast<const uint32_t*>(spv_miss[i]->data());
				vkCreateShaderModule(ctx->device(), &createInfo, nullptr, &shader_modules_miss[i]);

				VkRayTracingShaderGroupCreateInfoKHR group = {};
				group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
				group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
				group.generalShader = (unsigned)stages.size();
				group.closestHitShader = VK_SHADER_UNUSED_KHR;
				group.anyHitShader = VK_SHADER_UNUSED_KHR;
				group.intersectionShader = VK_SHADER_UNUSED_KHR;

				VkPipelineShaderStageCreateInfo stage = {};
				stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				stage.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
				stage.module = shader_modules_miss[i];
				stage.pName = s_main;
				stages.push_back(stage);				
				groups.push_back(group);
			}

			for (size_t i = 0; i < spv_hit.size(); i++)
			{
				VkRayTracingShaderGroupCreateInfoKHR group = {};
				group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
				group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
				group.generalShader = VK_SHADER_UNUSED_KHR;
				group.closestHitShader = (unsigned)stages.size(); 
				group.anyHitShader = VK_SHADER_UNUSED_KHR;
				group.intersectionShader = VK_SHADER_UNUSED_KHR;

				{
					VkShaderModuleCreateInfo createInfo = {};
					createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
					createInfo.codeSize = spv_hit[i].closest_hit->size() * sizeof(unsigned);
					createInfo.pCode = reinterpret_cast<const uint32_t*>(spv_hit[i].closest_hit->data());
					vkCreateShaderModule(ctx->device(), &createInfo, nullptr, &shader_modules_closest_hit[i]);

					VkPipelineShaderStageCreateInfo stage = {};
					stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
					stage.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
					stage.module = shader_modules_closest_hit[i];
					stage.pName = s_main;
					stages.push_back(stage);

				}

				if (spv_hit[i].intersection!=nullptr)
				{
					VkShaderModuleCreateInfo createInfo = {};
					createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
					createInfo.codeSize = spv_hit[i].intersection->size() * sizeof(unsigned);
					createInfo.pCode = reinterpret_cast<const uint32_t*>(spv_hit[i].intersection->data());
					vkCreateShaderModule(ctx->device(), &createInfo, nullptr, &shader_modules_intersection[i]);

					group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
					group.intersectionShader = (unsigned)stages.size();

					VkPipelineShaderStageCreateInfo stage = {};
					stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
					stage.stage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
					stage.module = shader_modules_intersection[i];
					stage.pName = s_main;
					stages.push_back(stage);					
				}
				groups.push_back(group);
			}

			{
				VkFlags all_ray_trace_bits = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR | 
					VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR | VK_SHADER_STAGE_INTERSECTION_BIT_KHR | VK_SHADER_STAGE_CALLABLE_BIT_KHR;

				std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings(1);
				descriptorSetLayoutBindings[0] = {};
				descriptorSetLayoutBindings[0].binding = 0;
				descriptorSetLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorSetLayoutBindings[0].descriptorCount = 1;
				descriptorSetLayoutBindings[0].stageFlags = all_ray_trace_bits;

				m_num_tlas = num_tlas;
				if (num_tlas > 0)
				{
					VkDescriptorSetLayoutBinding binding_tlas = {};
					binding_tlas.binding = 10;
					binding_tlas.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
					binding_tlas.descriptorCount = (unsigned)num_tlas;
					binding_tlas.stageFlags = all_ray_trace_bits;
					descriptorSetLayoutBindings.push_back(binding_tlas);
				}

				m_num_tex2d = num_tex2d;
				if (num_tex2d > 0)
				{
					VkDescriptorSetLayoutBinding binding_tex2d = {};
					binding_tex2d.binding = 1;
					binding_tex2d.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					binding_tex2d.descriptorCount = (unsigned)num_tex2d;
					binding_tex2d.stageFlags = all_ray_trace_bits;
					descriptorSetLayoutBindings.push_back(binding_tex2d);
				}

				m_num_tex3d = num_tex3d;
				if (num_tex3d > 0)
				{
					VkDescriptorSetLayoutBinding binding_tex3d = {};
					binding_tex3d.binding = 2;
					binding_tex3d.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					binding_tex3d.descriptorCount = (unsigned)num_tex3d;
					binding_tex3d.stageFlags = all_ray_trace_bits;
					descriptorSetLayoutBindings.push_back(binding_tex3d);
				}

				m_num_cubemap = num_cubemap;
				if (num_cubemap > 0)
				{
					VkDescriptorSetLayoutBinding binding_cubemap = {};
					binding_cubemap.binding = 3;
					binding_cubemap.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					binding_cubemap.descriptorCount = (unsigned)num_cubemap;
					binding_cubemap.stageFlags = all_ray_trace_bits;
					descriptorSetLayoutBindings.push_back(binding_cubemap);
				}

				if (num_tex2d > 0 || num_tex3d > 0 || num_cubemap > 0)
				{
					m_sampler = new Sampler;
				}

				VkDescriptorSetLayoutCreateInfo layoutInfo = {};
				layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				layoutInfo.bindingCount = (unsigned)descriptorSetLayoutBindings.size();
				layoutInfo.pBindings = descriptorSetLayoutBindings.data();

				vkCreateDescriptorSetLayout(ctx->device(), &layoutInfo, nullptr, &m_descriptorSetLayout);
			}

			{
				VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
				pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
				pipelineLayoutInfo.setLayoutCount = 1;
				pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
				vkCreatePipelineLayout(ctx->device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout);


				VkRayTracingPipelineCreateInfoKHR rayPipelineInfo = {};
				rayPipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
				rayPipelineInfo.stageCount = (unsigned)stages.size();
				rayPipelineInfo.pStages = stages.data();
				rayPipelineInfo.groupCount = (unsigned)groups.size();
				rayPipelineInfo.pGroups = groups.data();
				rayPipelineInfo.maxRecursionDepth = maxRecursionDepth;
				rayPipelineInfo.layout = m_pipelineLayout;
				rayPipelineInfo.libraries.sType = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR;

				vkCreateRayTracingPipelinesKHR(ctx->device(), nullptr, 1, &rayPipelineInfo, nullptr, &m_pipeline);
			}			

			for (size_t i = 0; i < spv_hit.size(); i++)
			{
				vkDestroyShaderModule(ctx->device(), shader_modules_closest_hit[i], nullptr);
				if (shader_modules_intersection[i]!=nullptr)
					vkDestroyShaderModule(ctx->device(), shader_modules_intersection[i], nullptr);
			}
			for (size_t i = 0; i < spv_miss.size(); i++)
			{
				vkDestroyShaderModule(ctx->device(), shader_modules_miss[i], nullptr);
			}
			vkDestroyShaderModule(ctx->device(), shader_module_raygen, nullptr);

			{
				unsigned progIdSize = ctx->raytracing_properties().shaderGroupHandleSize;
				unsigned baseAlignment = ctx->raytracing_properties().shaderGroupBaseAlignment;
				unsigned sbtSize_get = progIdSize * (unsigned)groups.size();
				unsigned sbtSize = baseAlignment * (unsigned)groups.size();

				m_shaderBindingTableBuffer = new DeviceBuffer(sbtSize, 0);

				unsigned char* shaderHandleStorage_get = (unsigned char*)malloc(sbtSize_get);
				unsigned char* shaderHandleStorage = (unsigned char*)malloc(sbtSize);

				vkGetRayTracingShaderGroupHandlesKHR(ctx->device(), m_pipeline, 0, (unsigned)groups.size(), sbtSize_get, shaderHandleStorage_get);
				for (unsigned i = 0; i < (unsigned)groups.size(); i++)
					memcpy(shaderHandleStorage + i * baseAlignment, shaderHandleStorage_get + i * progIdSize, progIdSize);

				m_shaderBindingTableBuffer->upload(shaderHandleStorage);
				free(shaderHandleStorage);
				free(shaderHandleStorage_get);

				m_sbt_entry_raygen = {};
				m_sbt_entry_raygen.buffer = m_shaderBindingTableBuffer->buf();
				m_sbt_entry_raygen.offset = 0;
				m_sbt_entry_raygen.stride = baseAlignment;
				m_sbt_entry_raygen.size = baseAlignment;

				m_sbt_entry_miss = {};
				m_sbt_entry_miss.buffer = m_shaderBindingTableBuffer->buf();
				m_sbt_entry_miss.offset = baseAlignment;
				m_sbt_entry_miss.stride = baseAlignment;
				m_sbt_entry_miss.size = baseAlignment* spv_miss.size();

				m_sbt_entry_hit = {};
				m_sbt_entry_hit.buffer = m_shaderBindingTableBuffer->buf();
				m_sbt_entry_hit.offset = m_sbt_entry_miss.offset + m_sbt_entry_miss.size;
				m_sbt_entry_hit.stride = baseAlignment;
				m_sbt_entry_hit.size = baseAlignment * spv_hit.size();

				m_sbt_entry_callable = {};
			}
			
		}

		RayTracePipeline::~RayTracePipeline()
		{
			auto iter = m_recyclers.begin();
			while (iter != m_recyclers.end())
			{
				delete iter->second;
				iter++;
			}
			delete m_shaderBindingTableBuffer;

			const Context* ctx = Context::get_context();
			vkDestroyPipeline(ctx->device(), m_pipeline, nullptr);
			vkDestroyPipelineLayout(ctx->device(), m_pipelineLayout, nullptr);
			vkDestroyDescriptorSetLayout(ctx->device(), m_descriptorSetLayout, nullptr);
			delete m_sampler;
		}


		CommandBufferRecycler* RayTracePipeline::recycler() const
		{
			std::thread::id threadId = std::this_thread::get_id();
			{
				std::shared_lock<std::shared_mutex> locker(m_mu_streams);
				auto iter = m_recyclers.find(threadId);
				if (iter != m_recyclers.end())
				{
					CommandBufferRecycler* ret = iter->second;
					return ret;
				}
			}

			CommandBufferRecycler* ret = new CommandBufferRecycler;

			{
				std::unique_lock<std::shared_mutex> locker(m_mu_streams);
				m_recyclers[threadId] = ret;
			}

			return ret;

		}

		RayTraceCommandBuffer::RayTraceCommandBuffer(const RayTracePipeline* pipeline, size_t ubo_size)
		{
			const Context* ctx = Context::get_context();

			m_pipeline = pipeline;
			m_ubo = nullptr;
			if (ubo_size > 0)
				m_ubo = new DeviceBuffer(ubo_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
			{
				std::vector<VkDescriptorPoolSize> poolSizes(1);
				poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				poolSizes[0].descriptorCount = 1;

				if (pipeline->num_tex2d() > 0)
				{
					VkDescriptorPoolSize pool_size_tex2d = {};
					pool_size_tex2d.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					pool_size_tex2d.descriptorCount = (unsigned)pipeline->num_tex2d();
					poolSizes.push_back(pool_size_tex2d);
				}

				VkDescriptorPoolCreateInfo poolInfo = {};
				poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				poolInfo.poolSizeCount = (unsigned)poolSizes.size();
				poolInfo.pPoolSizes = poolSizes.data();
				poolInfo.maxSets = 1;
				vkCreateDescriptorPool(ctx->device(), &poolInfo, nullptr, &m_descriptorPool);
			}

			{
				VkDescriptorSetAllocateInfo allocInfo = {};
				allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				allocInfo.descriptorPool = m_descriptorPool;
				allocInfo.descriptorSetCount = 1;
				allocInfo.pSetLayouts = &pipeline->layout_desc();
				vkAllocateDescriptorSets(ctx->device(), &allocInfo, &m_descriptorSet);
			}

			if (m_ubo != nullptr)
			{
				VkDescriptorBufferInfo uboInfo = {};
				uboInfo.buffer = m_ubo->buf();
				uboInfo.range = VK_WHOLE_SIZE;
				VkWriteDescriptorSet descriptorWrites[1];
				descriptorWrites[0] = {};
				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].dstSet = m_descriptorSet;
				descriptorWrites[0].dstBinding = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].pBufferInfo = &uboInfo;
				vkUpdateDescriptorSets(ctx->device(), 1, descriptorWrites, 0, nullptr);
			}
		}

		RayTraceCommandBuffer::~RayTraceCommandBuffer()
		{
			const Context* ctx = Context::get_context();
			vkDestroyDescriptorPool(ctx->device(), m_descriptorPool, nullptr);
			delete m_ubo;
		}

		void RayTraceCommandBuffer::Recycle()
		{
			m_pipeline->recycler()->RecycleCommandBuffer(this);
		}

		void RayTraceCommandBuffer::trace(void* param_data, TopLevelAS** arr_tlas, Texture2D** tex2ds, Texture3D** tex3ds, TextureCube** cubemaps, unsigned dim_x, unsigned dim_y, unsigned dim_z)
		{
			const Context* ctx = Context::get_context();
			if (m_ubo != nullptr)
				m_ubo->upload(param_data);

			std::vector<VkAccelerationStructureKHR> v_tlas(m_pipeline->num_tlas());
			for (size_t i = 0; i < m_pipeline->num_tlas(); i++)
				v_tlas[i] = arr_tlas[i]->structure();
			
			VkWriteDescriptorSetAccelerationStructureKHR descriptorAccelerationStructureInfo = {};
			descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
			descriptorAccelerationStructureInfo.accelerationStructureCount = (unsigned)v_tlas.size();
			descriptorAccelerationStructureInfo.pAccelerationStructures = v_tlas.data();

			std::vector<VkDescriptorImageInfo> tex2dInfos(m_pipeline->num_tex2d());
			for (size_t i = 0; i < m_pipeline->num_tex2d(); ++i)
			{
				tex2dInfos[i] = {};
				tex2dInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				tex2dInfos[i].imageView = tex2ds[i]->view();
				tex2dInfos[i].sampler = m_pipeline->sampler()->sampler();
				tex2ds[i]->apply_barrier(*this, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			}

			std::vector<VkDescriptorImageInfo> tex3dInfos(m_pipeline->num_tex3d());
			for (size_t i = 0; i < m_pipeline->num_tex3d(); ++i)
			{
				tex3dInfos[i] = {};
				tex3dInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				tex3dInfos[i].imageView = tex3ds[i]->view();
				tex3dInfos[i].sampler = m_pipeline->sampler()->sampler();
				tex3ds[i]->apply_barrier(*this, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			}

			std::vector<VkDescriptorImageInfo> cubemapInfos(m_pipeline->num_cubemap());
			for (size_t i = 0; i < m_pipeline->num_cubemap(); ++i)
			{
				cubemapInfos[i] = {};
				cubemapInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				cubemapInfos[i].imageView = cubemaps[i]->view();
				cubemapInfos[i].sampler = m_pipeline->sampler()->sampler();
				cubemaps[i]->apply_barrier(*this, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			}

			std::vector<VkWriteDescriptorSet> list_wds;
			if (v_tlas.size() > 0)
			{
				VkWriteDescriptorSet wds = {};
				wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				wds.dstSet = m_descriptorSet;
				wds.dstBinding = 10;
				wds.descriptorCount = (unsigned)v_tlas.size();
				wds.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
				wds.pNext = &descriptorAccelerationStructureInfo;
				list_wds.push_back(wds);
			}
			if (m_pipeline->num_tex2d() > 0)
			{
				VkWriteDescriptorSet wds = {};
				wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				wds.dstSet = m_descriptorSet;
				wds.dstBinding = 1;
				wds.descriptorCount = (uint32_t)m_pipeline->num_tex2d();
				wds.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				wds.pImageInfo = tex2dInfos.data();
				list_wds.push_back(wds);
			}
			if (m_pipeline->num_tex3d() > 0)
			{
				VkWriteDescriptorSet wds = {};
				wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				wds.dstSet = m_descriptorSet;
				wds.dstBinding = 2;
				wds.descriptorCount = (uint32_t)m_pipeline->num_tex3d();
				wds.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				wds.pImageInfo = tex3dInfos.data();
				list_wds.push_back(wds);
			}
			if (m_pipeline->num_cubemap() > 0)
			{
				VkWriteDescriptorSet wds = {};
				wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				wds.dstSet = m_descriptorSet;
				wds.dstBinding = 3;
				wds.descriptorCount = (uint32_t)m_pipeline->num_cubemap();
				wds.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				wds.pImageInfo = cubemapInfos.data();
				list_wds.push_back(wds);
			}

			vkUpdateDescriptorSets(ctx->device(), (unsigned)list_wds.size(), list_wds.data(), 0, nullptr);

			if (m_ubo != nullptr)
				m_ubo->apply_barrier(*this, VK_ACCESS_UNIFORM_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

			vkCmdBindPipeline(m_buf, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_pipeline->pipeline());
			vkCmdBindDescriptorSets(m_buf, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_pipeline->layout_pipeline(), 0, 1, &m_descriptorSet, 0, 0);
			vkCmdTraceRaysKHR(m_buf, &m_pipeline->sbt_entry_raygen(), &m_pipeline->sbt_entry_miss(), &m_pipeline->sbt_entry_hit(), &m_pipeline->sbt_entry_callable(), dim_x, dim_y, dim_z);

		}
	}
}
