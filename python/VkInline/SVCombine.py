from .Native import ffi, native
from .utils import *

def SVCombine_Create(elem_map, operations):
    param_names = [param_name for param_name, elem in elem_map.items()]
    o_param_names = StrArray(param_names)
    elems = [elem for param_name, elem in elem_map.items()]
    o_elems = ObjArray(elems)
    return native.n_svcombine_create(o_elems.m_cptr, o_param_names.m_cptr, operations.encode('utf-8'))

    