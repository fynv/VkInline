#include "spirv_cross.hpp"
using namespace SPIRV_CROSS_NAMESPACE;
#include <unqlite.h>
#include "crc64.h"

bool GLSL2SPV_Compute(const char* InputCString, const std::unordered_map<std::string, const char*>* headers, std::vector<unsigned int>& SpirV);
bool GLSL2SPV_Vertex(const char* InputCString, const std::unordered_map<std::string, const char*>* headers, std::vector<unsigned int>& SpirV);
bool GLSL2SPV_Fragment(const char* InputCString, const std::unordered_map<std::string, const char*>* headers, std::vector<unsigned int>& SpirV);

namespace VkInline
{
	static char s_name_db[] = "__spv_cache__.db";

	Context& Context::get_context()
	{
		static Context s_ctx;
		return s_ctx;
	}

	Context::Context()
	{
		m_verbose = false;
		m_name_header_of_dynamic_code = "header_of_structs.h";
		this->add_built_in_header(m_name_header_of_dynamic_code.c_str(), m_header_of_dynamic_code.c_str());
	}

	Context::~Context()
	{
		auto context = Internal::Context::get_context();
		context->Wait();

#ifdef _VkInlineEX
		for (size_t i = 0; i < m_cache_raytrace_pipelines.size(); i++)
			delete m_cache_raytrace_pipelines[i];
#endif

		for (size_t i = 0; i < m_cache_compute_pipelines.size(); i++)
			delete m_cache_compute_pipelines[i];

		for (size_t i = 0; i < m_cache_render_passes.size(); i++)
			delete m_cache_render_passes[i];

		Internal::Context::get_context(true);
	}

	void Context::set_verbose(bool verbose)
	{
		m_verbose = verbose;
	}

	static void print_code(const char* name, const char* fullCode)
	{
		printf("%s:\n", name);
		const char* p = fullCode;
		int line_num = 1;
		while (true)
		{
			const char* p_nl = strchr(p, '\n');
			if (!p_nl)
				p_nl = p + strlen(p);

			char line[1024];
			int len = (int)(p_nl - p);
			if (len > 1023) len = 1023;
			memcpy(line, p, len);
			line[len] = 0;
			printf("%d\t%s\n", line_num, line);
			if (!*p_nl) break;
			p = p_nl + 1;
			line_num++;
		}
		puts("");
	}

	static inline unsigned long long s_get_hash(const char* source_code)
	{
		uint64_t len = (uint64_t)strlen(source_code);
		return (unsigned long long)crc64(0, (unsigned char*)source_code, len);
	}

	size_t Context::size_of(const char* cls)
	{
		// try to find in the context cache first
		std::unique_lock<std::mutex> lock(m_mutex_sizes);
		decltype(m_size_of_types)::iterator it = m_size_of_types.find(cls);
		if (it != m_size_of_types.end())
		{
			size_t size= it->second;
			return size;
		}

		// reflect from device code
		std::string saxpy =
			"#version 460\n"
			"#extension GL_GOOGLE_include_directive : enable\n"
			"#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable\n"
			"#extension GL_EXT_buffer_reference2 : enable\n"
			"#extension GL_EXT_nonuniform_qualifier : enable\n"
			"#extension GL_EXT_scalar_block_layout : enable\n";

		saxpy += "layout(binding = 1) uniform sampler2D arr_tex2d[];\n";
		saxpy += "layout(binding = 2) uniform sampler3D arr_tex3d[];\n";

		for (size_t i = 0; i < m_code_blocks.size(); i++)
		{
			saxpy += m_code_blocks[i];
		}
		saxpy += std::string("#include \"") + m_name_header_of_dynamic_code + "\"\n";

		saxpy +=
			"layout(scalar, binding = 0) buffer Params\n"
			"{\n    ";
		saxpy += cls;
		saxpy +=
			"[] x;\n"
			"};\n"
			"void main(){}\n";

		if (m_verbose)
		{
			{
				std::shared_lock<std::shared_mutex> lock(m_mutex_dynamic_code);
				print_code(m_name_header_of_dynamic_code.c_str(), m_header_of_dynamic_code.c_str());
			}
			print_code("saxpy.comp", saxpy.c_str());
		}

		unsigned long long hash;
		size_t size = (size_t)(-1);

		/// Try finding an existing spv in disk cache
		{
			hash = s_get_hash(saxpy.c_str());
			char key[64];
			sprintf(key, "%016llx", hash);
			unqlite *pDb;
			if (UNQLITE_OK == unqlite_open(&pDb, s_name_db, UNQLITE_OPEN_CREATE))
			{
				unqlite_int64 nBytes = sizeof(size_t);
				unqlite_kv_fetch(pDb, key, -1, &size, &nBytes);
				unqlite_close(pDb);
			}
		}

		if (size == (size_t)(-1))
		{
			std::vector<unsigned> spv;
			if (!GLSL2SPV_Compute(saxpy.c_str(), &m_header_map, spv))
			{
				if (!m_verbose)
				{
					{
						std::shared_lock<std::shared_mutex> lock(m_mutex_dynamic_code);
						print_code(m_name_header_of_dynamic_code.c_str(), m_header_of_dynamic_code.c_str());
					}
					print_code("saxpy.comp", saxpy.c_str());
				}
				return size;
			}
			spirv_cross::Compiler comp(std::move(spv));
			ShaderResources res = comp.get_shader_resources();
			auto ssbo = res.storage_buffers[0];
			auto type = comp.get_type(ssbo.base_type_id);
			size = comp.type_struct_member_array_stride(type, 0);

			{
				char key[64];
				sprintf(key, "%016llx", hash);
				unqlite *pDb;
				if (UNQLITE_OK == unqlite_open(&pDb, s_name_db, UNQLITE_OPEN_CREATE))
				{
					unqlite_kv_store(pDb, key, -1, &size, sizeof(size_t));
					unqlite_close(pDb);
				}
			}
		}

		// cache the result
		m_size_of_types[cls] = size;
		return size;
	}

