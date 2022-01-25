//
//  RealSpacePaletteObject.h
//  iff
//
//  Created by Fabien Sanglard on 12/21/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <cstdint>

#include "Texture.h"
#include "ByteSlice.h"

class IffLexer;
class IffChunk;

class RSPalette
{
public:
	RSPalette();
	~RSPalette();

	void InitFromIFF(IffLexer* lexer);

	VGAPalette* GetColorPalette(void);
	void SetColorFlag(uint32_t flag);

	VGAPalette* GetBWPalette(void);
	void SetBWFlag(uint32_t flag);

private:
	void ParsePALT(IffChunk* chunk);
	void ParseBLWH(IffChunk* chunk);
	void ParseCMAP(IffChunk* chunk);

	DataBufferPtr _buffer;

	uint32_t colorFlag;
	VGAPalette colors;

	uint32_t bwFlag;
	VGAPalette bwColors;
};
