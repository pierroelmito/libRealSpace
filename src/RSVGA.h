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
	bool& ShowPalette() { return _showPalette; }

	void Init();
	void Release() {}

	void SetPalette(const VGAPalette& newPalette);
	bool DrawShape(RLEShape& shape);
	template <typename... T>
	void PrintText(RSFont* font, const Point2D& coo, const uint8_t color, const size_t interLetterSpace, const size_t spaceSize, const char* fmt, T&&... args)
	{
		char buffer[512]{};
		auto sz = snprintf(buffer, sizeof(buffer), fmt, args...);
		DrawText(font, coo, buffer, color, 0, sz, interLetterSpace, spaceSize);
	}
	void DrawText(RSFont* font, const Point2D& coo, const char* text, const uint8_t color, const size_t start, const uint32_t size, const size_t interLetterSpace, const size_t spaceSize);
	void VSync(float fade = 0.0f);
	void Clear(void);
	void FillLineColor(size_t lineIndex, uint8_t color);

private:
	VGAPalette palette;
	uint8_t frameBuffer[WIDTH * HEIGHT];
	bool _showPalette{ false };
};