	bool Context::query_struct(const char* name_struct, size_t* member_offsets)
	{
		// try to find in the context cache first
		std::unique_lock<std::mutex> lock(m_mutex_offsets);
		{
			decltype(m_offsets_of_structs)::iterator it = m_offsets_of_structs.find(name_struct);
			if (it != m_offsets_of_structs.end())
			{
				memcpy(member_offsets, it->second.data(), sizeof(size_t)*it->second.size());
				return true;
			}
		}

		// reflect from device code
		std::string saxpy =
			"#version 460\n"
			"#extension GL_GOOGLE_include_directive : enable\n"
			"#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable\n"
			"#extension GL_EXT_buffer_reference2 : enable\n"
			"#extension GL_EXT_nonuniform_qualifier : enable\n"
			"#extension GL_EXT_scalar_block_layout : enable\n";

		saxpy += "layout(binding = 1) uniform sampler2D arr_tex2d[];\n";
		saxpy += "layout(binding = 2) uniform sampler3D arr_tex3d[];\n";

		for (size_t i = 0; i < m_code_blocks.size(); i++)
		{
			saxpy += m_code_blocks[i];
		}
		saxpy += std::string("#include \"") + m_name_header_of_dynamic_code + "\"\n";
		saxpy +=
			"layout(scalar, binding = 0) uniform Params\n"
			"{\n    ";
		saxpy += name_struct;
		saxpy +=
			" x;\n"
			"};\n"
			"void main(){}\n";

		if (m_verbose)
		{
			{
				std::shared_lock<std::shared_mutex> lock(m_mutex_dynamic_code);
				print_code(m_name_header_of_dynamic_code.c_str(), m_header_of_dynamic_code.c_str());
			}
			print_code("saxpy.comp", saxpy.c_str());
		}

		unsigned long long hash;
		size_t num_members = (size_t)(-1);

		/// Try finding an existing spv in disk cache
		{
			hash = s_get_hash(saxpy.c_str());
			char key[64];
			sprintf(key, "%016llx", hash);
			unqlite *pDb;
			if (UNQLITE_OK == unqlite_open(&pDb, s_name_db, UNQLITE_OPEN_CREATE))
			{
				unqlite_int64 nBytes;
				if (UNQLITE_OK == unqlite_kv_fetch(pDb, key, -1, nullptr, &nBytes))
				{
					num_members = nBytes / sizeof(size_t) - 1;
					unqlite_kv_fetch(pDb, key, -1, member_offsets, &nBytes);
				}
				unqlite_close(pDb);
			}
		}

		if (num_members == (size_t)(-1))
		{
			size_t size_struct = size_of(name_struct);
			if (size_struct == (size_t)(-1)) return false;
			std::vector<unsigned> spv;
			if (!GLSL2SPV_Compute(saxpy.c_str(), &m_header_map, spv))
			{
				if (!m_verbose)
				{
					{
						std::shared_lock<std::shared_mutex> lock(m_mutex_dynamic_code);
						print_code(m_name_header_of_dynamic_code.c_str(), m_header_of_dynamic_code.c_str());
					}
					print_code("saxpy.comp", saxpy.c_str());
				}
				return false;
			}
			spirv_cross::Compiler comp(std::move(spv));
			ShaderResources res = comp.get_shader_resources();
			auto ubo = res.uniform_buffers[0];
			auto type = comp.get_type(ubo.base_type_id);
			auto member_type = comp.get_type(type.member_types[0]);
			num_members = member_type.member_types.size();
			for (size_t i = 0; i < num_members; i++)
				member_offsets[i] = comp.type_struct_member_offset(member_type, (unsigned)i);
			member_offsets[num_members] = size_struct;
			{
				char key[64];
				sprintf(key, "%016llx", hash);
				unqlite *pDb;
				if (UNQLITE_OK == unqlite_open(&pDb, s_name_db, UNQLITE_OPEN_CREATE))
				{
					unqlite_kv_store(pDb, key, -1, member_offsets, sizeof(size_t)*(num_members + 1));
					unqlite_close(pDb);
				}
			}
		}

		// cache the result
		m_offsets_of_structs[name_struct].resize(num_members + 1);
		memcpy(m_offsets_of_structs[name_struct].data(), member_offsets, sizeof(size_t)*(num_members + 1));
		return true;
	}

	void Context::add_built_in_header(const char* name, const char* content)
	{
		m_header_map[name] = content;
	}

	void Context::add_code_block(const char* code)
	{
		m_code_blocks.push_back(code);
	}

	void Context::add_inlcude_filename(const char* fn)
	{
		char line[1024];
		sprintf(line, "#include \"%s\"\n", fn);
		add_code_block(line);
	}

