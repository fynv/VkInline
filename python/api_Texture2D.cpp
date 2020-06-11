#include "api.h"
#include "Context.h"
using namespace VkInline;

void* n_texture2d_create(int width, int height, unsigned vkformat, unsigned isDepth, unsigned isStencil)
{
	return new Texture2D(width, height, vkformat, isDepth!=0, isStencil!=0);
}

void n_texture2d_release(void* tex2d)
{
	delete (Texture2D*)tex2d;
}

int n_texture2d_width(void* _tex2d)
{
	Texture2D* tex2d = (Texture2D*)_tex2d;
	return tex2d->width();
}

int n_texture2d_height(void* _tex2d)
{
	Texture2D* tex2d = (Texture2D*)_tex2d;
	return tex2d->height();
}

unsigned n_texture2d_pixelsize(void* _tex2d)
{
	Texture2D* tex2d = (Texture2D*)_tex2d;
	return tex2d->pixel_size();
}

unsigned n_texture2d_channelcount(void* _tex2d)
{
	Texture2D* tex2d = (Texture2D*)_tex2d;
	return tex2d->channel_count();
}

unsigned n_texture2d_vkformat(void* _tex2d)
{
	Texture2D* tex2d = (Texture2D*)_tex2d;
	return tex2d->vkformat();
}

void n_texture2d_upload(void* _tex2d, void* hdata)
{
	Texture2D* tex2d = (Texture2D*)_tex2d;
	tex2d->upload(hdata);
}

void n_texture2d_download(void* _tex2d, void* hdata)
{
	Texture2D* tex2d = (Texture2D*)_tex2d;
	tex2d->download(hdata);
}

