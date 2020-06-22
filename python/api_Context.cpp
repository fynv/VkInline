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

void* n_computer_create(void* ptr_param_list, const char* body, unsigned type_locked)
{
	StrArray* param_list = (StrArray*)ptr_param_list;
	size_t num_params = param_list->size();
	std::vector<const char*> params(num_params);
	for (size_t i = 0; i < num_params; i++)
		params[i] = (*param_list)[i].c_str();
	Computer* cptr = new Computer(params, body, type_locked!=0);
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

void* n_drawcall_create(const char* code_body_vert, const char* code_body_frag)
{
	return new DrawCall(code_body_vert, code_body_frag);
}

void n_drawcall_destroy(void* cptr)
{
	delete (DrawCall*)cptr;
}

void n_drawcall_set_depth_enable(void* cptr, unsigned enable)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_depth_enable(enable != 0);
}

void n_drawcall_set_depth_write(void* cptr, unsigned enable)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_depth_write(enable != 0);
}


void n_drawcall_set_color_write(void* cptr, unsigned enable)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_color_write(enable != 0);
}

void n_drawcall_set_alpha_write(void* cptr, unsigned enable)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_alpha_write(enable != 0);
}

void n_drawcall_set_alpha_blend(void* cptr, unsigned enable)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_alpha_blend(enable != 0);
}

void n_drawcall_set_depth_compare_op(void* cptr, unsigned op)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_depth_comapre_op(op);
}

void* n_rasterizer_create(void* ptr_param_list, unsigned type_locked)
{
	StrArray* param_list = (StrArray*)ptr_param_list;
	size_t num_params = param_list->size();
	std::vector<const char*> params(num_params);
	for (size_t i = 0; i < num_params; i++)
	{
		params[i] = (*param_list)[i].c_str();
	}
	Rasterizer* cptr = new Rasterizer(params, type_locked);
	return cptr;
}

void n_rasterizer_destroy(void* cptr)
{
	delete (Rasterizer*)cptr;
}

int n_rasterizer_num_params(void* cptr)
{
	Rasterizer* rasterizer = (Rasterizer*)cptr;
	return (int)rasterizer->num_params();
}

void n_rasterizer_set_clear_color_buf(void* cptr, int i, unsigned clear)
{
	Rasterizer* rasterizer = (Rasterizer*)cptr;
	rasterizer->set_clear_color_buf(i, clear != 0);
}

void n_rasterizer_set_clear_depth_buf(void* cptr, unsigned clear)
{
	Rasterizer* rasterizer = (Rasterizer*)cptr;
	rasterizer->set_clear_depth_buf(clear != 0);
}

void n_rasterizer_add_draw_call(void* cptr, void* draw_call)
{
	Rasterizer* rasterizer = (Rasterizer*)cptr;
	rasterizer->add_draw_call((DrawCall*)draw_call);
}

int n_rasterizer_launch(void* cptr, void* ptr_colorBufs, void* _depthBuf, void* ptr_resolveBufs, 
	float* clear_colors, float clear_depth,	void* ptr_arg_list, void* ptr_tex2d_list, unsigned* vertex_counts)
{
	Rasterizer* rasterizer = (Rasterizer*)cptr;
	Tex2DArray* colorBufs = (Tex2DArray*)ptr_colorBufs;
	Texture2D* depthBuf = (Texture2D*)_depthBuf;
	Tex2DArray* resolveBufs = (Tex2DArray*)ptr_resolveBufs;
	PtrArray* arg_list = (PtrArray*)ptr_arg_list;
	Tex2DArray* tex2d_list = (Tex2DArray*)ptr_tex2d_list;

	if (rasterizer->launch(*colorBufs, depthBuf, *resolveBufs, clear_colors, clear_depth, arg_list->data(), *tex2d_list, vertex_counts))
		return 0;
	else
		return -1;
}



