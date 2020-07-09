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


class HitShaders:
    def __init__(self, closest_hit, intersection=None):
        p_closest_hit = closest_hit.encode('utf-8')
        if intersection == None:
            p_intersection = ffi.NULL
        else:
            p_intersection = intersection.encode('utf-8')
        self.m_cptr = native.n_hit_shaders_create(p_closest_hit, p_intersection)

    def __del__(self):
        native.n_hit_shaders_destroy(self.m_cptr)


class RayTracer:
    def __init__(self, param_names, body_raygen, lst_body_miss, lst_body_hit, max_recursion_depth=1, type_locked=False):
        o_param_names = StrArray(param_names)
        o_body_miss = StrArray(lst_body_miss)
        self.m_lst_body_hit = lst_body_hit
        o_body_hit = ObjArray(lst_body_hit)
        self.m_cptr = native.n_raytracer_create(o_param_names.m_cptr, body_raygen.encode('utf-8'), o_body_miss.m_cptr, o_body_hit.m_cptr, max_recursion_depth, type_locked)

    def __del__(self):
        native.n_raytracer_destroy(self.m_cptr)

    def num_params(self):
        return native.n_raytracer_num_params(self.m_cptr)

    def launch(self, glbDim, args, lst_tlas, tex2ds=[], tex3ds=[], times_submission = 1):
        d_glbDim = Dim3(glbDim)
        arg_list = ObjArray(args)
        tlas_list = ObjArray(lst_tlas)
        tex2d_list = ObjArray(tex2ds)
        tex3d_list = ObjArray(tex3ds)
        native.n_raytracer_launch(
            self.m_cptr, 
            d_glbDim.m_cptr,         
            arg_list.m_cptr,
            tlas_list.m_cptr,
            tex2d_list.m_cptr,
            tex3d_list.m_cptr,
            times_submission)


