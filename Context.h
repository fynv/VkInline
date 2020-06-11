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


}
