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

class SVMat2x2(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svmat2x2_create((value[0].x, value[0].y, value[1].x, value[1].y))
    def value(self):
        v = b'\x00'*16
        native.n_svmat2x2_value(self.m_cptr, ffi.from_buffer('float[]', v))
        elems = struct.unpack('4f', v)
        return glm.mat2x2(elems[0:2], elems[2:4])

class SVMat2x3(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svmat2x3_create((value[0].x, value[0].y, value[0].z, value[1].x, value[1].y, value[1].z))
    def value(self):
        v = b'\x00'*24
        native.n_svmat2x3_value(self.m_cptr, ffi.from_buffer('float[]', v))
        elems = struct.unpack('6f', v)
        return glm.mat2x3(elems[0:3], elems[3:6])

class SVMat2x4(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svmat2x4_create((value[0].x, value[0].y, value[0].z, value[0].w, value[1].x, value[1].y, value[1].z, value[1].w))
    def value(self):
        v = b'\x00'*32
        native.n_svmat2x4_value(self.m_cptr, ffi.from_buffer('float[]', v))
        elems = struct.unpack('8f', v)
        return glm.mat2x4(elems[0:4], elems[4:8])

class SVMat3x2(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svmat3x2_create((value[0].x, value[0].y, value[1].x, value[1].y, value[2].x, value[2].y))
    def value(self):
        v = b'\x00'*24
        native.n_svmat3x2_value(self.m_cptr, ffi.from_buffer('float[]', v))
        elems = struct.unpack('6f', v)
        return glm.mat3x2(elems[0:2], elems[2:4], elems[4:6])

class SVMat3x3(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svmat3x3_create((value[0].x, value[0].y, value[0].z, value[1].x, value[1].y, value[1].z, value[2].x, value[2].y, value[2].z))
    def value(self):
        v = b'\x00'*36
        native.n_svmat3x3_value(self.m_cptr, ffi.from_buffer('float[]', v))
        elems = struct.unpack('9f', v)
        return glm.mat3x3(elems[0:3], elems[3:6], elems[6:9])

class SVMat3x4(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svmat3x4_create((value[0].x, value[0].y, value[0].z, value[0].w, value[1].x, value[1].y, value[1].z, value[1].w, value[2].x, value[2].y, value[2].z, value[2].w))
    def value(self):
        v = b'\x00'*48
        native.n_svmat3x4_value(self.m_cptr, ffi.from_buffer('float[]', v))
        elems = struct.unpack('12f', v)
        return glm.mat3x4(elems[0:4], elems[4:8], elems[8:12])

class SVMat4x2(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svmat4x2_create((value[0].x, value[0].y, value[1].x, value[1].y, value[2].x, value[2].y, value[3].x, value[3].y))
    def value(self):
        v = b'\x00'*32
        native.n_svmat4x2_value(self.m_cptr, ffi.from_buffer('float[]', v))
        elems = struct.unpack('8f', v)
        return glm.mat4x2(elems[0:2], elems[2:4], elems[4:6], elems[6:8])

class SVMat4x3(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svmat4x3_create((value[0].x, value[0].y, value[0].z, value[1].x, value[1].y, value[1].z, value[2].x, value[2].y, value[2].z, value[3].x, value[3].y, value[3].z))
    def value(self):
        v = b'\x00'*48
        native.n_svmat4x3_value(self.m_cptr, ffi.from_buffer('float[]', v))
        elems = struct.unpack('12f', v)
        return glm.mat4x3(elems[0:3], elems[3:6], elems[6:9], elems[9:12])

class SVMat4x4(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svmat4x4_create((value[0].x, value[0].y, value[0].z, value[0].w, value[1].x, value[1].y, value[1].z, value[1].w, value[2].x, value[2].y, value[2].z, value[2].w, value[3].x, value[3].y, value[3].z, value[3].w))
    def value(self):
        v = b'\x00'*64
        native.n_svmat4x4_value(self.m_cptr, ffi.from_buffer('float[]', v))
        elems = struct.unpack('16f', v)
        return glm.mat4x4(elems[0:4], elems[4:8], elems[8:12], elems[12:16])

class SVDMat2x2(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svdmat2x2_create((value[0].x, value[0].y, value[1].x, value[1].y))
    def value(self):
        v = b'\x00'*32
        native.n_svdmat2x2_value(self.m_cptr, ffi.from_buffer('double[]', v))
        elems = struct.unpack('4d', v)
        return glm.dmat2x2(elems[0:2], elems[2:4])

class SVDMat2x3(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svdmat2x3_create((value[0].x, value[0].y, value[0].z, value[1].x, value[1].y, value[1].z))
    def value(self):
        v = b'\x00'*48
        native.n_svdmat2x3_value(self.m_cptr, ffi.from_buffer('double[]', v))
        elems = struct.unpack('6d', v)
        return glm.dmat2x3(elems[0:3], elems[3:6])

class SVDMat2x4(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svdmat2x4_create((value[0].x, value[0].y, value[0].z, value[0].w, value[1].x, value[1].y, value[1].z, value[1].w))
    def value(self):
        v = b'\x00'*64
        native.n_svdmat2x4_value(self.m_cptr, ffi.from_buffer('double[]', v))
        elems = struct.unpack('8d', v)
        return glm.dmat2x4(elems[0:4], elems[4:8])

class SVDMat3x2(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svdmat3x2_create((value[0].x, value[0].y, value[1].x, value[1].y, value[2].x, value[2].y))
    def value(self):
        v = b'\x00'*48
        native.n_svdmat3x2_value(self.m_cptr, ffi.from_buffer('double[]', v))
        elems = struct.unpack('6d', v)
        return glm.dmat3x2(elems[0:2], elems[2:4], elems[4:6])

class SVDMat3x3(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svdmat3x3_create((value[0].x, value[0].y, value[0].z, value[1].x, value[1].y, value[1].z, value[2].x, value[2].y, value[2].z))
    def value(self):
        v = b'\x00'*72
        native.n_svdmat3x3_value(self.m_cptr, ffi.from_buffer('double[]', v))
        elems = struct.unpack('9d', v)
        return glm.dmat3x3(elems[0:3], elems[3:6], elems[6:9])

class SVDMat3x4(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svdmat3x4_create((value[0].x, value[0].y, value[0].z, value[0].w, value[1].x, value[1].y, value[1].z, value[1].w, value[2].x, value[2].y, value[2].z, value[2].w))
    def value(self):
        v = b'\x00'*96
        native.n_svdmat3x4_value(self.m_cptr, ffi.from_buffer('double[]', v))
        elems = struct.unpack('12d', v)
        return glm.dmat3x4(elems[0:4], elems[4:8], elems[8:12])

class SVDMat4x2(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svdmat4x2_create((value[0].x, value[0].y, value[1].x, value[1].y, value[2].x, value[2].y, value[3].x, value[3].y))
    def value(self):
        v = b'\x00'*64
        native.n_svdmat4x2_value(self.m_cptr, ffi.from_buffer('double[]', v))
        elems = struct.unpack('8d', v)
        return glm.dmat4x2(elems[0:2], elems[2:4], elems[4:6], elems[6:8])

class SVDMat4x3(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svdmat4x3_create((value[0].x, value[0].y, value[0].z, value[1].x, value[1].y, value[1].z, value[2].x, value[2].y, value[2].z, value[3].x, value[3].y, value[3].z))
    def value(self):
        v = b'\x00'*96
        native.n_svdmat4x3_value(self.m_cptr, ffi.from_buffer('double[]', v))
        elems = struct.unpack('12d', v)
        return glm.dmat4x3(elems[0:3], elems[3:6], elems[6:9], elems[9:12])

class SVDMat4x4(ShaderViewable):
    def __init__(self, value):
        self.m_cptr = native.n_svdmat4x4_create((value[0].x, value[0].y, value[0].z, value[0].w, value[1].x, value[1].y, value[1].z, value[1].w, value[2].x, value[2].y, value[2].z, value[2].w, value[3].x, value[3].y, value[3].z, value[3].w))
    def value(self):
        v = b'\x00'*128
        native.n_svdmat4x4_value(self.m_cptr, ffi.from_buffer('double[]', v))
        elems = struct.unpack('16d', v)
        return glm.dmat4x4(elems[0:4], elems[4:8], elems[8:12], elems[12:16])
