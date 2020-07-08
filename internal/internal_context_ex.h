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
			BaseLevelAS(uint32_t geometryCount, const VkAccelerationStructureCreateGeometryTypeInfoKHR* geoTypeInfo, const VkAccelerationStructureGeometryKHR* pGeometries, const VkAccelerationStructureBuildOffsetInfoKHR* offsets);
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

	}
}

