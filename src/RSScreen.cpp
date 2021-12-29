//
//  Screen.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/27/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "RSScreen.h"

#include "precomp.h"

#include <SDL2/SDL.h>

static SDL_Window *sdlWindow;
static SDL_Renderer *sdlRenderer;

RSScreen::RSScreen()
{
}

RSScreen::~RSScreen()
{
}

void RSScreen::SetTitle(const char* title)
{
	SDL_SetWindowTitle(sdlWindow, title);
}

void RSScreen::Init(int32_t zoomFactor)
{
	const int32_t w = 320 * zoomFactor;
	const int32_t h = 200 * zoomFactor;

	this->scale =zoomFactor;
	this->width = w;
	this->height = h;

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
}

void RSScreen::Refresh(void)
{
	SDL_GL_SwapWindow(sdlWindow);
}
