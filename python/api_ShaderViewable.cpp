#include "api.h"
#include "ShaderViewable.h"
using namespace VkInline;

const char* n_sv_name_view_type(void* cptr)
{
	ShaderViewable* sv = (ShaderViewable*)cptr;
	return sv->name_view_type().c_str();
}

void n_sv_destroy(void* cptr)
{
	ShaderViewable* sv = (ShaderViewable*)cptr;
	delete sv;
}


void* n_svint32_create(int v)
{
	return new SVInt32(v);
}

int n_svint32_value(void* cptr)
{
	ShaderViewable* sv = (ShaderViewable*)cptr;
	return *(int32_t*)sv->view().data();
}

void* n_svuint32_create(unsigned v)
{
	return new SVUInt32(v);
}

unsigned n_svuint32_value(void* cptr)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	return *(uint32_t*)dv->view().data();
}

void* n_svfloat_create(float v)
{
	return new SVFloat(v);
}

float n_svfloat_value(void* cptr)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	return *(float*)dv->view().data();
}

void* n_svdouble_create(double v)
{
	return new SVDouble(v);
}

double n_svdouble_value(void* cptr)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	return *(double*)dv->view().data();
}


void* n_svivec2_create(const int* v)
{
	return new SVIVec2(v);
}

void n_svivec2_value(void* cptr, int* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(int) * 2);
}

void* n_svivec3_create(const int* v)
{
	return new SVIVec3(v);
}

void n_svivec3_value(void* cptr, int* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(int) * 3);
}

void* n_svivec4_create(const int* v)
{
	return new SVIVec4(v);
}

void n_svivec4_value(void* cptr, int* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(int) * 4);
}


void* n_svuvec2_create(const unsigned* v)
{
	return new SVUVec2(v);
}

void n_svuvec2_value(void* cptr, unsigned* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(unsigned) * 2);
}

void* n_svuvec3_create(const unsigned* v)
{
	return new SVUVec3(v);
}

void n_svuvec3_value(void* cptr, unsigned* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(unsigned) * 3);
}

void* n_svuvec4_create(const unsigned* v)
{
	return new SVUVec4(v);
}

void n_svuvec4_value(void* cptr, unsigned* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(unsigned) * 4);
}

void* n_svvec2_create(const float* v)
{
	return new SVVec2(v);
}

void n_svvec2_value(void* cptr, float* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(float) * 2);
}

void* n_svvec3_create(const float* v)
{
	return new SVVec3(v);
}

void n_svvec3_value(void* cptr, float* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(float) * 3);
}

void* n_svvec4_create(const float* v)
{
	return new SVVec4(v);
}

void n_svvec4_value(void* cptr, float* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(float) * 4);
}


void* n_svdvec2_create(const double* v)
{
	return new SVDVec2(v);
}

void n_svdvec2_value(void* cptr, double* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(double) * 2);
}

void* n_svdvec3_create(const double* v)
{
	return new SVDVec3(v);
}

void n_svdvec3_value(void* cptr, double* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(double) * 3);
}

void* n_svdvec4_create(const double* v)
{
	return new SVDVec4(v);
}

void n_svdvec4_value(void* cptr, double* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(double) * 4);
}

void* n_svmat2x2_create(const float* v)
{
	return new SVMat2x2(v);
}

void n_svmat2x2_value(void* cptr, float* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(float) * 2 * 2);
}


void* n_svmat2x3_create(const float* v)
{
	return new SVMat2x3(v);
}

void n_svmat2x3_value(void* cptr, float* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(float) * 2 * 3);
}

void* n_svmat2x4_create(const float* v)
{
	return new SVMat2x4(v);
}

void n_svmat2x4_value(void* cptr, float* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(float) * 2 * 4);
}

void* n_svmat3x2_create(const float* v)
{
	return new SVMat3x2(v);
}

void n_svmat3x2_value(void* cptr, float* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(float) * 3 * 2);
}

void* n_svmat3x3_create(const float* v)
{
	return new SVMat3x3(v);
}

void n_svmat3x3_value(void* cptr, float* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(float) * 3 * 3);
}

void* n_svmat3x4_create(const float* v)
{
	return new SVMat3x4(v);
}

void n_svmat3x4_value(void* cptr, float* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(float) * 3 * 4);
}

void* n_svmat4x2_create(const float* v)
{
	return new SVMat4x2(v);
}

void n_svmat4x2_value(void* cptr, float* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(float) * 4 * 2);
}


void* n_svmat4x3_create(const float* v)
{
	return new SVMat4x3(v);
}

void n_svmat4x3_value(void* cptr, float* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(float) * 4 * 3);
}

void* n_svmat4x4_create(const float* v)
{
	return new SVMat4x4(v);
}

void n_svmat4x4_value(void* cptr, float* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(float) * 4 * 4);
}

void* n_svdmat2x2_create(const double* v)
{
	return new SVDMat2x2(v);
}

void n_svdmat2x2_value(void* cptr, double* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(double) * 2 * 2);
}


void* n_svdmat2x3_create(const double* v)
{
	return new SVDMat2x3(v);
}

void n_svdmat2x3_value(void* cptr, double* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(double) * 2 * 3);
}

void* n_svdmat2x4_create(const double* v)
{
	return new SVDMat2x4(v);
}

void n_svdmat2x4_value(void* cptr, double* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(double) * 2 * 4);
}

void* n_svdmat3x2_create(const double* v)
{
	return new SVDMat3x2(v);
}

void n_svdmat3x2_value(void* cptr, double* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(double) * 3 * 2);
}

void* n_svdmat3x3_create(const double* v)
{
	return new SVDMat3x3(v);
}

void n_svdmat3x3_value(void* cptr, double* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(double) * 3 * 3);
}

void* n_svdmat3x4_create(const double* v)
{
	return new SVDMat3x4(v);
}

void n_svdmat3x4_value(void* cptr, double* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(double) * 3 * 4);
}

void* n_svdmat4x2_create(const double* v)
{
	return new SVDMat4x2(v);
}

void n_svdmat4x2_value(void* cptr, double* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(double) * 4 * 2);
}


void* n_svdmat4x3_create(const double* v)
{
	return new SVDMat4x3(v);
}

void n_svdmat4x3_value(void* cptr, double* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(double) * 4 * 3);
}

void* n_svdmat4x4_create(const double* v)
{
	return new SVDMat4x4(v);
}

void n_svdmat4x4_value(void* cptr, double* v)
{
	ShaderViewable* dv = (ShaderViewable*)cptr;
	auto view = dv->view();
	memcpy(v, view.data(), sizeof(double) * 4 * 4);
}
