#include "api.h"
#include "Context.h"
using namespace VkInline;

void* n_texture3d_create(int dimX, int dimY, int dimZ, unsigned vkformat)
{
	return new Texture3D(dimX, dimY, dimZ, vkformat);
}

void n_texture3d_release(void* tex3d)
{
	delete (Texture3D*)tex3d;
}

int n_texture3d_dimX(void* _tex3d)
{
	Texture3D* tex3d = (Texture3D*)_tex3d;
	return tex3d->dimX();
}

int n_texture3d_dimY(void* _tex3d)
{
	Texture3D* tex3d = (Texture3D*)_tex3d;
	return tex3d->dimY();
}

int n_texture3d_dimZ(void* _tex3d)
{
	Texture3D* tex3d = (Texture3D*)_tex3d;
	return tex3d->dimZ();
}

unsigned n_texture3d_pixelsize(void* _tex3d)
{
	Texture3D* tex3d = (Texture3D*)_tex3d;
	return tex3d->pixel_size();
}

unsigned n_texture3d_channelcount(void* _tex3d)
{
	Texture3D* tex3d = (Texture3D*)_tex3d;
	return tex3d->channel_count();
}

unsigned n_texture3d_vkformat(void* _tex3d)
{
	Texture3D* tex3d = (Texture3D*)_tex3d;
	return tex3d->vkformat();
}

void n_texture3d_upload(void* _tex3d, void* hdata)
{
	Texture3D* tex3d = (Texture3D*)_tex3d;
	tex3d->upload(hdata);
}

void n_texture3d_download(void* _tex3d, void* hdata)
{
	Texture3D* tex3d = (Texture3D*)_tex3d;
	tex3d->download(hdata);
}

