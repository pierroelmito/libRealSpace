//
//  Screen.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/27/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "RSScreen.h"

#include "precomp.h"

#if USE_RAYLIB

#include <raylib.h>

#else

#include <SDL2/SDL.h>
static SDL_Window *sdlWindow;
static SDL_Renderer *sdlRenderer;

#endif

RSScreen::RSScreen()
{
}

RSScreen::~RSScreen()
{
}

void RSScreen::SetTitle(const char* title)
{
#if USE_RAYLIB
	SetWindowTitle(title);
#else
	SDL_SetWindowTitle(sdlWindow, title);
#endif
}

void RSScreen::Init(int32_t zoomFactor)
{
	const int32_t w = 320 * zoomFactor;
	const int32_t h = 200 * zoomFactor;

	this->scale =zoomFactor;
	this->width = w;
	this->height = h;

#if USE_RAYLIB
	InitWindow(width, height, "yolo");
	SetTargetFPS(60);
#else
	SDL_CreateWindowAndRenderer(width, height, SDL_WINDOW_HIDDEN, &sdlWindow, &sdlRenderer);

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
		printf("Unable to initialize SDL:  %s\n",SDL_GetError());
		return ;
	}

	sdlWindow = SDL_CreateWindow("RealSpace OBJ Viewer",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,this->width,this->height,SDL_WINDOW_OPENGL);
	// Create an OpenGL context associated with the window.
	SDL_GL_CreateContext(sdlWindow);
	//glViewport(0,0,this->width,this->height); // Reset The Current Viewport
	SDL_ShowWindow(sdlWindow);
#endif
}

bool RSScreen::StartFrame()
{
#if USE_RAYLIB
	const bool r = !WindowShouldClose();
	BeginDrawing();
	ClearBackground(PINK);
	return r;
#else
	return true;
#endif
}

void RSScreen::EndFrame()
{
#if USE_RAYLIB
	const int fps = GetFPS();
	char buffer[512]{};
	snprintf(buffer, sizeof(buffer), "toto %d", fps);
	DrawText(buffer, 4, 4, 20, DARKBLUE);
	EndDrawing();
#endif
}

void RSScreen::Refresh(void)
{
#if USE_RAYLIB
#else
	SDL_GL_SwapWindow(sdlWindow);
#endif
}
