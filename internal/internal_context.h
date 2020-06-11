#pragma once

#include "volk.h"
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <queue>
#include <thread>

namespace VkInline
{
	namespace Internal
	{
		class CommandBuffer;

		class Context
		{
		public:
			static const Context* get_context(bool cleanup = false, bool is_trying = false);
			const VkInstance& instance() const { return m_instance; }
			const VkPhysicalDevice& physicalDevice() const { return m_physicalDevice; }
			const VkDevice& device() const { return m_device; }

			struct Stream
			{
				VkCommandPool m_commandPool;
				std::queue<CommandBuffer*> m_queue_wait;
				std::mutex m_mutex_pool;
			};
			Stream* stream() const;

			void SubmitCommandBuffer(CommandBuffer* cmdBuf, size_t n = 1) const;
			void WaitUtil(CommandBuffer* lastCmdBuf) const;
			void Wait() const;

		private:
			VkDebugUtilsMessengerEXT m_debugMessenger;
			VkInstance m_instance;
			VkPhysicalDevice m_physicalDevice;
			VkPhysicalDeviceBufferDeviceAddressFeaturesEXT m_bufferDeviceAddressFeatures;
			VkPhysicalDeviceDescriptorIndexingFeatures m_descriptorIndexingFeatures;
			VkPhysicalDeviceScalarBlockLayoutFeatures m_scalarBlockLayoutFeatures;
			VkPhysicalDeviceFeatures2 m_features2;
			uint32_t m_queueFamily;
			float m_queuePriority;
			VkDevice m_device;

			VkQueue m_queue;
			std::mutex* m_mu_queue;

			std::unordered_map<std::thread::id, Stream*>* m_streams;
			std::shared_mutex* m_mu_streams;

			Stream* _stream(std::thread::id threadId) const;

			bool m_is_valid;
			bool _init_vulkan();
			Context();
			~Context();

		};

		class CommandBuffer
		{
		public:
			const VkCommandBuffer& buf() const { return m_buf; }
			const VkFence& fence() const { return m_fence; }

			CommandBuffer();
			virtual ~CommandBuffer();

			virtual void Recycle();

		protected:
			Context::Stream* m_stream;
			VkCommandBuffer m_buf;
			VkFence m_fence;

		};

		class AutoCommandBuffer : public CommandBuffer
		{
		public:
			AutoCommandBuffer() {}
			virtual ~AutoCommandBuffer() {}
			virtual void Recycle()
			{
				delete this;
			}
		};

		class CommandBufferRecycler
		{
		public:
			CommandBufferRecycler();
			~CommandBufferRecycler();
			void RecycleCommandBuffer(CommandBuffer* cmdbuf);
			CommandBuffer* RetriveCommandBuffer();


		private:
			std::queue<CommandBuffer*> m_queue_recycle;
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
			UploadBuffer(VkDeviceSize size, VkBufferUsageFlags usage = 0);
			virtual ~UploadBuffer();

			void upload(const void* hdata);
			void zero();
		};

		class DownloadBuffer : public Buffer
		{
		public:
			DownloadBuffer(VkDeviceSize size, VkBufferUsageFlags usage = 0);
			virtual ~DownloadBuffer();

			void download(void* hdata);
		};

		class DeviceBuffer : public Buffer
		{
		public:
			DeviceBuffer(VkDeviceSize size, VkBufferUsageFlags usage);
			virtual ~DeviceBuffer();

			void upload(const void* hdata);
			void zero();
			void download(void* hdata, VkDeviceSize begin = 0, VkDeviceSize end = (VkDeviceSize)(-1)) const;

		};

		class Texture2D
		{
		public:
			int width() const { return m_width; }
			int height() const { return m_height; }
			unsigned pixel_size() const;
			unsigned channel_count() const;
			const VkFormat& format() const { return m_format; }
			const VkImageAspectFlags& aspect() const { return m_aspect; }
			const VkImage& image() const { return m_image; }
			const VkDeviceMemory& memory() const { return m_mem; }
			const VkImageView& view() const { return m_view; }

			Texture2D(int width, int height, VkFormat format, VkImageAspectFlags aspectFlags, VkImageUsageFlags usage);
			~Texture2D();

			void upload(const void* hdata);
			void download(void* hdata) const;


		private:
			int m_width;
			int m_height;
			VkFormat m_format;
			VkImageAspectFlags m_aspect;
			VkImage m_image;
			VkDeviceMemory m_mem;
			VkImageView m_view;
		};

		class Sampler
		{
		public:
			Sampler();
			~Sampler();

			const VkSampler& sampler() const { return m_sampler; }

		private:
			VkSampler m_sampler;
		};


		class ComputePipeline
		{
		public:
			ComputePipeline(const std::vector<unsigned>& spv, size_t num_tex2d);
			~ComputePipeline();

			const VkDescriptorSetLayout& layout_desc() const { return m_descriptorSetLayout; }
			const VkPipelineLayout& layout_pipeline() const { return m_pipelineLayout; }
			const VkPipeline& pipeline() const { return m_pipeline; }
			size_t num_tex2d() const { return m_num_tex2d; }
			Sampler* sampler() const { return m_sampler; }
			CommandBufferRecycler* recycler() const;

			void bind(const CommandBuffer& cmdbuf) const;
			void dispatch(const CommandBuffer& cmdbuf, unsigned dim_x, unsigned dim_y, unsigned dim_z) const;

		private:
			VkDescriptorSetLayout m_descriptorSetLayout;
			VkPipelineLayout m_pipelineLayout;
			VkPipeline m_pipeline;

			size_t m_num_tex2d;
			Sampler* m_sampler;

			std::unordered_map<std::thread::id, CommandBufferRecycler*>* m_recyclers;
			std::shared_mutex* m_mu_streams;

		};

		class ComputeCommandBuffer : public CommandBuffer
		{
		public:
			ComputeCommandBuffer(const ComputePipeline* pipeline, size_t ubo_size);
			~ComputeCommandBuffer();

			virtual void Recycle();
			void dispatch(void* param_data, Texture2D** tex2ds, unsigned dim_x, unsigned dim_y, unsigned dim_z);

		private:
			const ComputePipeline* m_pipeline;
			DeviceBuffer* m_ubo;
			VkDescriptorPool m_descriptorPool;
			VkDescriptorSet m_descriptorSet;

		};


	}

}