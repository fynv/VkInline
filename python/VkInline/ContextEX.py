import numpy as np
from .Native import ffi, native
from .ShaderViewable import *
from .utils import *

class BaseLevelAS:
    def __init__(self, gpuInd = None, gpuPos = None, gpuAABB = None):
        if gpuAABB == None:
            if gpuInd == None:
                self.m_buf_ind = None
                p_buf_ind = ffi.NULL
            else:
                self.m_buf_ind = gpuInd.m_buf
                p_buf_ind = self.m_buf_ind.m_cptr
            self.m_buf_pos = gpuPos.m_buf
            p_buf_pos = self.m_buf_pos.m_cptr
            self.m_cptr = native.n_blas_create_triangles(p_buf_ind, p_buf_pos)
        else:
            self.m_buf_aabb = gpuAABB.m_buf
            p_buf_aabb = self.m_buf_aabb.m_cptr
            self.m_cptr = native.n_blas_create_procedure(p_buf_aabb)

    def __del__(self):
        native.n_blas_destroy(self.m_cptr)

class Mat4:
    def __init__(self, value):
        self.m_cptr = native.n_mat4_create((value[0].x, value[0].y, value[0].z, value[0].w, value[1].x, value[1].y, value[1].z, value[1].w, value[2].x, value[2].y, value[2].z, value[2].w, value[3].x, value[3].y, value[3].z, value[3].w))

    def __del__(self):
        native.n_mat4_destroy(self.m_cptr)


class TopLevelAS:
    def __init__(self, l2_lst_blas_trans_pairs):
        self.m_lst_lst_blases = [[p[0] for p in lst_i] for lst_i in l2_lst_blas_trans_pairs]
        self.m_lst_lst_transes = [[Mat4(p[1]) for p in lst_i] for lst_i in l2_lst_blas_trans_pairs]
        lst_obj_arr_blases = [ObjArray(lst_i) for lst_i in self.m_lst_lst_blases]
        lst_obj_arr_transes = [ObjArray(lst_i) for lst_i in self.m_lst_lst_transes]
        obj_arr_obj_arr_blases = ObjArray(lst_obj_arr_blases)
        obj_arr_obj_arr_transes = ObjArray(lst_obj_arr_transes)
        self.m_cptr = native.n_tlas_create(obj_arr_obj_arr_blases.m_cptr, obj_arr_obj_arr_transes.m_cptr)

    def __del__(self):
        native.n_tlas_destroy(self.m_cptr)






