from .Native import ffi, native
import numpy as np

class Texture3D:
    def __init__(self, dimX, dimY, dimZ, vkformat):
        self.m_cptr = native.n_texture3d_create(dimX, dimY, dimZ, vkformat)

    def __del__(self):
        native.n_texture3d_release(self.m_cptr)

    def dimX(self):
        return native.n_texture3d_dimX(self.m_cptr)

    def dimY(self):
        return native.n_texture3d_dimY(self.m_cptr)

    def dimZ(self):
        return native.n_texture3d_dimZ(self.m_cptr)

    def pixel_size(self):
        return native.n_texture3d_pixelsize(self.m_cptr)

    def channel_count(self):
        return native.n_texture3d_channelcount(self.m_cptr)

    def vkformat(self):
        return native.n_texture3d_vkformat(self.m_cptr)

    def upload(self, nparr):
        ffiptr = ffi.cast("void *", nparr.__array_interface__['data'][0])
        native.n_texture3d_upload(self.m_cptr, ffiptr)

    def download(self, nparr):
        ffiptr = ffi.cast("void *", nparr.__array_interface__['data'][0])
        native.n_texture3d_download(self.m_cptr, ffiptr)


