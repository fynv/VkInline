#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <thread>
#include <mutex>
#include "ShaderViewable.h"

namespace VkInline
{
	namespace Internal
	{
		class Texture2D;
		class Texture3D;
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
		unsigned sample_count() const;
		unsigned vkformat() const;

		Internal::Texture2D* internal() { return m_tex; }
		const Internal::Texture2D* internal() const { return m_tex; }

		Texture2D(int width, int height, unsigned vkformat, bool isDepth = false, bool isStencil = false, unsigned sampleCount = 1);
		~Texture2D();

		void upload(const void* hdata);
		void download(void* hdata) const;

		void apply_barrier_as_texture(const Internal::CommandBuffer& cmdbuf, unsigned dstFlags) const;
		void apply_barrier_as_attachment(const Internal::CommandBuffer& cmdbuf);

	private:
		Internal::Texture2D* m_tex;
	};

	class Texture3D
	{
	public:
		int dimX() const;
		int dimY() const;
		int dimZ() const;
		unsigned pixel_size() const;
		unsigned channel_count() const;
		unsigned vkformat() const;

		Internal::Texture3D* internal() { return m_tex; }
		const Internal::Texture3D* internal() const { return m_tex; }

		Texture3D(int dimX, int dimY, int dimZ, unsigned vkformat);
		~Texture3D();

		void upload(const void* hdata);
		void download(void* hdata) const;

		void apply_barrier(const Internal::CommandBuffer& cmdbuf, unsigned dstFlags) const;

	private:
		Internal::Texture3D* m_tex;
	};

	class Computer
	{
	public:
		size_t num_params() const { return m_param_names.size(); }
		Computer(const std::vector<const char*>& param_names, const char* code_body, bool type_locked = false);
		bool launch(dim_type gridDim, dim_type blockDim, const ShaderViewable** args, const std::vector<Texture2D*>& tex2ds, const std::vector<Texture3D*>& tex3ds);

	private:
		std::vector<std::string> m_param_names;
		std::string m_code_body;

		bool m_type_locked;
		unsigned m_kid;
		std::vector<size_t> m_offsets;
		std::mutex m_mu_type_lock;
		
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
		void set_depth_comapre_op(unsigned op) { m_compare_op = op; }

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
		unsigned m_compare_op;

		char m_dummy;
	};

	class Rasterizer
	{
	public:
		size_t num_params() const { return m_param_names.size(); }
		Rasterizer(const std::vector<const char*>& param_names, bool type_locked = false);

		void set_clear_color_buf(int i, bool clear);
		void set_clear_depth_buf(bool clear);

		void add_draw_call(const DrawCall* draw_call);
		bool launch(const std::vector<Texture2D*>& colorBufs, Texture2D* depthBuf, const std::vector<Texture2D*>& resolveBufs, float* clear_colors, float clear_depth,
			const ShaderViewable** args, const std::vector<Texture2D*>& tex2ds, const std::vector<Texture3D*>& tex3ds, unsigned* vertex_counts);

	private:
		std::vector<std::string> m_param_names;
		std::vector<bool> m_clear_color_buf;
		bool m_clear_depth_buf;
		std::vector<const DrawCall*> m_draw_calls;

		bool m_type_locked;
		unsigned m_rpid;
		std::vector<size_t> m_offsets;
		std::mutex m_mu_type_lock;
	};


}
