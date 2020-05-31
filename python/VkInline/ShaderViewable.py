from .Native import ffi, native
import struct
import glm

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

class SVIVec2(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svivec2_create((value.x, value.y))
    def value(self):
        v = b'\x00'*8
        native.n_svivec2_value(self.m_cptr, ffi.from_buffer('int[]', v))
        return glm.ivec2(struct.unpack('2i', v))

class SVIVec3(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svivec3_create((value.x, value.y, value.z))
    def value(self):
        v = b'\x00'*12
        native.n_svivec3_value(self.m_cptr, ffi.from_buffer('int[]', v))
        return glm.ivec3(struct.unpack('3i', v))

class SVIVec4(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svivec4_create((value.x, value.y, value.z, value.w))
    def value(self):
        v = b'\x00'*16
        native.n_svivec4_value(self.m_cptr, ffi.from_buffer('int[]', v))
        return glm.ivec4(struct.unpack('4i', v))

class SVUVec2(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svuvec2_create((value.x, value.y))
    def value(self):
        v = b'\x00'*8
        native.n_svuvec2_value(self.m_cptr, ffi.from_buffer('unsigned[]', v))
        return glm.uvec2(struct.unpack('2I', v))

class SVUVec3(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svuvec3_create((value.x, value.y, value.z))
    def value(self):
        v = b'\x00'*12
        native.n_svuvec3_value(self.m_cptr, ffi.from_buffer('unsigned[]', v))
        return glm.uvec3(struct.unpack('3I', v))

class SVUVec4(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svuvec4_create((value.x, value.y, value.z, value.w))
    def value(self):
        v = b'\x00'*16
        native.n_svuvec4_value(self.m_cptr, ffi.from_buffer('unsigned[]', v))
        return glm.uvec4(struct.unpack('4I', v))

class SVVec2(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svvec2_create((value.x, value.y))
    def value(self):
        v = b'\x00'*8
        native.n_svvec2_value(self.m_cptr, ffi.from_buffer('float[]', v))
        return glm.vec2(struct.unpack('2f', v))

class SVVec3(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svvec3_create((value.x, value.y, value.z))
    def value(self):
        v = b'\x00'*12
        native.n_svvec3_value(self.m_cptr, ffi.from_buffer('float[]', v))
        return glm.vec3(struct.unpack('3f', v))

class SVVec4(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svvec4_create((value.x, value.y, value.z, value.w))
    def value(self):
        v = b'\x00'*16
        native.n_svvec4_value(self.m_cptr, ffi.from_buffer('float[]', v))
        return glm.vec4(struct.unpack('4f', v))

class SVDVec2(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svdvec2_create((value.x, value.y))
    def value(self):
        v = b'\x00'*16
        native.n_svdvec2_value(self.m_cptr, ffi.from_buffer('double[]', v))
        return glm.dvec2(struct.unpack('2d', v))

class SVDVec3(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svdvec3_create((value.x, value.y, value.z))
    def value(self):
        v = b'\x00'*24
        native.n_svdvec3_value(self.m_cptr, ffi.from_buffer('double[]', v))
        return glm.dvec3(struct.unpack('3d', v))

class SVDVec4(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svdvec4_create((value.x, value.y, value.z, value.w))
    def value(self):
        v = b'\x00'*32
        native.n_svdvec4_value(self.m_cptr, ffi.from_buffer('double[]', v))
        return glm.dvec4(struct.unpack('4d', v))

