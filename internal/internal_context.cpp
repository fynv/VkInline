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
				appInfo.applicationVersion = VK_MAKE_VERSION(1, 1, 0);
				appInfo.pEngineName = "No Engine";
				appInfo.engineVersion = VK_MAKE_VERSION(1, 1, 0);
				appInfo.apiVersion = VK_API_VERSION_1_1;

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
				m_bufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT;
				m_bufferDeviceAddressFeatures.pNext = &m_descriptorIndexingFeatures;
				m_descriptorIndexingFeatures = {};
				m_descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
				m_descriptorIndexingFeatures.pNext = &m_scalarBlockLayoutFeatures;
				m_scalarBlockLayoutFeatures = {};
				m_scalarBlockLayoutFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES_EXT;
				m_features2 = {};
				m_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
				m_features2.pNext = &m_bufferDeviceAddressFeatures;
				vkGetPhysicalDeviceFeatures2(m_physicalDevice, &m_features2);
			}

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
					VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
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
					printf("Failed to create vulkan device\n");
					return false;
				}
			}

			vkGetDeviceQueue(m_device, m_queueFamily, 0, &m_queue);

			return true;

		}

		Context::Context()
		{
			m_is_valid = _init_vulkan();
			if (!m_is_valid) return;
			m_mu_queue = new std::mutex;
			m_streams = new std::unordered_map<std::thread::id, Stream*>;
			m_mu_streams = new std::shared_mutex;
		}

		Context::~Context()
		{
			if (!m_is_valid) return;

			auto iter = m_streams->begin();
			while (iter != m_streams->end())
			{
				vkDestroyCommandPool(m_device, iter->second->m_commandPool, nullptr);
				delete iter->second;
				iter++;
			}

			delete m_mu_streams;
			delete m_streams;
			delete m_mu_queue;

#ifdef _DEBUG
			vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
#endif
			vkDestroyDevice(m_device, nullptr);
			vkDestroyInstance(m_instance, nullptr);
		}

		Context::Stream* Context::_stream(std::thread::id threadId) const
		{
			{
				std::shared_lock<std::shared_mutex> locker(*m_mu_streams);
				auto iter = m_streams->find(threadId);
				if (iter != m_streams->end()) return iter->second;
			}
			Context::Stream* stream = new Context::Stream;
			{
				std::unique_lock<std::shared_mutex> locker(*m_mu_streams);
				(*m_streams)[threadId] = stream;
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
				std::unique_lock<std::mutex> locker(*m_mu_queue);
				for (size_t i = 0; i < n; i++)
				{
					VkFence f = 0;
					if (i == n - 1) f = cmdBuf->fence();
					vkQueueSubmit(m_queue, 1, &submitInfo, f);
				}
			}
			s->m_queue_wait.push(cmdBuf);
		}

		void Context::WaitUtil(CommandBuffer* lastCmdBuf) const
		{
			Context::Stream* s = stream();
			while (s->m_queue_wait.size() > 0)
			{
				CommandBuffer* cb = s->m_queue_wait.front();
				s->m_queue_wait.pop();
				vkWaitForFences(m_device, 1, &cb->fence(), VK_TRUE, UINT64_MAX);
				vkResetFences(m_device, 1, &cb->fence());
				vkResetCommandBuffer(cb->buf(), 0);
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
			return vkGetBufferDeviceAddressEXT(ctx->device(), &bufAdrInfo);
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

			vkAllocateMemory(ctx->device(), &memoryAllocateInfo, nullptr, &m_mem);
			vkBindBufferMemory(ctx->device(), m_buf, m_mem, 0);
		}

		Buffer::~Buffer()
		{
			const Context* ctx = Context::get_context();
			vkDestroyBuffer(ctx->device(), m_buf, nullptr);
			vkFreeMemory(ctx->device(), m_mem, nullptr);
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
			CommandBuf_DevBufUpload(VkDeviceSize size, VkBuffer buf) : m_staging_buf(size)
			{
				m_staging_buf.zero();
				_upload(size, buf);
			}

			CommandBuf_DevBufUpload(VkDeviceSize size, VkBuffer buf, const void* hdata) : m_staging_buf(size)
			{
				m_staging_buf.upload(hdata);
				_upload(size, buf);
			}

		private:
			void _upload(VkDeviceSize size, VkBuffer buf)
			{
				VkBufferMemoryBarrier barriers[1];
				barriers[0] = {};
				barriers[0].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
				barriers[0].buffer = buf;
				barriers[0].offset = 0;
				barriers[0].size = VK_WHOLE_SIZE;
				barriers[0].srcAccessMask = 0;
				barriers[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

				vkCmdPipelineBarrier(
					m_buf,
					VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					0,
					0, nullptr,
					1, barriers,
					0, nullptr
				);

				VkBufferCopy copyRegion = {};
				copyRegion.srcOffset = 0;
				copyRegion.dstOffset = 0;
				copyRegion.size = size;
				vkCmdCopyBuffer(m_buf, m_staging_buf.buf(), buf, 1, &copyRegion);
			}

			UploadBuffer m_staging_buf;
		};


		void DeviceBuffer::upload(const void* hdata)
		{
			auto cmdBuf = new CommandBuf_DevBufUpload(m_size, m_buf, hdata);
			const Context* ctx = Context::get_context();
			ctx->SubmitCommandBuffer(cmdBuf);
		}

		void DeviceBuffer::zero()
		{
			auto cmdBuf = new CommandBuf_DevBufUpload(m_size, m_buf);
			const Context* ctx = Context::get_context();
			ctx->SubmitCommandBuffer(cmdBuf);
		}

		void DeviceBuffer::download(void* hdata, VkDeviceSize begin, VkDeviceSize end) const
		{
			if (end > m_size) end = m_size;
			if (end <= begin) return;

			DownloadBuffer staging_buf(end - begin);
			auto cmdBuf = new AutoCommandBuffer;

			VkBufferMemoryBarrier barriers[1];
			barriers[0] = {};
			barriers[0].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			barriers[0].buffer = m_buf;
			barriers[0].offset = 0;
			barriers[0].size = VK_WHOLE_SIZE;
			barriers[0].srcAccessMask = 0;
			barriers[0].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			vkCmdPipelineBarrier(
				cmdBuf->buf(),
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				0,
				0, nullptr,
				1, barriers,
				0, nullptr
			);

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

		Texture2D::Texture2D(int width, int height, VkFormat format, VkImageAspectFlags aspectFlags, VkImageUsageFlags usage)
		{
			m_width = width;
			m_height = height;
			m_format = format;
			m_aspect = aspectFlags;
			if (width == 0 || height == 0) return;

			const Context* ctx = Context::get_context();

			usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

			VkFormatProperties format_props;
			vkGetPhysicalDeviceFormatProperties(ctx->physicalDevice(), format, &format_props);		
			if ((format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) == 0
				|| (format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) == 0)
				usage &= ~VK_IMAGE_USAGE_SAMPLED_BIT;
			if ((format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) == 0)
				usage &= ~VK_IMAGE_USAGE_STORAGE_BIT;
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
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = format;
			createInfo.subresourceRange.aspectMask = aspectFlags;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;
			vkCreateImageView(ctx->device(), &createInfo, nullptr, &m_view);
		}


		Texture2D::~Texture2D()
		{
			if (m_width == 0 || m_height == 0) return;
			const Context* ctx = Context::get_context();
			vkDestroyImageView(ctx->device(), m_view, nullptr);
			vkDestroyImage(ctx->device(), m_image, nullptr);
			vkFreeMemory(ctx->device(), m_mem, nullptr);
		}

		class CommandBuf_TexUpload : public AutoCommandBuffer
		{
		public:
			CommandBuf_TexUpload(int width, int height, unsigned pixel_size, VkImage image, VkImageAspectFlags aspectFlags, const void* hdata)
				: m_staging_buf(width*height*pixel_size)
			{
				m_staging_buf.upload(hdata);

				VkImageMemoryBarrier barrier = {};
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.image = image;
				barrier.subresourceRange.aspectMask = aspectFlags;
				barrier.subresourceRange.baseMipLevel = 0;
				barrier.subresourceRange.levelCount = 1;
				barrier.subresourceRange.baseArrayLayer = 0;
				barrier.subresourceRange.layerCount = 1;
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				vkCmdPipelineBarrier(
					m_buf,
					VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					0,
					0, nullptr,
					0, nullptr,
					1, &barrier
				);

				VkBufferImageCopy region = {};
				region.imageSubresource.aspectMask = aspectFlags;
				region.imageSubresource.layerCount = 1;
				region.imageExtent = {
					(uint32_t)width,
					(uint32_t)height,
					1
				};

				vkCmdCopyBufferToImage(
					m_buf,
					m_staging_buf.buf(),
					image,
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
			auto cmdBuf = new CommandBuf_TexUpload(m_width, m_height, pixel_size(), m_image, m_aspect, hdata);
			const Context* ctx = Context::get_context();
			ctx->SubmitCommandBuffer(cmdBuf);
		}

		void Texture2D::download(void* hdata) const
		{
			if (m_width == 0 || m_height == 0) return;
			DownloadBuffer staging_buf(m_width*m_height*pixel_size());

			auto cmdBuf = new AutoCommandBuffer;

			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = m_image;
			barrier.subresourceRange.aspectMask = m_aspect;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(
				cmdBuf->buf(),
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

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

		ComputePipeline::ComputePipeline(const std::vector<unsigned>& spv, size_t num_tex2d)
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
			m_recyclers = new std::unordered_map<std::thread::id, CommandBufferRecycler*>;
			m_mu_streams = new std::shared_mutex;

			vkDestroyShaderModule(ctx->device(), shader_module, nullptr);
		}

		ComputePipeline::~ComputePipeline()
		{
			auto iter = m_recyclers->begin();
			while (iter != m_recyclers->end())
			{
				delete iter->second;
				iter++;
			}
			delete m_mu_streams;
			delete m_recyclers;
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
				std::shared_lock<std::shared_mutex> locker(*m_mu_streams);
				auto iter = m_recyclers->find(threadId);
				if (iter != m_recyclers->end())
				{
					CommandBufferRecycler* ret = iter->second;
					return ret;
				}
			}

			CommandBufferRecycler* ret = new CommandBufferRecycler;

			{
				std::unique_lock<std::shared_mutex> locker(*m_mu_streams);
				(*m_recyclers)[threadId] = ret;
			}

			return ret;

		}

		void ComputePipeline::bind(const CommandBuffer& cmdbuf) const
		{
			vkCmdBindPipeline(cmdbuf.buf(), VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline);
		}

		void ComputePipeline::dispatch(const CommandBuffer& cmdbuf, unsigned dim_x, unsigned dim_y, unsigned dim_z) const
		{
			vkCmdDispatch(cmdbuf.buf(), dim_x, dim_y, dim_z);
		}


		ComputeCommandBuffer::ComputeCommandBuffer(const ComputePipeline* pipeline, size_t ubo_size)
		{
			const Context* ctx = Context::get_context();

			m_pipeline = pipeline;
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

		void ComputeCommandBuffer::dispatch(void* param_data, Texture2D** tex2ds, unsigned dim_x, unsigned dim_y, unsigned dim_z)
		{
			const Context* ctx = Context::get_context();
			m_ubo->upload(param_data);

			if (m_pipeline->num_tex2d() > 0)
			{
				std::vector<VkDescriptorImageInfo> imageInfos(m_pipeline->num_tex2d());
				for (size_t i = 0; i < m_pipeline->num_tex2d(); ++i)
				{
					imageInfos[i] = {};
					imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageInfos[i].imageView = tex2ds[i]->view();
					imageInfos[i].sampler = m_pipeline->sampler()->sampler();
				}

				VkWriteDescriptorSet wds = {};
				wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				wds.dstSet = m_descriptorSet;
				wds.dstBinding = 1;
				wds.descriptorCount = (uint32_t)m_pipeline->num_tex2d();
				wds.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				wds.pImageInfo = imageInfos.data();
				vkUpdateDescriptorSets(ctx->device(), 1, &wds, 0, nullptr);
			}

			VkBufferMemoryBarrier barriers[1];
			barriers[0] = {};
			barriers[0].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			barriers[0].buffer = m_ubo->buf();
			barriers[0].offset = 0;
			barriers[0].size = VK_WHOLE_SIZE;
			barriers[0].srcAccessMask = 0;
			barriers[0].dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT;
			barriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			vkCmdPipelineBarrier(
				m_buf,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				0,
				0, nullptr,
				1, barriers,
				0, nullptr
			);
			m_pipeline->bind(*this);
			vkCmdBindDescriptorSets(m_buf, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline->layout_pipeline(), 0, 1, &m_descriptorSet, 0, 0);
			m_pipeline->dispatch(*this, dim_x, dim_y, dim_z);
		}
	}
}
