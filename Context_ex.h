namespace VkInline
{
	namespace Internal
	{
		class BaseLevelAS;
		class TopLevelAS;
	}

	class BaseLevelAS
	{
	public:
		BaseLevelAS(SVBuffer* indBuf, SVBuffer* posBuf);
		BaseLevelAS(SVBuffer* aabbBuf);
		~BaseLevelAS();

		Internal::BaseLevelAS* internal() { return m_blas; }
		const Internal::BaseLevelAS* internal() const { return m_blas; }

	private:
		Internal::BaseLevelAS* m_blas;

	};

	class Mat4
	{
	public:
		Mat4(const float* data);
		const float* data() const { return m_data; }

	private:
		float m_data[16];
	};

	struct BLAS_EX
	{
		const BaseLevelAS* blas;
		const Mat4* trans;
	};

	class TopLevelAS
	{
	public:
		TopLevelAS(const std::vector<std::vector<BLAS_EX>>& blases);
		~TopLevelAS();

		Internal::TopLevelAS* internal() { return m_tlas; }
		const Internal::TopLevelAS* internal() const { return m_tlas; }

	private:
		Internal::TopLevelAS* m_tlas;
	};

}
