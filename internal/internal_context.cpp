#include "internal_context.h"
#include "vk_format_utils.h"
#include <memory.h>
#include <vector>

namespace VkInline
{
	namespace Internal
	{
		const Context* Context::get_context(bool cleanup)
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
			return s_ctx;
		}

		VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
		{
			printf("validation layer: %s\n", pCallbackData->pMessage);
			return VK_FALSE;
		}

		bool Context::_init_vulkan()
		{
			if (volkInitialize() != VK_SUCCESS) return false;

			{
				VkApplicationInfo appInfo = {};
				appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
				appInfo.pApplicationName = "TextureGen";
				appInfo.applicationVersion = VK_MAKE_VERSION(1, 2, 0);
				appInfo.pEngineName = "No Engine";
				appInfo.engineVersion = VK_MAKE_VERSION(1, 2, 0);
				appInfo.apiVersion = VK_API_VERSION_1_2;

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

				if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) return false;

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

			m_bufferDeviceAddressFeatures = {};
			m_descriptorIndexingFeatures = {};
			m_scalarBlockLayoutFeatures = {};
			{
				m_bufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
				m_bufferDeviceAddressFeatures.pNext = &m_descriptorIndexingFeatures;
				m_descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
				m_descriptorIndexingFeatures.pNext = &m_scalarBlockLayoutFeatures;
				m_scalarBlockLayoutFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES_EXT;
				m_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
				m_features2.pNext = &m_bufferDeviceAddressFeatures;
				m_features2.features = {};
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
					VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
					VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
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

				if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) return false;
			}

			vkGetDeviceQueue(m_device, m_queueFamily, 0, &m_queue);

