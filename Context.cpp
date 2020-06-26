#include "Context.h"
#include "internal_context.h"
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <shared_mutex>

namespace VkInline
{
	struct Attachement
	{
		Texture2D* tex;
		bool clear_at_load;
	};

	class Context
	{
	public:
		static Context& get_context();
		void set_verbose(bool verbose = true);

		// reflection 
		size_t size_of(const char* cls);
		bool query_struct(const char* name_struct, size_t* member_offsets);
		
		bool launch_compute(dim_type gridDim, size_t num_params, const ShaderViewable** args, Texture2D* const * tex2ds, Texture3D* const * tex3ds, unsigned kid, const size_t* offsets);
		bool launch_compute(dim_type gridDim, dim_type blockDim, const std::vector<CapturedShaderViewable>& arg_map, const std::vector<Texture2D*>& tex2ds, const std::vector<Texture3D*>& tex3ds, const char* code_body);
		bool launch_compute(dim_type gridDim, dim_type blockDim, const std::vector<CapturedShaderViewable>& arg_map, const std::vector<Texture2D*>& tex2ds, const std::vector<Texture3D*>& tex3ds, const char* code_body, unsigned& kid, size_t* offsets);

		bool launch_rasterization(Texture2D* const * colorBufs, Texture2D* depthBuf, Texture2D* const* resolveBufs, float* clear_colors, float clear_depth,
			size_t num_params, const ShaderViewable** args, Texture2D* const* tex2ds, Texture3D* const* tex3ds, unsigned* vertex_counts, unsigned rpid, const size_t* offsets);
		bool launch_rasterization(const std::vector<Attachement>& colorBufs, Attachement depthBuf, const std::vector<Attachement>& resolveBufs, float* clear_colors, float clear_depth,
			const std::vector<CapturedShaderViewable>& arg_map, const std::vector<Texture2D*>& tex2ds, const std::vector<Texture3D*>& tex3ds, const std::vector<const DrawCall*>& draw_calls, unsigned* vertex_counts);
		bool launch_rasterization(const std::vector<Attachement>& colorBufs, Attachement depthBuf, const std::vector<Attachement>& resolveBufs, float* clear_colors, float clear_depth,
			const std::vector<CapturedShaderViewable>& arg_map, const std::vector<Texture2D*>& tex2ds, const std::vector<Texture3D*>& tex3ds, const std::vector<const DrawCall*>& draw_calls, unsigned* vertex_counts, unsigned& rpid, size_t* offsets);

		void add_built_in_header(const char* name, const char* content);
		void add_code_block(const char* code);
		void add_inlcude_filename(const char* fn);

		std::string add_dynamic_code(const char* code);

	private:
		Context();
		~Context();

		unsigned _build_compute_pipeline(dim_type blockDim, const std::vector<CapturedShaderViewable>& arg_map, size_t num_tex2d, size_t num_tex3d, const char* code_body);
		unsigned _build_render_pass(
			const std::vector <Internal::AttachmentInfo>& color_attachmentInfo, const Internal::AttachmentInfo* depth_attachmentInfo, const std::vector <Internal::AttachmentInfo>& resolve_attachmentInfo,
			const std::vector<CapturedShaderViewable>& arg_map, size_t num_tex2d, size_t num_tex3d, const std::vector<const DrawCall*>& draw_calls);

		bool m_verbose;
		std::unordered_map<std::string, const char*> m_header_map;
		std::vector<std::string> m_code_blocks;

		std::string m_header_of_dynamic_code;
		std::string m_name_header_of_dynamic_code;
		std::unordered_set<int64_t> m_known_code;
		std::shared_mutex m_mutex_dynamic_code;

		std::unordered_map<std::string, size_t> m_size_of_types;
		std::mutex m_mutex_sizes;

		std::unordered_map<std::string, std::vector<size_t>> m_offsets_of_structs;
		std::mutex m_mutex_offsets;

		std::vector <Internal::ComputePipeline*> m_cache_compute_pipelines;
		std::unordered_map<int64_t, unsigned> m_map_compute_pipelines;
		std::shared_mutex m_mutex_compute_pipelines;

		std::vector <Internal::RenderPass*> m_cache_render_passes;
		std::unordered_map<int64_t, unsigned> m_map_render_passes;
		std::shared_mutex m_mutex_render_passes;

	};
}

#include "impl_context.inl"

namespace VkInline
{
	bool TryInit()
	{
		auto context = Internal::Context::get_context(false, true);
		if (context == nullptr) return false;
		Context& ctx = Context::get_context();
		return true;
	}

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

	void Wait()
	{
		auto context = Internal::Context::get_context();
		context->Wait();
	}

	int Texture2D::width() const
	{
		return m_tex->width();
	}

	int Texture2D::height() const
	{
		return m_tex->height();
	}

	unsigned Texture2D::pixel_size() const
	{
		return m_tex->pixel_size();
	}

