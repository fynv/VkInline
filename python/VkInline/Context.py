import numpy as np
from .Native import ffi, native
from .ShaderViewable import *
from .utils import *

def Set_Verbose(verbose=True):
    native.n_set_verbose(verbose)

def Size_Of(clsname):
    return native.n_size_of(clsname.encode('utf-8'))

def Add_Built_In_Header(filename, filecontent):
    native.n_add_built_in_header(filename.encode('utf-8'), filecontent.encode('utf-8'))

def Add_Inlcude_Filename(filename):
    native.n_add_inlcude_filename(filename.encode('utf-8'))

def Add_Code_Block(code):
    native.n_add_code_block(code.encode('utf-8'))

def Wait():
    native.n_wait()

class Computer:
    def __init__(self, param_names, body, type_locked=False):
        o_param_names = StrArray(param_names)
        self.m_cptr = native.n_computer_create(o_param_names.m_cptr, body.encode('utf-8'), type_locked)

    def __del__(self):
        native.n_computer_destroy(self.m_cptr)

    def num_params(self):
        return native.n_computer_num_params(self.m_cptr)

    def launch(self, gridDim, blockDim, args, tex2ds=[], tex3ds=[], cubemaps=[], times_submission = 1):
        d_gridDim = Dim3(gridDim)
        d_blockDim = Dim3(blockDim)
        arg_list = ObjArray(args)
        tex2d_list = ObjArray(tex2ds)
        tex3d_list = ObjArray(tex3ds)
        cubemap_list = ObjArray(cubemaps)
        native.n_computer_launch(
            self.m_cptr, 
            d_gridDim.m_cptr, 
            d_blockDim.m_cptr, 
            arg_list.m_cptr,
            tex2d_list.m_cptr,
            tex3d_list.m_cptr,
            cubemap_list.m_cptr,
            times_submission)

class For:
    def __init__(self, param_names, name_inner, body, block_size=128, type_locked=False):
        self.block_size = block_size
        param_names = param_names + ['_begin', '_end']
        o_param_names = StrArray(param_names)
        whole_body = body + '''
void main()
{{
    uint id = gl_GlobalInvocationID.x + _begin;
    if(id>=_end) return;
    {0}(id);
}}
'''.format(name_inner)
        self.m_cptr = native.n_computer_create(o_param_names.m_cptr, whole_body.encode('utf-8'), type_locked)

    def __del__(self):
        native.n_computer_destroy(self.m_cptr)

    def num_params(self):
        return native.n_computer_num_params(self.m_cptr) - 2

    def launch(self, begin, end, args, tex2ds=[], tex3ds=[], cubemaps=[], times_submission = 1): 
        svbegin = SVUInt32(begin)
        svend = SVUInt32(end)
        args = args + [svbegin, svend]
        numBlocks = int((end - begin + self.block_size - 1) / self.block_size)
        d_gridDim = Dim3(numBlocks)
        d_blockDim = Dim3(self.block_size)
        arg_list = ObjArray(args)
        tex2d_list = ObjArray(tex2ds)
        tex3d_list = ObjArray(tex3ds)
        cubemap_list = ObjArray(cubemaps)
        native.n_computer_launch(
            self.m_cptr, 
            d_gridDim.m_cptr, 
            d_blockDim.m_cptr, 
            arg_list.m_cptr,
            tex2d_list.m_cptr,
            tex3d_list.m_cptr,
            cubemap_list.m_cptr,
            times_submission)

    def launch_n(self, n, args, tex2ds=[], tex3ds=[], cubemaps=[], times_submission = 1):
        svbegin = SVUInt32(0)
        svend = SVUInt32(n)
        args = args + [svbegin, svend]
        numBlocks = int((n + self.block_size - 1) / self.block_size)
        d_gridDim = Dim3(numBlocks)
        d_blockDim = Dim3(self.block_size)
        arg_list = ObjArray(args)
        tex2d_list = ObjArray(tex2ds)
        tex3d_list = ObjArray(tex3ds)
        cubemap_list = ObjArray(cubemaps)
        native.n_computer_launch(
            self.m_cptr, 
            d_gridDim.m_cptr, 
            d_blockDim.m_cptr, 
            arg_list.m_cptr,
            tex2d_list.m_cptr,
            tex3d_list.m_cptr,
            cubemap_list.m_cptr,
            times_submission)      

