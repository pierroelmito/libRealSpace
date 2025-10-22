//
//  RSImage.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/31/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "RSImage.h"

#include "SCRenderer.h"

extern SCRenderer Renderer;

RSImage::RSImage()
{
}

RSImage::~RSImage()
{
	if (data)
		free(data);
}

void RSImage::Create(const char name[8], uint32_t width, uint32_t height, uint32_t flags)
{
	strcpy(this->name, name);
	this->flags = flags;
	this->width = width;
	this->height = height;
	this->data = (uint8_t*)malloc(this->width * this->height);
	this->palette = &Renderer.GetPalette();
	this->texture.Set(*this);
	dirty = true;
}

void RSImage::UpdateContent(uint8_t* src)
{
	memcpy(this->data, src, width * height);
	this->dirty = true;
}

void RSImage::SyncTexture()
{
	if (!this->dirty)
		return;
	texture.UpdateContent(*this);
	Renderer.UploadTextureContentToGPU(&texture);
	dirty = false;
}

uint8_t* RSImage::GetData()
{
	dirty = true;
	return data;
}

void RSImage::ClearContent()
{
	memset(this->data, 0, this->width * this->height);
	dirty = true;
}

void RSImage::SetPalette(VGAPalette* palette)
{
	this->palette = palette;
	dirty = true;
}

RSTexture* RSImage::GetTexture()
{
	SyncTexture();
	return &texture;
}
