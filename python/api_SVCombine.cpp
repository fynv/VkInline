#include "api.h"
#include "SVCombine.h"
using namespace VkInline;

typedef std::vector<std::string> StrArray;
typedef std::vector<const ShaderViewable*> PtrArray;

void* n_svcombine_create(void* ptr_svs, void* ptr_names, const char* operations)
{
	PtrArray* svs = (PtrArray*)ptr_svs;
	StrArray* names = (StrArray*)ptr_names;
	size_t num_params = svs->size();
	std::vector<CapturedShaderViewable> arg_map(num_params);
	for (size_t i = 0; i < num_params; i++)
	{
		arg_map[i].obj_name = (*names)[i].c_str();
		arg_map[i].obj = (*svs)[i];
	}

	return new SVCombine(arg_map, operations);
}


