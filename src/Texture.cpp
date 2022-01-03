//
//  Texture.cpp
//  iff
//
//  Created by Fabien Sanglard on 12/20/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

#include <cassert>

RSTexture::RSTexture()
: data(0)
, locFlag(DISK)
{
}

RSTexture::~RSTexture()
{
	Renderer.DeleteTextureInGPU(this);
	if (data)
		free(data);
}

void RSTexture::Set(RSImage* image)
{
	strncpy(this->name,image->name,8);
	this->width = image->width;
	this->height = image->height;
	this->data = (uint8_t*)malloc(width*height*4);
	locFlag = RAM;
	//UpdateContent(image);
}

void RSTexture::UpdateContent(RSImage* image)
{
	uint8_t* src = image->data;
	uint8_t* dst = this->data;
	VGAPalette* palette = image->palette;

	for(int i = 0 ; i < image->height; i++) {
		for(int j = 0 ; j < image->width; j++) {
			const uint8_t* psrcIndex = src + j + i * image->width;
			const uint8_t srcIndex = *psrcIndex;
			const Texel* src = palette->GetRGBColor(srcIndex);
			dst[0] = src->r;
			dst[1] = src->g;
			dst[2] = src->b;
			dst[3] = src->a;
			if ((image->flags & IMAGE_FLAG_COPY_PALINDEX_TO_ALPHA) != 0) {
				if (dst[3] != 0)
					dst[3] = srcIndex;
				//if (src->r == 0 && src->g == 0 && src->b == 0)
				//	dst[3] = 0;
			} else {
				if (src->r == 0 && src->g == 0 && src->b == 0)
					dst[3] = 0;
			}
			dst+=4;
		}
	}
}
