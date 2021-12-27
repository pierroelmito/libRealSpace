//
//  Game.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <cstdio>
#include <cstdlib>

#include <stack>

class IActivity;

class GameEngine
{
public:
	GameEngine();
	~GameEngine();

	void Init(void);
	void Run(void);

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
	template <class T>
	void MakeActivity() {
		T* activity = new T();
		activity->Init();
		AddActivity(activity);
	}
	void AddActivity(IActivity* activity);
	void StopTopActivity(void);
	IActivity* GetCurrentActivity(void);

	bool PumpEvents(void);

private:
	std::stack<IActivity*> activities;
};
