//
//  Screen.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/27/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "RSScreen.h"

#include "precomp.h"

#define SOKOL_GLCORE33
#define SOKOL_GFX_IMPL
#include <sokol_gfx.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

GLFWwindow* win = nullptr;

RSScreen::RSScreen()
{
}

RSScreen::~RSScreen()
{
}

void RSScreen::SetTitle(const char* title)
{
	glfwSetWindowTitle(win, title);
}

void RSScreen::Init(int32_t zoomFactor)
{
	const int32_t w = 320 * zoomFactor;
	const int32_t h = 200 * zoomFactor;

	this->scale =zoomFactor;
	this->width = w;
	this->height = h;

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	win = glfwCreateWindow(width, height, "toto", 0, 0);
	glfwMakeContextCurrent(win);
	glfwSwapInterval(1);

	sg_desc desc{ 0 };
	desc.buffer_pool_size = 1 << 15;
	sg_setup(desc);
}

bool RSScreen::StartFrame()
{
	return !glfwWindowShouldClose(win);
}

void RSScreen::EndFrame()
{
	sg_commit();

	glfwPollEvents();
}

void RSScreen::Refresh(void)
{
	glfwSwapBuffers(win);
}
