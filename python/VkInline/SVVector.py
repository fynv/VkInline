import numpy as np
from .Context import *
from .ShaderViewable import *
from .SVBuffer import *
from .SVCombine import SVCombine_Create

class SVVector(ShaderViewable):
    def __init__(self, elem_type, size, ptr_host_data=None, streamId=0):
        self.m_size = SVUInt32(size)
        self.m_buf = SVBuffer(elem_type, size, ptr_host_data, streamId)
        self.m_cptr = SVCombine_Create({'size':  self.m_size, 'data': self.m_buf},
'''
uint get_size(in Comb_#hash# vec)
{{
     return vec.size;
}}

{0} get_value(in Comb_#hash# vec, in uint id)
{{
    return vec.data[id].v;
}}

void set_value(in Comb_#hash# vec, in uint id, in {0} value)
{{
    vec.data[id].v = value;
}}
'''.format(elem_type))

    def name_elem_type(self):
        return self.m_buf.name_elem_type()

    def elem_size(self):
        return self.m_buf.elem_size()

    def size(self):
        return self.m_buf.size()

    def to_host(self, begin = 0, end = -1, streamId=0):
        elem_type = self.name_elem_type()
        if elem_type=='int':
            nptype = np.int32
        elif elem_type=='uint':
            nptype = np.uint32        
        elif elem_type=='float':
            nptype = np.float32
        elif elem_type=='double':
            nptype = np.float64
        if end == -1:
            end = self.size()
        ret = np.empty(end - begin, dtype=nptype)
        self.m_buf.to_host(ret.__array_interface__['data'][0], begin, end, streamId)
        return ret

def device_vector_from_numpy(nparr, streamId=0):
    if nparr.dtype == np.int32:
        elem_type = 'int'
    elif nparr.dtype == np.uint32:
        elem_type = 'uint'       
    elif nparr.dtype == np.float32:
        elem_type = 'float'
    elif nparr.dtype == np.float64:
        elem_type = 'double'
    size = len(nparr)
    ptr_host_data = nparr.__array_interface__['data'][0]
    return SVVector(elem_type, size, ptr_host_data, streamId)

def device_vector_from_list(lst, elem_type, streamId=0):
    if elem_type=='int':
        nptype = np.int32
    elif elem_type=='uint':
        nptype = np.uint32
    elif elem_type=='float':
        nptype = np.float32
    elif elem_type=='double':
        nptype = np.float64
    nparr = np.array(lst, dtype=nptype)
    size = len(lst)
    ptr_host_data = nparr.__array_interface__['data'][0]
    return SVVector(elem_type, size, ptr_host_data, streamId)
