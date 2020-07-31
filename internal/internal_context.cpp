#include "internal_context.h"
#include "vk_format_utils.h"
#include <memory.h>
#include <vector>

namespace VkInline
{
	namespace Internal
	{
		const Context* Context::get_context(bool cleanup, bool is_trying)
		{
			static Context* s_ctx = nullptr;
			if (!cleanup)
			{
				if (s_ctx == nullptr) s_ctx = new Context;
			}
			else
			{
				delete s_ctx;
				s_ctx = nullptr;
			}
			if (s_ctx && !s_ctx->m_is_valid)
			{
				if (is_trying)
				{
					delete s_ctx;
					s_ctx = nullptr;
				}
				else
				{
					exit(0);
				}
			}
			return s_ctx;
		}

		VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
		{
			printf("validation layer: %s\n", pCallbackData->pMessage);
			return VK_FALSE;
		}

		bool Context::_init_vulkan()
		{
			if (volkInitialize() != VK_SUCCESS)
			{
				printf("volk initialization failed.\n");
				return false;
			}

			{
				VkApplicationInfo appInfo = {};
				appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
				appInfo.pApplicationName = "TextureGen";
				appInfo.pEngineName = "No Engine";

#ifndef _VkInlineEX
				appInfo.applicationVersion = VK_MAKE_VERSION(1, 1, 0);				
				appInfo.engineVersion = VK_MAKE_VERSION(1, 1, 0);
				appInfo.apiVersion = VK_API_VERSION_1_1;
#else
				appInfo.applicationVersion = VK_MAKE_VERSION(1, 2, 0);
				appInfo.engineVersion = VK_MAKE_VERSION(1, 2, 0);
				appInfo.apiVersion = VK_API_VERSION_1_2;
#endif

				const char* name_extensions[] = {
					VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
					VK_EXT_DEBUG_UTILS_EXTENSION_NAME
				};

				char str_validationLayers[] = "VK_LAYER_KHRONOS_validation";
				const char* validationLayers[] = { str_validationLayers };

				VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
				debugCreateInfo = {};
				debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
				debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
				debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
				debugCreateInfo.pfnUserCallback = debugCallback;

				VkInstanceCreateInfo createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
				createInfo.pApplicationInfo = &appInfo;
				createInfo.enabledExtensionCount = sizeof(name_extensions) / sizeof(const char*);
				createInfo.ppEnabledExtensionNames = name_extensions;

#ifdef _DEBUG
				createInfo.enabledLayerCount = 1;
				createInfo.ppEnabledLayerNames = validationLayers;
				createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
#endif

				if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
				{
					printf("Failed to create vulkan instance\n");
					return false;
				}

#ifdef _DEBUG
				PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
				vkCreateDebugUtilsMessengerEXT(m_instance, &debugCreateInfo, nullptr, &m_debugMessenger);
#endif
			}
			volkLoadInstance(m_instance);

			m_physicalDevice = VK_NULL_HANDLE;
			{
				// select physical device
				uint32_t deviceCount = 0;
				vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
				std::vector<VkPhysicalDevice> ph_devices(deviceCount);
				vkEnumeratePhysicalDevices(m_instance, &deviceCount, ph_devices.data());
				m_physicalDevice = ph_devices[0];
			}

			
			{
				m_bufferDeviceAddressFeatures = {};
#ifndef _VkInlineEX
				m_bufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT;
#else
				m_bufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
#endif
				m_bufferDeviceAddressFeatures.pNext = &m_descriptorIndexingFeatures;
				m_descriptorIndexingFeatures = {};
				m_descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
				m_descriptorIndexingFeatures.pNext = &m_scalarBlockLayoutFeatures;
				m_scalarBlockLayoutFeatures = {};
				m_scalarBlockLayoutFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES_EXT;

#ifdef _VkInlineEX
				m_scalarBlockLayoutFeatures.pNext = &m_raytracingFeatures;
				m_raytracingFeatures = {};
				m_raytracingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_FEATURES_KHR;
#endif

				m_features2 = {};
				m_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
				m_features2.pNext = &m_bufferDeviceAddressFeatures;
				vkGetPhysicalDeviceFeatures2(m_physicalDevice, &m_features2);
			}


#ifdef _VkInlineEX
			{
				m_raytracingProperties = {};
				m_raytracingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_KHR;
				VkPhysicalDeviceProperties2 props = {};
				props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
				props.pNext = &m_raytracingProperties;
				vkGetPhysicalDeviceProperties2(m_physicalDevice, &props);
			}
#endif

			m_queueFamily = (uint32_t)(-1);
			{
				uint32_t queueFamilyCount = 0;
				vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
				std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
				vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());

				for (uint32_t i = 0; i < queueFamilyCount; i++)
					if (m_queueFamily == (uint32_t)(-1) && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) m_queueFamily = i;
			}

			// logical device/queue
			m_queuePriority = 1.0f;

			{
				VkDeviceQueueCreateInfo queueCreateInfo = {};
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.queueFamilyIndex = m_queueFamily;
				queueCreateInfo.queueCount = 1;
				queueCreateInfo.pQueuePriorities = &m_queuePriority;

				const char* name_extensions[] = {
#ifndef _VkInlineEX
					VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
#else
					VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
					VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
					VK_KHR_RAY_TRACING_EXTENSION_NAME,
					VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
					VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
#endif
					VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
					VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME
				};

				VkDeviceCreateInfo createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
				createInfo.pQueueCreateInfos = &queueCreateInfo;
				createInfo.queueCreateInfoCount = 1;
				createInfo.enabledExtensionCount = sizeof(name_extensions) / sizeof(const char*);
				createInfo.ppEnabledExtensionNames = name_extensions;
				createInfo.pNext = &m_features2;

				if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
				{
#ifndef _VkInlineEX
					printf("Failed to create vulkan device\n");
#else
					printf("Failed to create vulkan device. (VK_KHR_ray_tracing not supported, switching to Vulkan 1.1.)\n");
#endif
					return false;
				}
			}

			vkGetDeviceQueue(m_device, m_queueFamily, 0, &m_queue);

			return true;

		}

		Context::Context()
		{
			m_is_valid = _init_vulkan();
		}

		Context::~Context()
		{
			if (!m_is_valid) return;

			auto iter = m_streams.begin();
			while (iter != m_streams.end())
			{
				vkDestroyCommandPool(m_device, iter->second->m_commandPool, nullptr);
				delete iter->second;
				iter++;
			}

#ifdef _DEBUG
			vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
#endif
			vkDestroyDevice(m_device, nullptr);
			// vkDestroyInstance(m_instance, nullptr); // cause halt for beta driver
		}

		Context::Stream* Context::_stream(std::thread::id threadId) const
		{
			{
				std::shared_lock<std::shared_mutex> locker(m_mu_streams);
				auto iter = m_streams.find(threadId);
				if (iter != m_streams.end()) return iter->second;
			}
			Context::Stream* stream = new Context::Stream;
			{
				std::unique_lock<std::shared_mutex> locker(m_mu_streams);
				m_streams[threadId] = stream;
			}

			VkCommandPoolCreateInfo poolInfo = {};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.queueFamilyIndex = m_queueFamily;
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			vkCreateCommandPool(m_device, &poolInfo, nullptr, &stream->m_commandPool);
			return stream;
		}

