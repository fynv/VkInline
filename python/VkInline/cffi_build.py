import os
if os.path.exists('VkInline/cffi.py'):
    os.remove('VkInline/cffi.py')

import cffi
ffibuilder = cffi.FFI()
ffibuilder.set_source("VkInline.cffi", None)

ffibuilder.cdef("""
// utils
void* n_string_array_create(unsigned long long size, const char* const* strs);
unsigned long long n_string_array_size(void* ptr_arr);
void n_string_array_destroy(void* ptr_arr);
void* n_pointer_array_create(unsigned long long size, const void* const* ptrs);
unsigned long long n_pointer_array_size(void* ptr_arr);
void n_pointer_array_destroy(void* ptr_arr);
void* n_dim3_create(unsigned x, unsigned y, unsigned z);
void n_dim3_destroy(void* cptr);
void* n_launch_param_from_count(unsigned count);
void* n_launch_param_from_buffer(void* buf);
void n_launch_param_destroy(void* lp);

// Context
int n_vkinline_try_init();
void n_set_verbose(unsigned verbose);
unsigned long long n_size_of(const char* cls);
void n_add_built_in_header(const char* filename, const char* filecontent);
void n_add_inlcude_filename(const char* fn);
void n_add_code_block(const char* line);
void n_wait();

void* n_computer_create(void* ptr_param_list, const char* body, unsigned type_locked);
void n_computer_destroy(void* cptr);
int n_computer_num_params(void* cptr);
int n_computer_launch(void* ptr_kernel, void* ptr_gridDim, void* ptr_blockDim, void* ptr_arg_list, 
		void* ptr_tex2d_list, void* ptr_tex3d_list, void* ptr_cubemap_list, unsigned times_submission);

void* n_drawcall_create(const char* code_body_vert, const char* code_body_frag);
void n_drawcall_destroy(void* cptr);

void n_drawcall_set_primitive_topology(void* cptr, unsigned topo);
void n_drawcall_set_primitive_restart(void* cptr, unsigned enable);
void n_drawcall_set_polygon_mode(void* cptr, unsigned mode);
void n_drawcall_set_cull_mode(void* cptr, unsigned mode);
void n_drawcall_set_front_face(void* cptr, unsigned mode);
void n_drawcall_set_line_width(void* cptr, float width);
void n_drawcall_set_depth_enable(void* cptr, unsigned enable);
void n_drawcall_set_depth_write(void* cptr, unsigned enable);
void n_drawcall_set_depth_compare_op(void* cptr, unsigned op);

void n_drawcall_set_color_write(void* cptr, unsigned enable);
void n_drawcall_set_color_write_r(void* cptr, unsigned enable);
void n_drawcall_set_color_write_g(void* cptr, unsigned enable);
void n_drawcall_set_color_write_b(void* cptr, unsigned enable);
void n_drawcall_set_alpha_write(void* cptr, unsigned enable);
void n_drawcall_set_blend_enable(void* cptr, unsigned enable);
void n_drawcall_set_src_color_blend_factor(void* cptr, unsigned factor);
void n_drawcall_set_dst_color_blend_factor(void* cptr, unsigned factor);
void n_drawcall_set_color_blend_op(void* cptr, unsigned op);
void n_drawcall_set_src_alpha_blend_factor(void* cptr, unsigned factor);
void n_drawcall_set_dst_alpha_blend_factor(void* cptr, unsigned factor);
void n_drawcall_set_alpha_blend_op(void* cptr, unsigned op);

void n_drawcall_set_blend_constants(void* cptr, float r, float g, float b, float a);

void n_drawcall_set_ith_color_write(void* cptr, int i, unsigned enable);
void n_drawcall_set_ith_color_write_r(void* cptr, int i, unsigned enable);
void n_drawcall_set_ith_color_write_g(void* cptr, int i, unsigned enable);
void n_drawcall_set_ith_color_write_b(void* cptr, int i, unsigned enable);
void n_drawcall_set_ith_alpha_write(void* cptr, int i, unsigned enable);
void n_drawcall_set_ith_blend_enable(void* cptr, int i, unsigned enable);
void n_drawcall_set_ith_src_color_blend_factor(void* cptr, int i, unsigned factor);
void n_drawcall_set_ith_dst_color_blend_factor(void* cptr, int i, unsigned factor);
void n_drawcall_set_ith_color_blend_op(void* cptr, int i, unsigned op);
void n_drawcall_set_ith_src_alpha_blend_factor(void* cptr, int i, unsigned factor);
void n_drawcall_set_ith_dst_alpha_blend_factor(void* cptr, int i, unsigned factor);
void n_drawcall_set_ith_alpha_blend_op(void* cptr, int i, unsigned op);

void* n_rasterizer_create(void* ptr_param_list, unsigned type_locked);
void n_rasterizer_destroy(void* cptr);
int n_rasterizer_num_params(void* cptr);
void n_rasterizer_set_clear_color_buf(void* cptr, int i, unsigned clear);
void n_rasterizer_set_clear_depth_buf(void* cptr, unsigned clear);
void n_rasterizer_add_draw_call(void* cptr, void* draw_call);
int n_rasterizer_launch(void* cptr, void* ptr_colorBufs, void* _depthBuf, void* ptr_resolveBufs, 
		float* clear_colors, float clear_depth,	void* ptr_arg_list, void* ptr_tex2d_list, void* ptr_tex3d_list, void* ptr_cubemap_list,
		void** ptr_launch_params, unsigned times_submission);

// ShaderViewable
const char* n_sv_name_view_type(void* cptr);
void n_sv_destroy(void* cptr);
void* n_svint32_create(int v);
int n_svint32_value(void* cptr);
void* n_svuint32_create(unsigned v);
unsigned n_svuint32_value(void* cptr);
void* n_svfloat_create(float v);
float n_svfloat_value(void* cptr);
void* n_svdouble_create(double v);
double n_svdouble_value(void* cptr);

void* n_svivec2_create(const int* v);
void n_svivec2_value(void* cptr, int* v);
void* n_svivec3_create(const int* v);
void n_svivec3_value(void* cptr, int* v);
void* n_svivec4_create(const int* v);
void n_svivec4_value(void* cptr, int* v);

void* n_svuvec2_create(const unsigned* v);
void n_svuvec2_value(void* cptr, unsigned* v);
void* n_svuvec3_create(const unsigned* v);
void n_svuvec3_value(void* cptr, unsigned* v);
void* n_svuvec4_create(const unsigned* v);
void n_svuvec4_value(void* cptr, unsigned* v);

void* n_svvec2_create(const float* v);
void n_svvec2_value(void* cptr, float* v);
void* n_svvec3_create(const float* v);
void n_svvec3_value(void* cptr, float* v);
void* n_svvec4_create(const float* v);
void n_svvec4_value(void* cptr, float* v);

void* n_svdvec2_create(const double* v);
void n_svdvec2_value(void* cptr, double* v);
void* n_svdvec3_create(const double* v);
void n_svdvec3_value(void* cptr, double* v);
void* n_svdvec4_create(const double* v);
void n_svdvec4_value(void* cptr, double* v);

void* n_svmat2x2_create(const float* v);
void n_svmat2x2_value(void* cptr, float* v);
void* n_svmat2x3_create(const float* v);
void n_svmat2x3_value(void* cptr, float* v);
void* n_svmat2x4_create(const float* v);
void n_svmat2x4_value(void* cptr, float* v);
void* n_svmat3x2_create(const float* v);
void n_svmat3x2_value(void* cptr, float* v);
void* n_svmat3x3_create(const float* v);
void n_svmat3x3_value(void* cptr, float* v);
void* n_svmat3x4_create(const float* v);
void n_svmat3x4_value(void* cptr, float* v);
void* n_svmat4x2_create(const float* v);
void n_svmat4x2_value(void* cptr, float* v);
void* n_svmat4x3_create(const float* v);
void n_svmat4x3_value(void* cptr, float* v);
void* n_svmat4x4_create(const float* v);
void n_svmat4x4_value(void* cptr, float* v);

void* n_svdmat2x2_create(const double* v);
void n_svdmat2x2_value(void* cptr, double* v);
void* n_svdmat2x3_create(const double* v);
void n_svdmat2x3_value(void* cptr, double* v);
void* n_svdmat2x4_create(const double* v);
void n_svdmat2x4_value(void* cptr, double* v);
void* n_svdmat3x2_create(const double* v);
void n_svdmat3x2_value(void* cptr, double* v);
void* n_svdmat3x3_create(const double* v);
void n_svdmat3x3_value(void* cptr, double* v);
void* n_svdmat3x4_create(const double* v);
void n_svdmat3x4_value(void* cptr, double* v);
void* n_svdmat4x2_create(const double* v);
void n_svdmat4x2_value(void* cptr, double* v);
void* n_svdmat4x3_create(const double* v);
void n_svdmat4x3_value(void* cptr, double* v);
void* n_svdmat4x4_create(const double* v);
void n_svdmat4x4_value(void* cptr, double* v);

// SVBuffer
void* n_svbuffer_create(const char* elem_type, unsigned long long size, void* hdata);
const char* n_svbuffer_name_elem_type(void* cptr);
unsigned long long n_svbuffer_elem_size(void* cptr);
unsigned long long n_svbuffer_size(void* cptr);
void n_svbuffer_from_host(void* cptr, void* hdata);
void n_svbuffer_to_host(void* cptr, void* hdata, unsigned long long begin, unsigned long long end);

// SVCombine
void* n_svcombine_create(void* ptr_svs, void* ptr_names, const char* operations);

// SVObjBuffer
void* n_svobjbuffer_create(void* ptr_svs);
const char* n_svobjbuffer_name_elem_type(void* cptr);
unsigned long long n_svobjbuffer_elem_size(void* cptr);
unsigned long long n_svobjbuffer_size(void* cptr);
void n_svobjbuffer_update(void* cptr);

// Texture2D
void* n_texture2d_create(int width, int height, unsigned vkformat, unsigned isDepth, unsigned isStencil, unsigned sampleCount);
void n_texture2d_release(void* tex2d);
int n_texture2d_width(void* tex2d);
int n_texture2d_height(void* tex2d);
unsigned n_texture2d_pixelsize(void* tex2d);
unsigned n_texture2d_channelcount(void* tex2d);
unsigned n_texture2d_samplecount(void* tex2d);
unsigned n_texture2d_vkformat(void* tex2d);
void n_texture2d_upload(void* tex2d, void* hdata);
void n_texture2d_download(void* tex2d, void* hdata);

// Texture3D
void* n_texture3d_create(int dimX, int dimY, int dimZ, unsigned vkformat);
void n_texture3d_release(void* tex3d);
int n_texture3d_dimX(void* tex3d);
int n_texture3d_dimY(void* tex3d);
int n_texture3d_dimZ(void* tex3d);
unsigned n_texture3d_pixelsize(void* tex3d);
unsigned n_texture3d_channelcount(void* tex3d);
unsigned n_texture3d_vkformat(void* tex3d);
void n_texture3d_upload(void* tex3d, void* hdata);
void n_texture3d_download(void* tex3d, void* hdata);

// Cubemap
void* n_cubemap_create(int width, int height, unsigned vkformat);
void n_cubemap_release(void* cubemap);
int n_cubemap_width(void* cubemap);
int n_cubemap_height(void* cubemap);
unsigned n_cubemap_pixelsize(void* cubemap);
unsigned n_cubemap_channelcount(void* cubemap);
unsigned n_cubemap_vkformat(void* cubemap);
void n_cubemap_upload(void* cubemap, void* hdata);
void n_cubemap_download(void* cubemap, void* hdata);

// Extensions
void* n_blas_create_triangles(void* indBuf, void* posBuf);
void* n_blas_create_procedure(void* aabbBuf);
void n_blas_destroy(void* ptr_blas);
void* n_mat4_create(const float* v);
void n_mat4_destroy(void* ptr);
void* n_tlas_create(void* ptr_blases, void* ptr_transes);
void n_tlas_destroy(void* ptr);
void* n_hit_shaders_create(const char* closest_hit, const char* intersection);
void n_hit_shaders_destroy(void* ptr);
void* n_raytracer_create(void* ptr_param_list, const char* body_raygen, void* ptr_body_miss, void* ptr_body_hit, unsigned maxRecursionDepth, unsigned type_locked);
void n_raytracer_destroy(void* cptr);
int n_raytracer_num_params(void* cptr);
int n_raytracer_launch(void* ptr_raytracer, void* ptr_glbDim, void* ptr_arg_list, void* ptr_tlas_list, void* ptr_tex2d_list, void* ptr_tex3d_list, void* ptr_cubemap_list, unsigned times_submission);
""")


ffibuilder.compile()

