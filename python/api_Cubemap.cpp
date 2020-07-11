#include "api.h"
#include "Context.h"
using namespace VkInline;

void* n_cubemap_create(int width, int height, unsigned vkformat)
{
	return new Cubemap(width, height, vkformat);
}

void n_cubemap_release(void* cubemap)
{
	delete (Cubemap*)cubemap;
}

int n_cubemap_width(void* _cubemap)
{
	Cubemap* cubemap = (Cubemap*)_cubemap;
	return cubemap->width();
}

int n_cubemap_height(void* _cubemap)
{
	Cubemap* cubemap = (Cubemap*)_cubemap;
	return cubemap->height();
}

unsigned n_cubemap_pixelsize(void* _cubemap)
{
	Cubemap* cubemap = (Cubemap*)_cubemap;
	return cubemap->pixel_size();
}

unsigned n_cubemap_channelcount(void* _cubemap)
{
	Cubemap* cubemap = (Cubemap*)_cubemap;
	return cubemap->channel_count();
}

unsigned n_cubemap_vkformat(void* _cubemap)
{
	Cubemap* cubemap = (Cubemap*)_cubemap;
	return cubemap->vkformat();
}

void n_cubemap_upload(void* _cubemap, void* hdata)
{
	Cubemap* cubemap = (Cubemap*)_cubemap;
	return cubemap->upload(hdata);
}

void n_cubemap_download(void* _cubemap, void* hdata)
{
	Cubemap* cubemap = (Cubemap*)_cubemap;
	return cubemap->download(hdata);
}
