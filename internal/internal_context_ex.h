/***************
Known-limitations:
* Objects are opaque. No anyhit-shader for now.
******************/
namespace VkInline
{
	namespace Internal
	{
		class AS
		{
		public:
			const VkAccelerationStructureKHR & structure() const { return m_structure; }

		protected:
			AS();
			virtual ~AS();
			VkAccelerationStructureKHR  m_structure;
		};

		class BaseLevelAS : public AS
		{
		public:
			BaseLevelAS(
				const VkAccelerationStructureBuildGeometryInfoKHR& geoBuildInfo,
				const VkAccelerationStructureGeometryKHR* pGeometries,
				const VkAccelerationStructureBuildRangeInfoKHR** ranges);

			virtual ~BaseLevelAS();

		private:
			DeviceBuffer* m_scratchBuffer;
			DeviceBuffer* m_resultBuffer;
		};

		class TopLevelAS : public AS
		{
		public:
			TopLevelAS(size_t num_hitgroups, size_t* num_instances, const VkAccelerationStructureKHR** pblases, const float*** ptransforms);
			virtual ~TopLevelAS();

		private:
			DeviceBuffer* m_scratchBuffer;
			DeviceBuffer* m_resultBuffer;
			DeviceBuffer* m_instancesBuffer;
		};

		struct HitShaders
		{
			const std::vector<unsigned>* closest_hit;
			const std::vector<unsigned>* intersection;
			// const std::vector<unsigned>* any_hit;			
		};

		class RayTracePipeline
		{
		public:
			RayTracePipeline(const std::vector<unsigned>& spv_raygen,
				const std::vector<const std::vector<unsigned>*>& spv_miss,
				const std::vector<HitShaders>& spv_hit,
				unsigned maxRecursionDepth, size_t num_tlas, size_t num_tex2d, size_t num_tex3d, size_t num_cubemap);

			~RayTracePipeline();

			const VkDescriptorSetLayout& layout_desc() const { return m_descriptorSetLayout; }
			const VkPipelineLayout& layout_pipeline() const { return m_pipelineLayout; }
			const VkPipeline& pipeline() const { return m_pipeline; }
			const VkStridedDeviceAddressRegionKHR& sbt_entry_raygen() const { return m_sbt_entry_raygen; }
			const VkStridedDeviceAddressRegionKHR& sbt_entry_miss() const { return m_sbt_entry_miss; }
			const VkStridedDeviceAddressRegionKHR& sbt_entry_hit() const { return m_sbt_entry_hit; }
			const VkStridedDeviceAddressRegionKHR& sbt_entry_callable() const { return m_sbt_entry_callable; }

			size_t num_tlas() const { return m_num_tlas; }
			size_t num_tex2d() const { return m_num_tex2d; }
			size_t num_tex3d() const { return m_num_tex3d; }
			size_t num_cubemap() const { return m_num_cubemap; }
			Sampler* sampler() const { return m_sampler; }
			CommandBufferRecycler* recycler() const;


		private:
			VkDescriptorSetLayout m_descriptorSetLayout;
			VkPipelineLayout m_pipelineLayout;
			VkPipeline m_pipeline;
			DeviceBuffer* m_shaderBindingTableBuffer;
			VkStridedDeviceAddressRegionKHR m_sbt_entry_raygen;
			VkStridedDeviceAddressRegionKHR m_sbt_entry_miss;
			VkStridedDeviceAddressRegionKHR m_sbt_entry_hit;
			VkStridedDeviceAddressRegionKHR m_sbt_entry_callable;

			size_t m_num_tlas;
			size_t m_num_tex2d;
			size_t m_num_tex3d;
			size_t m_num_cubemap;
			Sampler* m_sampler;

			mutable std::unordered_map<std::thread::id, CommandBufferRecycler*> m_recyclers;
			mutable std::shared_mutex m_mu_streams;

		};

		class RayTraceCommandBuffer : public CommandBuffer
		{
		public:
			RayTraceCommandBuffer(const RayTracePipeline* pipeline, size_t ubo_size);
			~RayTraceCommandBuffer();

			virtual void Recycle();
			void trace(void* param_data, TopLevelAS** arr_tlas, Texture2D** tex2ds, Texture3D** tex3ds, TextureCube** cubemaps, unsigned dim_x, unsigned dim_y, unsigned dim_z);

		private:
			const RayTracePipeline* m_pipeline;
			DeviceBuffer* m_ubo;
			VkDescriptorPool m_descriptorPool;
			VkDescriptorSet m_descriptorSet;

		};
	}
}

