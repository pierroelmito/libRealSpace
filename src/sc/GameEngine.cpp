//
//  Game.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "main.h"

#include "IActivity.h"
#include "UserProperties.h"
#include <raylib.h>

GameEngine::GameEngine() { }

GameEngine::~GameEngine() { }

void GameEngine::Init()
{
	const int scale = UserProperties::Get().Ints.Get("WindowScale", 3);

	Assets.Init(); // Load all TREs and PAKs
	FontManager.Init(Assets.tres[AssetManager::TRE_MISC]);
	ConvAssets.Init(); // Load assets needed for Conversations (char and background)
	Screen.Init(scale); // Load Main Palette and Initialize the GL
	VGA.Init();
	Audio.Init();
	Renderer.Init();
	Mouse.Init(); // Load the Mouse Cursor
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

void GameEngine::Run()
{
	GTime pt = GetTime();

	while (Screen.StartFrame() && activities.size() > 0) {
		// Audio.Update();

		Vector2 mpos = GetMousePosition();
		Mouse.SetPosition({ int(mpos.x) / Screen.scale, int(mpos.y) / Screen.scale });
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			Mouse.buttons[0].event = SCMouseButton::PRESSED;
		} else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
			Mouse.buttons[0].event = SCMouseButton::RELEASED;
		}

		// Allow the active activity to Run and Render
		IActivity* currentActivity = activities.top();
		if (currentActivity->IsRunning()) {
			currentActivity->Focus();
			const GTime t = GetTime();
			const GTime dt = t - pt;
			pt = t;
			const auto w = GetScreenWidth();
			const auto h = GetScreenHeight();
			currentActivity->RunFrame({ t, t - currentActivity->GetStartTime(), dt, 0, w, h });
			currentActivity->UnFocus();
		} else {
			activities.pop();
			delete currentActivity;
			if (!activities.empty())
				activities.top()->SetStartTime(GetTime());
		}

		// Also clear the Mouse flags.
		Mouse.FlushEvents();
		Screen.EndFrame();
	}
}

void GameEngine::AddActivity(IActivity* activity)
{
	activity->Start(GetTime());
	this->activities.push(activity);
}

void GameEngine::StopTopActivity()
{
	activities.top()->Stop();
}
