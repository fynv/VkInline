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
        shape = [1, 1, 1]
        if elem_type=='int':
            nptype = np.int32
        elif elem_type=='uint':
            nptype = np.uint32        
        elif elem_type=='float':
            nptype = np.float32
        elif elem_type=='double':
            nptype = np.float64

        elif elem_type=='ivec2':
            nptype = np.int32
            shape[2] = 2
        elif elem_type=='ivec3':
            nptype = np.int32
            shape[2] = 3
        elif elem_type=='ivec4':
            nptype = np.int32
            shape[2] = 4

        elif elem_type=='uvec2':
            nptype = np.uint32
            shape[2] = 2
        elif elem_type=='uvec3':
            nptype = np.uint32
            shape[2] = 3
        elif elem_type=='uvec4':
            nptype = np.uint32
            shape[2] = 4            

        elif elem_type=='vec2':
            nptype = np.float32
            shape[2] = 2
        elif elem_type=='vec3':
            nptype = np.float32
            shape[2] = 3
        elif elem_type=='vec4':
            nptype = np.float32
            shape[2] = 4

        elif elem_type=='dvec2':
            nptype = np.float64
            shape[2] = 2
        elif elem_type=='dvec3':
            nptype = np.float64
            shape[2] = 3
        elif elem_type=='dvec4':
            nptype = np.float64
            shape[2] = 4

        if end == -1:
            end = self.size()
        shape[0] = end - begin
        if shape[1] == 1:
            if shape[2] == 1: # scalar
                ret = np.empty(shape[0], dtype=nptype)
            else: # vec
                ret = np.empty((shape[0], shape[2]), dtype=nptype)
        else: # matrix
            pass
        self.m_buf.to_host(ret.__array_interface__['data'][0], begin, end, streamId)
        return ret

def device_vector_from_numpy(nparr, streamId=0):
    shape = nparr.shape
    if len(shape)<2:
        shape = [shape[0], 1, 1]
    elif len(shape)<3:
        shape = [shape[0], 1, shape[1]]

    if nparr.dtype == np.int32:
        if shape[1]==1:
            if shape[2] == 1:
                elem_type = 'int'
            elif shape[2] == 2:
                elem_type = 'ivec2'
            elif shape[2] == 3:
                elem_type = 'ivec3'
            elif shape[2] == 4:
                elem_type = 'ivec4'

    elif nparr.dtype == np.uint32:
        if shape[1]==1:
            if shape[2] == 1:
                elem_type = 'uint'
            elif shape[2] == 2:
                elem_type = 'uvec2'
            elif shape[2] == 3:
                elem_type = 'uvec3'
            elif shape[2] == 4:
                elem_type = 'uvec4'

    elif nparr.dtype == np.float32:
        if shape[1]==1:
            if shape[2] == 1:
                elem_type = 'float'
            elif shape[2] == 2:
                elem_type = 'vec2'
            elif shape[2] == 3:
                elem_type = 'vec3'
            elif shape[2] == 4:
                elem_type = 'vec4'

    elif nparr.dtype == np.float64:
        if shape[1]==1:
            if shape[2] == 1:
                elem_type = 'double'
            elif shape[2] == 2:
                elem_type = 'dvec2'
            elif shape[2] == 3:
                elem_type = 'dvec3'
            elif shape[2] == 4:
                elem_type = 'dvec4'

    ptr_host_data = nparr.__array_interface__['data'][0]
    return SVVector(elem_type, shape[0], ptr_host_data, streamId)

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
    return device_vector_from_numpy(nparr, streamId)
