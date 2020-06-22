#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <thread>
#include "ShaderViewable.h"

namespace VkInline
{
	namespace Internal
	{
		class Texture2D;
		class CommandBufferRecycler;
	}

	struct dim_type
	{
		unsigned int x, y, z;
	};
	
	bool TryInit();
	void SetVerbose(bool verbose = true);

	// reflection 
	size_t SizeOf(const char* cls);
	bool QueryStruct(const char* name_struct, size_t* offsets);

	void AddBuiltInHeader(const char* name, const char* content);
	void AddCodeBlock(const char* code);
	void AddInlcudeFilename(const char* fn);
	std::string Add_Dynamic_Code(const char* code);

	void Wait();

	class Texture2D
	{
	public:
		int width() const;
		int height() const;
		unsigned pixel_size() const;
		unsigned channel_count() const;
		unsigned vkformat() const;

		Internal::Texture2D* internal() { return m_tex; }
		const Internal::Texture2D* internal() const { return m_tex; }

		Texture2D(int width, int height, unsigned vkformat, bool isDepth = false, bool isStencil = false);
		~Texture2D();

		void upload(const void* hdata);
		void download(void* hdata) const;

		void apply_barrier_as_texture(const Internal::CommandBuffer& cmdbuf, unsigned dstFlags) const;

	private:
		Internal::Texture2D* m_tex;
	};

	class Computer
	{
	public:
		size_t num_params() const { return m_param_names.size(); }
		Computer(const std::vector<const char*>& param_names, const char* code_body);
		bool launch(dim_type gridDim, dim_type blockDim, const ShaderViewable** args, const std::vector<Texture2D*>& tex2ds);

	private:
		std::vector<std::string> m_param_names;
		std::string m_code_body;
		
	};

	class DrawCall
	{
	public:
		DrawCall(const char* code_body_vert, const char* code_body_frag);

		void set_depth_enable(bool enable) { m_depth_enable = enable; }
		void set_depth_write(bool enable) { m_depth_write = enable; }
		void set_color_write(bool enable) { m_color_write = enable; }
		void set_alpha_write(bool enable) { m_alpha_write = enable; }
		void set_alpha_blend(bool enable) { m_alpha_blend = enable;  }

		const char* code_body_vert() const { return m_code_body_vert.c_str(); }
		const char* code_body_frag() const { return m_code_body_frag.c_str(); }

		size_t size_states() const;
		void get_states(void* p_data) const;
		

	private:
		std::string m_code_body_vert;
		std::string m_code_body_frag;
		
		bool m_depth_enable;
		bool m_depth_write;
		bool m_color_write;
		bool m_alpha_write;
		bool m_alpha_blend;

		char m_dummy;
	};

	class Rasterizer
	{
	public:
		size_t num_params() const { return m_param_names.size(); }
		Rasterizer(const std::vector<const char*>& param_names);

		void set_clear_color_buf(int i, bool clear);
		void set_clear_depth_buf(bool clear);

		void add_draw_call(const DrawCall* draw_call);
		bool launch(const std::vector<Texture2D*>&  colorBufs, Texture2D* depthBuf, float* clear_colors, float clear_depth,
			const ShaderViewable** args, const std::vector<Texture2D*>& tex2ds, unsigned* vertex_counts);

	private:
		std::vector<std::string> m_param_names;
		std::vector<bool> m_clear_color_buf;
		bool m_clear_depth_buf;
		std::vector<const DrawCall*> m_draw_calls;
	};


}
