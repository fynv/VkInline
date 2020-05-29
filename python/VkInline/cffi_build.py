import os
if os.path.exists('VkInline/cffi.py'):
    os.remove('VkInline/cffi.py')

import cffi
ffibuilder = cffi.FFI()
ffibuilder.set_source("VkInline.cffi", None)

ffibuilder.cdef("""
// utils
void* n_string_array_create(unsigned long long size, const char* const* strs);
unsigned long long n_string_array_size(void* ptr_arr);
void n_string_array_destroy(void* ptr_arr);
void* n_pointer_array_create(unsigned long long size, const void* const* ptrs);
unsigned long long n_pointer_array_size(void* ptr_arr);
void n_pointer_array_destroy(void* ptr_arr);
void* n_dim3_create(unsigned x, unsigned y, unsigned z);
void n_dim3_destroy(void* cptr);

// Context
void n_set_verbose(unsigned verbose);
unsigned long long n_size_of(const char* cls);
void n_add_built_in_header(const char* filename, const char* filecontent);
void n_add_inlcude_filename(const char* fn);
void n_add_code_block(const char* line);
void n_wait(int streamId);

void* n_computer_create(void* ptr_param_list, const char* body);
void n_computer_destroy(void* cptr);
int n_computer_num_params(void* cptr);
int n_computer_launch(void* ptr_kernel, void* ptr_gridDim, void* ptr_blockDim, void* ptr_arg_list, int streamId);

// ShaderViewable
const char* n_sv_name_view_type(void* cptr);
void n_sv_destroy(void* cptr);
void* n_svint32_create(int v);
int n_svint32_value(void* cptr);
void* n_svuint32_create(unsigned v);
unsigned n_svuint32_value(void* cptr);
void* n_svfloat_create(float v);
float n_svfloat_value(void* cptr);
void* n_svdouble_create(double v);
double n_svdouble_value(void* cptr);

// SVBuffer
void* n_svbuffer_create(const char* elem_type, unsigned long long size, void* hdata, int streamId);
const char* n_svbuffer_name_elem_type(void* cptr);
unsigned long long n_svbuffer_elem_size(void* cptr);
unsigned long long n_svbuffer_size(void* cptr);
void n_svbuffer_from_host(void* cptr, void* hdata, int streamId);
void n_svbuffer_to_host(void* cptr, void* hdata, unsigned long long begin, unsigned long long end, int streamId);

// SVCombine
void* n_svcombine_create(void* ptr_svs, void* ptr_names, const char* operations);

// SVObjBuffer
void* n_svobjbuffer_create(void* ptr_svs);
const char* n_svobjbuffer_name_elem_type(void* cptr);
unsigned long long n_svobjbuffer_elem_size(void* cptr);
unsigned long long n_svobjbuffer_size(void* cptr);
void n_svobjbuffer_update(void* cptr);
""")


ffibuilder.compile()

