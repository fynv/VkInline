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



