//
//  Game.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include <stack>

class IActivity;

class GameEngine
{
public:
	GameEngine();
	~GameEngine();

	void Init();
	void Release();
	void Run();

	template <typename... T>
	void Log(const char* text, const T&... ts)
	{
		fprintf(stdout, text, ts...);
	}
	template <typename... T>
	void LogError(const char* text, const T&... ts)
	{
		fprintf(stderr, text, ts...);
	}
	template <typename... T>
	void Terminate(const char* reason, const T&... ts)
	{
		Log("Terminating: ");
		fprintf(stdout, reason, ts...);
		Log("\n");
		exit(0);
	}

	//Add an activity on the top of the stack.
	template <class T, typename... ARGS>
	T& MakeActivity(const ARGS&... args) {
		T* activity = new T();
		activity->Init(args...);
		AddActivity(activity);
		return *activity;
	}
	void AddActivity(IActivity* activity);
	void StopTopActivity(void);
	IActivity* GetCurrentActivity(void);

	void SetMouseLock(bool lock);
	bool IsKeyPressed(uint32_t keyCode);
	bool PumpEvents(void);

private:
	std::stack<IActivity*> activities;
};
