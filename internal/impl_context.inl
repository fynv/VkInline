#include "spirv_cross.hpp"
using namespace SPIRV_CROSS_NAMESPACE;
#include <unqlite.h>
#include "crc64.h"

bool GLSL2SPV_Compute(const char* InputCString, const std::unordered_map<std::string, const char*>* headers, std::vector<unsigned int>& SpirV);

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

		for (size_t i = 0; i < m_cache_compute_pipelines.size(); i++)
			delete m_cache_compute_pipelines[i];
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
			"#extension GL_EXT_scalar_block_layout : enable\n";

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

		/// Try finding an existing ptx in disk cache
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
			"#extension GL_EXT_scalar_block_layout : enable\n";

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

		/// Try finding an existing ptx in disk cache
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

/*	class Signature
	{
	public:
		Signature() {}
		~Signature() {}

		void push_feature(void* feature, size_t size)
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
*/

	unsigned Context::_build_compute_pipeline(dim_type blockDim, const std::vector<CapturedShaderViewable>& arg_map, size_t num_tex2d, const char* code_body)
	{
		std::string saxpy =
			"#version 460\n"
			"#extension GL_GOOGLE_include_directive : enable\n"
			"#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable\n"
			"#extension GL_EXT_buffer_reference2 : enable\n"
			"#extension GL_EXT_nonuniform_qualifier : enable\n"
			"#extension GL_EXT_scalar_block_layout : enable\n";

		for (size_t i = 0; i < m_code_blocks.size(); i++)
		{
			saxpy += m_code_blocks[i];
		}
		saxpy += std::string("#include \"") + m_name_header_of_dynamic_code + "\"\n";
		saxpy +=
			"layout(scalar, binding = 0) uniform Params\n"
			"{\n";
		char line[1024];
		for (size_t i = 0; i < arg_map.size(); i++)
		{
			sprintf(line, "    %s %s;\n", arg_map[i].obj->name_view_type().c_str(), arg_map[i].obj_name);
			saxpy += line;
		}
		saxpy += "};\n";

		if (num_tex2d > 0)
		{
			sprintf(line, "layout(binding = 1) uniform sampler2D[%d] arr_tex2d;\n", (int)num_tex2d);
			saxpy += line;
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

		unsigned long long hash = s_get_hash(saxpy.c_str());
		unsigned kid = (unsigned)(-1);

		std::unique_lock<std::shared_mutex> lock(m_mutex_compute_pipelines);
			
		auto it = m_map_compute_pipelines.find(hash);
		if (it != m_map_compute_pipelines.end())
		{
			kid = it->second;
			return kid;
		}

		std::vector<unsigned int> spv;
		{
			/// Try finding an existing spv in cache
			{
				char key[64];
				sprintf(key, "%016llx", hash);
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
						sprintf(key, "%016llx", hash);
						unqlite *pDb;
						if (UNQLITE_OK == unqlite_open(&pDb, s_name_db, UNQLITE_OPEN_CREATE))
						{
							unqlite_kv_store(pDb, key, -1, spv.data(), spv.size() * sizeof(unsigned int));
							unqlite_close(pDb);
						}
					}
				}
			}
		}

		Internal::ComputePipeline* pipeline = new Internal::ComputePipeline(spv, num_tex2d);
		m_cache_compute_pipelines.push_back(pipeline);
		kid = (unsigned)m_cache_compute_pipelines.size() - 1;
		m_map_compute_pipelines[hash] = kid;

		return kid;
	}

	bool Context::launch_compute(dim_type gridDim, dim_type blockDim, const std::vector<CapturedShaderViewable>& arg_map, const std::vector<Texture2D*>& tex2ds, const char* code_body)
	{
		// query uniform
		std::vector<size_t> offsets(arg_map.size() + 1);
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

		unsigned kid = _build_compute_pipeline(blockDim, arg_map, tex2ds.size(), code_body);
		if (kid == (unsigned)(-1)) return false;
		Internal::ComputePipeline* pipeline;
		{
			std::shared_lock<std::shared_mutex> lock(m_mutex_compute_pipelines);
			pipeline = m_cache_compute_pipelines[kid];
		}

		ViewBuf h_uniform(offsets[arg_map.size()]);
		for (size_t i = 0; i < arg_map.size(); i++)
		{
			ViewBuf vb = arg_map[i].obj->view();
			memcpy(h_uniform.data() + offsets[i], vb.data(), vb.size());
		}

		Internal::CommandBufferRecycler* recycler = pipeline->recycler();
		Internal::ComputeCommandBuffer* cmdBuf = (Internal::ComputeCommandBuffer*)recycler->RetriveCommandBuffer();
		if (cmdBuf ==nullptr)
		{
			cmdBuf = new Internal::ComputeCommandBuffer(pipeline, offsets[arg_map.size()]);
		}

		for (size_t i = 0; i < arg_map.size(); i++)
		{
			arg_map[i].obj->apply_barriers(*cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		}

		std::vector<Internal::Texture2D*> i_tex2ds(tex2ds.size());
		for (size_t i = 0; i < i_tex2ds.size(); i++)
		{
			tex2ds[i]->apply_barrier_as_texture(*cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			i_tex2ds[i] = tex2ds[i]->internal();
		}
		cmdBuf->dispatch(h_uniform.data(), i_tex2ds.data(), gridDim.x, gridDim.y, gridDim.z);

		const Internal::Context* ctx = Internal::Context::get_context();
		ctx->SubmitCommandBuffer(cmdBuf);

		return true;
	}


}

