#pragma once

#include <vector>
#include <string>
#include "ShaderViewable.h"

namespace VkInline
{
	struct dim_type
	{
		unsigned int x, y, z;
	};

	void SetVerbose(bool verbose = true);

	// reflection 
	size_t SizeOf(const char* cls);
	bool QueryStruct(const char* name_struct, size_t* offsets);

	void AddBuiltInHeader(const char* name, const char* content);
	void AddCodeBlock(const char* code);
	void AddInlcudeFilename(const char* fn);
	std::string Add_Dynamic_Code(const char* code);

	void Wait(int streamId = 0);

	class Computer
	{
	public:
		size_t num_params() const { return m_param_names.size(); }
		Computer(const std::vector<const char*>& param_names, const char* code_body);
		bool launch(dim_type gridDim, dim_type blockDim, const ShaderViewable** args, int streamId = 0);

	private:
		std::vector<std::string> m_param_names;
		std::string m_code_body;
	};

}
