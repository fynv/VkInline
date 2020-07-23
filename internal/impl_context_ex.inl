bool GLSL2SPV_Raygen(const char* InputCString, const std::unordered_map<std::string, std::string>* headers, std::vector<unsigned int>& SpirV);
bool GLSL2SPV_Miss(const char* InputCString, const std::unordered_map<std::string, std::string>* headers, std::vector<unsigned int>& SpirV);
bool GLSL2SPV_ClosestHit(const char* InputCString, const std::unordered_map<std::string, std::string>* headers, std::vector<unsigned int>& SpirV);
bool GLSL2SPV_Intersect(const char* InputCString, const std::unordered_map<std::string, std::string>* headers, std::vector<unsigned int>& SpirV);

namespace VkInline
{
	unsigned Context::_build_raytrace_pipeline(const std::vector<CapturedShaderViewable>& arg_map, unsigned maxRecursionDepth, 
		size_t num_tlas, size_t num_tex2d, size_t num_tex3d, size_t num_cubemap,
		const char* body_raygen, const std::vector<const char*>& body_miss,	const std::vector<const BodyHitShaders*>& body_hit)
	{
		Signature sig;
		sig.push_feature(&maxRecursionDepth, sizeof(unsigned));
		sig.push_feature(&num_tlas, sizeof(size_t));
		sig.push_feature(&num_tex2d, sizeof(size_t));
		sig.push_feature(&num_tex3d, sizeof(size_t));
		sig.push_feature(&num_cubemap, sizeof(size_t));

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
			"#extension GL_EXT_scalar_block_layout : enable\n"
			"#extension GL_EXT_shader_atomic_float : enable\n"
			"#extension GL_EXT_ray_tracing : enable\n";

		saxpy += "layout(binding = 10, set = 0) uniform accelerationStructureEXT arr_tlas[];\n";
		saxpy += "layout(binding = 1) uniform sampler2D arr_tex2d[];\n";
		saxpy += "layout(binding = 2) uniform sampler3D arr_tex3d[];\n";
		saxpy += "layout(binding = 3) uniform samplerCube arr_cubemap[];\n";

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

		std::string code_raygen;
		unsigned long long hash_raygen;

		{
			code_raygen = saxpy + body_raygen;

			if (m_verbose)
			{
				{
					std::shared_lock<std::shared_mutex> lock(m_mutex_dynamic_code);
					print_code(m_name_header_of_dynamic_code.c_str(), m_header_of_dynamic_code.c_str());
				}
				print_code("raygen.rgen", code_raygen.c_str());
			}

			hash_raygen = s_get_hash(code_raygen.c_str());
			sig.push_feature(&hash_raygen, sizeof(unsigned long long));
		}

		std::vector<std::string> code_miss(body_miss.size());
		std::vector<unsigned long long> hash_miss(body_miss.size());
		for (size_t i = 0; i < body_miss.size(); i++)
		{
			code_miss[i]= saxpy + body_miss[i];
			if (m_verbose)
			{
				char name[128];
				sprintf(name, "miss_%d.rmiss", (int)i);
				print_code(name, code_miss[i].c_str());			
			}
			hash_miss[i] = s_get_hash(code_miss[i].c_str());
			sig.push_feature(&hash_miss[i], sizeof(unsigned long long));
		}

		std::vector<std::string> code_closest_hit(body_hit.size());
		std::vector<unsigned long long> hash_closest_hit(body_hit.size());
		std::vector<std::string> code_intersection(body_hit.size());
		std::vector<unsigned long long> hash_intersection(body_hit.size());

		for (size_t i = 0; i < body_hit.size(); i++)
		{
			code_closest_hit[i] = saxpy + body_hit[i]->body_closest_hit();
			const char* p_body_intersection = body_hit[i]->body_intersection();
			if (p_body_intersection != nullptr)
				code_intersection[i] = saxpy + p_body_intersection;

			if (m_verbose)
			{
				char name[128];
				sprintf(name, "closesthit_%d.rchit", (int)i);
				print_code(name, code_closest_hit[i].c_str());
				if (p_body_intersection != nullptr)
				{
					sprintf(name, "intersection_%d.rint", (int)i);
					print_code(name, code_intersection[i].c_str());
				}
			}

			hash_closest_hit[i] = s_get_hash(code_closest_hit[i].c_str());
			sig.push_feature(&hash_closest_hit[i], sizeof(unsigned long long));
			if (p_body_intersection != nullptr)
			{
				hash_intersection[i] = s_get_hash(code_intersection[i].c_str());
				sig.push_feature(&hash_intersection[i], sizeof(unsigned long long));
			}
		}

		unsigned long long hash_pipeline = sig.get_hash();
		unsigned kid = (unsigned)(-1);

		std::unique_lock<std::shared_mutex> lock(m_mutex_raytrace_pipelines);

		auto it = m_map_raytrace_pipelines.find(hash_pipeline);
		if (it != m_map_raytrace_pipelines.end())
		{
			kid = it->second;
			return kid;
		}

		std::vector<unsigned> spv_raygen;
		std::vector<std::vector<unsigned>> spv_miss(body_miss.size());
		std::vector<std::vector<unsigned>> spv_closesthit(body_hit.size());
		std::vector<std::vector<unsigned>> spv_intersection(body_hit.size());
		std::vector<const std::vector<unsigned>*> p_spv_miss(body_miss.size());
		std::vector<Internal::HitShaders> p_hit(body_hit.size());

		{
			/// Try finding an existing spv in cache
			{
				char key[64];
				sprintf(key, "%016llx", hash_raygen);
				unqlite *pDb;
				if (UNQLITE_OK == unqlite_open(&pDb, s_name_db, UNQLITE_OPEN_CREATE))
				{
					unqlite_int64 nBytes;
					if (UNQLITE_OK == unqlite_kv_fetch(pDb, key, -1, NULL, &nBytes))
					{
						size_t spv_size = nBytes / sizeof(unsigned int);
						spv_raygen.resize(spv_size);
						unqlite_kv_fetch(pDb, key, -1, spv_raygen.data(), &nBytes);
					}
					unqlite_close(pDb);
				}
			}

			if (spv_raygen.size() < 1)
			{
				if (!GLSL2SPV_Raygen(code_raygen.c_str(), &m_header_map, spv_raygen))
				{
					if (!m_verbose)
					{
						{
							std::shared_lock<std::shared_mutex> lock(m_mutex_dynamic_code);
							print_code(m_name_header_of_dynamic_code.c_str(), m_header_of_dynamic_code.c_str());
						}
						print_code("raygen.rgen", code_raygen.c_str());
					}
					return kid;
				}
				{
					char key[64];
					sprintf(key, "%016llx", hash_raygen);
					unqlite *pDb;
					if (UNQLITE_OK == unqlite_open(&pDb, s_name_db, UNQLITE_OPEN_CREATE))
					{
						unqlite_kv_store(pDb, key, -1, spv_raygen.data(), spv_raygen.size() * sizeof(unsigned int));
						unqlite_close(pDb);
					}
				}
			}
		}

		for (size_t i = 0; i < body_miss.size(); i++)
		{
			/// Try finding an existing spv in cache
			{
				char key[64];
				sprintf(key, "%016llx", hash_miss[i]);
				unqlite *pDb;
				if (UNQLITE_OK == unqlite_open(&pDb, s_name_db, UNQLITE_OPEN_CREATE))
				{
					unqlite_int64 nBytes;
					if (UNQLITE_OK == unqlite_kv_fetch(pDb, key, -1, NULL, &nBytes))
					{
						size_t spv_size = nBytes / sizeof(unsigned int);
						spv_miss[i].resize(spv_size);
						unqlite_kv_fetch(pDb, key, -1, spv_miss[i].data(), &nBytes);
					}
					unqlite_close(pDb);
				}
			}

			if (spv_miss[i].size() < 1)
			{
				if (!GLSL2SPV_Miss(code_miss[i].c_str(), &m_header_map, spv_miss[i]))
				{
					if (!m_verbose)
					{
						{
							std::shared_lock<std::shared_mutex> lock(m_mutex_dynamic_code);
							print_code(m_name_header_of_dynamic_code.c_str(), m_header_of_dynamic_code.c_str());
						}
						char name[128];
						sprintf(name, "miss_%d.rmiss", (int)i);
						print_code(name, code_miss[i].c_str());
					}
					return kid;
				}
				{
					char key[64];
					sprintf(key, "%016llx", hash_miss[i]);
					unqlite *pDb;
					if (UNQLITE_OK == unqlite_open(&pDb, s_name_db, UNQLITE_OPEN_CREATE))
					{
						unqlite_kv_store(pDb, key, -1, spv_miss[i].data(), spv_miss[i].size() * sizeof(unsigned int));
						unqlite_close(pDb);
					}
				}
			}

			p_spv_miss[i] = &spv_miss[i];
		}

		for (size_t i = 0; i < body_hit.size(); i++)
		{
			/// Try finding an existing spv in cache
			{
				char key[64];
				sprintf(key, "%016llx", hash_closest_hit[i]);
				unqlite *pDb;
				if (UNQLITE_OK == unqlite_open(&pDb, s_name_db, UNQLITE_OPEN_CREATE))
				{
					unqlite_int64 nBytes;
					if (UNQLITE_OK == unqlite_kv_fetch(pDb, key, -1, NULL, &nBytes))
					{
						size_t spv_size = nBytes / sizeof(unsigned int);
						spv_closesthit[i].resize(spv_size);
						unqlite_kv_fetch(pDb, key, -1, spv_closesthit[i].data(), &nBytes);
					}
					unqlite_close(pDb);
				}
			}

			if (spv_closesthit[i].size() < 1)
			{
				if (!GLSL2SPV_ClosestHit(code_closest_hit[i].c_str(), &m_header_map, spv_closesthit[i]))
				{
					if (!m_verbose)
					{
						{
							std::shared_lock<std::shared_mutex> lock(m_mutex_dynamic_code);
							print_code(m_name_header_of_dynamic_code.c_str(), m_header_of_dynamic_code.c_str());
						}
						char name[128];
						sprintf(name, "closesthit_%d.rchit", (int)i);
						print_code(name, code_closest_hit[i].c_str());					
					}
					return kid;
				}
				{
					char key[64];
					sprintf(key, "%016llx", hash_closest_hit[i]);
					unqlite *pDb;
					if (UNQLITE_OK == unqlite_open(&pDb, s_name_db, UNQLITE_OPEN_CREATE))
					{
						unqlite_kv_store(pDb, key, -1, spv_closesthit[i].data(), spv_closesthit[i].size() * sizeof(unsigned int));
						unqlite_close(pDb);
					}
				}
			}

			p_hit[i].closest_hit = &spv_closesthit[i];
			p_hit[i].intersection = nullptr;

			if (!code_intersection[i].empty())
			{
				/// Try finding an existing spv in cache
				{
					char key[64];
					sprintf(key, "%016llx", hash_intersection[i]);
					unqlite *pDb;
					if (UNQLITE_OK == unqlite_open(&pDb, s_name_db, UNQLITE_OPEN_CREATE))
					{
						unqlite_int64 nBytes;
						if (UNQLITE_OK == unqlite_kv_fetch(pDb, key, -1, NULL, &nBytes))
						{
							size_t spv_size = nBytes / sizeof(unsigned int);
							spv_intersection[i].resize(spv_size);
							unqlite_kv_fetch(pDb, key, -1, spv_intersection[i].data(), &nBytes);
						}
						unqlite_close(pDb);
					}
				}

				if (spv_intersection[i].size() < 1)
				{
					if (!GLSL2SPV_Intersect(code_intersection[i].c_str(), &m_header_map, spv_intersection[i]))
					{
						if (!m_verbose)
						{
							{
								std::shared_lock<std::shared_mutex> lock(m_mutex_dynamic_code);
								print_code(m_name_header_of_dynamic_code.c_str(), m_header_of_dynamic_code.c_str());
							}
							char name[128];
							sprintf(name, "intersection_%d.rint", (int)i);
							print_code(name, code_intersection[i].c_str());
						}
						return kid;
					}
					{
						char key[64];
						sprintf(key, "%016llx", hash_intersection[i]);
						unqlite *pDb;
						if (UNQLITE_OK == unqlite_open(&pDb, s_name_db, UNQLITE_OPEN_CREATE))
						{
							unqlite_kv_store(pDb, key, -1, spv_intersection[i].data(), spv_intersection[i].size() * sizeof(unsigned int));
							unqlite_close(pDb);
						}
					}
				}

				p_hit[i].intersection = &spv_intersection[i];
			}
		}

		Internal::RayTracePipeline* pipeline = new Internal::RayTracePipeline(spv_raygen, p_spv_miss, p_hit, maxRecursionDepth, num_tlas, num_tex2d, num_tex3d, num_cubemap);
		m_cache_raytrace_pipelines.push_back(pipeline);
		kid = (unsigned)m_cache_raytrace_pipelines.size() - 1;
		m_map_raytrace_pipelines[hash_pipeline] = kid;
		return kid;
	}

