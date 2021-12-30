//
//  Game.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

#if USE_RAYLIB
#include <raylib.h>
#else
#include <SDL2/SDL.h>
#endif

#include "IActivity.h"

GameEngine::GameEngine()
{
}

GameEngine::~GameEngine()
{
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
}

bool GameEngine::AnyInput()
{
#if USE_RAYLIB
#else
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
#if USE_RAYLIB
	/*
	PollInputEvents();
	int key{};
	bool r{ false };
	do {
		key = GetKeyPressed();
		if (key != 0) {
			if (keyCode == key)
				r = true;
		}
	} while (key != 0);
	*/
	return ::IsKeyPressed(KEY_ENTER);
#else
	SDL_Event keybEvents[5];
	int numKeybEvents = SDL_PeepEvents(keybEvents,5,SDL_PEEKEVENT,SDL_KEYDOWN,SDL_KEYDOWN);
	for(int i= 0 ; i < numKeybEvents ; i++){
		SDL_Event* event = &keybEvents[i];
		if (event->key.keysym.sym == keyCode)
			return true;
	}
#endif
	return false;
}

bool GameEngine::PumpEvents(void)
{
#if USE_RAYLIB
	Vector2 mpos = GetMousePosition();
	Mouse.SetPosition({
		int(mpos.x * 320.0f / Screen.width),
		int(mpos.y * 200.0f / Screen.height)
	});
	for (int i = 0; i < 3; ++i) {
		if (IsMouseButtonPressed(i))
			Mouse.buttons[i].event = SCMouseButton::PRESSED;
		else if (IsMouseButtonReleased(i))
			Mouse.buttons[i].event = SCMouseButton::RELEASED;
	}
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
#if USE_RAYLIB
			currentActivity->RunFrame({ GetTime() });
#else
			currentActivity->RunFrame({ SDL_GetTicks() });
#endif
			currentActivity->UnFocus();
		} else{
			activities.pop();
			delete currentActivity;
		}

		//Swap GL buffer
		Screen.Refresh();

#if USE_RAYLIB
#else
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
#if USE_RAYLIB
	activity->Start(GetTime());
#else
	activity->Start(SDL_GetTicks());
#endif
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
