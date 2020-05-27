from .Native import ffi, native
from .ShaderViewable import ShaderViewable

class SVObjBuffer(ShaderViewable):
    def __init__(self, lst_svobjs):
        self.lst_svobjs = lst_svobjs
        o_svobjs = ObjArray(lst_svobjs)
        self.m_cptr = native.n_svobjbuffer_create(o_svobjs.m_cptr)

    def name_elem_type(self):
        return ffi.string(native.n_svobjbuffer_name_elem_type(self.m_cptr)).decode('utf-8')

    def elem_size(self):
        return native.n_svobjbuffer_elem_size(self.m_cptr)

    def size(self):
        return native.n_svobjbuffer_size(self.m_cptr)

    def update(self):
        native.n_svobjbuffer_update(self.m_cptr)


