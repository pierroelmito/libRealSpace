//
//  VGA.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/27/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include "Texture.h"
#include "RLEShape.h"

class RSFont;

class RSVGA
{
public:
	static constexpr uint32_t WIDTH = 320;
	static constexpr uint32_t HEIGHT = 200;

	RSVGA();
	~RSVGA();

	const VGAPalette& GetPalette() const { return palette; }
	uint8_t* GetFrameBuffer(void){ return frameBuffer;}

	void Init(void);
	void SetPalette(const VGAPalette& newPalette);
	bool DrawShape(RLEShape& shape);
	void DrawText(RSFont* font, Point2D* coo, char* text, uint8_t color,size_t start, uint32_t size,size_t interLetterSpace, size_t spaceSize);
	void VSync(void);
	void Clear(void);
	void FillLineColor(size_t lineIndex, uint8_t color);

private:
	VGAPalette palette;
	uint8_t frameBuffer[WIDTH * HEIGHT];
};
