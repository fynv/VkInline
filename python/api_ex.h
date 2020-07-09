extern "C"
{
	PY_VkInline_API void* n_blas_create_triangles(void* indBuf, void* posBuf);
	PY_VkInline_API void* n_blas_create_procedure(void* aabbBuf);
	PY_VkInline_API void n_blas_destroy(void* ptr_blas);
	PY_VkInline_API void* n_mat4_create(const float* v);
	PY_VkInline_API void n_mat4_destroy(void* ptr);
	PY_VkInline_API void* n_tlas_create(void* ptr_blases, void* ptr_transes);
	PY_VkInline_API void n_tlas_destroy(void* ptr);
	PY_VkInline_API void* n_hit_shaders_create(const char* closest_hit, const char* intersection);
	PY_VkInline_API void n_hit_shaders_destroy(void* ptr);
	PY_VkInline_API void* n_raytracer_create(void* ptr_param_list, const char* body_raygen, void* ptr_body_miss, void* ptr_body_hit, unsigned maxRecursionDepth, unsigned type_locked);
	PY_VkInline_API void n_raytracer_destroy(void* cptr);
	PY_VkInline_API int n_raytracer_num_params(void* cptr);
	PY_VkInline_API int n_raytracer_launch(void* ptr_raytracer, void* ptr_glbDim, void* ptr_arg_list, void* ptr_tlas_list, void* ptr_tex2d_list, void* ptr_tex3d_list, unsigned times_submission);
}

