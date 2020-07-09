from .Native import ffi,native
import numbers

class StrArray:
    def __init__(self, arr):
        c_strs = [ffi.from_buffer('char[]', s.encode('utf-8')) for s in arr]
        self.m_cptr = native.n_string_array_create(len(c_strs), c_strs)

    def __del__(self):
        native.n_string_array_destroy(self.m_cptr)

    def size(self):
        return native.n_string_array_size(self.m_cptr)

class ObjArray:
    def __init__(self, arr):
        c_ptrs = [obj.m_cptr for obj in arr]
        self.m_cptr = native.n_pointer_array_create(len(c_ptrs), c_ptrs)
            
    def __del__(self):
        native.n_pointer_array_destroy(self.m_cptr)

    def size(self):
        return native.n_pointer_array_size(self.m_cptr)

class Dim3:
    def __init__(self, t):
        tp = [1,1,1]
        if type(t) is tuple:
            tp[0:len(t)] = t[:]
        else:
            tp[0]=t        

        self.m_cptr = native.n_dim3_create(tp[0],tp[1],tp[2])

    def __del__(self):
        native.n_dim3_destroy(self.m_cptr)

class LaunchParam:
    def __init__(self, obj):
        if isinstance(obj, numbers.Number):            
            self.m_cptr = native.n_launch_param_from_count(obj)
        else:
            self.m_buf = obj.m_buf
            self.m_cptr = native.n_launch_param_from_buffer(obj.m_buf.m_cptr)

    def __del__(self):
        native.n_launch_param_destroy(self.m_cptr)

