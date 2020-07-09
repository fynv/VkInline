void* n_blas_create_triangles(void* indBuf, void* posBuf)
{
	return new BaseLevelAS((SVBuffer*)indBuf, (SVBuffer*)posBuf);
}

void* n_blas_create_procedure(void* aabbBuf)
{
	return new BaseLevelAS((SVBuffer*)aabbBuf);
}

void n_blas_destroy(void* ptr_blas)
{
	delete (BaseLevelAS*)ptr_blas;
}

void* n_mat4_create(const float* v)
{
	return new Mat4(v);
}

void n_mat4_destroy(void* ptr)
{
	delete (Mat4*)ptr;
}

void* n_tlas_create(void* ptr_blases, void* ptr_transes)
{
	std::vector<std::vector<const BaseLevelAS*>*>* blases = (std::vector<std::vector<const BaseLevelAS*>*>*)ptr_blases;
	std::vector<std::vector<const Mat4*>*>* transes = (std::vector<std::vector<const Mat4*>*>*)ptr_transes;
	size_t num_hitgroups = blases->size();
	std::vector<std::vector<BLAS_EX>> blasex(num_hitgroups);
	for (size_t i = 0; i < num_hitgroups; i++)
	{
		std::vector<const BaseLevelAS*>* pblases = (*blases)[i];
		std::vector<const Mat4*>* ptranses = (*transes)[i];
		size_t num_blases = pblases->size();
		blasex[i].resize(num_blases);
		for (size_t j = 0; j < num_blases; j++)
		{
			blasex[i][j].blas = (*pblases)[j];
			blasex[i][j].trans = (*ptranses)[j];
		}
	}
	return new TopLevelAS(blasex);
}

void n_tlas_destroy(void* ptr)
{
	delete (TopLevelAS*)ptr;
}

void* n_hit_shaders_create(const char* closest_hit, const char* intersection)
{
	return new BodyHitShaders(closest_hit, intersection);
}

void n_hit_shaders_destroy(void* ptr)
{
	delete (BodyHitShaders*)ptr;
}

void* n_raytracer_create(void* ptr_param_list, const char* body_raygen, void* ptr_body_miss, void* ptr_body_hit, unsigned maxRecursionDepth, unsigned type_locked)
{
	StrArray* param_list = (StrArray*)ptr_param_list;
	size_t num_params = param_list->size();
	StrArray* body_miss = (StrArray*)ptr_body_miss;
	size_t num_miss = body_miss->size();
	std::vector<const BodyHitShaders*>* body_hit = (std::vector<const BodyHitShaders*>*)ptr_body_hit;

	std::vector<const char*> params(num_params);
	for (size_t i = 0; i < num_params; i++)
		params[i] = (*param_list)[i].c_str();

	std::vector<const char*> body_misses(num_miss);
	for (size_t i = 0; i < num_miss; i++)
		body_misses[i] = (*body_miss)[i].c_str();

	return new RayTracer(params, body_raygen, body_misses, *body_hit, maxRecursionDepth, type_locked != 0);
}

void n_raytracer_destroy(void* cptr)
{
	delete (RayTracer*)cptr;
}

int n_raytracer_num_params(void* cptr)
{
	RayTracer* raytracer = (RayTracer*)cptr;
	return (int)raytracer->num_params();
}

int n_raytracer_launch(void* ptr_raytracer, void* ptr_glbDim, void* ptr_arg_list, void* ptr_tlas_list, void* ptr_tex2d_list, void* ptr_tex3d_list, unsigned times_submission)
{
	RayTracer* raytracer = (RayTracer*)ptr_raytracer;
	size_t num_params = raytracer->num_params();

	dim_type* glbDim = (dim_type*)ptr_glbDim;

	PtrArray* arg_list = (PtrArray*)ptr_arg_list;
	std::vector<TopLevelAS*>* tlas_list = (std::vector<TopLevelAS*>*)ptr_tlas_list;
	Tex2DArray* tex2d_list = (Tex2DArray*)ptr_tex2d_list;
	Tex3DArray* tex3d_list = (Tex3DArray*)ptr_tex3d_list;

	size_t size = arg_list->size();
	if (num_params != size)
	{
		printf("Wrong number of arguments received. %d required, %d received.", (int)num_params, (int)size);
		return -1;
	}

	if (raytracer->launch(*glbDim, arg_list->data(), *tlas_list, *tex2d_list, *tex3d_list, times_submission))
		return 0;
	else
		return -1;
}



