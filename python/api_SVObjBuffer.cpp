#include "api.h"
#include "SVObjBuffer.h"
using namespace VkInline;

typedef std::vector<const ShaderViewable*> PtrArray;

void* n_svobjbuffer_create(void* ptr_svs)
{
	PtrArray* elems = (PtrArray*)ptr_svs;
	return new SVObjBuffer(*elems);
}

const char* n_svobjbuffer_name_elem_type(void* cptr)
{
	SVObjBuffer* svbuf = (SVObjBuffer*)cptr;
	return svbuf->name_elem_type().c_str();
}

unsigned long long n_svobjbuffer_elem_size(void* cptr)
{
	SVObjBuffer* svbuf = (SVObjBuffer*)cptr;
	return svbuf->elem_size();
}

unsigned long long n_svobjbuffer_size(void* cptr)
{
	SVObjBuffer* svbuf = (SVObjBuffer*)cptr;
	return svbuf->size();
}

void n_svobjbuffer_update(void* cptr)
{
	SVObjBuffer* svbuf = (SVObjBuffer*)cptr;
	svbuf->update();
}

