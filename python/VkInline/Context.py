from .Native import native
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
    def __init__(self, param_names, body):
        o_param_names = StrArray(param_names)
        self.m_cptr = native.n_computer_create(o_param_names.m_cptr, body.encode('utf-8'))

    def __del__(self):
        native.n_computer_destroy(self.m_cptr)

    def num_params(self):
        return native.n_computer_num_params(self.m_cptr)

    def launch(self, gridDim, blockDim, args, tex2ds=[]):
        d_gridDim = Dim3(gridDim)
        d_blockDim = Dim3(blockDim)
        arg_list = ObjArray(args)
        tex2d_list = Texture2DArray(tex2ds)
        native.n_computer_launch(
            self.m_cptr, 
            d_gridDim.m_cptr, 
            d_blockDim.m_cptr, 
            arg_list.m_cptr,
            tex2d_list.m_cptr)

class For:
    def __init__(self, param_names, name_inner, body, block_size=128):
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
        self.m_cptr = native.n_computer_create(o_param_names.m_cptr, whole_body.encode('utf-8'))

    def __del__(self):
        native.n_computer_destroy(self.m_cptr)

    def num_params(self):
        return native.n_computer_num_params(self.m_cptr) - 2

    def launch(self, begin, end, args, tex2ds=[]): 
        svbegin = SVUInt32(begin)
        svend = SVUInt32(end)
        args = args + [svbegin, svend]
        numBlocks = int((end - begin + self.block_size - 1) / self.block_size)
        d_gridDim = Dim3(numBlocks)
        d_blockDim = Dim3(self.block_size)
        arg_list = ObjArray(args)
        tex2d_list = Texture2DArray(tex2ds)
        native.n_computer_launch(
            self.m_cptr, 
            d_gridDim.m_cptr, 
            d_blockDim.m_cptr, 
            arg_list.m_cptr,
            tex2d_list.m_cptr)

    def launch_n(self, n, args, tex2ds=[]):
        svbegin = SVUInt32(0)
        svend = SVUInt32(n)
        args = args + [svbegin, svend]
        numBlocks = int((n + self.block_size - 1) / self.block_size)
        d_gridDim = Dim3(numBlocks)
        d_blockDim = Dim3(self.block_size)
        arg_list = ObjArray(args)
        tex2d_list = Texture2DArray(tex2ds)
        native.n_computer_launch(
            self.m_cptr, 
            d_gridDim.m_cptr, 
            d_blockDim.m_cptr, 
            arg_list.m_cptr,
            tex2d_list.m_cptr)      

