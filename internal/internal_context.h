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
			mutable std::mutex m_mu_queue;

			mutable std::unordered_map<std::thread::id, Stream*> m_streams;
			mutable std::shared_mutex m_mu_streams;

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

			void apply_barrier(const CommandBuffer& cmdbuf, VkAccessFlags dstAccessMask, VkPipelineStageFlags dstStageMask) const;

		protected:
			VkDeviceSize m_size;
			VkBuffer m_buf;
			VkDeviceMemory m_mem;

			mutable VkAccessFlags m_cur_access_mask;

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
			const VkSampleCountFlagBits& samples() const { return m_sampleCount; }
			const VkImage& image() const { return m_image; }
			const VkDeviceMemory& memory() const { return m_mem; }
			const VkImageView& view() const { return m_view; }

			Texture2D(int width, int height, VkFormat format, VkImageAspectFlags aspectFlags, VkImageUsageFlags usage, VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT);
			~Texture2D();

			void apply_barrier(const CommandBuffer& cmdbuf, VkImageLayout newLayout, VkAccessFlags dstAccessMask, VkPipelineStageFlags dstStageMask) const;

			void upload(const void* hdata);
			void download(void* hdata) const;


		private:
			int m_width;
			int m_height;
			VkFormat m_format;
			VkImageAspectFlags m_aspect;
			VkSampleCountFlagBits m_sampleCount;
			VkImage m_image;
			VkDeviceMemory m_mem;
			VkImageView m_view;

			mutable VkImageLayout m_cur_layout;
			mutable VkAccessFlags m_cur_access_mask;
			
		};

		class Texture3D
		{
		public:
			int dimX() const { return m_dims[0]; }
			int dimY() const { return m_dims[1]; }
			int dimZ() const { return m_dims[2]; }
			unsigned pixel_size() const;
			unsigned channel_count() const;
			const VkFormat& format() const { return m_format; }
			const VkImage& image() const { return m_image; }
			const VkDeviceMemory& memory() const { return m_mem; }
			const VkImageView& view() const { return m_view; }

			Texture3D(int dimX, int dimY, int dimZ, VkFormat format);
			~Texture3D();

			void apply_barrier(const CommandBuffer& cmdbuf, VkImageLayout newLayout, VkAccessFlags dstAccessMask, VkPipelineStageFlags dstStageMask) const;

			void upload(const void* hdata);
			void download(void* hdata) const;

		private:
			int m_dims[3];
			VkFormat m_format;
			VkImage m_image;
			VkDeviceMemory m_mem;
			VkImageView m_view;

			mutable VkImageLayout m_cur_layout;
			mutable VkAccessFlags m_cur_access_mask;
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
			ComputePipeline(const std::vector<unsigned>& spv, size_t num_tex2d, size_t num_tex3d);
			~ComputePipeline();

			const VkDescriptorSetLayout& layout_desc() const { return m_descriptorSetLayout; }
			const VkPipelineLayout& layout_pipeline() const { return m_pipelineLayout; }
			const VkPipeline& pipeline() const { return m_pipeline; }
			size_t num_tex2d() const { return m_num_tex2d; }
			size_t num_tex3d() const { return m_num_tex3d; }
			Sampler* sampler() const { return m_sampler; }
			CommandBufferRecycler* recycler() const;

		private:
			VkDescriptorSetLayout m_descriptorSetLayout;
			VkPipelineLayout m_pipelineLayout;
			VkPipeline m_pipeline;

			size_t m_num_tex2d;
			size_t m_num_tex3d;
			Sampler* m_sampler;

			mutable std::unordered_map<std::thread::id, CommandBufferRecycler*> m_recyclers;
			mutable std::shared_mutex m_mu_streams;

		};

		class ComputeCommandBuffer : public CommandBuffer
		{
		public:
			ComputeCommandBuffer(const ComputePipeline* pipeline, size_t ubo_size);
			~ComputeCommandBuffer();

			virtual void Recycle();
			void dispatch(void* param_data, Texture2D** tex2ds, Texture3D** tex3ds, unsigned dim_x, unsigned dim_y, unsigned dim_z);

		private:
			const ComputePipeline* m_pipeline;
			DeviceBuffer* m_ubo;
			VkDescriptorPool m_descriptorPool;
			VkDescriptorSet m_descriptorSet;

		};

		struct AttachmentInfo
		{
			VkFormat format;
			VkSampleCountFlagBits sample_count;
			bool clear_at_load;
		};

		struct GraphicsPipelineStates
		{
			VkPipelineInputAssemblyStateCreateInfo inputAssembly;
			VkPipelineRasterizationStateCreateInfo rasterizer;
			std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
			VkPipelineColorBlendStateCreateInfo colorBlending;
			VkPipelineDepthStencilStateCreateInfo depthStencil;
		};

		struct GraphicsPipelineInfo
		{
			const std::vector<unsigned>* spv_vert;
			const std::vector<unsigned>* spv_frag;
			GraphicsPipelineStates states;
		};

		class RenderPass
		{
		public:
			RenderPass(
				const std::vector<AttachmentInfo>& color_attachmentInfo,
				const AttachmentInfo* depth_attachmentInfo,
				const std::vector<AttachmentInfo>& resolve_attachmentInfo,
				const std::vector<GraphicsPipelineInfo>& pipelineInfo,
				size_t num_tex2d, size_t num_tex3d);

			~RenderPass();		

			const VkDescriptorSetLayout& layout_desc() const { return m_descriptorSetLayout; }
			const VkPipelineLayout& layout_pipeline() const { return m_pipelineLayout; }
			const VkRenderPass& render_pass() const { return m_renderPass; }
			size_t num_pipelines() const { return m_pipelines.size(); }
			size_t num_color_attachments() const { return m_num_color_attachments; }
			bool has_depth_attachment() const { return m_has_depth_attachment; }
			size_t num_resolve_attachments() const { return m_num_resolve_attachments; }
			unsigned sample_count() const { return m_sample_count;  }
			const VkPipeline& pipeline(int i) const { return m_pipelines[i]; }
			size_t num_tex2d() const { return m_num_tex2d; }
			size_t num_tex3d() const { return m_num_tex3d; }
			Sampler* sampler() const { return m_sampler; }
			CommandBufferRecycler* recycler() const;

		private:
			VkDescriptorSetLayout m_descriptorSetLayout;
			VkPipelineLayout m_pipelineLayout;
			VkRenderPass m_renderPass;
			std::vector<VkPipeline> m_pipelines;

			size_t m_num_color_attachments;
			bool m_has_depth_attachment;
			size_t m_num_resolve_attachments;
			unsigned m_sample_count;
			
			size_t m_num_tex2d;
			size_t m_num_tex3d;
			Sampler* m_sampler;		

			mutable std::unordered_map<std::thread::id, CommandBufferRecycler*> m_recyclers;
			mutable std::shared_mutex m_mu_streams;
		};

		class RenderPassCommandBuffer : public CommandBuffer
		{
		public:
			RenderPassCommandBuffer(const RenderPass* render_pass, size_t ubo_size);
			~RenderPassCommandBuffer();

			struct DrawParam
			{
				unsigned count;
				Buffer* indBuf;
			};

			virtual void Recycle();
			void draw(Texture2D** colorBufs, Texture2D* depthBuf, Texture2D** resolveBufs, float* clear_colors, float clear_depth,
				void* param_data, Texture2D** tex2ds, Texture3D** tex3ds, DrawParam* draw_params);
			
		private:
			const RenderPass* m_render_pass;
			DeviceBuffer* m_ubo;
			VkDescriptorPool m_descriptorPool;
			VkDescriptorSet m_descriptorSet;
			VkFramebuffer m_framebuffer;
		};
		

	}

}