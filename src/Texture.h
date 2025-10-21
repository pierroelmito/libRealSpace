//
//  Texture.h
//  iff
//
//  Created by Fabien Sanglard on 12/20/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <cstdio>

#include <raylib.h>

#include "ByteStream.h"

struct VGAPalette {
	std::array<Color, 256> colors;

	void SetColor(uint8_t value, const Color& texel)
	{
		colors[value] = texel;
	}

	Color& GetRGBColor(uint8_t value)
	{
		return colors.at(value);
	}

	void Diff(const VGAPalette& other)
	{
		for (int i = 0; i < 256; i++) {
			if (colors[i].r != other.colors[i].r || colors[i].g != other.colors[i].g || colors[i].b != other.colors[i].b || colors[i].a != other.colors[i].a)
				printf("diff: %d.\n", i);
		}
	}

	void ReadPatch(ByteStream* s, int colOffset)
	{
		const int16_t offset = uint16_t(s->ReadShort() + colOffset);
		const int16_t numColors = s->ReadShort();

		if (offset + numColors > 256) {
			printf("VGAPalette::ReadPatch => Error, this will overflow.\n");
			return;
		}

		for (uint16_t i = 0; i < numColors; i++) {
			colors[offset + i].r = s->ReadByte() * 255 / 63.0f;
			colors[offset + i].g = s->ReadByte() * 255 / 63.0f;
			colors[offset + i].b = s->ReadByte() * 255 / 63.0f;
			colors[offset + i].a = 1;
		}
	}
};

class RSImage;

class RSTexture {
public:
	enum Location {
		DISK = 0x1,
		RAM = 0x2,
		VRAM = 0x4,
	};

	RSTexture();
	~RSTexture();

	Image img {};
	Texture tex {};

	char name[8];
	uint8_t locFlag {};
	void Set(RSImage& image);
	void UpdateContent(RSImage& image);
};