	unsigned Texture2D::channel_count() const
	{
		return m_tex->channel_count();
	}

	unsigned Texture2D::sample_count() const
	{
		return m_tex->samples();
	}

	unsigned Texture2D::vkformat() const
	{
		return m_tex->format();
	}

	Texture2D::Texture2D(int width, int height, unsigned vkformat, bool isDepth, bool isStencil, unsigned sampleCount)
	{
		VkImageAspectFlags aspect = 0;
		if (isDepth) aspect |= VK_IMAGE_ASPECT_DEPTH_BIT;
		if (isStencil) aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
		if (aspect == 0) aspect |= VK_IMAGE_ASPECT_COLOR_BIT;

		VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT;
		if (isDepth || isStencil)
			usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		else
			usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		m_tex = new Internal::Texture2D(width, height, (VkFormat)vkformat, aspect, usage, (VkSampleCountFlagBits)sampleCount);

	}

	Texture2D::~Texture2D()
	{
		delete m_tex;
	}

	void Texture2D::upload(const void* hdata)
	{
		m_tex->upload(hdata);
	}

	void Texture2D::download(void* hdata) const
	{
		m_tex->download(hdata);
	}

	int Texture3D::dimX() const
	{
		return m_tex->dimX();
	}

	int Texture3D::dimY() const
	{
		return m_tex->dimY();
	}

	int Texture3D::dimZ() const
	{
		return m_tex->dimZ();
	}

	unsigned Texture3D::pixel_size() const
	{
		return m_tex->pixel_size();
	}

	unsigned Texture3D::channel_count() const
	{
		return m_tex->channel_count();
	}

	unsigned Texture3D::vkformat() const
	{
		return m_tex->format();
	}

	Texture3D::Texture3D(int dimX, int dimY, int dimZ, unsigned vkformat)
	{
		m_tex = new Internal::Texture3D(dimX, dimY, dimZ, (VkFormat)vkformat);
	}

	Texture3D::~Texture3D()
	{
		delete m_tex;
	}

	void Texture3D::upload(const void* hdata)
	{
		m_tex->upload(hdata);
	}

	void Texture3D::download(void* hdata) const
	{
		m_tex->download(hdata);
	}

