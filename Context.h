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
		class TextureCube;
		class CommandBufferRecycler;
		struct GraphicsPipelineStates;
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

	private:
		Internal::Texture3D* m_tex;
	};

	class Cubemap
	{
	public:
		int width() const;
		int height() const;
		unsigned pixel_size() const;
		unsigned channel_count() const;
		unsigned vkformat() const;

		Internal::TextureCube* internal() { return m_tex; }
		const Internal::TextureCube* internal() const { return m_tex; }

		Cubemap(int width, int height, unsigned vkformat);
		~Cubemap();

		void upload(const void* hdata);
		void download(void* hdata) const;

	private:
		Internal::TextureCube* m_tex;
	};

	class Computer
	{
	public:
		size_t num_params() const { return m_param_names.size(); }
		Computer(const std::vector<const char*>& param_names, const char* code_body, bool type_locked = false);
		bool launch(dim_type gridDim, dim_type blockDim, const ShaderViewable** args, 
			const std::vector<Texture2D*>& tex2ds, const std::vector<Texture3D*>& tex3ds, const std::vector<Cubemap*>& cubemaps, size_t times_submission = 1);

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
		~DrawCall();

		void set_primitive_topology(unsigned topo);
		void set_primitive_restart(bool enable);

		void set_polygon_mode(unsigned mode);
		void set_cull_mode(unsigned mode);
		void set_front_face(unsigned mode);
		void set_line_width(float width);

		void set_depth_enable(bool enable);
		void set_depth_write(bool enable);
		void set_depth_comapre_op(unsigned op);

		void set_color_write(bool enable) { m_color_write_r = m_color_write_g = m_color_write_b = enable; }
		void set_color_write_r(bool enable) { m_color_write_r = enable; }
		void set_color_write_g(bool enable) { m_color_write_g = enable; }
		void set_color_write_b(bool enable) { m_color_write_b = enable; }
		void set_alpha_write(bool enable) { m_alpha_write = enable; }
		void set_blend_enable(bool enable) { m_blend_enable = enable; }
		void set_src_color_blend_factor(unsigned factor) { m_src_color_blend_factor = factor; }
		void set_dst_color_blend_factor(unsigned factor) { m_dst_color_blend_factor = factor; }
		void set_color_blend_op(unsigned op) { m_color_blend_op = op; }
		void set_src_alpha_blend_factor(unsigned factor) { m_src_alpha_blend_factor = factor; }
		void set_dst_alpha_blend_factor(unsigned factor) { m_dst_alpha_blend_factor = factor; }
		void set_alpha_blend_op(unsigned op) { m_alpha_blend_op = op; }

		void set_blend_constants(float r, float g, float b, float a);

		void set_ith_color_write(int i, bool enable);
		void set_ith_color_write_r(int i, bool enable);
		void set_ith_color_write_g(int i, bool enable);
		void set_ith_color_write_b(int i, bool enable);
		void set_ith_alpha_write(int i, bool enable);
		void set_ith_blend_enable(int i, bool enable);
		void set_ith_src_color_blend_factor(int i, unsigned factor);
		void set_ith_dst_color_blend_factor(int i, unsigned factor);
		void set_ith_color_blend_op(int i, unsigned op);
		void set_ith_src_alpha_blend_factor(int i, unsigned factor);
		void set_ith_dst_alpha_blend_factor(int i, unsigned factor);
		void set_ith_alpha_blend_op(int i, unsigned op);

		const char* code_body_vert() const { return m_code_body_vert.c_str(); }
		const char* code_body_frag() const { return m_code_body_frag.c_str(); }

		const Internal::GraphicsPipelineStates& get_states(int num_color_att) const;		

	private:
		std::string m_code_body_vert;
		std::string m_code_body_frag;

		bool m_color_write_r;
		bool m_color_write_g;
		bool m_color_write_b;
		bool m_alpha_write;
		bool m_blend_enable;
		unsigned m_src_color_blend_factor;
		unsigned m_dst_color_blend_factor;
		unsigned m_color_blend_op;
		unsigned m_src_alpha_blend_factor;
		unsigned m_dst_alpha_blend_factor;
		unsigned m_alpha_blend_op;

		Internal::GraphicsPipelineStates* m_states;
		mutable std::mutex m_mu_colorBlendAttachments;
		void _resize_color_att(int num_color_att) const;

	};

	class SVBuffer;

	class Rasterizer
	{
	public:
		size_t num_params() const { return m_param_names.size(); }
		Rasterizer(const std::vector<const char*>& param_names, bool type_locked = false);

		void set_clear_color_buf(int i, bool clear);
		void set_clear_depth_buf(bool clear);

		void add_draw_call(const DrawCall* draw_call);

		struct LaunchParam
		{
			unsigned count;
			SVBuffer* indBuf;
		};

		bool launch(const std::vector<Texture2D*>& colorBufs, Texture2D* depthBuf, const std::vector<Texture2D*>& resolveBufs, float* clear_colors, float clear_depth, const ShaderViewable** args, 
			const std::vector<Texture2D*>& tex2ds, const std::vector<Texture3D*>& tex3ds, const std::vector<Cubemap*>& cubemaps,
			Rasterizer::LaunchParam** launch_params, size_t times_submission = 1);

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

#ifdef _VkInlineEX
#include "Context_ex.h"
#endif

