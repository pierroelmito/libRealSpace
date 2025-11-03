//
//  Texture.cpp
//  iff
//
//  Created by Fabien Sanglard on 12/20/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "Texture.h"

#include "RSImage.h"

#include <cassert>

#include <vector>

RSTexture::RSTexture()
{
	img.width = 0;
	img.height = 0;
	tex.width = 0;
	tex.height = 0;
}

RSTexture::~RSTexture()
{
	if (tex.width != 0 && tex.height != 0)
		UnloadTexture(tex);
	if (img.width != 0 && img.height != 0)
		UnloadImage(img);
}

void RSTexture::Set(RSImage& image)
{
	assert(image.width < 2048 && image.height < 2048);
	strncpy(name, image.name, 8);
	img = GenImageColor(image.width, image.height, PINK);
	tex = LoadTextureFromImage(img);
	SetTextureFilter(tex, TEXTURE_FILTER_POINT);
}

void FillAlphaWithAppropriateColors(size_t w, size_t h, uint8_t* data)
{
	std::vector<int> dist(w * h, 10000);

	auto copy = [data, w, h](int fo, int delta) {
		uint8_t* dst = data + (fo);
		uint8_t* src = data + (fo + 4 * delta);
		for (int i = 0; i < 3; ++i) {
			assert(fo + i >= 0 && fo + i < w * h * 4);
			dst[i] = src[i];
		}
	};

	for (int y = 0; y < h; ++y) {
		int d = 0;
		for (int x = 0; x < w; ++x) {
			const int bo = y * w + x;
			const int fo = 4 * bo;
			const bool alpha = data[fo + 3] == 0;
			d = alpha ? d + 1 : 0;
			if (alpha && x - d >= 0) {
				dist[bo] = d;
				copy(fo, -d);
			}
		}
		d = 0;
		for (int x = w - 1; x >= 0; --x) {
			const int bo = y * w + x;
			const int fo = 4 * bo;
			const bool alpha = data[fo + 3] == 0;
			d = alpha ? d + 1 : 0;
			if (alpha && x + d < w && dist[bo] > d) {
				dist[bo] = d;
				copy(fo, d);
			}
		}
	}

	for (int x = 0; x < w; ++x) {
		int d = 0;
		for (int y = 0; y < h; ++y) {
			const int bo = y * w + x;
			const int fo = 4 * bo;
			const bool alpha = data[fo + 3] == 0;
			d = alpha ? d + 1 : 0;
			if (alpha && y - d >= 0 && dist[bo] > d) {
				dist[bo] = d;
				copy(fo, -d * w);
			}
		}
		d = 0;
		for (int y = h - 1; y >= 0; --y) {
			const int bo = y * w + x;
			const int fo = 4 * bo;
			const bool alpha = data[fo + 3] == 0;
			d = alpha ? d + 1 : 0;
			if (alpha && y + d < h && dist[bo] > d) {
				dist[bo] = d;
				copy(fo, d * w);
			}
		}
	}
}

void RSTexture::UpdateContent(RSImage& image)
{
	assert(image.width < 2048 && image.height < 2048);

	uint8_t* const baseSrc = image.data;
	uint8_t* const baseDst = (uint8_t*)img.data;
	VGAPalette* const palette = image.palette;

	uint8_t* src = baseSrc;
	uint8_t* dst = baseDst;
	bool hasAlpha = false;

	for (int i = 0; i < image.height; i++) {
		for (int j = 0; j < image.width; j++) {
			const uint8_t* psrcIndex = src + j + i * image.width;
			const uint8_t srcIndex = *psrcIndex;
			const Color& src = palette->GetRGBColor(srcIndex);
			dst[0] = src.r;
			dst[1] = src.g;
			dst[2] = src.b;
			dst[3] = src.a;
			if (src.r == 0 && src.g == 0 && src.b == 0)
				dst[3] = 0;
			// force alpha on delimiters...
			if (image.width == 64 && image.height == 64 && src.r == 174 && src.g == 28 && src.b == 0)
				dst[3] = 0;
			const bool alpha = dst[3] == 0;
			hasAlpha = hasAlpha || alpha;
			dst += 4;
		}
	}

	if (hasAlpha)
		FillAlphaWithAppropriateColors(image.width, image.height, baseDst);

	if ((image.flags & IMAGE_FLAG_COPY_PALINDEX_TO_ALPHA) != 0) {
		dst = (uint8_t*)img.data;
		for (int i = 0; i < image.height; i++) {
			for (int j = 0; j < image.width; j++) {
				const uint8_t* psrcIndex = src + j + i * image.width;
				const uint8_t srcIndex = *psrcIndex;
				const bool alpha = dst[3] == 0;
				if (alpha)
					dst[3] = srcIndex;
				dst += 4;
			}
		}
	}
}
