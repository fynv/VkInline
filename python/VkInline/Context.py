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

    def launch(self, gridDim, blockDim, args, tex2ds=[], tex3ds=[]):
        d_gridDim = Dim3(gridDim)
        d_blockDim = Dim3(blockDim)
        arg_list = ObjArray(args)
        tex2d_list = Texture2DArray(tex2ds)
        tex3d_list = Texture3DArray(tex3ds)
        native.n_computer_launch(
            self.m_cptr, 
            d_gridDim.m_cptr, 
            d_blockDim.m_cptr, 
            arg_list.m_cptr,
            tex2d_list.m_cptr,
            tex3d_list.m_cptr)

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

    def launch(self, begin, end, args, tex2ds=[], tex3ds=[]): 
        svbegin = SVUInt32(begin)
        svend = SVUInt32(end)
        args = args + [svbegin, svend]
        numBlocks = int((end - begin + self.block_size - 1) / self.block_size)
        d_gridDim = Dim3(numBlocks)
        d_blockDim = Dim3(self.block_size)
        arg_list = ObjArray(args)
        tex2d_list = Texture2DArray(tex2ds)
        tex3d_list = Texture3DArray(tex3ds)
        native.n_computer_launch(
            self.m_cptr, 
            d_gridDim.m_cptr, 
            d_blockDim.m_cptr, 
            arg_list.m_cptr,
            tex2d_list.m_cptr,
            tex3d_list.m_cptr)

    def launch_n(self, n, args, tex2ds=[], tex3ds=[]):
        svbegin = SVUInt32(0)
        svend = SVUInt32(n)
        args = args + [svbegin, svend]
        numBlocks = int((n + self.block_size - 1) / self.block_size)
        d_gridDim = Dim3(numBlocks)
        d_blockDim = Dim3(self.block_size)
        arg_list = ObjArray(args)
        tex2d_list = Texture2DArray(tex2ds)
        tex3d_list = Texture3DArray(tex3ds)
        native.n_computer_launch(
            self.m_cptr, 
            d_gridDim.m_cptr, 
            d_blockDim.m_cptr, 
            arg_list.m_cptr,
            tex2d_list.m_cptr,
            tex3d_list.m_cptr)      

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

        if 'color_write' in options:
            native.n_drawcall_set_color_write(self.m_cptr, options['color_write'])

        if 'alpha_write' in options:
            native.n_drawcall_set_alpha_write(self.m_cptr, options['alpha_write'])

        if 'alpha_blend' in options:
            native.n_drawcall_set_alpha_blend(self.m_cptr, options['alpha_blend'])

        if 'depth_compare_op' in options:
            native.n_drawcall_set_depth_compare_op(self.m_cptr, options['depth_compare_op'])

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

    def launch(self, launch_params, colorBufs, depthBuf, clear_colors, clear_depth, args, tex2ds=[], tex3ds=[], resolveBufs=[]):
        colorBuf_list = Texture2DArray(colorBufs)
        p_depthBuf = ffi.NULL
        if depthBuf!=None:
            p_depthBuf = depthBuf.m_cptr
        resolveBuf_list = Texture2DArray(resolveBufs)
        arg_list = ObjArray(args)
        tex2d_list = Texture2DArray(tex2ds)
        tex3d_list = Texture3DArray(tex3ds)
        launch_param_list = [LaunchParam(obj) for obj in launch_params]
        ptrs_launch_param_list = [lp.m_cptr for lp in launch_param_list]
        native.n_rasterizer_launch(self.m_cptr, colorBuf_list.m_cptr, p_depthBuf, resolveBuf_list.m_cptr, clear_colors, clear_depth, arg_list.m_cptr, tex2d_list.m_cptr, tex3d_list.m_cptr, ptrs_launch_param_list)



