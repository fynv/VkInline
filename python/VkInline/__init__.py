from .Native import native
if native.n_vkinline_try_init()==0:
	raise ImportError('cannot import VkInline')

from .Context import *
from .ShaderViewable import *
from .SVVector import *
from .SVObjVector import *
from .Texture2D import *

