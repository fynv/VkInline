#include "api.h"
#include "Context.h"
using namespace VkInline;
#include <string>
#include <vector>

typedef std::vector<std::string> StrArray;
typedef std::vector<const ShaderViewable*> PtrArray;
typedef std::vector<Texture2D*> Tex2DArray;
typedef std::vector<Texture3D*> Tex3DArray;

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
	delete (Computer*)cptr;
}

int n_computer_num_params(void* cptr)
{
	Computer* kernel = (Computer*)cptr;
	return (int)kernel->num_params();
}


int n_computer_launch(void* ptr_kernel, void* ptr_gridDim, void* ptr_blockDim, void* ptr_arg_list, void* ptr_tex2d_list, void* ptr_tex3d_list, unsigned times_submission)
{
	Computer* kernel = (Computer*)ptr_kernel;
	size_t num_params = kernel->num_params();

	dim_type* gridDim = (dim_type*)ptr_gridDim;
	dim_type* blockDim = (dim_type*)ptr_blockDim;

	PtrArray* arg_list = (PtrArray*)ptr_arg_list;
	Tex2DArray* tex2d_list = (Tex2DArray*)ptr_tex2d_list;
	Tex3DArray* tex3d_list = (Tex3DArray*)ptr_tex3d_list;

	size_t size = arg_list->size();
	if (num_params != size)
	{
		printf("Wrong number of arguments received. %d required, %d received.", (int)num_params, (int)size);
		return -1;
	}

	if (kernel->launch(*gridDim, *blockDim, arg_list->data(), *tex2d_list, *tex3d_list, times_submission))
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

void n_drawcall_set_primitive_topology(void* cptr, unsigned topo)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_primitive_topology(topo);
}

void n_drawcall_set_primitive_restart(void* cptr, unsigned enable)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_primitive_restart(enable!=0);
}

void n_drawcall_set_polygon_mode(void* cptr, unsigned mode)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_polygon_mode(mode);
}

void n_drawcall_set_cull_mode(void* cptr, unsigned mode)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_cull_mode(mode);
}

void n_drawcall_set_front_face(void* cptr, unsigned mode)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_front_face(mode);
}

void n_drawcall_set_line_width(void* cptr, float width)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_line_width(width);
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

void n_drawcall_set_depth_compare_op(void* cptr, unsigned op)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_depth_comapre_op(op);
}

void n_drawcall_set_color_write(void* cptr, unsigned enable)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_color_write(enable != 0);
}

void n_drawcall_set_color_write_r(void* cptr, unsigned enable)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_color_write_r(enable != 0);
}

void n_drawcall_set_color_write_g(void* cptr, unsigned enable)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_color_write_g(enable != 0);
}

void n_drawcall_set_color_write_b(void* cptr, unsigned enable)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_color_write_b(enable != 0);
}

void n_drawcall_set_alpha_write(void* cptr, unsigned enable)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_alpha_write(enable != 0);
}

void n_drawcall_set_blend_enable(void* cptr, unsigned enable)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_blend_enable(enable != 0);
}

void n_drawcall_set_src_color_blend_factor(void* cptr, unsigned factor)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_src_color_blend_factor(factor);
}

void n_drawcall_set_dst_color_blend_factor(void* cptr, unsigned factor)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_dst_color_blend_factor(factor);
}

void n_drawcall_set_color_blend_op(void* cptr, unsigned op)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_color_blend_op(op);
}

void n_drawcall_set_src_alpha_blend_factor(void* cptr, unsigned factor)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_src_alpha_blend_factor(factor);
}

void n_drawcall_set_dst_alpha_blend_factor(void* cptr, unsigned factor)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_dst_alpha_blend_factor(factor);
}

void n_drawcall_set_alpha_blend_op(void* cptr, unsigned op)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_alpha_blend_op(op);
}

void n_drawcall_set_blend_constants(void* cptr, float r, float g, float b, float a)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_blend_constants(r, g, b, a);
}

void n_drawcall_set_ith_color_write(void* cptr, int i, unsigned enable)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_ith_color_write(i, enable != 0);
}

void n_drawcall_set_ith_color_write_r(void* cptr, int i, unsigned enable)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_ith_color_write_r(i, enable != 0);
}

void n_drawcall_set_ith_color_write_g(void* cptr, int i, unsigned enable)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_ith_color_write_g(i, enable != 0);
}

void n_drawcall_set_ith_color_write_b(void* cptr, int i, unsigned enable)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_ith_color_write_b(i, enable != 0);
}

void n_drawcall_set_ith_alpha_write(void* cptr, int i, unsigned enable)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_ith_alpha_write(i, enable != 0);
}

void n_drawcall_set_ith_blend_enable(void* cptr, int i, unsigned enable)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_ith_blend_enable(i, enable != 0);
}

void n_drawcall_set_ith_src_color_blend_factor(void* cptr, int i, unsigned factor)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_ith_src_color_blend_factor(i, factor);
}

void n_drawcall_set_ith_dst_color_blend_factor(void* cptr, int i, unsigned factor)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_ith_dst_color_blend_factor(i, factor);
}

void n_drawcall_set_ith_color_blend_op(void* cptr, int i, unsigned op)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_ith_color_blend_op(i, op);
}

void n_drawcall_set_ith_src_alpha_blend_factor(void* cptr, int i, unsigned factor)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_ith_src_alpha_blend_factor(i, factor);
}

void n_drawcall_set_ith_dst_alpha_blend_factor(void* cptr, int i, unsigned factor)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_ith_dst_alpha_blend_factor(i, factor);
}

void n_drawcall_set_ith_alpha_blend_op(void* cptr, int i, unsigned op)
{
	DrawCall* dc = (DrawCall*)cptr;
	dc->set_ith_alpha_blend_op(i, op);
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
	float* clear_colors, float clear_depth,	void* ptr_arg_list, void* ptr_tex2d_list, void* ptr_tex3d_list, void** ptr_launch_params, unsigned times_submission)
{
	Rasterizer* rasterizer = (Rasterizer*)cptr;
	Tex2DArray* colorBufs = (Tex2DArray*)ptr_colorBufs;
	Texture2D* depthBuf = (Texture2D*)_depthBuf;
	Tex2DArray* resolveBufs = (Tex2DArray*)ptr_resolveBufs;
	PtrArray* arg_list = (PtrArray*)ptr_arg_list;
	Tex2DArray* tex2d_list = (Tex2DArray*)ptr_tex2d_list;
	Tex3DArray* tex3d_list = (Tex3DArray*)ptr_tex3d_list;
	Rasterizer::LaunchParam** launch_params = (Rasterizer::LaunchParam**)ptr_launch_params;

	if (rasterizer->launch(*colorBufs, depthBuf, *resolveBufs, clear_colors, clear_depth, arg_list->data(), *tex2d_list, *tex3d_list, launch_params, times_submission))
		return 0;
	else
		return -1;
}


#ifdef _VkInlineEX
#include "api_Context_ex.inl"
#endif


