#include "Context.h"
#include "internal_context.h"
#include <unordered_map>
#include <unordered_set>

namespace VkInline
{
	class Context
	{
	public:
		static Context& get_context();
		void set_verbose(bool verbose = true);

		// reflection 
		size_t size_of(const char* cls);
		bool query_struct(const char* name_struct, size_t* member_offsets);
		bool launch_compute(dim_type gridDim, dim_type blockDim, const std::vector<CapturedShaderViewable>& arg_map, const char* code_body, int streamId);

		void add_built_in_header(const char* name, const char* content);
		void add_code_block(const char* code);
		void add_inlcude_filename(const char* fn);

		std::string add_dynamic_code(const char* code);

	private:
		Context();
		~Context();

		unsigned _build_compute_pipeline(dim_type blockDim, const std::vector<CapturedShaderViewable>& arg_map, const char* code_body, size_t ubo_size);

		bool m_verbose;
		std::unordered_map<std::string, const char*> m_header_map;
		std::vector<std::string> m_code_blocks;

		std::string m_header_of_dynamic_code;
		std::string m_name_header_of_dynamic_code;
		std::unordered_set<int64_t> m_known_code;

		std::unordered_map<std::string, size_t> m_size_of_types;
		std::unordered_map<std::string, std::vector<size_t>> m_offsets_of_structs;

		std::vector <Internal::ComputePipeline*> m_cache_compute_pipelines;
		std::unordered_map<int64_t, unsigned> m_map_compute_pipelines;

	};

}

#include "impl_context.inl"

namespace VkInline
{
	void SetVerbose(bool verbose)
	{
		Context& ctx = Context::get_context();
		ctx.set_verbose(verbose);
	}

	size_t SizeOf(const char* cls)
	{
		Context& ctx = Context::get_context();
		return ctx.size_of(cls);
	}

	bool QueryStruct(const char* name_struct, size_t* offsets)
	{
		Context& ctx = Context::get_context();
		return ctx.query_struct(name_struct, offsets);
	}

	void AddBuiltInHeader(const char* name, const char* content)
	{
		Context& ctx = Context::get_context();
		ctx.add_built_in_header(name, content);
	}

	void AddCodeBlock(const char* code)
	{
		Context& ctx = Context::get_context();
		ctx.add_code_block(code);
	}

	void AddInlcudeFilename(const char* fn)
	{
		Context& ctx = Context::get_context();
		ctx.add_inlcude_filename(fn);
	}

	std::string Add_Dynamic_Code(const char* code)
	{
		return Context::get_context().add_dynamic_code(code);
	}

	void Wait(int streamId)
	{
		auto context = Internal::Context::get_context();
		context->Wait(streamId);
	}

	Computer::Computer(const std::vector<const char*>& param_names, const char* code_body) :
		m_param_names(param_names.size()), m_code_body(code_body)
	{
		for (size_t i = 0; i < param_names.size(); i++)
			m_param_names[i] = param_names[i];
	}

	bool Computer::launch(dim_type gridDim, dim_type blockDim, const ShaderViewable** args, int streamId)
	{
		Context& ctx = Context::get_context();
		std::vector<CapturedShaderViewable> arg_map(m_param_names.size());
		for (size_t i = 0; i < m_param_names.size(); i++)
		{
			arg_map[i].obj_name = m_param_names[i].c_str();
			arg_map[i].obj = args[i];
		}
		return ctx.launch_compute(gridDim, blockDim, arg_map, m_code_body.c_str(), streamId);
	}


}

