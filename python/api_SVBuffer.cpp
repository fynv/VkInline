#include "api.h"
#include "SVBuffer.h"
using namespace VkInline;

void* n_svbuffer_create(const char* elem_type, unsigned long long size, void* hdata)
{
	return new SVBuffer(elem_type, size, hdata);
}

const char* n_svbuffer_name_elem_type(void* cptr)
{
	SVBuffer* svbuf = (SVBuffer*)cptr;
	return svbuf->name_elem_type().c_str();
}

unsigned long long n_svbuffer_elem_size(void* cptr)
{
	SVBuffer* svbuf = (SVBuffer*)cptr;
	return svbuf->elem_size();
}

unsigned long long n_svbuffer_size(void* cptr)
{
	SVBuffer* svbuf = (SVBuffer*)cptr;
	return svbuf->size();
}

void n_svbuffer_from_host(void* cptr, void* hdata)
{
	SVBuffer* svbuf = (SVBuffer*)cptr;
	svbuf->from_host(hdata);
}

void n_svbuffer_to_host(void* cptr, void* hdata, unsigned long long begin, unsigned long long end)
{
	SVBuffer* svbuf = (SVBuffer*)cptr;
	svbuf->to_host(hdata, begin, end);
}