		Context::Stream* Context::stream() const
		{
			return _stream(std::this_thread::get_id());
		}

		CommandBuffer::CommandBuffer()
		{
			const Context* ctx = Context::get_context();
			m_stream = ctx->stream();

			VkCommandBufferAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = m_stream->m_commandPool;
			allocInfo.commandBufferCount = 1;

			{
				std::unique_lock<std::mutex> locker(m_stream->m_mutex_pool);
				vkAllocateCommandBuffers(ctx->device(), &allocInfo, &m_buf);
			}

			VkFenceCreateInfo fenceInfo = {};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			vkCreateFence(ctx->device(), &fenceInfo, nullptr, &m_fence);

			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			vkBeginCommandBuffer(m_buf, &beginInfo);

			m_submitted = false;
		}

		CommandBuffer::~CommandBuffer()
		{
			const Context* ctx = Context::get_context();
			{
				std::unique_lock<std::mutex> locker(m_stream->m_mutex_pool);
				vkFreeCommandBuffers(ctx->device(), m_stream->m_commandPool, 1, &m_buf);
			}
			vkDestroyFence(ctx->device(), m_fence, nullptr);
		}

		void CommandBuffer::Recycle()
		{
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			vkBeginCommandBuffer(m_buf, &beginInfo);
		}

		void Context::SubmitCommandBuffer(CommandBuffer* cmdBuf, size_t n) const
		{
			vkEndCommandBuffer(cmdBuf->buf());
			Context::Stream* s = stream();
			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &cmdBuf->buf();

			{
				std::unique_lock<std::mutex> locker(m_mu_queue);
				for (size_t i = 0; i < n; i++)
				{
					VkFence f = 0;
					if (i == n - 1) f = cmdBuf->fence();
					vkQueueSubmit(m_queue, 1, &submitInfo, f);
				}
			}
			s->m_queue_wait.push(cmdBuf);
			cmdBuf->m_submitted = true;
		}

		void Context::WaitUtil(CommandBuffer* lastCmdBuf) const
		{
			if (lastCmdBuf != nullptr && !lastCmdBuf->m_submitted) return;
			Context::Stream* s = stream();
			while (s->m_queue_wait.size() > 0)
			{
				CommandBuffer* cb = s->m_queue_wait.front();
				s->m_queue_wait.pop();
				vkWaitForFences(m_device, 1, &cb->fence(), VK_TRUE, UINT64_MAX);
				vkResetFences(m_device, 1, &cb->fence());
				vkResetCommandBuffer(cb->buf(), 0);
				cb->m_submitted = false;
				cb->Recycle();
				if (cb == lastCmdBuf) return;
			}
		}

		void Context::Wait() const
		{
			WaitUtil(nullptr);
		}

		CommandBufferRecycler::CommandBufferRecycler() { }

		CommandBufferRecycler::~CommandBufferRecycler()
		{
			while (m_queue_recycle.size() > 0)
			{
				CommandBuffer* cb = m_queue_recycle.front();
				m_queue_recycle.pop();
				delete cb;
			}
		}

		void CommandBufferRecycler::RecycleCommandBuffer(CommandBuffer* cmdbuf)
		{
			m_queue_recycle.push(cmdbuf);
		}

		CommandBuffer* CommandBufferRecycler::RetriveCommandBuffer()
		{
			CommandBuffer* ret = nullptr;
			if (m_queue_recycle.size() > 0)
			{
				ret = m_queue_recycle.front();
				m_queue_recycle.pop();

				VkCommandBufferBeginInfo beginInfo = {};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
				vkBeginCommandBuffer(ret->buf(), &beginInfo);

			}
			return ret;
		}

		VkDeviceAddress Buffer::get_device_address() const
		{
			if (m_size == 0) return 0;
			const Context* ctx = Context::get_context();

			VkBufferDeviceAddressInfo bufAdrInfo = {};
			bufAdrInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
			bufAdrInfo.buffer = m_buf;
#ifndef _VkInlineEX
			return vkGetBufferDeviceAddressEXT(ctx->device(), &bufAdrInfo);
#else
			return vkGetBufferDeviceAddressKHR(ctx->device(), &bufAdrInfo);
#endif
		}

