#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define PY_VkInline_API __declspec(dllexport)
#else
#define PY_VkInline_API 
#endif

extern "C"
{
	// utils
	PY_VkInline_API void* n_string_array_create(unsigned long long size, const char* const* strs);
	PY_VkInline_API unsigned long long n_string_array_size(void* ptr_arr);
	PY_VkInline_API void n_string_array_destroy(void* ptr_arr);
	PY_VkInline_API void* n_pointer_array_create(unsigned long long size, const void* const* ptrs);
	PY_VkInline_API unsigned long long n_pointer_array_size(void* ptr_arr);
	PY_VkInline_API void n_pointer_array_destroy(void* ptr_arr);
	PY_VkInline_API void* n_dim3_create(unsigned x, unsigned y, unsigned z);
	PY_VkInline_API void n_dim3_destroy(void* cptr);
	PY_VkInline_API void *n_tex2d_array_create(unsigned long long size, void** ptrs);
	PY_VkInline_API unsigned long long n_tex2d_array_size(void* ptr_arr);
	PY_VkInline_API void n_tex2d_array_destroy(void* ptr_arr);

	// Context
	PY_VkInline_API int n_vkinline_try_init();
	PY_VkInline_API void n_set_verbose(unsigned verbose);
	PY_VkInline_API unsigned long long n_size_of(const char* cls);
	PY_VkInline_API void n_add_built_in_header(const char* filename, const char* filecontent);
	PY_VkInline_API void n_add_inlcude_filename(const char* fn);
	PY_VkInline_API void n_add_code_block(const char* line);
	PY_VkInline_API void n_wait();

	PY_VkInline_API void* n_computer_create(void* ptr_param_list, const char* body);
	PY_VkInline_API void n_computer_destroy(void* cptr);
	PY_VkInline_API int n_computer_num_params(void* cptr);
	PY_VkInline_API int n_computer_launch(void* ptr_kernel, void* ptr_gridDim, void* ptr_blockDim, void* ptr_arg_list, void* ptr_tex2d_list);

	// ShaderViewable
	PY_VkInline_API const char* n_sv_name_view_type(void* cptr);
	PY_VkInline_API void n_sv_destroy(void* cptr);
	PY_VkInline_API void* n_svint32_create(int v);
	PY_VkInline_API int n_svint32_value(void* cptr);
	PY_VkInline_API void* n_svuint32_create(unsigned v);
	PY_VkInline_API unsigned n_svuint32_value(void* cptr);
	PY_VkInline_API void* n_svfloat_create(float v);
	PY_VkInline_API float n_svfloat_value(void* cptr);
	PY_VkInline_API void* n_svdouble_create(double v);
	PY_VkInline_API double n_svdouble_value(void* cptr);

	PY_VkInline_API void* n_svivec2_create(const int* v);
	PY_VkInline_API void n_svivec2_value(void* cptr, int* v);
	PY_VkInline_API void* n_svivec3_create(const int* v);
	PY_VkInline_API void n_svivec3_value(void* cptr, int* v);
	PY_VkInline_API void* n_svivec4_create(const int* v);
	PY_VkInline_API void n_svivec4_value(void* cptr, int* v);

	PY_VkInline_API void* n_svuvec2_create(const unsigned* v);
	PY_VkInline_API void n_svuvec2_value(void* cptr, unsigned* v);
	PY_VkInline_API void* n_svuvec3_create(const unsigned* v);
	PY_VkInline_API void n_svuvec3_value(void* cptr, unsigned* v);
	PY_VkInline_API void* n_svuvec4_create(const unsigned* v);
	PY_VkInline_API void n_svuvec4_value(void* cptr, unsigned* v);

	PY_VkInline_API void* n_svvec2_create(const float* v);
	PY_VkInline_API void n_svvec2_value(void* cptr, float* v);
	PY_VkInline_API void* n_svvec3_create(const float* v);
	PY_VkInline_API void n_svvec3_value(void* cptr, float* v);
	PY_VkInline_API void* n_svvec4_create(const float* v);
	PY_VkInline_API void n_svvec4_value(void* cptr, float* v);

	PY_VkInline_API void* n_svdvec2_create(const double* v);
	PY_VkInline_API void n_svdvec2_value(void* cptr, double* v);
	PY_VkInline_API void* n_svdvec3_create(const double* v);
	PY_VkInline_API void n_svdvec3_value(void* cptr, double* v);
	PY_VkInline_API void* n_svdvec4_create(const double* v);
	PY_VkInline_API void n_svdvec4_value(void* cptr, double* v);

	PY_VkInline_API void* n_svmat2x2_create(const float* v);
	PY_VkInline_API void n_svmat2x2_value(void* cptr, float* v);
	PY_VkInline_API void* n_svmat2x3_create(const float* v);
	PY_VkInline_API void n_svmat2x3_value(void* cptr, float* v);
	PY_VkInline_API void* n_svmat2x4_create(const float* v);
	PY_VkInline_API void n_svmat2x4_value(void* cptr, float* v);
	PY_VkInline_API void* n_svmat3x2_create(const float* v);
	PY_VkInline_API void n_svmat3x2_value(void* cptr, float* v);
	PY_VkInline_API void* n_svmat3x3_create(const float* v);
	PY_VkInline_API void n_svmat3x3_value(void* cptr, float* v);
	PY_VkInline_API void* n_svmat3x4_create(const float* v);
	PY_VkInline_API void n_svmat3x4_value(void* cptr, float* v);
	PY_VkInline_API void* n_svmat4x2_create(const float* v);
	PY_VkInline_API void n_svmat4x2_value(void* cptr, float* v);
	PY_VkInline_API void* n_svmat4x3_create(const float* v);
	PY_VkInline_API void n_svmat4x3_value(void* cptr, float* v);
	PY_VkInline_API void* n_svmat4x4_create(const float* v);
	PY_VkInline_API void n_svmat4x4_value(void* cptr, float* v);

	PY_VkInline_API void* n_svdmat2x2_create(const double* v);
	PY_VkInline_API void n_svdmat2x2_value(void* cptr, double* v);
	PY_VkInline_API void* n_svdmat2x3_create(const double* v);
	PY_VkInline_API void n_svdmat2x3_value(void* cptr, double* v);
	PY_VkInline_API void* n_svdmat2x4_create(const double* v);
	PY_VkInline_API void n_svdmat2x4_value(void* cptr, double* v);
	PY_VkInline_API void* n_svdmat3x2_create(const double* v);
	PY_VkInline_API void n_svdmat3x2_value(void* cptr, double* v);
	PY_VkInline_API void* n_svdmat3x3_create(const double* v);
	PY_VkInline_API void n_svdmat3x3_value(void* cptr, double* v);
	PY_VkInline_API void* n_svdmat3x4_create(const double* v);
	PY_VkInline_API void n_svdmat3x4_value(void* cptr, double* v);
	PY_VkInline_API void* n_svdmat4x2_create(const double* v);
	PY_VkInline_API void n_svdmat4x2_value(void* cptr, double* v);
	PY_VkInline_API void* n_svdmat4x3_create(const double* v);
	PY_VkInline_API void n_svdmat4x3_value(void* cptr, double* v);
	PY_VkInline_API void* n_svdmat4x4_create(const double* v);
	PY_VkInline_API void n_svdmat4x4_value(void* cptr, double* v);

	// SVBuffer
	PY_VkInline_API void* n_svbuffer_create(const char* elem_type, unsigned long long size, void* hdata);
	PY_VkInline_API const char* n_svbuffer_name_elem_type(void* cptr);
	PY_VkInline_API unsigned long long n_svbuffer_elem_size(void* cptr);
	PY_VkInline_API unsigned long long n_svbuffer_size(void* cptr);
	PY_VkInline_API void n_svbuffer_from_host(void* cptr, void* hdata);
	PY_VkInline_API void n_svbuffer_to_host(void* cptr, void* hdata, unsigned long long begin, unsigned long long end);

	// SVCombine
	PY_VkInline_API void* n_svcombine_create(void* ptr_svs, void* ptr_names, const char* operations);

	// SVObjBuffer
	PY_VkInline_API void* n_svobjbuffer_create(void* ptr_svs);
	PY_VkInline_API const char* n_svobjbuffer_name_elem_type(void* cptr);
	PY_VkInline_API unsigned long long n_svobjbuffer_elem_size(void* cptr);
	PY_VkInline_API unsigned long long n_svobjbuffer_size(void* cptr);
	PY_VkInline_API void n_svobjbuffer_update(void* cptr);

	// Texture2D
	PY_VkInline_API void* n_texture2d_create(int width, int height, unsigned vkformat, unsigned isDepth, unsigned isStencil);
	PY_VkInline_API void n_texture2d_release(void* tex2d);
	PY_VkInline_API int n_texture2d_width(void* _tex2d);
	PY_VkInline_API int n_texture2d_height(void* _tex2d);
	PY_VkInline_API unsigned n_texture2d_pixelsize(void* _tex2d);
	PY_VkInline_API unsigned n_texture2d_channelcount(void* _tex2d);
	PY_VkInline_API unsigned n_texture2d_vkformat(void* _tex2d);
	PY_VkInline_API void n_texture2d_upload(void* _tex2d, void* hdata);
	PY_VkInline_API void n_texture2d_download(void* _tex2d, void* hdata);


}

