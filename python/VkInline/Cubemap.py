from .Native import ffi, native
import numpy as np

class Cubemap:
    def __init__(self, width, height, vkformat):
        self.m_cptr = native.n_cubemap_create(width, height, vkformat)

    def __del__(self):
        native.n_cubemap_release(self.m_cptr)

    def width(self):
        return native.n_cubemap_width(self.m_cptr)

    def height(self):
        return native.n_cubemap_height(self.m_cptr)

    def pixel_size(self):
        return native.n_cubemap_pixelsize(self.m_cptr)

    def channel_count(self):
        return native.n_cubemap_channelcount(self.m_cptr)

    def vkformat(self):
        return native.n_cubemap_vkformat(self.m_cptr)

    def upload(self, nparr):
        ffiptr = ffi.cast("void *", nparr.__array_interface__['data'][0])
        native.n_cubemap_upload(self.m_cptr, ffiptr)

    def download(self, nparr):
        ffiptr = ffi.cast("void *", nparr.__array_interface__['data'][0])
        native.n_cubemap_download(self.m_cptr, ffiptr)