	static void replace_str(std::string & str, const char* from, const char* to)
	{
		size_t len_from = strlen(from);
		size_t len_to = strlen(to);
		size_t pos = 0;
		while (true)
		{
			pos = str.find(from, pos);
			if (pos >= std::string::npos) break;
			str.replace(pos, len_from, to);
			pos += len_to;
		}
	}

	std::string Context::add_dynamic_code(const char* code)
	{
		unsigned long long hash = s_get_hash(code);
		char str_hash[32];
		sprintf(str_hash, "%016llx", hash);

		std::unique_lock<std::shared_mutex> lock(m_mutex_dynamic_code);

		auto it = m_known_code.find(hash);
		if (it != m_known_code.end())
			return str_hash;

		std::string str_code = code;
		replace_str(str_code, "#hash#", str_hash);

		m_header_of_dynamic_code += str_code.data();
		m_header_map[m_name_header_of_dynamic_code] = m_header_of_dynamic_code.c_str();
		m_known_code.insert(hash);

		return str_hash;
	}

	class Signature
	{
	public:
		Signature() {}
		~Signature() {}

		void push_feature(const void* feature, size_t size)
		{
			size_t offset = m_data.size();
			m_data.resize(offset + size);
			memcpy(m_data.data() + offset, feature, size);
		}

		unsigned long long get_hash()
		{
			return (unsigned long long)crc64(0, m_data.data(), m_data.size());
		}

	private:
		std::vector<unsigned char> m_data;

	};

	unsigned Context::_build_compute_pipeline(dim_type blockDim, const std::vector<CapturedShaderViewable>& arg_map, size_t num_tex2d, size_t num_tex3d, const char* code_body)
	{
		Signature sig;
		sig.push_feature(&num_tex2d, sizeof(size_t));
		sig.push_feature(&num_tex3d, sizeof(size_t));

		std::string saxpy =
			"#version 460\n"
			"#extension GL_GOOGLE_include_directive : enable\n"
			"#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable\n"
			"#extension GL_EXT_buffer_reference2 : enable\n"
			"#extension GL_EXT_nonuniform_qualifier : enable\n"
			"#extension GL_EXT_scalar_block_layout : enable\n";		

		saxpy += "layout(binding = 1) uniform sampler2D arr_tex2d[];\n";
		saxpy += "layout(binding = 2) uniform sampler3D arr_tex3d[];\n";

		for (size_t i = 0; i < m_code_blocks.size(); i++)
		{
			saxpy += m_code_blocks[i];
		}

		saxpy += std::string("#include \"") + m_name_header_of_dynamic_code + "\"\n";

		char line[1024];
	
		if (arg_map.size() > 0)
		{
			saxpy +=
				"layout(scalar, binding = 0) uniform Params\n"
				"{\n";
			for (size_t i = 0; i < arg_map.size(); i++)
			{
				sprintf(line, "    %s %s;\n", arg_map[i].obj->name_view_type().c_str(), arg_map[i].obj_name);
				saxpy += line;
			}
			saxpy += "};\n";
		}

		sprintf(line, "layout(local_size_x = %d, local_size_y = %d, local_size_z = %d) in;\n", blockDim.x, blockDim.y, blockDim.z);
		saxpy += line;
		saxpy += std::string(code_body);

		if (m_verbose)
		{
			{
				std::shared_lock<std::shared_mutex> lock(m_mutex_dynamic_code);
				print_code(m_name_header_of_dynamic_code.c_str(), m_header_of_dynamic_code.c_str());
			}
			print_code("saxpy.comp", saxpy.c_str());
		}	

		unsigned long long hash_shader = s_get_hash(saxpy.c_str());
		sig.push_feature(&hash_shader, sizeof(unsigned long long));
		unsigned long long hash_pipeline = sig.get_hash();
		unsigned kid = (unsigned)(-1);

		std::unique_lock<std::shared_mutex> lock(m_mutex_compute_pipelines);
			
		auto it = m_map_compute_pipelines.find(hash_pipeline);
		if (it != m_map_compute_pipelines.end())
		{
			kid = it->second;
			return kid;
		}

		std::vector<unsigned int> spv;
		/// Try finding an existing spv in cache
		{
			char key[64];
			sprintf(key, "%016llx", hash_shader);
			unqlite *pDb;
			if (UNQLITE_OK == unqlite_open(&pDb, s_name_db, UNQLITE_OPEN_CREATE))
			{
				unqlite_int64 nBytes;
				if (UNQLITE_OK == unqlite_kv_fetch(pDb, key, -1, NULL, &nBytes))
				{
					size_t spv_size = nBytes / sizeof(unsigned int);
					spv.resize(spv_size);
					unqlite_kv_fetch(pDb, key, -1, spv.data(), &nBytes);
				}
				unqlite_close(pDb);
			}
		}

		if (spv.size() < 1)
		{
			if (!GLSL2SPV_Compute(saxpy.c_str(), &m_header_map, spv))
			{
				if (!m_verbose)
				{
					{
						std::shared_lock<std::shared_mutex> lock(m_mutex_dynamic_code);
						print_code(m_name_header_of_dynamic_code.c_str(), m_header_of_dynamic_code.c_str());
					}
					print_code("saxpy.comp", saxpy.c_str());
				}
				return kid;
			}
			{
				char key[64];
				sprintf(key, "%016llx", hash_shader);
				unqlite *pDb;
				if (UNQLITE_OK == unqlite_open(&pDb, s_name_db, UNQLITE_OPEN_CREATE))
				{
					unqlite_kv_store(pDb, key, -1, spv.data(), spv.size() * sizeof(unsigned int));
					unqlite_close(pDb);
				}
			}
		}

		Internal::ComputePipeline* pipeline = new Internal::ComputePipeline(spv, num_tex2d, num_tex3d);
		m_cache_compute_pipelines.push_back(pipeline);
		kid = (unsigned)m_cache_compute_pipelines.size() - 1;
		m_map_compute_pipelines[hash_pipeline] = kid;

		return kid;
	}

