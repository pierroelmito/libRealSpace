//
//  IActivity.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <vector>

#include "Texture.h"
#include "PakArchive.h"
#include "PaletteIDs.h"
#include "ShapeIDs.h"
#include "RLEShape.h"
#include "SCMouse.h"
#include "SCButton.h"

class SCButton;

using GTime = double;
constexpr GTime TimeToMSec = 1000.0;

class IActivity
{
public:
	struct FrameParams
	{
		GTime currentTime{};
	};

	virtual ~IActivity();

	virtual void Focus() { focused = true;}
	virtual void UnFocus() { focused = false;}
	virtual void Start(GTime startTime)
	{
		this->startTime = startTime;
		this->running = true;
	}

	virtual void Init() = 0;
	virtual void RunFrame(const FrameParams& p) = 0;

	void Stop() { running = false;}
	bool IsRunning() const { return running; }
	void SetTitle(const char* title);
	void Frame2D(std::initializer_list<RLEShape*> shapes);

protected:
	IActivity();
	SCButton* CheckButtons();
	void DrawButtons();

	VGAPalette palette;
	std::vector<SCButton*> buttons;
	GTime startTime{};

private:
	bool running;
	bool focused;
};

