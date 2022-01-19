//
//  VGA.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/27/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "RSVGA.h"

#include "precomp.h"

#include "RSFont.h"

RSVGA::RSVGA()
{
}

RSVGA::~RSVGA()
{
}

void RSVGA::Clear(void)
{
	memset(frameBuffer, 0, WIDTH * HEIGHT);
}

bool RSVGA::DrawShape(RLEShape& shape)
{
	size_t byteRead;
	return shape.Expand(GetFrameBuffer(), &byteRead);
}

void RSVGA::Init(void)
{
	//Load the default palette
	IffLexer lexer ;
	lexer.InitFromFile("PALETTE.IFF");
	//lexer.List(stdout);

	RSPalette palette;
	palette.InitFromIFF(&lexer);
	SetPalette(*palette.GetColorPalette());
}

void RSVGA::SetPalette(const VGAPalette& newPalette)
{
	palette = newPalette;
}

void RSVGA::VSync(void)
{
	Texel data[WIDTH * HEIGHT];
	for (size_t i = 0; i < WIDTH * HEIGHT; i++) {
		data[i] = *palette.GetRGBColor(frameBuffer[i]);
		data[i].a = 0xff;
	}
	SCRenderer::UpdateBitmapQuad(data, WIDTH, HEIGHT);
}

void RSVGA::FillLineColor(size_t lineIndex, uint8_t color)
{
	memset(frameBuffer+lineIndex * WIDTH, color, WIDTH);
}

void RSVGA::DrawText(RSFont* font, Point2D* coo, char* text, uint8_t color,size_t start, uint32_t size,size_t interLetterSpace, size_t spaceSize)
{
	if (text == NULL)
		return;

	if (size <= 0)
		return;

	for (size_t i =0; i < size; i++) {
		char chartoDraw = text[start+i];
		RLEShape* shape = font->GetShapeForChar(chartoDraw);

		shape->SetColorOffset(color);
		//Adjust height
		int32_t lineHeight = coo->y;
		coo->y -= shape->GetHeight();

		if (chartoDraw== 'p' ||
			chartoDraw== 'y' ||
			chartoDraw== 'g' )
			coo->y += 1;

		shape->SetPosition(*coo);
		DrawShape(*shape);
		coo->y = lineHeight;

		if (chartoDraw == ' ')
			coo->x += spaceSize ;
		else
			coo->x+=shape->GetWidth() + interLetterSpace;
	}
}