			return true;

		}

		Context::Context()
		{
			if (!_init_vulkan()) exit(0);
			m_mu_queue = new std::mutex;
			m_streams = new std::unordered_map<int, Stream>;
			m_mu_streams = new std::mutex;
		}

		Context::~Context()
		{
			auto iter = m_streams->begin();
			while (iter != m_streams->end())
			{
				while (iter->second.m_queue_recycle.size() > 0)
				{
					CommandBuffer cb = iter->second.m_queue_recycle.front();
					iter->second.m_queue_recycle.pop();
					vkDestroyFence(m_device, cb.m_fence, nullptr);
				}
				vkDestroyCommandPool(m_device, iter->second.m_commandPool, nullptr);
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

		Context::Stream& Context::_stream(int i) const
		{
			std::unique_lock<std::mutex> locker(*m_mu_streams);
			auto iter = m_streams->find(i);
			if (iter != m_streams->end()) return iter->second;
			Context::Stream& stream = (*m_streams)[i];
			locker.unlock();
			VkCommandPoolCreateInfo poolInfo = {};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.queueFamilyIndex = m_queueFamily;
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			vkCreateCommandPool(m_device, &poolInfo, nullptr, &stream.m_commandPool);
			return stream;
		}

		CommandBuffer Context::NewCommandBuffer(int streamId) const
		{
			CommandBuffer ret;
			ret.m_streamId = streamId;

			Context::Stream& s = _stream(streamId);

			if (s.m_queue_recycle.size() > 0)
			{
				ret = s.m_queue_recycle.front();
				s.m_queue_recycle.pop();
			}
			else
			{
				VkCommandBufferAllocateInfo allocInfo = {};
				allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				allocInfo.commandPool = s.m_commandPool;
				allocInfo.commandBufferCount = 1;
				vkAllocateCommandBuffers(m_device, &allocInfo, &ret.m_buf);
				VkFenceCreateInfo fenceInfo = {};
				fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				vkCreateFence(m_device, &fenceInfo, nullptr, &ret.m_fence);
			}

			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			vkBeginCommandBuffer(ret.m_buf, &beginInfo);
			return ret;
		}

		void Context::SubmitCommandBuffer(const CommandBuffer& cmdBuf, size_t n) const
		{
			vkEndCommandBuffer(cmdBuf.m_buf);
			Context::Stream& s = _stream(cmdBuf.m_streamId);
			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &cmdBuf.m_buf;

			{
				std::unique_lock<std::mutex> locker(*m_mu_queue);
				for (size_t i = 0; i < n; i++)
				{
					VkFence f = 0;
					if (i == n - 1) f = cmdBuf.m_fence;
					vkQueueSubmit(m_queue, 1, &submitInfo, f);
				}
			}
			s.m_queue_wait.push(cmdBuf);
		}

		void Context::Wait(int streamId) const
		{
			Context::Stream& s = _stream(streamId);
			while (s.m_queue_wait.size() > 0)
			{
				CommandBuffer cb = s.m_queue_wait.front();
				s.m_queue_wait.pop();
				vkWaitForFences(m_device, 1, &cb.m_fence, VK_TRUE, UINT64_MAX);
				vkResetFences(m_device, 1, &cb.m_fence);
				vkResetCommandBuffer(cb.m_buf, 0);
				s.m_queue_recycle.push(cb);
			}
		}


		VkDeviceAddress Buffer::get_device_address() const
		{
			if (m_size == 0) return 0;
			const Context* ctx = Context::get_context();

			VkBufferDeviceAddressInfo bufAdrInfo = {};
			bufAdrInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
			bufAdrInfo.buffer = m_buf;
			return vkGetBufferDeviceAddressKHR(ctx->device(), &bufAdrInfo);
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

			VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo = {};
			memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;

			if ((usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) != 0)
				memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

			memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;

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

		void DeviceBuffer::upload(const void* hdata, int streamId)
		{
			UploadBuffer staging_buf(m_size);
			staging_buf.upload(hdata);

			const Context* ctx = Context::get_context();
			CommandBuffer cmdBuf = ctx->NewCommandBuffer(streamId);

			VkBufferMemoryBarrier barriers[1];
			barriers[0] = {};
			barriers[0].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			barriers[0].buffer = m_buf;
			barriers[0].offset = 0;
			barriers[0].size = VK_WHOLE_SIZE;
			barriers[0].srcAccessMask = 0;
			barriers[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			vkCmdPipelineBarrier(
				cmdBuf.m_buf,
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
			copyRegion.size = m_size;
			vkCmdCopyBuffer(cmdBuf.m_buf, staging_buf.buf(), m_buf, 1, &copyRegion);
			ctx->SubmitCommandBuffer(cmdBuf);
			ctx->Wait(streamId);
		}

		void DeviceBuffer::zero(int streamId)
		{
			UploadBuffer staging_buf(m_size);
			staging_buf.zero();

			const Context* ctx = Context::get_context();
			CommandBuffer cmdBuf = ctx->NewCommandBuffer(streamId);

			VkBufferMemoryBarrier barriers[1];
			barriers[0] = {};
			barriers[0].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			barriers[0].buffer = m_buf;
			barriers[0].offset = 0;
			barriers[0].size = VK_WHOLE_SIZE;
			barriers[0].srcAccessMask = 0;
			barriers[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			vkCmdPipelineBarrier(
				cmdBuf.m_buf,
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
			copyRegion.size = m_size;
			vkCmdCopyBuffer(cmdBuf.m_buf, staging_buf.buf(), m_buf, 1, &copyRegion);
			ctx->SubmitCommandBuffer(cmdBuf);
			ctx->Wait(streamId);
		}

		void DeviceBuffer::download(void* hdata, VkDeviceSize begin, VkDeviceSize end, int streamId) const
		{
			if (end > m_size) end = m_size;
			if (end <= begin) return;

			DownloadBuffer staging_buf(end - begin);

			const Context* ctx = Context::get_context();
			CommandBuffer cmdBuf = ctx->NewCommandBuffer(streamId);

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
				cmdBuf.m_buf,
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
			vkCmdCopyBuffer(cmdBuf.m_buf, m_buf, staging_buf.buf(), 1, &copyRegion);
			ctx->SubmitCommandBuffer(cmdBuf);
			ctx->Wait(streamId);

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
			imageInfo.usage = usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
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

		void Texture2D::upload(const void* hdata, int streamId)
		{
			if (m_width == 0 || m_height == 0) return;
			UploadBuffer staging_buf(m_width*m_height*pixel_size());
			staging_buf.upload(hdata);

			const Context* ctx = Context::get_context();
			CommandBuffer cmdBuf = ctx->NewCommandBuffer(streamId);

			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = m_image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			vkCmdPipelineBarrier(
				cmdBuf.m_buf,
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

			vkCmdCopyBufferToImage(
				cmdBuf.m_buf,
				staging_buf.buf(),
				m_image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&region
			);

			ctx->SubmitCommandBuffer(cmdBuf);
			ctx->Wait(streamId);
		}

		void Texture2D::download(void* hdata, int streamId) const
		{
			if (m_width == 0 || m_height == 0) return;
			DownloadBuffer staging_buf(m_width*m_height*pixel_size());

			const Context* ctx = Context::get_context();
			CommandBuffer cmdBuf = ctx->NewCommandBuffer(streamId);

			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = m_image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(
				cmdBuf.m_buf,
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
				cmdBuf.m_buf,
				m_image,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				staging_buf.buf(),
				1,
				&region
			);

			ctx->SubmitCommandBuffer(cmdBuf);
			ctx->Wait(streamId);

			staging_buf.download(hdata);
		}


		ComputePipeline::ComputePipeline(const std::vector<unsigned>& spv, size_t ubo_size)
		{
			const Context* ctx = Context::get_context();
			{
				VkShaderModuleCreateInfo createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				createInfo.codeSize = spv.size()*sizeof(unsigned);
				createInfo.pCode = reinterpret_cast<const uint32_t*>(spv.data());
				vkCreateShaderModule(ctx->device(), &createInfo, nullptr, &m_shaderModule);
			}
			{
				VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[1];
				descriptorSetLayoutBindings[0] = {};
				descriptorSetLayoutBindings[0].binding = 0;
				descriptorSetLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorSetLayoutBindings[0].descriptorCount = 1;
				descriptorSetLayoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

				VkDescriptorSetLayoutCreateInfo layoutInfo = {};
				layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				layoutInfo.bindingCount = 1;
				layoutInfo.pBindings = descriptorSetLayoutBindings;

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
				computeShaderStageInfo.module = m_shaderModule;
				computeShaderStageInfo.pName = "main";

				VkComputePipelineCreateInfo pipelineInfo = {};
				pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
				pipelineInfo.stage = computeShaderStageInfo;
				pipelineInfo.layout = m_pipelineLayout;

				vkCreateComputePipelines(ctx->device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline);
			}

			m_ubo = new UploadBuffer(ubo_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

			{
				VkDescriptorPoolSize poolSizes[1];
				poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				poolSizes[0].descriptorCount = 1;

				VkDescriptorPoolCreateInfo poolInfo = {};
				poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				poolInfo.poolSizeCount = 1;
				poolInfo.pPoolSizes = poolSizes;
				poolInfo.maxSets = 1;
				vkCreateDescriptorPool(ctx->device(), &poolInfo, nullptr, &m_descriptorPool);
			}

			{
				VkDescriptorSetAllocateInfo allocInfo = {};
				allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				allocInfo.descriptorPool = m_descriptorPool;
				allocInfo.descriptorSetCount = 1;
				allocInfo.pSetLayouts = &m_descriptorSetLayout;
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

		ComputePipeline::~ComputePipeline()
		{
			const Context* ctx = Context::get_context();
			vkDestroyDescriptorPool(ctx->device(), m_descriptorPool, nullptr);
			vkDestroyPipeline(ctx->device(), m_pipeline, nullptr);
			vkDestroyPipelineLayout(ctx->device(), m_pipelineLayout, nullptr);
			vkDestroyDescriptorSetLayout(ctx->device(), m_descriptorSetLayout, nullptr);
			vkDestroyShaderModule(ctx->device(), m_shaderModule, nullptr);
			delete m_ubo;
		}

		void ComputePipeline::launch(const CommandBuffer& cmdbuf, void* param_data, unsigned dim_x, unsigned dim_y, unsigned dim_z) const
		{
			m_ubo->upload(param_data);

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
				cmdbuf.m_buf,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				0,
				0, nullptr,
				1, barriers,
				0, nullptr
			);

			vkCmdBindPipeline(cmdbuf.m_buf, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline);
			vkCmdBindDescriptorSets(cmdbuf.m_buf, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, 0);
			vkCmdDispatch(cmdbuf.m_buf, dim_x, dim_y, dim_z);
		}
	}
}
