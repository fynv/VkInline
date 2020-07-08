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