class DrawCall:
    def __init__(self, code_body_vert, code_body_frag, options={}):
        self.m_cptr = native.n_drawcall_create(code_body_vert.encode('utf-8'), code_body_frag.encode('utf-8'))

        if 'primitive_topology' in options:
            native.n_drawcall_set_primitive_topology(self.m_cptr, options['primitive_topology'])

        if 'primitive_restart' in options:
            native.n_drawcall_set_primitive_restart(self.m_cptr, options['primitive_restart'])        

        if 'polygon_mode' in options:
            native.n_drawcall_set_polygon_mode(self.m_cptr, options['polygon_mode']) 

        if 'cull_mode' in options:
            native.n_drawcall_set_cull_mode(self.m_cptr, options['cull_mode'])       
            
        if 'front_face' in options:
            native.n_drawcall_set_front_face(self.m_cptr, options['front_face'])            
            
        if 'line_width' in options:
            native.n_drawcall_set_line_width(self.m_cptr, options['line_width'])                           

        if 'depth_enable' in options:
            native.n_drawcall_set_depth_enable(self.m_cptr, options['depth_enable'])

        if 'depth_write' in options:
            native.n_drawcall_set_depth_write(self.m_cptr, options['depth_write'])

        if 'depth_compare_op' in options:
            native.n_drawcall_set_depth_compare_op(self.m_cptr, options['depth_compare_op'])            

        if 'color_write' in options:
            native.n_drawcall_set_color_write(self.m_cptr, options['color_write'])

        if 'color_write_r' in options:
            native.n_drawcall_set_color_write_r(self.m_cptr, options['color_write_r'])

        if 'color_write_g' in options:
            native.n_drawcall_set_color_write_g(self.m_cptr, options['color_write_g'])

        if 'color_write_b' in options:
            native.n_drawcall_set_color_write_b(self.m_cptr, options['color_write_b'])

        if 'alpha_write' in options:
            native.n_drawcall_set_alpha_write(self.m_cptr, options['alpha_write'])

        if 'blend_enable' in options:
            native.n_drawcall_set_blend_enable(self.m_cptr, options['blend_enable'])

        # for compatibility of legacy code
        if 'alpha_blend' in options:
            native.n_drawcall_set_blend_enable(self.m_cptr, options['alpha_blend'])

        if 'src_color_blend_factor' in options:
            native.n_drawcall_set_src_color_blend_factor(self.m_cptr, options['src_color_blend_factor'])

        if 'dst_color_blend_factor' in options:
            native.n_drawcall_set_dst_color_blend_factor(self.m_cptr, options['dst_color_blend_factor'])

        if 'color_blend_op' in options:
            native.n_drawcall_set_color_blend_op(self.m_cptr, options['color_blend_op'])

        if 'src_alpha_blend_factor' in options:
            native.n_drawcall_set_src_alpha_blend_factor(self.m_cptr, options['src_alpha_blend_factor'])

        if 'dst_alpha_blend_factor' in options:
            native.n_drawcall_set_dst_alpha_blend_factor(self.m_cptr, options['dst_alpha_blend_factor'])                      

        if 'alpha_blend_op' in options:
            native.n_drawcall_set_alpha_blend_op(self.m_cptr, options['alpha_blend_op'])  

        if 'blend_constants' in options:
            c = options['blend_constants']
            native.n_drawcall_set_alpha_blend_op(self.m_cptr, c[0], c[1], c[2], c[3])  

        if 'color_attachements' in options:
            lst = options['color_attachements']
            for i in range(len(lst)):
                if 'color_write' in lst[i]:
                    native.n_drawcall_set_ith_color_write(self.m_cptr, i, lst[i]['color_write'])
                if 'color_write_r' in lst[i]:
                    native.n_drawcall_set_ith_color_write_r(self.m_cptr, i, lst[i]['color_write_r'])
                if 'color_write_g' in lst[i]:
                    native.n_drawcall_set_ith_color_write_g(self.m_cptr, i, lst[i]['color_write_g'])
                if 'color_write_b' in lst[i]:
                    native.n_drawcall_set_ith_color_write_b(self.m_cptr, i, lst[i]['color_write_b'])
                if 'alpha_write' in lst[i]:
                    native.n_drawcall_set_ith_alpha_write(self.m_cptr, i, lst[i]['alpha_write'])
                if 'blend_enable' in lst[i]:
                    native.n_drawcall_set_ith_blend_enable(self.m_cptr, i, lst[i]['blend_enable'])
                if 'src_color_blend_factor' in lst[i]:
                    native.n_drawcall_set_ith_src_color_blend_factor(self.m_cptr, i, lst[i]['src_color_blend_factor'])
                if 'dst_color_blend_factor' in lst[i]:
                    native.n_drawcall_set_ith_dst_color_blend_factor(self.m_cptr, i, lst[i]['dst_color_blend_factor'])
                if 'color_blend_op' in lst[i]:
                    native.n_drawcall_set_ith_color_blend_op(self.m_cptr, i, lst[i]['color_blend_op'])
                if 'src_alpha_blend_factor' in lst[i]:
                    native.n_drawcall_set_ith_src_alpha_blend_factor(self.m_cptr, i, lst[i]['src_alpha_blend_factor'])
                if 'dst_alpha_blend_factor' in lst[i]:
                    native.n_drawcall_set_ith_dst_alpha_blend_factor(self.m_cptr, i, lst[i]['dst_alpha_blend_factor'])
                if 'alpha_blend_op' in lst[i]:
                    native.n_drawcall_set_ith_alpha_blend_op(self.m_cptr, i, lst[i]['alpha_blend_op'])


    def __del__(self):
        native.n_drawcall_destroy(self.m_cptr)


