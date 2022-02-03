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

std::set<int> JustPressed;

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
		JustPressed.insert(key);
	if (mods == GLFW_MOD_CONTROL && key == GLFW_KEY_R && action == GLFW_PRESS)
		UserProperties::Get().Reload();
}

void GameEngine::Init()
{
	const int scale = UserProperties::Get().Ints.Get("WindowScale", 3);

	Assets.Init(); //Load all TREs and PAKs
	FontManager.Init(Assets.tres[AssetManager::TRE_MISC]);
	ConvAssets.Init(); //Load assets needed for Conversations (char and background)
	Screen.Init(scale); //Load Main Palette and Initialize the GL
	VGA.Init();
	Audio.Init();
	Renderer.Init();
	Mouse.Init(); //Load the Mouse Cursor

	glfwSetMouseButtonCallback(win, MouseButtonCallback);
	glfwSetKeyCallback(win, KeyCallback);
	glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

void GameEngine::Release()
{
	Mouse.Release();
	Renderer.Release();
	Audio.Release();
	VGA.Release();
	Screen.Release();
	ConvAssets.Release();
	FontManager.Release();
	Assets.Release();
}

bool GameEngine::IsKeyPressed(uint32_t keyCode)
{
	return glfwGetKey(win, keyCode) == GLFW_PRESS;
}

bool GameEngine::PumpEvents(void)
{
#if 1
	double mx, my;
	glfwGetCursorPos(win, &mx, &my);
	double x = 320.0 * saturate(mx / double(Screen.width));
	double y = 200.0 * saturate(my / double(Screen.height));
	Mouse.SetPosition({
		int32_t(x),
		int32_t(y)
	});
#else
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

		//Audio.Update();

		//Allow the active activity to Run and Render
		IActivity* currentActivity = activities.top();
		if (currentActivity->IsRunning()) {
			currentActivity->Focus();
			const GTime t = glfwGetTime();
			currentActivity->RunFrame({ JustPressed, t, t - currentActivity->GetStartTime() });
			currentActivity->UnFocus();
		} else{
			activities.pop();
			delete currentActivity;
			if (!activities.empty())
				activities.top()->SetStartTime(glfwGetTime());
		}

		JustPressed.clear();

		//Swap GL buffer
		Screen.Refresh();

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
