#pragma once

#include "volk.h"
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <mutex>
#include <queue>

namespace VkInline
{
	namespace Internal
	{
		struct CommandBuffer
		{
			int m_streamId;
			VkCommandBuffer m_buf;
			VkFence m_fence;
		};

		class Context
		{
		public:
			static const Context* get_context(bool cleanup = false);
			const VkInstance& instance() const { return m_instance; }
			const VkPhysicalDevice& physicalDevice() const { return m_physicalDevice; }
			const VkDevice& device() const { return m_device; }

			CommandBuffer NewCommandBuffer(int streamId = 0) const;
			void SubmitCommandBuffer(const CommandBuffer& cmdBuf, size_t n = 1) const;
			void Wait(int streamId = 0) const;

		private:
			VkDebugUtilsMessengerEXT m_debugMessenger;
			VkInstance m_instance;
			VkPhysicalDevice m_physicalDevice;
			VkPhysicalDeviceBufferDeviceAddressFeatures m_bufferDeviceAddressFeatures;
			VkPhysicalDeviceDescriptorIndexingFeatures m_descriptorIndexingFeatures;
			VkPhysicalDeviceScalarBlockLayoutFeatures m_scalarBlockLayoutFeatures;
			VkPhysicalDeviceFeatures2 m_features2;
			uint32_t m_queueFamily;
			float m_queuePriority;
			VkDevice m_device;

			VkQueue m_queue;
			std::mutex* m_mu_queue;

			struct Stream
			{
				VkCommandPool m_commandPool;
				std::queue<CommandBuffer> m_queue_wait;
				std::queue<CommandBuffer> m_queue_recycle;
			};

			std::unordered_map<int, Stream>* m_streams;
			std::mutex* m_mu_streams;

			Stream& _stream(int i) const;

			bool _init_vulkan();
			Context();
			~Context();
		};

		class Buffer
		{
		public:
			VkDeviceSize size() const { return m_size; }
			const VkBuffer& buf() const { return m_buf; }
			const VkDeviceMemory& memory() const { return m_mem; }

			VkDeviceAddress get_device_address() const;

		protected:
			VkDeviceSize m_size;
			VkBuffer m_buf;
			VkDeviceMemory m_mem;

			Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags);
			virtual ~Buffer();

		};

		class UploadBuffer : public Buffer
		{
		public:
			UploadBuffer(VkDeviceSize size, VkBufferUsageFlags usage=0);
			virtual ~UploadBuffer();

			void upload(const void* hdata);
			void zero();

		};

		class DownloadBuffer : public Buffer
		{
		public:
			DownloadBuffer(VkDeviceSize size, VkBufferUsageFlags usage=0);
			virtual ~DownloadBuffer();

			void download(void* hdata);
		};

		class DeviceBuffer : public Buffer
		{
		public:
			DeviceBuffer(VkDeviceSize size, VkBufferUsageFlags usage);
			virtual ~DeviceBuffer();

			void upload(const void* hdata, int streamId = 0);
			void zero(int streamId = 0);
			void download(void* hdata, VkDeviceSize begin = 0, VkDeviceSize end = (VkDeviceSize)(-1), int streamId = 0) const;

		};

		class ComputePipeline
		{
		public:
			ComputePipeline(const std::vector<unsigned>& spv, size_t ubo_size);
			~ComputePipeline();

			void launch(const CommandBuffer& cmdbuf, void* param_data, unsigned dim_x, unsigned dim_y, unsigned dim_z) const;

		private:
			VkShaderModule m_shaderModule;
			VkDescriptorSetLayout m_descriptorSetLayout;
			VkPipelineLayout m_pipelineLayout;
			VkPipeline m_pipeline;
			UploadBuffer* m_ubo;
			VkDescriptorPool m_descriptorPool;
			VkDescriptorSet m_descriptorSet;

		};

	}

}