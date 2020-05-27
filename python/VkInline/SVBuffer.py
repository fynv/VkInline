from .Native import ffi, native
from .ShaderViewable import ShaderViewable

class SVBuffer(ShaderViewable):
    def __init__(self, elem_type, size, ptr_host_data=None, streamId=0):
        ffiptr = ffi.NULL
        if ptr_host_data!=None:
            ffiptr = ffi.cast("void *", ptr_host_data)
        self.m_cptr = native.n_svbuffer_create(elem_type.encode('utf-8'), size, ffiptr, streamId)

    def name_elem_type(self):
        return ffi.string(native.n_svbuffer_name_elem_type(self.m_cptr)).decode('utf-8')

    def elem_size(self):
        return native.n_svbuffer_elem_size(self.m_cptr)

    def size(self):
        return native.n_svbuffer_size(self.m_cptr)

    def from_host(self, ptr_host_data, streamId=0):
        native.n_svbuffer_from_host(self.m_cptr, ffi.cast("void *", ptr_host_data), streamId)

    def to_host(self, ptr_host_data, begin = 0, end = -1, streamId=0):
        native.n_svbuffer_to_host(self.m_cptr, ffi.cast("void *", ptr_host_data), begin, end, streamId)

    