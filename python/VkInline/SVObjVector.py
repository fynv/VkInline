import numpy as np
from .Context import *
from .ShaderViewable import *
from .SVObjBuffer import *
from .SVCombine import SVCombine_Create

class SVObjVector(ShaderViewable):
    def __init__(self, lst_svobjs):
        self.m_size = SVUInt32(len(lst_svobjs))
        self.m_buf = SVObjBuffer(lst_svobjs)
        self.m_cptr = SVCombine_Create({'size':  self.m_size, 'data': self.m_buf},
'''
    uint get_size(in Comb_#hash# vec)
    {
         return vec.size;
    }

    {0} get_value(in Comb_#hash# vec, in uint id)
    {
        return vec.data[id].v;
    }
'''.format(self.elem_type))        	

    def name_elem_type(self):
        return self.m_buf.name_elem_type()

    def elem_size(self):
        return self.m_buf.elem_size()

    def size(self):
        return self.m_buf.size()