	void Texture3D::apply_barrier(const Internal::CommandBuffer& cmdbuf, unsigned dstFlags) const
	{
		VkImageMemoryBarrier barriers[1];
		barriers[0] = {};
		barriers[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barriers[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barriers[0].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[0].image = m_tex->image();
		barriers[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barriers[0].subresourceRange.baseMipLevel = 0;
		barriers[0].subresourceRange.levelCount = 1;
		barriers[0].subresourceRange.baseArrayLayer = 0;
		barriers[0].subresourceRange.layerCount = 1;
		barriers[0].srcAccessMask = 0;
		barriers[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(
			cmdbuf.buf(),
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			dstFlags,
			0,
			0, nullptr,
			0, nullptr,
			1, barriers
		);
	}

	Computer::Computer(const std::vector<const char*>& param_names, const char* code_body, bool type_locked) :
		m_param_names(param_names.size()), m_code_body(code_body), m_type_locked(type_locked)
	{
		for (size_t i = 0; i < param_names.size(); i++)
			m_param_names[i] = param_names[i];

		m_kid = (unsigned)(-1);
	}

	bool Computer::launch(dim_type gridDim, dim_type blockDim, const ShaderViewable** args, const std::vector<Texture2D*>& tex2ds, const std::vector<Texture3D*>& tex3ds)
	{
		Context& ctx = Context::get_context();
		if (!m_type_locked)
		{
			std::vector<CapturedShaderViewable> arg_map(m_param_names.size());
			for (size_t i = 0; i < m_param_names.size(); i++)
			{
				arg_map[i].obj_name = m_param_names[i].c_str();
				arg_map[i].obj = args[i];
			}
			return ctx.launch_compute(gridDim, blockDim, arg_map, tex2ds, tex3ds, m_code_body.c_str());
		}
		else
		{
			std::unique_lock<std::mutex> locker(m_mu_type_lock);
			if (m_kid == (unsigned)(-1))
			{
				std::vector<CapturedShaderViewable> arg_map(m_param_names.size());
				for (size_t i = 0; i < m_param_names.size(); i++)
				{
					arg_map[i].obj_name = m_param_names[i].c_str();
					arg_map[i].obj = args[i];
				}
				m_offsets.resize(m_param_names.size() + 1);
				return ctx.launch_compute(gridDim, blockDim, arg_map, tex2ds, tex3ds, m_code_body.c_str(), m_kid, m_offsets.data());
			}
			else
			{
				locker.unlock();
				return ctx.launch_compute(gridDim, m_param_names.size(), args, tex2ds.data(), tex3ds.data(), m_kid, m_offsets.data());
			}
		}
	}

	DrawCall::DrawCall(const char* code_body_vert, const char* code_body_frag)
	{
		m_code_body_vert = code_body_vert;
		m_code_body_frag = code_body_frag;

		m_depth_enable = true;
		m_depth_write = true;
		m_color_write = true;
		m_alpha_write = true;
		m_alpha_blend = false;
		m_compare_op = 1;
	}

	size_t DrawCall::size_states() const
	{
		return &m_dummy - (char*)&m_depth_enable;
	}

	void DrawCall::get_states(void* p_data) const
	{
		memcpy(p_data, &m_depth_enable, size_states());
	}

	Rasterizer::Rasterizer(const std::vector<const char*>& param_names, bool type_locked)
		: m_param_names(param_names.size()), m_type_locked(type_locked)
	{
		for (size_t i = 0; i < param_names.size(); i++)
			m_param_names[i] = param_names[i];

		m_clear_depth_buf = true;
		m_rpid = (unsigned)(-1);
	}

	void Rasterizer::set_clear_color_buf(int i, bool clear)
	{
		if (m_clear_depth_buf && m_rpid != (unsigned)(-1)) return;
		if (i >= m_clear_color_buf.size())
			m_clear_color_buf.resize(i + 1, true);
		m_clear_color_buf[i] = clear;
	}

	void Rasterizer::set_clear_depth_buf(bool clear)
	{
		if (m_clear_depth_buf && m_rpid != (unsigned)(-1)) return;
		m_clear_depth_buf = clear;
	}

	void Rasterizer::add_draw_call(const DrawCall* draw_call)
	{
		if (m_clear_depth_buf && m_rpid != (unsigned)(-1)) return;
		m_draw_calls.push_back(draw_call);
	}

	bool Rasterizer::launch(const std::vector<Texture2D*>&  colorBufs, Texture2D* depthBuf, const std::vector<Texture2D*>& resolveBufs, float* clear_colors, float clear_depth,
		const ShaderViewable** args, const std::vector<Texture2D*>& tex2ds, const std::vector<Texture3D*>& tex3ds, unsigned* vertex_counts)
	{
		Context& ctx = Context::get_context();
		if (!m_type_locked)
		{
			std::vector<Attachement> color_att(colorBufs.size());
			for (size_t i = 0; i < colorBufs.size(); i++)
			{
				color_att[i].tex = colorBufs[i];
				color_att[i].clear_at_load = true;
				if (i < m_clear_color_buf.size())
					color_att[i].clear_at_load = m_clear_color_buf[i];
			}
			Attachement depth_att = { depthBuf, m_clear_depth_buf };
			std::vector<Attachement> resolve_att(resolveBufs.size());
			for (size_t i = 0; i < resolveBufs.size(); i++)
			{
				resolve_att[i].tex = resolveBufs[i];
				resolve_att[i].clear_at_load = false;
			}

			std::vector<CapturedShaderViewable> arg_map(m_param_names.size());
			for (size_t i = 0; i < m_param_names.size(); i++)
			{
				arg_map[i].obj_name = m_param_names[i].c_str();
				arg_map[i].obj = args[i];
			}

			return ctx.launch_rasterization(color_att, depth_att, resolve_att, clear_colors, clear_depth, arg_map, tex2ds, tex3ds, m_draw_calls, vertex_counts);
		}
		else
		{
			std::unique_lock<std::mutex> locker(m_mu_type_lock);
			if (m_rpid == (unsigned)(-1))
			{
				std::vector<Attachement> color_att(colorBufs.size());
				for (size_t i = 0; i < colorBufs.size(); i++)
				{
					color_att[i].tex = colorBufs[i];
					color_att[i].clear_at_load = true;
					if (i < m_clear_color_buf.size())
						color_att[i].clear_at_load = m_clear_color_buf[i];
				}
				Attachement depth_att = { depthBuf, m_clear_depth_buf };
				std::vector<Attachement> resolve_att(resolveBufs.size());
				for (size_t i = 0; i < resolveBufs.size(); i++)
				{
					resolve_att[i].tex = resolveBufs[i];
					resolve_att[i].clear_at_load = false;
				}

				std::vector<CapturedShaderViewable> arg_map(m_param_names.size());
				for (size_t i = 0; i < m_param_names.size(); i++)
				{
					arg_map[i].obj_name = m_param_names[i].c_str();
					arg_map[i].obj = args[i];
				}
				m_offsets.resize(m_param_names.size() + 1);
				return ctx.launch_rasterization(color_att, depth_att, resolve_att, clear_colors, clear_depth, arg_map, tex2ds, tex3ds, m_draw_calls, vertex_counts, m_rpid, m_offsets.data());
			}
			else
			{
				locker.unlock();
				return ctx.launch_rasterization(colorBufs.data(), depthBuf, resolveBufs.data(), clear_colors, clear_depth, m_param_names.size(), args, tex2ds.data(), tex3ds.data(), vertex_counts, m_rpid, m_offsets.data());
			}
		}
	
	}

}
