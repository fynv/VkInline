extern "C"
{
	PY_VkInline_API void* n_blas_create_triangles(void* indBuf, void* posBuf);
	PY_VkInline_API void* n_blas_create_procedure(void* aabbBuf);
	PY_VkInline_API void n_blas_destroy(void* ptr_blas);
	PY_VkInline_API void* n_mat4_create(const float* v);
	PY_VkInline_API void n_mat4_destroy(void* ptr);
	PY_VkInline_API void* n_tlas_create(void* ptr_blases, void* ptr_transes);
	PY_VkInline_API void n_tlas_destroy(void* ptr);
}

