from .Native import ffi, native
import numpy as np

class Texture2D:
    def __init__(self, width, height, vkformat, isDepth = False, isStencil=False):
        self.m_cptr = native.n_texture2d_create(width, height, vkformat, isDepth, isStencil)

    def __del__(self):
        native.n_texture2d_release(self.m_cptr)

    def width(self):
        return native.n_texture2d_width(self.m_cptr)

    def height(self):
        return native.n_texture2d_height(self.m_cptr)

    def pixel_size(self):
        return native.n_texture2d_pixelsize(self.m_cptr)

    def channel_count(self):
        return native.n_texture2d_channelcount(self.m_cptr)

    def vkformat(self):
        return native.n_texture2d_vkformat(self.m_cptr)

    def upload(self, nparr):
        ffiptr = ffi.cast("void *", nparr.__array_interface__['data'][0])
        native.n_texture2d_upload(self.m_cptr, ffiptr)

    def download(self, nparr):
        ffiptr = ffi.cast("void *", nparr.__array_interface__['data'][0])
        native.n_texture2d_download(self.m_cptr, ffiptr)


