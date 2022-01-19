//
//  IActivity.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <vector>
#include <memory>
#include <set>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Texture.h"
#include "PakArchive.h"
#include "PaletteIDs.h"
#include "ShapeIDs.h"
#include "RLEShape.h"
#include "SCMouse.h"
#include "SCButton.h"
#include "ShapeIDs.h"

class SCButton;

using GTime = double;
constexpr GTime TimeToMSec = 1000.0;

class IActivity
{
public:
	struct FrameParams
	{
		const std::set<int>& pressed;
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

	//virtual void Init() = 0;
	virtual void RunFrame(const FrameParams& p) = 0;

	void Stop() { running = false;}
	bool IsRunning() const { return running; }
	void SetTitle(const char* title);
	void Frame2D(std::vector<std::unique_ptr<RLEShape>>& shapes, std::function<void()> userDraw = {});

protected:
	IActivity();
	SCButton* CheckButtons();
	void DrawButtons();

	void ReadPatch(VGAPalette& pal, const ByteSlice& bytes);
	void ReadPatch(const ByteSlice& bytes);
	bool ReadPatches(std::initializer_list<int> patches, const char* pak = TRE_DATA_GAMEFLOW "OPTPALS.PAK");

	std::unique_ptr<PakArchive> GetPak(const char* label, const ByteSlice& bs);
	RLEShape& AddShape() { return *shapes.emplace_back(new RLEShape()); }

	bool InitShapes(std::initializer_list<PalBg> ids);
	bool InitShape(RLEShape& shp, const char* label, const ByteSlice& entry);
	void InitShapeAt(RLEShape& shp, const Point2D& position, const char* label, const ByteSlice& entry);

	SCButton* MakeButton(Point2D pos, Point2D size, PakArchive& subPak, size_t upEntry, size_t downEntry, SCButton::ActionFunction&& fn);

	VGAPalette palette;
	//std::vector<std::unique_ptr<RLEShape>> shapes;
	std::vector<std::unique_ptr<RLEShape>> shapes;
	std::vector<SCButton*> buttons;
	GTime startTime{};

private:
	bool running;
	bool focused;
};

