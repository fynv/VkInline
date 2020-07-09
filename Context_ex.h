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

	class BodyHitShaders
	{
	public:
		BodyHitShaders(const char* body_closest_hit, const char* body_intersection);

		const char* body_closest_hit() const;
		const char* body_intersection() const;

	private:
		std::string m_body_closest_hit;
		std::string m_body_intersection;		
	};

	class RayTracer
	{
	public:
		size_t num_params() const { return m_param_names.size(); }
		RayTracer(const std::vector<const char*>& param_names, const char* body_raygen, const std::vector<const char*>& body_miss, const std::vector<const BodyHitShaders*>& body_hit, unsigned maxRecursionDepth, bool type_locked = false);
		bool launch(dim_type glbDim, const ShaderViewable** args, const std::vector<TopLevelAS*>& arr_tlas, const std::vector<Texture2D*>& tex2ds, const std::vector<Texture3D*>& tex3ds, size_t times_submission = 1);

	private:
		std::vector<std::string> m_param_names;
		std::string m_body_raygen;
		std::vector<std::string> m_body_miss;
		std::vector<const BodyHitShaders*> m_body_hit;
		unsigned m_maxRecursionDepth;

		bool m_type_locked;
		unsigned m_kid;
		std::vector<size_t> m_offsets;
		std::mutex m_mu_type_lock;

	};




}
