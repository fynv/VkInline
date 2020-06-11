#include "Context.h"
#include "internal_context.h"
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <shared_mutex>

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
		bool launch_compute(dim_type gridDim, dim_type blockDim, const std::vector<CapturedShaderViewable>& arg_map, const std::vector<Texture2D*>& tex2ds,  const char* code_body);


		void add_built_in_header(const char* name, const char* content);
		void add_code_block(const char* code);
		void add_inlcude_filename(const char* fn);

		std::string add_dynamic_code(const char* code);

	private:
		Context();
		~Context();

		unsigned _build_compute_pipeline(dim_type blockDim, const std::vector<CapturedShaderViewable>& arg_map, size_t num_tex2d, const char* code_body);

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

	unsigned Texture2D::vkformat() const
	{
		return m_tex->format();
	}

	Texture2D::Texture2D(int width, int height, unsigned vkformat, bool isDepth, bool isStencil)
	{
		VkImageAspectFlags aspect = 0;
		if (isDepth) aspect |= VK_IMAGE_ASPECT_DEPTH_BIT;
		if (isStencil) aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
		if (aspect == 0) aspect |= VK_IMAGE_ASPECT_COLOR_BIT;

		VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
		if (isDepth || isStencil)
			usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		else
			usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		m_tex = new Internal::Texture2D(width, height, (VkFormat)vkformat, aspect, usage);

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

	void Texture2D::apply_barrier_as_texture(const Internal::CommandBuffer& cmdbuf, unsigned dstFlags) const
	{
		VkImageMemoryBarrier barriers[1];
		barriers[0] = {};
		barriers[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barriers[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barriers[0].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[0].image = m_tex->image();
		barriers[0].subresourceRange.aspectMask = m_tex->aspect();
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

	Computer::Computer(const std::vector<const char*>& param_names, const char* code_body) :
		m_param_names(param_names.size()), m_code_body(code_body)
	{
		for (size_t i = 0; i < param_names.size(); i++)
			m_param_names[i] = param_names[i];
	}

	bool Computer::launch(dim_type gridDim, dim_type blockDim, const ShaderViewable** args, const std::vector<Texture2D*>& tex2ds)
	{
		Context& ctx = Context::get_context();
		std::vector<CapturedShaderViewable> arg_map(m_param_names.size());
		for (size_t i = 0; i < m_param_names.size(); i++)
		{
			arg_map[i].obj_name = m_param_names[i].c_str();
			arg_map[i].obj = args[i];
		}
		return ctx.launch_compute(gridDim, blockDim, arg_map, tex2ds, m_code_body.c_str());
	}




}
