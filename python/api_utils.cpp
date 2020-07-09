#include "api.h"
#include "Context.h"
#include "SVBuffer.h"
using namespace VkInline;
#include <string>
#include <vector>

typedef std::vector<std::string> StrArray;
typedef std::vector<const void*> PtrArray;
typedef std::vector<Texture2D*> Tex2DArray;
typedef std::vector<Texture3D*> Tex3DArray;

void* n_string_array_create(unsigned long long size, const char* const* strs)
{
	StrArray* ret = new StrArray(size);
	for (size_t i = 0; i < size; i++)
		(*ret)[i] = strs[i];

	return ret;
}

unsigned long long n_string_array_size(void* ptr_arr)
{
	StrArray* arr = (StrArray*)ptr_arr;
	return arr->size();
}

void n_string_array_destroy(void* ptr_arr)
{
	StrArray* arr = (StrArray*)ptr_arr;
	delete arr;
}

void* n_pointer_array_create(unsigned long long size, const void* const* ptrs)
{
	PtrArray* ret = new PtrArray(size);
	memcpy(ret->data(), ptrs, sizeof(void*)*size);
	return ret;
}

unsigned long long n_pointer_array_size(void* ptr_arr)
{
	PtrArray* arr = (PtrArray*)ptr_arr;
	return arr->size();
}

void n_pointer_array_destroy(void* ptr_arr)
{
	PtrArray* arr = (PtrArray*)ptr_arr;
	delete arr;
}

void* n_dim3_create(unsigned x, unsigned y, unsigned z)
{
	dim_type* ret = new dim_type({ x,y,z });
	return ret;
}

void n_dim3_destroy(void* cptr)
{
	dim_type* v = (dim_type*)cptr;
	delete v;
}

void* n_launch_param_from_count(unsigned count)
{
	return new Rasterizer::LaunchParam({ count, nullptr });
}

void* n_launch_param_from_buffer(void* _buf)
{
	SVBuffer* buf = (SVBuffer*)_buf;
	return new Rasterizer::LaunchParam({ (unsigned)buf->size(), buf });
}

void n_launch_param_destroy(void* lp)
{
	delete (Rasterizer::LaunchParam*)lp;
}


