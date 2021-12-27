//
//  IActivity.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <vector>

#include "RLEShape.h"
#include "Texture.h"

class SCButton;

class IActivity
{
public:
	virtual ~IActivity();
	virtual void Focus() { focused = true;}
	virtual void UnFocus() { focused = false;}
	virtual void Init() = 0;
	virtual void Start() { running = true;}
	virtual void RunFrame () = 0;
	void Stop() { running = false;}
	bool IsRunning() const { return running; }
    void SetTitle(const char* title);

	VGAPalette palette;

protected:
    IActivity();
	SCButton* CheckButtons();
	void DrawButtons();

	std::vector<SCButton*> buttons;

private:
    bool running;
    bool focused;
};