	bool Context::launch_raytrace(dim_type glbDim, size_t num_params, const ShaderViewable** args, 
		TopLevelAS* const* arr_tlas,  Texture2D* const * tex2ds, Texture3D* const * tex3ds, Cubemap* const* cubemaps, 
		unsigned kid, const size_t* offsets, size_t times_submission)
	{
		Internal::RayTracePipeline* pipeline;
		{
			std::shared_lock<std::shared_mutex> lock(m_mutex_raytrace_pipelines);
			pipeline = m_cache_raytrace_pipelines[kid];
		}

		Internal::CommandBufferRecycler* recycler = pipeline->recycler();
		Internal::RayTraceCommandBuffer* cmdBuf = (Internal::RayTraceCommandBuffer*)recycler->RetriveCommandBuffer();
		if (cmdBuf == nullptr)
		{
			cmdBuf = new Internal::RayTraceCommandBuffer(pipeline, offsets[num_params]);
		}

		ViewBuf h_uniform(offsets[num_params]);
		for (size_t i = 0; i < num_params; i++)
		{
			ViewBuf vb = args[i]->view();
			memcpy(h_uniform.data() + offsets[i], vb.data(), vb.size());
		}

		for (size_t i = 0; i < num_params; i++)
		{
			args[i]->apply_barriers(*cmdBuf, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
		}

		std::vector<Internal::TopLevelAS*> i_tlas(pipeline->num_tlas());
		for (size_t i = 0; i < i_tlas.size(); i++)
			i_tlas[i] = arr_tlas[i]->internal();

		std::vector<Internal::Texture2D*> i_tex2ds(pipeline->num_tex2d());
		for (size_t i = 0; i < i_tex2ds.size(); i++)
			i_tex2ds[i] = tex2ds[i]->internal();

		std::vector<Internal::Texture3D*> i_tex3ds(pipeline->num_tex3d());
		for (size_t i = 0; i < i_tex3ds.size(); i++)
			i_tex3ds[i] = tex3ds[i]->internal();

		std::vector<Internal::TextureCube*> i_cubemaps(pipeline->num_cubemap());
		for (size_t i = 0; i < i_cubemaps.size(); i++)
			i_cubemaps[i] = cubemaps[i]->internal();

		cmdBuf->trace(h_uniform.data(), i_tlas.data(), i_tex2ds.data(), i_tex3ds.data(), i_cubemaps.data(), glbDim.x, glbDim.y, glbDim.z);

		const Internal::Context* ctx = Internal::Context::get_context();
		ctx->SubmitCommandBuffer(cmdBuf, times_submission);

		return true;
	}

	bool Context::launch_raytrace(dim_type glbDim, const std::vector<CapturedShaderViewable>& arg_map, unsigned maxRecursionDepth, 
		const std::vector<TopLevelAS*>& arr_tlas, const std::vector<Texture2D*>& tex2ds, const std::vector<Texture3D*>& tex3ds, const std::vector<Cubemap*>& cubemaps,
		const char* body_raygen, const std::vector<const char*>& body_miss, const std::vector<const BodyHitShaders*>& body_hit, size_t times_submission)
	{
		unsigned kid = _build_raytrace_pipeline(arg_map, maxRecursionDepth, arr_tlas.size(), tex2ds.size(), tex3ds.size(), cubemaps.size(), body_raygen, body_miss, body_hit);
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

		return launch_raytrace(glbDim, arg_map.size(), args.data(), arr_tlas.data(), tex2ds.data(), tex3ds.data(), cubemaps.data(), kid, offsets.data(), times_submission);
	}

	bool Context::launch_raytrace(dim_type glbDim, const std::vector<CapturedShaderViewable>& arg_map, unsigned maxRecursionDepth, 
		const std::vector<TopLevelAS*>& arr_tlas, const std::vector<Texture2D*>& tex2ds, const std::vector<Texture3D*>& tex3ds, const std::vector<Cubemap*>& cubemaps,
		const char* body_raygen, const std::vector<const char*>& body_miss, const std::vector<const BodyHitShaders*>& body_hit, unsigned& kid, size_t* offsets, size_t times_submission)
	{
		kid = _build_raytrace_pipeline(arg_map, maxRecursionDepth, arr_tlas.size(), tex2ds.size(), tex3ds.size(), cubemaps.size(), body_raygen, body_miss, body_hit);
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

		return launch_raytrace(glbDim, arg_map.size(), args.data(), arr_tlas.data(), tex2ds.data(), tex3ds.data(), cubemaps.data(), kid, offsets, times_submission);
	}
}

