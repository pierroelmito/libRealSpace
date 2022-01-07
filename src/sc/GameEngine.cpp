//
//  Game.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

#define SOKOL_GLCORE33
#include "sokol_gfx.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "IActivity.h"
#include "UserProperties.h"

extern GLFWwindow* win;

GameEngine::GameEngine()
{
}

GameEngine::~GameEngine()
{
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	auto& buttons = Mouse.buttons;
	if (action == GLFW_PRESS)
		buttons[button].event = SCMouseButton::PRESSED;
	if (action == GLFW_RELEASE)
		buttons[button].event = SCMouseButton::RELEASED;
}


void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (mods == GLFW_MOD_CONTROL && key == GLFW_KEY_R && action == GLFW_PRESS)
		UserProperties::Get().Reload();
}

void GameEngine::Init()
{
	Assets.Init(); //Load all TREs and PAKs
	FontManager.Init();
	ConvAssets.Init(); //Load assets needed for Conversations (char and background)
	Screen.Init(3); //Load Main Palette and Initialize the GL
	VGA.Init();
	Renderer.Init(2);
	Mouse.Init(); //Load the Mouse Cursor

	glfwSetMouseButtonCallback(win, MouseButtonCallback);
	glfwSetKeyCallback(win, KeyCallback);
}

bool GameEngine::AnyInput()
{
#if 0
	//Mouse
	SDL_Event mouseEvents[5];
	int numMouseEvents= SDL_PeepEvents(mouseEvents,5,SDL_PEEKEVENT,SDL_MOUSEBUTTONUP,SDL_MOUSEBUTTONUP);
	for(int i= 0 ; i < numMouseEvents ; i++){
		SDL_Event* event = &mouseEvents[i];
		switch (event->type) {
			case SDL_MOUSEBUTTONUP:
				return true;
			default:
				break;
		}
	}

	//Keyboard
	SDL_Event keybEvents[5];
	int numKeybEvents = SDL_PeepEvents(keybEvents,5,SDL_PEEKEVENT,SDL_KEYUP,SDL_KEYUP);
	for(int i= 0 ; i < numKeybEvents ; i++){
		SDL_Event* event = &keybEvents[i];
		switch (event->type) {
			default:
				return true;
		}
	}
#endif
	return false;
}

bool GameEngine::IsKeyPressed(uint32_t keyCode)
{
	return glfwGetKey(win, keyCode) == GLFW_PRESS;
#if 0
	SDL_Event keybEvents[5];
	int numKeybEvents = SDL_PeepEvents(keybEvents,5,SDL_PEEKEVENT,SDL_KEYDOWN,SDL_KEYDOWN);
	for(int i= 0 ; i < numKeybEvents ; i++){
		SDL_Event* event = &keybEvents[i];
		if (event->key.keysym.sym == keyCode)
			return true;
	}
	return false;
#endif
}

bool GameEngine::PumpEvents(void)
{
#if 1
	double mx, my;
	glfwGetCursorPos(win, &mx, &my);
	Mouse.SetPosition({
		int32_t(mx * 320.0 / Screen.width),
		int32_t(my * 200.0 / Screen.height)
	});
#else
	SDL_PumpEvents();

	//Mouse
	SDL_Event mouseEvents[5];
	int numMouseEvents= SDL_PeepEvents(mouseEvents,5,SDL_PEEKEVENT,SDL_MOUSEMOTION,SDL_MOUSEWHEEL);
	for(int i= 0 ; i < numMouseEvents ; i++){
		SDL_Event* event = &mouseEvents[i];
		switch (event->type) {
			case SDL_MOUSEMOTION:
				Point2D newPosition;
				newPosition.x = event->motion.x;
				newPosition.y = event->motion.y;
				newPosition.x *= 320.0f / Screen.width;
				newPosition.y *= 200.0f / Screen.height;
				Mouse.SetPosition(newPosition);
				break;
			case SDL_MOUSEBUTTONDOWN:
				Mouse.buttons[event->button.button-1].event = SCMouseButton::PRESSED;
				break;
			case SDL_MOUSEBUTTONUP:
				Mouse.buttons[event->button.button-1].event = SCMouseButton::RELEASED;
				break;
			default:
				break;
		}
	}

	//Joystick

	//Keyboard
	SDL_Event keybEvents[5];
	int numKeybEvents = SDL_PeepEvents(keybEvents,5,SDL_PEEKEVENT,SDL_KEYDOWN,SDL_TEXTINPUT);
	for(int i= 0 ; i < numKeybEvents ; i++){
		SDL_Event* event = &keybEvents[i];
		switch (event->type) {
			case SDL_KEYDOWN:
				if (event->key.keysym.scancode == SDL_SCANCODE_ESCAPE)
				return false;
			default:
				break;
		}
	}

	//Oculus VR

	//System events
	SDL_Event sysEvents[5];
	int numSysEvents = SDL_PeepEvents(sysEvents,5,SDL_PEEKEVENT,SDL_FIRSTEVENT,SDL_SYSWMEVENT);
	for(int i= 0 ; i < numSysEvents ; i++) {
		SDL_Event* event = &sysEvents[i];
		switch (event->type) {
			case SDL_APP_TERMINATING:
				Terminate("System request.");
				break;
			case SDL_QUIT:
				Terminate("System request.");
				break;
			//Verify is we should be capturing the mouse or not.
			case SDL_WINDOWEVENT:
				if (event->window.event == SDL_WINDOWEVENT_ENTER){
					Mouse.SetVisible(true);
					SDL_ShowCursor(SDL_DISABLE);
					return true;
				}
				if (event->window.event == SDL_WINDOWEVENT_LEAVE){
					Mouse.SetVisible(false);
					SDL_ShowCursor(SDL_ENABLE);
					return true;
				}
				break;
			default:
				break;
		}
	}
#endif
	return true;
}

void GameEngine::Run()
{
	while (Screen.StartFrame() && activities.size() > 0) {
		if (!PumpEvents())
			break;

		//Allow the active activity to Run and Render
		IActivity* currentActivity = activities.top();
		if (currentActivity->IsRunning()) {
			currentActivity->Focus();
			currentActivity->RunFrame({ glfwGetTime() });
			currentActivity->UnFocus();
		} else{
			activities.pop();
			delete currentActivity;
		}

		//Swap GL buffer
		Screen.Refresh();

#if 0
		//Flush all events since they should all have been interpreted.
		SDL_FlushEvents(SDL_FIRSTEVENT,SDL_LASTEVENT);
#endif

		//Also clear the Mouse flags.
		Mouse.FlushEvents();
		Screen.EndFrame();
	}
}

void GameEngine::AddActivity(IActivity* activity)
{
	activity->Start(glfwGetTime());
	this->activities.push(activity);
}

void GameEngine::StopTopActivity(void)
{
	IActivity* currentActivity;
	currentActivity = activities.top();
	currentActivity->Stop();
}

IActivity* GameEngine::GetCurrentActivity(void)
{
	return activities.top();
}
