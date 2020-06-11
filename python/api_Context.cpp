#include "api.h"
#include "Context.h"
using namespace VkInline;
#include <string>
#include <vector>

typedef std::vector<std::string> StrArray;
typedef std::vector<const ShaderViewable*> PtrArray;
typedef std::vector<Texture2D*> Tex2DArray;

int n_vkinline_try_init()
{
	return TryInit() ? 1 : 0;
}

void n_set_verbose(unsigned verbose)
{
	SetVerbose(verbose != 0);
}

unsigned long long n_size_of(const char* cls)
{
	return SizeOf(cls);
}

void n_add_built_in_header(const char* filename, const char* filecontent)
{
	AddBuiltInHeader(filename, filecontent);
}

void n_add_inlcude_filename(const char* fn)
{
	AddInlcudeFilename(fn);
}

void n_add_code_block(const char* line)
{
	AddCodeBlock(line);
}

void n_wait()
{
	Wait();
}

void* n_computer_create(void* ptr_param_list, const char* body)
{
	StrArray* param_list = (StrArray*)ptr_param_list;
	size_t num_params = param_list->size();
	std::vector<const char*> params(num_params);
	for (size_t i = 0; i < num_params; i++)
		params[i] = (*param_list)[i].c_str();
	Computer* cptr = new Computer(params, body);
	return cptr;
}

void n_computer_destroy(void* cptr)
{
	Computer* kernel = (Computer*)cptr;
	delete kernel;
}

int n_computer_num_params(void* cptr)
{
	Computer* kernel = (Computer*)cptr;
	return (int)kernel->num_params();
}


int n_computer_launch(void* ptr_kernel, void* ptr_gridDim, void* ptr_blockDim, void* ptr_arg_list, void* ptr_tex2d_list)
{
	Computer* kernel = (Computer*)ptr_kernel;
	size_t num_params = kernel->num_params();

	dim_type* gridDim = (dim_type*)ptr_gridDim;
	dim_type* blockDim = (dim_type*)ptr_blockDim;

	PtrArray* arg_list = (PtrArray*)ptr_arg_list;
	Tex2DArray* tex2d_list = (Tex2DArray*)ptr_tex2d_list;

	size_t size = arg_list->size();
	if (num_params != size)
	{
		printf("Wrong number of arguments received. %d required, %d received.", (int)num_params, (int)size);
		return -1;
	}

	if (kernel->launch(*gridDim, *blockDim, arg_list->data(), *tex2d_list))
		return 0;
	else
		return -1;
}


