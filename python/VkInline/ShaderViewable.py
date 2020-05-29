from .Native import ffi, native

class ShaderViewable:
    def name_view_type(self):
        return ffi.string(native.n_sv_name_view_type(self.m_cptr)).decode('utf-8')
    def __del__(self):
        native.n_sv_destroy(self.m_cptr)
    def value(self):
        s_type = self.name_view_type()
        return '[Shader-viewable object, type:  %s]'%s_type

class SVInt32(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svint32_create(value)
    def value(self):
        return native.n_svint32_value(self.m_cptr)

class SVUInt32(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svuint32_create(value)
    def value(self):
        return native.n_svuint32_value(self.m_cptr)

class SVFloat(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svfloat_create(value)
    def value(self):
        return native.n_svfloat_value(self.m_cptr)

class SVDouble(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svdouble_create(value)
    def value(self):
        return native.n_svdouble_value(self.m_cptr)   