	bool Context::launch_compute(dim_type gridDim, size_t num_params, const ShaderViewable** args, Texture2D* const* tex2ds, Texture3D* const* tex3ds, unsigned kid, const size_t* offsets, size_t times_submission)
	{
		Internal::ComputePipeline* pipeline;
		{
			std::shared_lock<std::shared_mutex> lock(m_mutex_compute_pipelines);
			pipeline = m_cache_compute_pipelines[kid];
		}

		ViewBuf h_uniform(offsets[num_params]);
		for (size_t i = 0; i < num_params; i++)
		{
			ViewBuf vb = args[i]->view();
			memcpy(h_uniform.data() + offsets[i], vb.data(), vb.size());
		}

		Internal::CommandBufferRecycler* recycler = pipeline->recycler();
		Internal::ComputeCommandBuffer* cmdBuf = (Internal::ComputeCommandBuffer*)recycler->RetriveCommandBuffer();
		if (cmdBuf == nullptr)
		{
			cmdBuf = new Internal::ComputeCommandBuffer(pipeline, offsets[num_params]);
		}

		for (size_t i = 0; i < num_params; i++)
		{
			args[i]->apply_barriers(*cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		}

		std::vector<Internal::Texture2D*> i_tex2ds(pipeline->num_tex2d());
		for (size_t i = 0; i < pipeline->num_tex2d(); i++)
			i_tex2ds[i] = tex2ds[i]->internal();

		std::vector<Internal::Texture3D*> i_tex3ds(pipeline->num_tex3d());
		for (size_t i = 0; i < pipeline->num_tex3d(); i++)
			i_tex3ds[i] = tex3ds[i]->internal();

		cmdBuf->dispatch(h_uniform.data(), i_tex2ds.data(), i_tex3ds.data(), gridDim.x, gridDim.y, gridDim.z);

		const Internal::Context* ctx = Internal::Context::get_context();
		ctx->SubmitCommandBuffer(cmdBuf, times_submission);

		return true;
	}

	bool Context::launch_compute(dim_type gridDim, dim_type blockDim, const std::vector<CapturedShaderViewable>& arg_map, const std::vector<Texture2D*>& tex2ds, const std::vector<Texture3D*>& tex3ds, const char* code_body, size_t times_submission)
	{
		unsigned kid = _build_compute_pipeline(blockDim, arg_map, tex2ds.size(), tex3ds.size(), code_body);
		if (kid == (unsigned)(-1)) return false;
		
		// query uniform
		std::vector<size_t> offsets(arg_map.size() + 1);

		if (arg_map.size() < 1)
		{
			offsets[0] = 0;
		}
		else
		{
			std::string structure =
				"struct Uni_#hash#\n"
				"{\n";

			char line[1024];
			for (size_t i = 0; i < arg_map.size(); i++)
			{
				sprintf(line, "    %s %s;\n", arg_map[i].obj->name_view_type().c_str(), arg_map[i].obj_name);
				structure += line;
			}
			structure += "};\n";
			std::string name = std::string("Uni_") + add_dynamic_code(structure.c_str());
			query_struct(name.c_str(), offsets.data());
		}
		
		std::vector<const ShaderViewable*> args(arg_map.size());
		for (size_t i = 0; i < arg_map.size(); i++)
		{
			args[i] = arg_map[i].obj;
		}

		return launch_compute(gridDim, arg_map.size(), args.data(), tex2ds.data(), tex3ds.data(), kid, offsets.data(), times_submission);
	}

	bool Context::launch_compute(dim_type gridDim, dim_type blockDim, const std::vector<CapturedShaderViewable>& arg_map, const std::vector<Texture2D*>& tex2ds, const std::vector<Texture3D*>& tex3ds, const char* code_body, unsigned& kid, size_t* offsets, size_t times_submission)
	{
		kid = _build_compute_pipeline(blockDim, arg_map, tex2ds.size(), tex3ds.size(), code_body);
		if (kid == (unsigned)(-1)) return false;

		// query uniform
		if (arg_map.size() < 1)
		{
			offsets[0] = 0;
		}
		else
		{
			std::string structure =
				"struct Uni_#hash#\n"
				"{\n";

			char line[1024];
			for (size_t i = 0; i < arg_map.size(); i++)
			{
				sprintf(line, "    %s %s;\n", arg_map[i].obj->name_view_type().c_str(), arg_map[i].obj_name);
				structure += line;
			}
			structure += "};\n";
			std::string name = std::string("Uni_") + add_dynamic_code(structure.c_str());
			query_struct(name.c_str(), offsets);
		}

		std::vector<const ShaderViewable*> args(arg_map.size());
		for (size_t i = 0; i < arg_map.size(); i++)
		{
			args[i] = arg_map[i].obj;
		}

		return launch_compute(gridDim, arg_map.size(), args.data(), tex2ds.data(), tex3ds.data(), kid, offsets, times_submission);
	}

	unsigned Context::_build_render_pass(
		const std::vector <Internal::AttachmentInfo>& color_attachmentInfo, const Internal::AttachmentInfo* depth_attachmentInfo, const std::vector <Internal::AttachmentInfo>& resolve_attachmentInfo,
		const std::vector<CapturedShaderViewable>& arg_map, size_t num_tex2d, size_t num_tex3d, const std::vector<const DrawCall*>& draw_calls)
	{
		Signature sig;
		sig.push_feature(&num_tex2d, sizeof(size_t));
		sig.push_feature(&num_tex3d, sizeof(size_t));

		for (size_t i = 0; i < color_attachmentInfo.size(); i++)
			sig.push_feature(&color_attachmentInfo[i], sizeof(Internal::AttachmentInfo));
		if (depth_attachmentInfo!=nullptr)
			sig.push_feature(depth_attachmentInfo, sizeof(Internal::AttachmentInfo));
		for (size_t i = 0; i< resolve_attachmentInfo.size(); i++)
			sig.push_feature(&resolve_attachmentInfo[i], sizeof(Internal::AttachmentInfo));

		if (m_verbose)
		{
			std::shared_lock<std::shared_mutex> lock(m_mutex_dynamic_code);
			print_code(m_name_header_of_dynamic_code.c_str(), m_header_of_dynamic_code.c_str());
		}

		std::string saxpy =
			"#version 460\n"
			"#extension GL_GOOGLE_include_directive : enable\n"
			"#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable\n"
			"#extension GL_EXT_buffer_reference2 : enable\n"
			"#extension GL_EXT_nonuniform_qualifier : enable\n"
			"#extension GL_EXT_scalar_block_layout : enable\n";

		saxpy += "layout(binding = 1) uniform sampler2D arr_tex2d[];\n";
		saxpy += "layout(binding = 2) uniform sampler3D arr_tex3d[];\n";

		for (size_t i = 0; i < m_code_blocks.size(); i++)
		{
			saxpy += m_code_blocks[i];
		}
		
		saxpy += std::string("#include \"") + m_name_header_of_dynamic_code + "\"\n";
		char line[1024];

		if (arg_map.size() > 0)
		{
			saxpy +=
				"layout(scalar, binding = 0) uniform Params\n"
				"{\n";
			for (size_t i = 0; i < arg_map.size(); i++)
			{
				sprintf(line, "    %s %s;\n", arg_map[i].obj->name_view_type().c_str(), arg_map[i].obj_name);
				saxpy += line;
			}
			saxpy += "};\n";
		}

		std::vector<std::string> code_vert(draw_calls.size());
		std::vector<std::string> code_frag(draw_calls.size());
		std::vector<unsigned long long> hash_vert(draw_calls.size());
		std::vector<unsigned long long> hash_frag(draw_calls.size());

		for (size_t i = 0; i < draw_calls.size(); i++)
		{
			code_vert[i] = saxpy + draw_calls[i]->code_body_vert();
			code_frag[i] = saxpy + draw_calls[i]->code_body_frag();

			if (m_verbose)
			{
				char name[128];
				sprintf(name, "vert_%d.vert", (int)i);
				print_code(name, code_vert[i].c_str());
				sprintf(name, "frag_%d.frag", (int)i);
				print_code(name, code_frag[i].c_str());
			}

			hash_vert[i] = s_get_hash(code_vert[i].c_str());
			hash_frag[i] = s_get_hash(code_frag[i].c_str());

			sig.push_feature(&hash_vert[i], sizeof(unsigned long long));
			sig.push_feature(&hash_frag[i], sizeof(unsigned long long));
			const Internal::GraphicsPipelineStates& states = draw_calls[i]->get_states((int)color_attachmentInfo.size());
			sig.push_feature(&states.inputAssembly.flags, sizeof(VkPipelineInputAssemblyStateCreateInfo)-((char*)&states.inputAssembly.flags- (char*)&states.inputAssembly));
			sig.push_feature(&states.rasterizer.flags, sizeof(VkPipelineRasterizationStateCreateInfo) - ((char*)&states.rasterizer.flags - (char*)&states.rasterizer));
			for (size_t j = 0; j < color_attachmentInfo.size(); j++)
				sig.push_feature(&states.colorBlendAttachments[j], sizeof(VkPipelineColorBlendAttachmentState));			
			sig.push_feature(&states.colorBlending.flags, ((char*)&states.colorBlending.attachmentCount - (char*)&states.colorBlending.flags));
			sig.push_feature(states.colorBlending.blendConstants, sizeof(float) * 4);
			sig.push_feature(&states.depthStencil.flags, sizeof(VkPipelineDepthStencilStateCreateInfo) - ((char*)&states.depthStencil.flags - (char*)&states.depthStencil));
		}

		unsigned long long hash_render_pass = sig.get_hash();
		unsigned rpid = (unsigned)(-1);

		std::unique_lock<std::shared_mutex> lock(m_mutex_render_passes);

		auto it = m_map_render_passes.find(hash_render_pass);
		if (it != m_map_render_passes.end())
		{
			rpid = it->second;
			return rpid;
		}

		std::vector<std::vector<unsigned>> spv_vert(draw_calls.size());
		std::vector<std::vector<unsigned>> spv_frag(draw_calls.size());
		std::vector<Internal::GraphicsPipelineInfo> pipelineInfo(draw_calls.size());

		for (size_t i = 0; i < draw_calls.size(); i++)
		{
			/// Try finding an existing spv in cache
			{
				char key[64];
				sprintf(key, "%016llx", hash_vert[i]);
				unqlite *pDb;
				if (UNQLITE_OK == unqlite_open(&pDb, s_name_db, UNQLITE_OPEN_CREATE))
				{
					unqlite_int64 nBytes;
					if (UNQLITE_OK == unqlite_kv_fetch(pDb, key, -1, NULL, &nBytes))
					{
						size_t spv_size = nBytes / sizeof(unsigned int);
						spv_vert[i].resize(spv_size);
						unqlite_kv_fetch(pDb, key, -1, spv_vert[i].data(), &nBytes);
					}
					unqlite_close(pDb);
				}
			}

			if (spv_vert[i].size() < 1)
			{
				if (!GLSL2SPV_Vertex(code_vert[i].c_str(), &m_header_map, spv_vert[i]))
				{
					if (!m_verbose)
					{
						{
							std::shared_lock<std::shared_mutex> lock(m_mutex_dynamic_code);
							print_code(m_name_header_of_dynamic_code.c_str(), m_header_of_dynamic_code.c_str());
						}
						char name[128];
						sprintf(name, "vert_%d.vert", (int)i);
						print_code(name, code_vert[i].c_str());
					}
					return rpid;
				}
				{
					char key[64];
					sprintf(key, "%016llx", hash_vert[i]);
					unqlite *pDb;
					if (UNQLITE_OK == unqlite_open(&pDb, s_name_db, UNQLITE_OPEN_CREATE))
					{
						unqlite_kv_store(pDb, key, -1, spv_vert[i].data(), spv_vert[i].size() * sizeof(unsigned int));
						unqlite_close(pDb);
					}
				}
			}


			/// Try finding an existing spv in cache
			{
				char key[64];
				sprintf(key, "%016llx", hash_frag[i]);
				unqlite *pDb;
				if (UNQLITE_OK == unqlite_open(&pDb, s_name_db, UNQLITE_OPEN_CREATE))
				{
					unqlite_int64 nBytes;
					if (UNQLITE_OK == unqlite_kv_fetch(pDb, key, -1, NULL, &nBytes))
					{
						size_t spv_size = nBytes / sizeof(unsigned int);
						spv_frag[i].resize(spv_size);
						unqlite_kv_fetch(pDb, key, -1, spv_frag[i].data(), &nBytes);
					}
					unqlite_close(pDb);
				}
			}

			if (spv_frag[i].size() < 1)
			{
				if (!GLSL2SPV_Fragment(code_frag[i].c_str(), &m_header_map, spv_frag[i]))
				{
					if (!m_verbose)
					{
						{
							std::shared_lock<std::shared_mutex> lock(m_mutex_dynamic_code);
							print_code(m_name_header_of_dynamic_code.c_str(), m_header_of_dynamic_code.c_str());
						}
						char name[128];
						sprintf(name, "frag_%d.frag", (int)i);
						print_code(name, code_frag[i].c_str());
					}
					return rpid;
				}
				{
					char key[64];
					sprintf(key, "%016llx", hash_frag[i]);
					unqlite *pDb;
					if (UNQLITE_OK == unqlite_open(&pDb, s_name_db, UNQLITE_OPEN_CREATE))
					{
						unqlite_kv_store(pDb, key, -1, spv_frag[i].data(), spv_frag[i].size() * sizeof(unsigned int));
						unqlite_close(pDb);
					}
				}
			}
			pipelineInfo[i].spv_vert = &spv_vert[i];
			pipelineInfo[i].spv_frag = &spv_frag[i];
			pipelineInfo[i].states = draw_calls[i]->get_states((int)color_attachmentInfo.size());
			pipelineInfo[i].states.colorBlending.attachmentCount = (unsigned)color_attachmentInfo.size();
		}

		Internal::RenderPass* renderpass = new Internal::RenderPass(color_attachmentInfo, depth_attachmentInfo, resolve_attachmentInfo, pipelineInfo, num_tex2d, num_tex3d);
		m_cache_render_passes.push_back(renderpass);
		rpid = (unsigned)m_cache_render_passes.size() - 1;
		m_map_render_passes[hash_render_pass] = rpid;

		return rpid;
	}

	bool Context::launch_rasterization(Texture2D* const* colorBufs, Texture2D* depthBuf, Texture2D* const* resolveBufs, float* clear_colors, float clear_depth,
		size_t num_params, const ShaderViewable** args, Texture2D* const* tex2ds, Texture3D* const* tex3ds, Rasterizer::LaunchParam** launch_params, unsigned rpid, const size_t* offsets, size_t times_submission)
	{
		Internal::RenderPass* renderpass;
		{
			std::shared_lock<std::shared_mutex> lock(m_mutex_render_passes);
			renderpass = m_cache_render_passes[rpid];
		}

		Internal::CommandBufferRecycler* recycler = renderpass->recycler();
		Internal::RenderPassCommandBuffer* cmdBuf = (Internal::RenderPassCommandBuffer*)recycler->RetriveCommandBuffer();
		if (cmdBuf == nullptr)
		{
			cmdBuf = new Internal::RenderPassCommandBuffer(renderpass, offsets[num_params]);
		}

		std::vector <Internal::Texture2D*> tex_colorBufs(renderpass->num_color_attachments());
		for (size_t i = 0; i < renderpass->num_color_attachments(); i++)
			tex_colorBufs[i] = colorBufs[i]->internal();

		Internal::Texture2D* tex_depthBuf = nullptr;
		if (renderpass->has_depth_attachment())
			tex_depthBuf = depthBuf->internal();

		std::vector <Internal::Texture2D*> tex_resolveBufs(renderpass->num_resolve_attachments());
		for (size_t i = 0; i < renderpass->num_resolve_attachments(); i++)
			tex_resolveBufs[i] = resolveBufs[i]->internal();

		ViewBuf h_uniform(offsets[num_params]);
		for (size_t i = 0; i < num_params; i++)
		{
			ViewBuf vb = args[i]->view();
			memcpy(h_uniform.data() + offsets[i], vb.data(), vb.size());
		}

		for (size_t i = 0; i < num_params; i++)
		{
			args[i]->apply_barriers(*cmdBuf, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
		}

		std::vector<Internal::Texture2D*> i_tex2ds(renderpass->num_tex2d());
		for (size_t i = 0; i < i_tex2ds.size(); i++)
			i_tex2ds[i] = tex2ds[i]->internal();

		std::vector<Internal::Texture3D*> i_tex3ds(renderpass->num_tex3d());
		for (size_t i = 0; i < i_tex3ds.size(); i++)
			i_tex3ds[i] = tex3ds[i]->internal();
		
		std::vector<Internal::RenderPassCommandBuffer::DrawParam> dps(renderpass->num_pipelines()); 
		for (size_t i = 0; i < renderpass->num_pipelines(); i++)
		{
			dps[i].count = launch_params[i]->count;
			dps[i].indType = VK_INDEX_TYPE_UINT16;
			dps[i].indBuf = nullptr;  
			if (launch_params[i]->indBuf != nullptr)
			{
				if (launch_params[i]->indBuf->elem_size()>2)
					dps[i].indType = VK_INDEX_TYPE_UINT32;
				dps[i].indBuf = launch_params[i]->indBuf->internal();
			}
		}
			 
		cmdBuf->draw(tex_colorBufs.data(), tex_depthBuf, tex_resolveBufs.data(), clear_colors, clear_depth, h_uniform.data(), i_tex2ds.data(), i_tex3ds.data(), dps.data());

		const Internal::Context* ctx = Internal::Context::get_context();
		ctx->SubmitCommandBuffer(cmdBuf, times_submission);

		return true;
	}

	bool Context::launch_rasterization(const std::vector<Attachement>& colorBufs, Attachement depthBuf, const std::vector<Attachement>& resolveBufs, float* clear_colors, float clear_depth,
		const std::vector<CapturedShaderViewable>& arg_map, const std::vector<Texture2D*>& tex2ds, const std::vector<Texture3D*>& tex3ds, const std::vector<const DrawCall*>& draw_calls, Rasterizer::LaunchParam** launch_params, size_t times_submission)
	{
		std::vector <Internal::AttachmentInfo> color_attachmentInfo(colorBufs.size());
		std::vector <Texture2D*> tex_colorBufs(colorBufs.size());
		for (size_t i = 0; i < colorBufs.size(); i++)
		{
			color_attachmentInfo[i].format = (VkFormat)colorBufs[i].tex->vkformat();
			color_attachmentInfo[i].sample_count = (VkSampleCountFlagBits)colorBufs[i].tex->sample_count();
			color_attachmentInfo[i].clear_at_load = colorBufs[i].clear_at_load;			
			tex_colorBufs[i] = colorBufs[i].tex;
		}

		Internal::AttachmentInfo depth_attachmentInfo;
		Internal::AttachmentInfo* p_depth_attachmentInfo = nullptr;
		Texture2D* tex_depthBuf = nullptr;
		if (depthBuf.tex != nullptr)
		{
			depth_attachmentInfo.format = (VkFormat)depthBuf.tex->vkformat();
			depth_attachmentInfo.sample_count = (VkSampleCountFlagBits)depthBuf.tex->sample_count();
			depth_attachmentInfo.clear_at_load = depthBuf.clear_at_load;
			p_depth_attachmentInfo = &depth_attachmentInfo;
			tex_depthBuf = depthBuf.tex;
		}		

		std::vector <Internal::AttachmentInfo> resolve_attachmentInfo(resolveBufs.size());
		std::vector <Texture2D*> tex_resolveBufs(resolveBufs.size());
		for (size_t i = 0; i < resolveBufs.size(); i++)
		{
			resolve_attachmentInfo[i].format = (VkFormat)resolveBufs[i].tex->vkformat();
			resolve_attachmentInfo[i].sample_count = (VkSampleCountFlagBits)resolveBufs[i].tex->sample_count();
			resolve_attachmentInfo[i].clear_at_load = resolveBufs[i].clear_at_load;
			tex_resolveBufs[i] = resolveBufs[i].tex;
		}

		unsigned rpid = _build_render_pass(color_attachmentInfo, p_depth_attachmentInfo, resolve_attachmentInfo, arg_map, tex2ds.size(), tex3ds.size(), draw_calls);
		if (rpid == (unsigned)(-1)) return false;
	
		// query uniform
		std::vector<size_t> offsets(arg_map.size() + 1);
		if (arg_map.size() < 1)
		{
			offsets[0] = 0;
		}
		else
		{
			std::string structure =
				"struct Uni_#hash#\n"
				"{\n";

			char line[1024];
			for (size_t i = 0; i < arg_map.size(); i++)
			{
				sprintf(line, "    %s %s;\n", arg_map[i].obj->name_view_type().c_str(), arg_map[i].obj_name);
				structure += line;
			}
			structure += "};\n";
			std::string name = std::string("Uni_") + add_dynamic_code(structure.c_str());
			query_struct(name.c_str(), offsets.data());
		}

		std::vector<const ShaderViewable*> args(arg_map.size());
		for (size_t i = 0; i < arg_map.size(); i++)
		{
			args[i] = arg_map[i].obj;
		}

		return launch_rasterization(tex_colorBufs.data(), tex_depthBuf, tex_resolveBufs.data(), clear_colors, clear_depth, arg_map.size(), args.data(), tex2ds.data(), tex3ds.data(), launch_params, rpid, offsets.data(), times_submission);
	}

	bool Context::launch_rasterization(const std::vector<Attachement>& colorBufs, Attachement depthBuf, const std::vector<Attachement>& resolveBufs, float* clear_colors, float clear_depth,
		const std::vector<CapturedShaderViewable>& arg_map, const std::vector<Texture2D*>& tex2ds, const std::vector<Texture3D*>& tex3ds, const std::vector<const DrawCall*>& draw_calls, Rasterizer::LaunchParam** launch_params, unsigned& rpid, size_t* offsets, size_t times_submission)
	{
		std::vector <Internal::AttachmentInfo> color_attachmentInfo(colorBufs.size());
		std::vector <Texture2D*> tex_colorBufs(colorBufs.size());
		for (size_t i = 0; i < colorBufs.size(); i++)
		{
			color_attachmentInfo[i].format = (VkFormat)colorBufs[i].tex->vkformat();
			color_attachmentInfo[i].sample_count = (VkSampleCountFlagBits)colorBufs[i].tex->sample_count();
			color_attachmentInfo[i].clear_at_load = colorBufs[i].clear_at_load;
			tex_colorBufs[i] = colorBufs[i].tex;
		}

		Internal::AttachmentInfo depth_attachmentInfo;
		Internal::AttachmentInfo* p_depth_attachmentInfo = nullptr;
		Texture2D* tex_depthBuf = nullptr;
		if (depthBuf.tex != nullptr)
		{
			depth_attachmentInfo.format = (VkFormat)depthBuf.tex->vkformat();
			depth_attachmentInfo.sample_count = (VkSampleCountFlagBits)depthBuf.tex->sample_count();
			depth_attachmentInfo.clear_at_load = depthBuf.clear_at_load;
			p_depth_attachmentInfo = &depth_attachmentInfo;
			tex_depthBuf = depthBuf.tex;
		}

		std::vector <Internal::AttachmentInfo> resolve_attachmentInfo(resolveBufs.size());
		std::vector <Texture2D*> tex_resolveBufs(resolveBufs.size());
		for (size_t i = 0; i < resolveBufs.size(); i++)
		{
			resolve_attachmentInfo[i].format = (VkFormat)resolveBufs[i].tex->vkformat();
			resolve_attachmentInfo[i].sample_count = (VkSampleCountFlagBits)resolveBufs[i].tex->sample_count();
			resolve_attachmentInfo[i].clear_at_load = resolveBufs[i].clear_at_load;
			tex_resolveBufs[i] = resolveBufs[i].tex;
		}

		rpid = _build_render_pass(color_attachmentInfo, p_depth_attachmentInfo, resolve_attachmentInfo, arg_map, tex2ds.size(), tex3ds.size(), draw_calls);
		if (rpid == (unsigned)(-1)) return false;

		// query uniform
		if (arg_map.size() < 1)
		{
			offsets[0] = 0;
		}
		else
		{
			std::string structure =
				"struct Uni_#hash#\n"
				"{\n";

			char line[1024];
			for (size_t i = 0; i < arg_map.size(); i++)
			{
				sprintf(line, "    %s %s;\n", arg_map[i].obj->name_view_type().c_str(), arg_map[i].obj_name);
				structure += line;
			}
			structure += "};\n";
			std::string name = std::string("Uni_") + add_dynamic_code(structure.c_str());
			query_struct(name.c_str(), offsets);
		}

		std::vector<const ShaderViewable*> args(arg_map.size());
		for (size_t i = 0; i < arg_map.size(); i++)
		{
			args[i] = arg_map[i].obj;
		}

		return launch_rasterization(tex_colorBufs.data(), tex_depthBuf, tex_resolveBufs.data(), clear_colors, clear_depth, arg_map.size(), args.data(), tex2ds.data(), tex3ds.data(), launch_params, rpid, offsets, times_submission);
	}
}