class Rasterizer:
    def __init__(self, param_names, type_locked=False):
        o_param_names = StrArray(param_names)
        self.m_cptr = native.n_rasterizer_create(o_param_names.m_cptr, type_locked)
        self.m_draw_calls = []

    def __del__(self):
        native.n_rasterizer_destroy(self.m_cptr)

    def num_params(self):
        return native.n_rasterizer_num_params(self.m_cptr)

    def set_clear_color_buf(self, i, clear):
        native.n_rasterizer_set_clear_color_buf(self.m_cptr, i, clear)

    def set_clear_depth_buf(self, clear):
        native.n_rasterizer_set_clear_depth_buf(self.m_cptr, clear)

    def add_draw_call(self, draw_call):
        self.m_draw_calls += [draw_call]
        native.n_rasterizer_add_draw_call(self.m_cptr, draw_call.m_cptr)

    def launch(self, launch_params, colorBufs, depthBuf, clear_colors, clear_depth, args, tex2ds=[], tex3ds=[], cubemaps=[], resolveBufs=[], times_submission = 1):
        colorBuf_list = ObjArray(colorBufs)
        p_depthBuf = ffi.NULL
        if depthBuf!=None:
            p_depthBuf = depthBuf.m_cptr
        resolveBuf_list = ObjArray(resolveBufs)
        arg_list = ObjArray(args)
        tex2d_list = ObjArray(tex2ds)
        tex3d_list = ObjArray(tex3ds)
        cubemap_list = ObjArray(cubemaps)
        launch_param_list = [LaunchParam(obj) for obj in launch_params]
        ptrs_launch_param_list = [lp.m_cptr for lp in launch_param_list]
        native.n_rasterizer_launch(
            self.m_cptr, 
            colorBuf_list.m_cptr, 
            p_depthBuf, 
            resolveBuf_list.m_cptr, 
            clear_colors, 
            clear_depth, 
            arg_list.m_cptr, 
            tex2d_list.m_cptr, 
            tex3d_list.m_cptr, 
            cubemap_list.m_cptr,
            ptrs_launch_param_list,
            times_submission)