		Buffer::Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags)
		{
			if (size == 0) return;
			m_size = size;

			const Context* ctx = Context::get_context();

			VkBufferCreateInfo bufferCreateInfo = {};
			bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferCreateInfo.size = size;
			bufferCreateInfo.usage = usage;
			bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			vkCreateBuffer(ctx->device(), &bufferCreateInfo, nullptr, &m_buf);

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(ctx->device(), m_buf, &memRequirements);

			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(ctx->physicalDevice(), &memProperties);

			uint32_t memoryTypeIndex = VK_MAX_MEMORY_TYPES;
			for (uint32_t k = 0; k < memProperties.memoryTypeCount; k++)
			{
				if ((memRequirements.memoryTypeBits & (1 << k)) == 0) continue;
				if ((flags & memProperties.memoryTypes[k].propertyFlags) == flags)
				{
					memoryTypeIndex = k;
					break;
				}
			}

			VkMemoryAllocateInfo memoryAllocateInfo = {};
			memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memoryAllocateInfo.allocationSize = memRequirements.size;
			memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

#ifdef _VkInlineEX
			VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo = {};
			memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
			if ((usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) != 0)
				memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
			memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
#endif

			vkAllocateMemory(ctx->device(), &memoryAllocateInfo, nullptr, &m_mem);
			vkBindBufferMemory(ctx->device(), m_buf, m_mem, 0);

			m_cur_access_mask = 0;
		}

		Buffer::~Buffer()
		{
			const Context* ctx = Context::get_context();
			vkDestroyBuffer(ctx->device(), m_buf, nullptr);
			vkFreeMemory(ctx->device(), m_mem, nullptr);
		}

		void Buffer::apply_barrier(const CommandBuffer& cmdbuf, VkAccessFlags dstAccessMask, VkPipelineStageFlags dstStageMask) const
		{
			VkBufferMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			barrier.buffer = m_buf;
			barrier.offset = 0;
			barrier.size = VK_WHOLE_SIZE;
			barrier.srcAccessMask = m_cur_access_mask;
			barrier.dstAccessMask = dstAccessMask;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			vkCmdPipelineBarrier(
				cmdbuf.buf(),
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				dstStageMask,
				0,
				0, nullptr,
				1, &barrier,
				0, nullptr
			);
			m_cur_access_mask = dstAccessMask;
		}


		UploadBuffer::UploadBuffer(VkDeviceSize size, VkBufferUsageFlags usage) :
			Buffer(size, usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {	}

		UploadBuffer::~UploadBuffer() {}

		void UploadBuffer::upload(const void* hdata)
		{
			if (m_size == 0) return;

			const Context* ctx = Context::get_context();

			void* data;
			vkMapMemory(ctx->device(), m_mem, 0, m_size, 0, &data);
			memcpy(data, hdata, m_size);
			vkUnmapMemory(ctx->device(), m_mem);
		}

		void UploadBuffer::zero()
		{
			if (m_size == 0) return;

			const Context* ctx = Context::get_context();

			void* data;
			vkMapMemory(ctx->device(), m_mem, 0, m_size, 0, &data);
			memset(data, 0, m_size);
			vkUnmapMemory(ctx->device(), m_mem);
		}

		DownloadBuffer::DownloadBuffer(VkDeviceSize size, VkBufferUsageFlags usage) :
			Buffer(size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT) { }

		DownloadBuffer::~DownloadBuffer() {}

		void DownloadBuffer::download(void* hdata)
		{
			if (m_size == 0) return;

			const Context* ctx = Context::get_context();

			void* data;
			vkMapMemory(ctx->device(), m_mem, 0, m_size, 0, &data);
			memcpy(hdata, data, m_size);
			vkUnmapMemory(ctx->device(), m_mem);
		}

		DeviceBuffer::DeviceBuffer(VkDeviceSize size, VkBufferUsageFlags usage) :
			Buffer(size, usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {}

		DeviceBuffer::~DeviceBuffer() {}

		class CommandBuf_DevBufUpload : public AutoCommandBuffer
		{
		public:
			CommandBuf_DevBufUpload(VkDeviceSize size, Buffer* buf) : m_staging_buf(size)
			{
				m_staging_buf.zero();
				_upload(size, buf);
			}

			CommandBuf_DevBufUpload(VkDeviceSize size, Buffer* buf, const void* hdata) : m_staging_buf(size)
			{
				m_staging_buf.upload(hdata);
				_upload(size, buf);
			}

		private:
			void _upload(VkDeviceSize size, Buffer* buf)
			{
				buf->apply_barrier(*this, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
				VkBufferCopy copyRegion = {};
				copyRegion.srcOffset = 0;
				copyRegion.dstOffset = 0;
				copyRegion.size = size;
				vkCmdCopyBuffer(m_buf, m_staging_buf.buf(), buf->buf(), 1, &copyRegion);
			}

			UploadBuffer m_staging_buf;
		};


		void DeviceBuffer::upload(const void* hdata)
		{
			auto cmdBuf = new CommandBuf_DevBufUpload(m_size, this, hdata);
			const Context* ctx = Context::get_context();
			ctx->SubmitCommandBuffer(cmdBuf);
		}

		void DeviceBuffer::zero()
		{
			auto cmdBuf = new CommandBuf_DevBufUpload(m_size, this);
			const Context* ctx = Context::get_context();
			ctx->SubmitCommandBuffer(cmdBuf);
		}

		void DeviceBuffer::download(void* hdata, VkDeviceSize begin, VkDeviceSize end) const
		{
			if (end > m_size) end = m_size;
			if (end <= begin) return;

			DownloadBuffer staging_buf(end - begin);
			auto cmdBuf = new AutoCommandBuffer;

			apply_barrier(*cmdBuf, VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

			VkBufferCopy copyRegion = {};
			copyRegion.srcOffset = begin;
			copyRegion.dstOffset = 0;
			copyRegion.size = end - begin;
			vkCmdCopyBuffer(cmdBuf->buf(), m_buf, staging_buf.buf(), 1, &copyRegion);
			const Context* ctx = Context::get_context();
			ctx->SubmitCommandBuffer(cmdBuf);
			ctx->Wait();

			staging_buf.download(hdata);
		}

		unsigned Texture2D::pixel_size() const
		{
			return FormatElementSize(m_format, m_aspect);
		}

		unsigned Texture2D::channel_count() const
		{
			return FormatChannelCount(m_format);
		}

		Texture2D::Texture2D(int width, int height, VkFormat format, VkImageAspectFlags aspectFlags, VkImageUsageFlags usage, VkSampleCountFlagBits sampleCount)
		{
			m_width = width;
			m_height = height;
			m_format = format;
			m_aspect = aspectFlags;
			m_sampleCount = sampleCount;
			if (width == 0 || height == 0) return;

			const Context* ctx = Context::get_context();

			usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

			VkFormatProperties format_props;
			vkGetPhysicalDeviceFormatProperties(ctx->physicalDevice(), format, &format_props);		
			if ((format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) == 0
				|| (format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) == 0)
				usage &= ~VK_IMAGE_USAGE_SAMPLED_BIT;			
			if ((format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) == 0)
				usage &= ~VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			if ((format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0)
				usage &= ~VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			if ((format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) == 0)
				usage &= ~VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			if ((format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) == 0)
				usage &= ~VK_IMAGE_USAGE_TRANSFER_DST_BIT;

			VkImageCreateInfo imageInfo = {};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = width;
			imageInfo.extent.height = height;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = format;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = usage;
			imageInfo.samples = sampleCount;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			vkCreateImage(ctx->device(), &imageInfo, nullptr, &m_image);

			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(ctx->device(), m_image, &memRequirements);

			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(ctx->physicalDevice(), &memProperties);

			uint32_t memoryTypeIndex = VK_MAX_MEMORY_TYPES;
			for (uint32_t k = 0; k < memProperties.memoryTypeCount; k++)
			{
				if ((memRequirements.memoryTypeBits & (1 << k)) == 0) continue;
				if ((VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT & memProperties.memoryTypes[k].propertyFlags) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
				{
					memoryTypeIndex = k;
					break;
				}
			}

			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = memoryTypeIndex;

			vkAllocateMemory(ctx->device(), &allocInfo, nullptr, &m_mem);
			vkBindImageMemory(ctx->device(), m_image, m_mem, 0);

			VkImageViewCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_image;
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = format;
			createInfo.subresourceRange.aspectMask = aspectFlags;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;
			vkCreateImageView(ctx->device(), &createInfo, nullptr, &m_view);

			m_cur_layout = VK_IMAGE_LAYOUT_UNDEFINED;
			m_cur_access_mask = 0;
		}

		Texture2D::~Texture2D()
		{
			if (m_width == 0 || m_height == 0) return;
			const Context* ctx = Context::get_context();
			vkDestroyImageView(ctx->device(), m_view, nullptr);
			vkDestroyImage(ctx->device(), m_image, nullptr);
			vkFreeMemory(ctx->device(), m_mem, nullptr);
		}

		void Texture2D::apply_barrier(const CommandBuffer& cmdbuf, VkImageLayout newLayout, VkAccessFlags dstAccessMask, VkPipelineStageFlags dstStageMask) const
		{
			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = m_cur_layout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = m_image;
			barrier.subresourceRange.aspectMask = m_aspect;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.srcAccessMask = m_cur_access_mask;
			barrier.dstAccessMask = dstAccessMask;

			vkCmdPipelineBarrier(
				cmdbuf.buf(),
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				dstStageMask,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			m_cur_layout = newLayout;
			m_cur_access_mask = dstAccessMask;
		}

		class CommandBuf_TexUpload : public AutoCommandBuffer
		{
		public:
			CommandBuf_TexUpload(int width, int height, unsigned pixel_size, unsigned samples, Texture2D* tex, const void* hdata)
				: m_staging_buf(width*height*pixel_size*samples)
			{
				m_staging_buf.upload(hdata);
				tex->apply_barrier(*this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		
				VkBufferImageCopy region = {};
				region.imageSubresource.aspectMask = tex->aspect();
				region.imageSubresource.layerCount = 1;
				region.imageExtent = {
					(uint32_t)width,
					(uint32_t)height,
					1
				};

				vkCmdCopyBufferToImage(
					m_buf,
					m_staging_buf.buf(),
					tex->image(),
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1,
					&region
				);
			}

		private:
			UploadBuffer m_staging_buf;
		};

		void Texture2D::upload(const void* hdata)
		{
			auto cmdBuf = new CommandBuf_TexUpload(m_width, m_height, pixel_size(), m_sampleCount, this, hdata);
			const Context* ctx = Context::get_context();
			ctx->SubmitCommandBuffer(cmdBuf);
		}

		void Texture2D::download(void* hdata) const
		{
			if (m_width == 0 || m_height == 0) return;
			DownloadBuffer staging_buf(m_width*m_height*pixel_size()*m_sampleCount);

			auto cmdBuf = new AutoCommandBuffer;

			apply_barrier(*cmdBuf, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);		

			VkBufferImageCopy region = {};
			region.imageSubresource.aspectMask = m_aspect;
			region.imageSubresource.layerCount = 1;
			region.imageExtent = {
				(uint32_t)m_width,
				(uint32_t)m_height,
				1
			};

			vkCmdCopyImageToBuffer(
				cmdBuf->buf(),
				m_image,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				staging_buf.buf(),
				1,
				&region
			);

			const Context* ctx = Context::get_context();
			ctx->SubmitCommandBuffer(cmdBuf);
			ctx->Wait();
			staging_buf.download(hdata);
		}

		unsigned Texture3D::pixel_size() const
		{
			return FormatElementSize(m_format, VK_IMAGE_ASPECT_COLOR_BIT);
		}

		unsigned Texture3D::channel_count() const
		{
			return FormatChannelCount(m_format);
		}

		Texture3D::Texture3D(int dimX, int dimY, int dimZ, VkFormat format)
		{
			m_dims[0] = dimX;
			m_dims[1] = dimY;
			m_dims[2] = dimZ;
			m_format = format;
			if (dimX == 0 || dimY == 0 || dimZ==0) return;

			const Context* ctx = Context::get_context();

			VkImageCreateInfo imageInfo = {};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_3D;
			imageInfo.extent.width = dimX;
			imageInfo.extent.height = dimY;
			imageInfo.extent.depth = dimZ;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = format;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			vkCreateImage(ctx->device(), &imageInfo, nullptr, &m_image);

			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(ctx->device(), m_image, &memRequirements);

			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(ctx->physicalDevice(), &memProperties);

			uint32_t memoryTypeIndex = VK_MAX_MEMORY_TYPES;
			for (uint32_t k = 0; k < memProperties.memoryTypeCount; k++)
			{
				if ((memRequirements.memoryTypeBits & (1 << k)) == 0) continue;
				if ((VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT & memProperties.memoryTypes[k].propertyFlags) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
				{
					memoryTypeIndex = k;
					break;
				}
			}

			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = memoryTypeIndex;

			vkAllocateMemory(ctx->device(), &allocInfo, nullptr, &m_mem);
			vkBindImageMemory(ctx->device(), m_image, m_mem, 0);

			VkImageViewCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_image;
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
			createInfo.format = format;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;
			vkCreateImageView(ctx->device(), &createInfo, nullptr, &m_view);
		}

		Texture3D::~Texture3D()
		{
			if (m_dims[0] == 0 || m_dims[1] == 0 || m_dims[2] == 0) return;
			const Context* ctx = Context::get_context();
			vkDestroyImageView(ctx->device(), m_view, nullptr);
			vkDestroyImage(ctx->device(), m_image, nullptr);
			vkFreeMemory(ctx->device(), m_mem, nullptr);
		}

		void Texture3D::apply_barrier(const CommandBuffer& cmdbuf, VkImageLayout newLayout, VkAccessFlags dstAccessMask, VkPipelineStageFlags dstStageMask) const
		{
			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = m_cur_layout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = m_image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.srcAccessMask = m_cur_access_mask;
			barrier.dstAccessMask = dstAccessMask;

			vkCmdPipelineBarrier(
				cmdbuf.buf(),
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				dstStageMask,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			m_cur_layout = newLayout;
			m_cur_access_mask = dstAccessMask;
		}


		class CommandBuf_Tex3DUpload : public AutoCommandBuffer
		{
		public:
			CommandBuf_Tex3DUpload(int dims[3], unsigned pixel_size, Texture3D* tex, const void* hdata)
				: m_staging_buf(dims[0] * dims[1] * dims[2] * pixel_size)
			{
				m_staging_buf.upload(hdata);
				tex->apply_barrier(*this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

				VkBufferImageCopy region = {};
				region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				region.imageSubresource.layerCount = 1;
				region.imageExtent = {
					(uint32_t)dims[0],
					(uint32_t)dims[1],
					(uint32_t)dims[2]
				};

				vkCmdCopyBufferToImage(
					m_buf,
					m_staging_buf.buf(),
					tex->image(),
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1,
					&region
				);
			}

		private:
			UploadBuffer m_staging_buf;
		};

		void Texture3D::upload(const void* hdata)
		{
			auto cmdBuf = new CommandBuf_Tex3DUpload(m_dims, pixel_size(), this, hdata);
			const Context* ctx = Context::get_context();
			ctx->SubmitCommandBuffer(cmdBuf);
		}

		void Texture3D::download(void* hdata) const
		{
			if (m_dims[0] == 0 || m_dims[1] == 0 || m_dims[2] == 0) return;
			DownloadBuffer staging_buf(m_dims[0] * m_dims[1] * m_dims[2] *pixel_size());

			auto cmdBuf = new AutoCommandBuffer;
			apply_barrier(*cmdBuf, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

			VkBufferImageCopy region = {};
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.layerCount = 1;
			region.imageExtent = {
				(uint32_t)m_dims[0],
				(uint32_t)m_dims[1],
				(uint32_t)m_dims[2]
			};

			vkCmdCopyImageToBuffer(
				cmdBuf->buf(),
				m_image,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				staging_buf.buf(),
				1,
				&region
			);

			const Context* ctx = Context::get_context();
			ctx->SubmitCommandBuffer(cmdBuf);
			ctx->Wait();
			staging_buf.download(hdata);
		}

		unsigned TextureCube::pixel_size() const
		{
			return FormatElementSize(m_format, VK_IMAGE_ASPECT_COLOR_BIT);
		}

		unsigned TextureCube::channel_count() const
		{
			return FormatChannelCount(m_format);
		}

		TextureCube::TextureCube(int width, int height, VkFormat format)
		{
			m_width = width;
			m_height = height;
			m_format = format;
			if (width == 0 || height == 0) return;

			const Context* ctx = Context::get_context();

			VkImageCreateInfo imageInfo = {};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = width;
			imageInfo.extent.height = height;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 6;
			imageInfo.format = format;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

			vkCreateImage(ctx->device(), &imageInfo, nullptr, &m_image);

			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(ctx->device(), m_image, &memRequirements);

			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(ctx->physicalDevice(), &memProperties);

			uint32_t memoryTypeIndex = VK_MAX_MEMORY_TYPES;
			for (uint32_t k = 0; k < memProperties.memoryTypeCount; k++)
			{
				if ((memRequirements.memoryTypeBits & (1 << k)) == 0) continue;
				if ((VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT & memProperties.memoryTypes[k].propertyFlags) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
				{
					memoryTypeIndex = k;
					break;
				}
			}

			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = memoryTypeIndex;

			vkAllocateMemory(ctx->device(), &allocInfo, nullptr, &m_mem);
			vkBindImageMemory(ctx->device(), m_image, m_mem, 0);

			VkImageViewCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_image;
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
			createInfo.format = format;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 6;
			vkCreateImageView(ctx->device(), &createInfo, nullptr, &m_view);

			m_cur_layout = VK_IMAGE_LAYOUT_UNDEFINED;
			m_cur_access_mask = 0;
		}

		TextureCube::~TextureCube()
		{
			if (m_width == 0 || m_height == 0) return;
			const Context* ctx = Context::get_context();
			vkDestroyImageView(ctx->device(), m_view, nullptr);
			vkDestroyImage(ctx->device(), m_image, nullptr);
			vkFreeMemory(ctx->device(), m_mem, nullptr);
		}

		void TextureCube::apply_barrier(const CommandBuffer& cmdbuf, VkImageLayout newLayout, VkAccessFlags dstAccessMask, VkPipelineStageFlags dstStageMask) const
		{
			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = m_cur_layout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = m_image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.srcAccessMask = m_cur_access_mask;
			barrier.dstAccessMask = dstAccessMask;

			vkCmdPipelineBarrier(
				cmdbuf.buf(),
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				dstStageMask,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			m_cur_layout = newLayout;
			m_cur_access_mask = dstAccessMask;
		}

		class CommandBuf_CubeTexUpload : public AutoCommandBuffer
		{
		public:
			CommandBuf_CubeTexUpload(int width, int height, unsigned pixel_size, TextureCube* tex, const void* hdata)
				: m_staging_buf(6*width*height*pixel_size)
			{
				m_staging_buf.upload(hdata);
				tex->apply_barrier(*this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

				VkBufferImageCopy region = {};
				region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				region.imageSubresource.layerCount = 6;
				region.imageExtent = {
					(uint32_t)width,
					(uint32_t)height,
					1
				};

				vkCmdCopyBufferToImage(
					m_buf,
					m_staging_buf.buf(),
					tex->image(),
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1,
					&region
				);
			}

		private:
			UploadBuffer m_staging_buf;
		};

		void TextureCube::upload(const void* hdata)
		{
			auto cmdBuf = new CommandBuf_CubeTexUpload(m_width, m_height, pixel_size(), this, hdata);
			const Context* ctx = Context::get_context();
			ctx->SubmitCommandBuffer(cmdBuf);
		}

		void TextureCube::download(void* hdata) const
		{
			if (m_width == 0 || m_height == 0) return;
			DownloadBuffer staging_buf(6*m_width*m_height*pixel_size());

			auto cmdBuf = new AutoCommandBuffer;

			apply_barrier(*cmdBuf, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

			VkBufferImageCopy region = {};
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.layerCount = 6;
			region.imageExtent = {
				(uint32_t)m_width,
				(uint32_t)m_height,
				1
			};

			vkCmdCopyImageToBuffer(
				cmdBuf->buf(),
				m_image,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				staging_buf.buf(),
				1,
				&region
			);

			const Context* ctx = Context::get_context();
			ctx->SubmitCommandBuffer(cmdBuf);
			ctx->Wait();
			staging_buf.download(hdata);
		}

		Sampler::Sampler()
		{
			const Context* ctx = Context::get_context();
			VkSamplerCreateInfo samplerInfo = {};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

			vkCreateSampler(ctx->device(), &samplerInfo, nullptr, &m_sampler);
		}

		Sampler::~Sampler()
		{
			const Context* ctx = Context::get_context();
			vkDestroySampler(ctx->device(), m_sampler, nullptr);
		}

		ComputePipeline::ComputePipeline(const std::vector<unsigned>& spv, size_t num_tex2d, size_t num_tex3d, size_t num_cubemap)
		{
			m_sampler = nullptr;
			const Context* ctx = Context::get_context();
			VkShaderModule shader_module;
			{
				VkShaderModuleCreateInfo createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				createInfo.codeSize = spv.size() * sizeof(unsigned);
				createInfo.pCode = reinterpret_cast<const uint32_t*>(spv.data());
				vkCreateShaderModule(ctx->device(), &createInfo, nullptr, &shader_module);
			}
			{
				std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings(1);
				descriptorSetLayoutBindings[0] = {};
				descriptorSetLayoutBindings[0].binding = 0;
				descriptorSetLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorSetLayoutBindings[0].descriptorCount = 1;
				descriptorSetLayoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

				m_num_tex2d = num_tex2d;
				if (num_tex2d > 0)
				{
					VkDescriptorSetLayoutBinding binding_tex2d = {};
					binding_tex2d.binding = 1;
					binding_tex2d.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					binding_tex2d.descriptorCount = (unsigned)num_tex2d;
					binding_tex2d.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
					descriptorSetLayoutBindings.push_back(binding_tex2d);					
				}

				m_num_tex3d = num_tex3d;
				if (num_tex3d > 0)
				{
					VkDescriptorSetLayoutBinding binding_tex3d = {};
					binding_tex3d.binding = 2;
					binding_tex3d.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					binding_tex3d.descriptorCount = (unsigned)num_tex3d;
					binding_tex3d.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
					descriptorSetLayoutBindings.push_back(binding_tex3d);
				}

				m_num_cubemap = num_cubemap;
				if (num_cubemap > 0)
				{
					VkDescriptorSetLayoutBinding binding_cubemap = {};
					binding_cubemap.binding = 3;
					binding_cubemap.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					binding_cubemap.descriptorCount = (unsigned)num_cubemap;
					binding_cubemap.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
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

				VkPipelineShaderStageCreateInfo computeShaderStageInfo = {};
				computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
				computeShaderStageInfo.module = shader_module;
				computeShaderStageInfo.pName = "main";

				VkComputePipelineCreateInfo pipelineInfo = {};
				pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
				pipelineInfo.stage = computeShaderStageInfo;
				pipelineInfo.layout = m_pipelineLayout;

				vkCreateComputePipelines(ctx->device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline);
			}
			vkDestroyShaderModule(ctx->device(), shader_module, nullptr);
		}

		ComputePipeline::~ComputePipeline()
		{
			auto iter = m_recyclers.begin();
			while (iter != m_recyclers.end())
			{
				delete iter->second;
				iter++;
			}
			const Context* ctx = Context::get_context();			
			vkDestroyPipeline(ctx->device(), m_pipeline, nullptr);
			vkDestroyPipelineLayout(ctx->device(), m_pipelineLayout, nullptr);
			vkDestroyDescriptorSetLayout(ctx->device(), m_descriptorSetLayout, nullptr);
			delete m_sampler;
		}

		CommandBufferRecycler* ComputePipeline::recycler() const
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

		ComputeCommandBuffer::ComputeCommandBuffer(const ComputePipeline* pipeline, size_t ubo_size)
		{
			const Context* ctx = Context::get_context();

			m_pipeline = pipeline;
			m_ubo = nullptr;
			if (ubo_size>0)
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

			if (m_ubo!=nullptr)
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

		ComputeCommandBuffer::~ComputeCommandBuffer()
		{
			const Context* ctx = Context::get_context();
			vkDestroyDescriptorPool(ctx->device(), m_descriptorPool, nullptr);
			delete m_ubo;
		}

		void ComputeCommandBuffer::Recycle()
		{
			m_pipeline->recycler()->RecycleCommandBuffer(this);
		}

		void ComputeCommandBuffer::dispatch(void* param_data, Texture2D** tex2ds, Texture3D** tex3ds, TextureCube** cubemaps, unsigned dim_x, unsigned dim_y, unsigned dim_z)
		{
			const Context* ctx = Context::get_context();
			if (m_ubo != nullptr)
				m_ubo->upload(param_data);

			std::vector<VkDescriptorImageInfo> tex2dInfos(m_pipeline->num_tex2d());
			for (size_t i = 0; i < m_pipeline->num_tex2d(); ++i)
			{
				tex2dInfos[i] = {};
				tex2dInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				tex2dInfos[i].imageView = tex2ds[i]->view();
				tex2dInfos[i].sampler = m_pipeline->sampler()->sampler();
				tex2ds[i]->apply_barrier(*this, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			}

			std::vector<VkDescriptorImageInfo> tex3dInfos(m_pipeline->num_tex3d());
			for (size_t i = 0; i < m_pipeline->num_tex3d(); ++i)
			{
				tex3dInfos[i] = {};
				tex3dInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				tex3dInfos[i].imageView = tex3ds[i]->view();
				tex3dInfos[i].sampler = m_pipeline->sampler()->sampler();
				tex3ds[i]->apply_barrier(*this, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			}			
			
			std::vector<VkDescriptorImageInfo> cubemapInfos(m_pipeline->num_cubemap());
			for (size_t i = 0; i < m_pipeline->num_cubemap(); ++i)
			{
				cubemapInfos[i] = {};
				cubemapInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				cubemapInfos[i].imageView = cubemaps[i]->view();
				cubemapInfos[i].sampler = m_pipeline->sampler()->sampler();
				cubemaps[i]->apply_barrier(*this, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			}

			std::vector<VkWriteDescriptorSet> list_wds;
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
				m_ubo->apply_barrier(*this, VK_ACCESS_UNIFORM_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			vkCmdBindPipeline(m_buf, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline->pipeline());
			vkCmdBindDescriptorSets(m_buf, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline->layout_pipeline(), 0, 1, &m_descriptorSet, 0, 0);
			vkCmdDispatch(m_buf, dim_x, dim_y, dim_z);
		}

		RenderPass::RenderPass(
			const std::vector<AttachmentInfo>& color_attachmentInfo,
			const AttachmentInfo* depth_attachmentInfo,
			const std::vector<AttachmentInfo>& resolve_attachmentInfo,
			const std::vector<GraphicsPipelineInfo>& pipelineInfo,
			size_t num_tex2d, size_t num_tex3d, size_t num_cubemap)
		{
			m_num_color_attachments = color_attachmentInfo.size();
			m_has_depth_attachment = depth_attachmentInfo != nullptr;
			m_num_resolve_attachments = resolve_attachmentInfo.size();
			m_sample_count = 1;
			if (m_num_resolve_attachments > 0)
				m_sample_count = color_attachmentInfo[0].sample_count;

			m_sampler = nullptr;
			const Context* ctx = Context::get_context();
			{
				std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings(1);
				descriptorSetLayoutBindings[0] = {};
				descriptorSetLayoutBindings[0].binding = 0;
				descriptorSetLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorSetLayoutBindings[0].descriptorCount = 1;
				descriptorSetLayoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

				m_num_tex2d = num_tex2d;
				if (num_tex2d > 0)
				{
					VkDescriptorSetLayoutBinding binding_tex2d = {};
					binding_tex2d.binding = 1;
					binding_tex2d.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					binding_tex2d.descriptorCount = (unsigned)num_tex2d;
					binding_tex2d.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
					descriptorSetLayoutBindings.push_back(binding_tex2d);
				}

				m_num_tex3d = num_tex3d;
				if (num_tex3d > 0)
				{
					VkDescriptorSetLayoutBinding binding_tex3d = {};
					binding_tex3d.binding = 2;
					binding_tex3d.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					binding_tex3d.descriptorCount = (unsigned)num_tex3d;
					binding_tex3d.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
					descriptorSetLayoutBindings.push_back(binding_tex3d);
				}

				m_num_cubemap = num_cubemap;
				if (m_num_cubemap > 0)
				{
					VkDescriptorSetLayoutBinding binding_cubemap = {};
					binding_cubemap.binding = 3;
					binding_cubemap.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					binding_cubemap.descriptorCount = (unsigned)m_num_cubemap;
					binding_cubemap.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
					descriptorSetLayoutBindings.push_back(binding_cubemap);
				}

				if (num_tex2d >0 || num_tex3d > 0 || num_cubemap>0)
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
			}

			{
				std::vector<VkAttachmentDescription> attachments(color_attachmentInfo.size());
				for (size_t i = 0; i < color_attachmentInfo.size(); i++)
				{
					attachments[i] = {};
					attachments[i].format = color_attachmentInfo[i].format;
					attachments[i].samples = color_attachmentInfo[i].sample_count;
					attachments[i].loadOp = color_attachmentInfo[i].clear_at_load? VK_ATTACHMENT_LOAD_OP_CLEAR: VK_ATTACHMENT_LOAD_OP_DONT_CARE;
					attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
					attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
					attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
					attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					attachments[i].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				}

				if (depth_attachmentInfo != nullptr)
				{
					VkAttachmentDescription depth_attachment = {};
					depth_attachment.format = depth_attachmentInfo->format;
					depth_attachment.samples = depth_attachmentInfo->sample_count;
					depth_attachment.loadOp = depth_attachmentInfo->clear_at_load? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
					depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
					depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
					depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
					depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					attachments.push_back(depth_attachment);
				}

				for (size_t i = 0; i < resolve_attachmentInfo.size(); i++)
				{
					VkAttachmentDescription colorAttachmentResolve = {};
					colorAttachmentResolve.format = resolve_attachmentInfo[i].format;
					colorAttachmentResolve.samples = resolve_attachmentInfo[i].sample_count;
					colorAttachmentResolve.loadOp = resolve_attachmentInfo[i].clear_at_load? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
					colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
					colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
					colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
					colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					attachments.push_back(colorAttachmentResolve);
				}

				std::vector<VkAttachmentReference> colorAttachmentRefs(color_attachmentInfo.size());
				for (size_t i = 0; i < color_attachmentInfo.size(); i++)
				{
					colorAttachmentRefs[i] = {};
					colorAttachmentRefs[i].attachment = (unsigned)i;
					colorAttachmentRefs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				}

				VkAttachmentReference depthAttachmentRef = {};
				depthAttachmentRef.attachment = (unsigned)color_attachmentInfo.size();
				depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				
				std::vector<VkAttachmentReference> colorAttachmentResolveRefs(resolve_attachmentInfo.size());
				for (size_t i = 0; i < resolve_attachmentInfo.size(); i++)
				{
					colorAttachmentResolveRefs[i] = {};
					colorAttachmentResolveRefs[i].attachment = (unsigned)i + (unsigned)color_attachmentInfo.size() + (depth_attachmentInfo != nullptr?1:0);
					colorAttachmentResolveRefs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				}

				VkSubpassDescription subpass = {};
				subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				subpass.colorAttachmentCount = (unsigned)colorAttachmentRefs.size();
				subpass.pColorAttachments = colorAttachmentRefs.data();
				if (depth_attachmentInfo != nullptr)
					subpass.pDepthStencilAttachment = &depthAttachmentRef;
				if (resolve_attachmentInfo.size() > 0)
					subpass.pResolveAttachments = colorAttachmentResolveRefs.data();

				VkRenderPassCreateInfo renderPassInfo = {};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
				renderPassInfo.attachmentCount = (unsigned)attachments.size();
				renderPassInfo.pAttachments = attachments.data();
				renderPassInfo.subpassCount = 1;
				renderPassInfo.pSubpasses = &subpass;

				vkCreateRenderPass(ctx->device(), &renderPassInfo, nullptr, &m_renderPass);
			}

			{
				std::vector<VkShaderModule> vert_mods(pipelineInfo.size());
				std::vector<VkShaderModule> frag_mods(pipelineInfo.size());
				std::vector<std::vector<VkPipelineShaderStageCreateInfo>> shaderStages(pipelineInfo.size());		

				VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
				vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

				VkViewport viewport;
				viewport.x = 0.0f;
				viewport.y = 0.0f;
				viewport.width = 0.0f;
				viewport.height = 0.0f;
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;

				VkRect2D scissor = {};
				scissor.offset = { 0, 0 };
				scissor.extent = { 0, 0 };

				VkPipelineViewportStateCreateInfo viewportState = {};
				viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
				viewportState.viewportCount = 1;
				viewportState.pViewports = &viewport;
				viewportState.scissorCount = 1;
				viewportState.pScissors = &scissor;

				VkPipelineMultisampleStateCreateInfo multisampling = {};
				multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
				multisampling.rasterizationSamples = (VkSampleCountFlagBits)m_sample_count;

				VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
				VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};	
				dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
				dynamicStateInfo.dynamicStateCount = sizeof(dynamicStates) / sizeof(VkDynamicState);
				dynamicStateInfo.pDynamicStates = dynamicStates;

				std::vector<VkGraphicsPipelineCreateInfo> pipelines(pipelineInfo.size());
				for (size_t i = 0; i < pipelineInfo.size(); i++)
				{
					shaderStages[i].resize(2);
					{
						VkShaderModuleCreateInfo createInfo = {};
						createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
						createInfo.codeSize = pipelineInfo[i].spv_vert->size() * sizeof(unsigned);
						createInfo.pCode = reinterpret_cast<const uint32_t*>(pipelineInfo[i].spv_vert->data());
						vkCreateShaderModule(ctx->device(), &createInfo, nullptr, &vert_mods[i]);
					}

					{
						VkShaderModuleCreateInfo createInfo = {};
						createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
						createInfo.codeSize = pipelineInfo[i].spv_frag->size() * sizeof(unsigned);
						createInfo.pCode = reinterpret_cast<const uint32_t*>(pipelineInfo[i].spv_frag->data());
						vkCreateShaderModule(ctx->device(), &createInfo, nullptr, &frag_mods[i]);
					}

					shaderStages[i][0] = {};
					shaderStages[i][0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
					shaderStages[i][0].stage = VK_SHADER_STAGE_VERTEX_BIT;
					shaderStages[i][0].module = vert_mods[i];
					shaderStages[i][0].pName = "main";

					shaderStages[i][1] = {};
					shaderStages[i][1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
					shaderStages[i][1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
					shaderStages[i][1].module = frag_mods[i];
					shaderStages[i][1].pName = "main";

					pipelines[i] = {};
					pipelines[i].sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
					pipelines[i].stageCount = 2;
					pipelines[i].pStages = shaderStages[i].data();
					pipelines[i].pVertexInputState = &vertexInputInfo;
					pipelines[i].pInputAssemblyState = &pipelineInfo[i].states.inputAssembly;
					pipelines[i].pViewportState = &viewportState;
					pipelines[i].pRasterizationState = &pipelineInfo[i].states.rasterizer;
					pipelines[i].pMultisampleState = &multisampling;
					pipelines[i].pColorBlendState = &pipelineInfo[i].states.colorBlending;
					pipelines[i].pDepthStencilState = &pipelineInfo[i].states.depthStencil;
					pipelines[i].pDynamicState = &dynamicStateInfo;
					pipelines[i].layout = m_pipelineLayout;
					pipelines[i].renderPass = m_renderPass;					
				}

				m_pipelines.resize(pipelineInfo.size());
				vkCreateGraphicsPipelines(ctx->device(), VK_NULL_HANDLE, (int)pipelines.size(), pipelines.data(), nullptr, m_pipelines.data());

				for (size_t i = 0; i < pipelineInfo.size(); i++)
				{
					vkDestroyShaderModule(ctx->device(), frag_mods[i], nullptr);
					vkDestroyShaderModule(ctx->device(), vert_mods[i], nullptr);
				}
			}
		}

		RenderPass::~RenderPass()
		{
			auto iter = m_recyclers.begin();
			while (iter != m_recyclers.end())
			{
				delete iter->second;
				iter++;
			}
			const Context* ctx = Context::get_context();

			for (size_t i = 0; i < m_pipelines.size(); i++)
			{
				vkDestroyPipeline(ctx->device(), m_pipelines[i], nullptr);
			}

			vkDestroyRenderPass(ctx->device(), m_renderPass, nullptr);
			vkDestroyPipelineLayout(ctx->device(), m_pipelineLayout, nullptr);
			vkDestroyDescriptorSetLayout(ctx->device(), m_descriptorSetLayout, nullptr);
			delete m_sampler;

		}

		CommandBufferRecycler* RenderPass::recycler() const
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

		RenderPassCommandBuffer::RenderPassCommandBuffer(const RenderPass* render_pass, size_t ubo_size)
		{
			const Context* ctx = Context::get_context();
			m_render_pass = render_pass;

			m_ubo = nullptr;
			if (ubo_size>0)
				m_ubo = new DeviceBuffer(ubo_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

			{
				std::vector<VkDescriptorPoolSize> poolSizes(1);
				poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				poolSizes[0].descriptorCount = 1;

				if (render_pass->num_tex2d() > 0)
				{
					VkDescriptorPoolSize pool_size_tex2d = {};
					pool_size_tex2d.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					pool_size_tex2d.descriptorCount = (unsigned)render_pass->num_tex2d();
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
				allocInfo.pSetLayouts = &render_pass->layout_desc();
				vkAllocateDescriptorSets(ctx->device(), &allocInfo, &m_descriptorSet);
			}

			if (m_ubo!=nullptr)
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

			m_framebuffer = nullptr;
		}

		RenderPassCommandBuffer::~RenderPassCommandBuffer()
		{
			const Context* ctx = Context::get_context();
			if (m_framebuffer != nullptr)
			{
				vkDestroyFramebuffer(ctx->device(), m_framebuffer, nullptr);
			}
			vkDestroyDescriptorPool(ctx->device(), m_descriptorPool, nullptr);
			delete m_ubo;
		}


		void RenderPassCommandBuffer::Recycle()
		{
			m_render_pass->recycler()->RecycleCommandBuffer(this);
		}

		void RenderPassCommandBuffer::draw(Texture2D** colorBufs, Texture2D* depthBuf, Texture2D** resolveBufs, float* clear_colors, float clear_depth,
			void* param_data, Texture2D** tex2ds, Texture3D** tex3ds, TextureCube** cubemaps, DrawParam* draw_params)
		{

			const Context* ctx = Context::get_context();
			if (m_ubo!=nullptr)
				m_ubo->upload(param_data);

			std::vector<VkDescriptorImageInfo> tex2dInfos(m_render_pass->num_tex2d());
			for (size_t i = 0; i < m_render_pass->num_tex2d(); ++i)
			{
				tex2dInfos[i] = {};
				tex2dInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				tex2dInfos[i].imageView = tex2ds[i]->view();
				tex2dInfos[i].sampler = m_render_pass->sampler()->sampler();
				tex2ds[i]->apply_barrier(*this, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
			}

			std::vector<VkDescriptorImageInfo> tex3dInfos(m_render_pass->num_tex3d());
			for (size_t i = 0; i < m_render_pass->num_tex3d(); ++i)
			{
				tex3dInfos[i] = {};
				tex3dInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				tex3dInfos[i].imageView = tex3ds[i]->view();
				tex3dInfos[i].sampler = m_render_pass->sampler()->sampler();
				tex3ds[i]->apply_barrier(*this, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
			}

			std::vector<VkDescriptorImageInfo> cubemapsInfos(m_render_pass->num_cubemap());
			for (size_t i = 0; i < m_render_pass->num_cubemap(); ++i)
			{
				cubemapsInfos[i] = {};
				cubemapsInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				cubemapsInfos[i].imageView = cubemaps[i]->view();
				cubemapsInfos[i].sampler = m_render_pass->sampler()->sampler();
				cubemaps[i]->apply_barrier(*this, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
			}

			std::vector<VkWriteDescriptorSet> list_wds;
			if (m_render_pass->num_tex2d() > 0)
			{
				VkWriteDescriptorSet wds = {};
				wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				wds.dstSet = m_descriptorSet;
				wds.dstBinding = 1;
				wds.descriptorCount = (uint32_t)m_render_pass->num_tex2d();
				wds.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				wds.pImageInfo = tex2dInfos.data();
				list_wds.push_back(wds);
			}
			if (m_render_pass->num_tex3d() > 0)
			{
				VkWriteDescriptorSet wds = {};
				wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				wds.dstSet = m_descriptorSet;
				wds.dstBinding = 2;
				wds.descriptorCount = (uint32_t)m_render_pass->num_tex3d();
				wds.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				wds.pImageInfo = tex3dInfos.data();
				list_wds.push_back(wds);
			}
			if (m_render_pass->num_cubemap() > 0)
			{
				VkWriteDescriptorSet wds = {};
				wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				wds.dstSet = m_descriptorSet;
				wds.dstBinding = 3;
				wds.descriptorCount = (uint32_t)m_render_pass->num_cubemap();
				wds.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				wds.pImageInfo = cubemapsInfos.data();
				list_wds.push_back(wds);
			}

			vkUpdateDescriptorSets(ctx->device(), (unsigned)list_wds.size(), list_wds.data(), 0, nullptr);

			int width, height;
			{
				if (m_framebuffer != nullptr)
				{
					vkDestroyFramebuffer(ctx->device(), m_framebuffer, nullptr);
				}

				std::vector<VkImageView> views(m_render_pass->num_color_attachments());
				for (size_t i = 0; i < m_render_pass->num_color_attachments(); i++)
				{
					views[i] = colorBufs[i]->view();
					if (i == 0)
					{
						width = colorBufs[i]->width();
						height = colorBufs[i]->height();
					}
					colorBufs[i]->apply_barrier(*this, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
				}

				if (m_render_pass->has_depth_attachment())
				{
					views.push_back(depthBuf->view());
					width = depthBuf->width();
					height = depthBuf->height();
					depthBuf->apply_barrier(*this, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
				}

				for (size_t i = 0; i < m_render_pass->num_resolve_attachments(); i++)
				{
					views.push_back(resolveBufs[i]->view());
				}

				{
					VkFramebufferCreateInfo framebufferInfo = {};
					framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
					framebufferInfo.renderPass = m_render_pass->render_pass();
					framebufferInfo.attachmentCount = (unsigned)views.size();
					framebufferInfo.pAttachments = views.data();
					framebufferInfo.width = width;
					framebufferInfo.height = height;
					framebufferInfo.layers = 1;
					vkCreateFramebuffer(ctx->device(), &framebufferInfo, nullptr, &m_framebuffer);
				}
			}

			std::vector<VkClearValue> clearValues(m_render_pass->num_color_attachments());
			for (size_t i = 0; i < m_render_pass->num_color_attachments(); i++)
				memcpy(&clearValues[i].color, clear_colors + i * 4, sizeof(float) * 4);
			
			if (m_render_pass->has_depth_attachment())
			{
				VkClearValue clear_value_depth;
				clear_value_depth.depthStencil.depth = clear_depth;
				clearValues.push_back(clear_value_depth);
			}

			if (m_ubo != nullptr)
				m_ubo->apply_barrier(*this, VK_ACCESS_UNIFORM_READ_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);

			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_render_pass->render_pass();
			renderPassInfo.framebuffer = m_framebuffer;
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = { (unsigned)width, (unsigned)height };
			renderPassInfo.clearValueCount = (unsigned)clearValues.size();
			renderPassInfo.pClearValues = clearValues.data();

			VkViewport viewport;
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)width;
			viewport.height = (float)height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor = {};
			scissor.offset = { 0, 0 };
			scissor.extent = { (unsigned)width, (unsigned)height };

			vkCmdBeginRenderPass(m_buf, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			for (size_t i = 0; i < m_render_pass->num_pipelines(); i++)
			{
				vkCmdBindPipeline(m_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_render_pass->pipeline((int)i));
				vkCmdSetViewport(m_buf, 0, 1, &viewport);
				vkCmdSetScissor(m_buf, 0, 1, &scissor);
				vkCmdBindDescriptorSets(m_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, m_render_pass->layout_pipeline(), 0, 1, &m_descriptorSet, 0, nullptr);
				if (draw_params[i].indBuf == nullptr)
				{
					vkCmdDraw(m_buf, draw_params[i].count, 1, 0, 0);
				}
				else
				{
					vkCmdBindIndexBuffer(m_buf, draw_params[i].indBuf->buf(), 0, draw_params[i].indType);
					vkCmdDrawIndexed(m_buf, draw_params[i].count, 1, 0, 0, 0);
				}
			}

			for (size_t i = 0; i < m_render_pass->num_resolve_attachments(); i++)
				resolveBufs[i]->apply_barrier(*this, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);

			vkCmdEndRenderPass(m_buf);

		}
	}
}

#ifdef _VkInlineEX
#include "internal_context_ex.inl"
#endif

