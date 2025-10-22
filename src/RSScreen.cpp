//
//  Screen.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/27/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "RSScreen.h"

#include <raylib.h>

#include "SCRenderer.h"

extern SCRenderer Renderer;

RSScreen::RSScreen() { }

RSScreen::~RSScreen() { }

void RSScreen::Init(int32_t zoomFactor)
{
	const int32_t w = 320 * zoomFactor;
	const int32_t h = 200 * zoomFactor;

	scale = zoomFactor;
	width = w;
	height = h;

	InitWindow(w, h, "SC");
	SetWindowState(FLAG_VSYNC_HINT);
	SetTargetFPS(60);
	SetExitKey(KEY_NULL);
}

void RSScreen::Release()
{
	CloseWindow();
}

void RSScreen::SetTitle(const char* title)
{
	SetWindowTitle(title);
}

bool RSScreen::StartFrame()
{
	BeginDrawing();
	ClearBackground(PINK);
	return !WindowShouldClose();
}

void RSScreen::EndFrame()
{
	EndDrawing();
}
