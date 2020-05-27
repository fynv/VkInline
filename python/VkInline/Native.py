import os
import sys
import site
from .cffi import ffi

if os.name == 'nt':
    fn_vkinline = 'PyVkInline.dll'
elif os.name == "posix":
    fn_vkinline = 'libPyVkInline.so'

path_vkinline = os.path.dirname(__file__)+"/"+fn_vkinline

native = ffi.dlopen(path_vkinline)

