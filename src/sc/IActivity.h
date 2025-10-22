//
//  IActivity.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <memory>
#include <vector>

#include <raylib.h>

#include "PakArchive.h"
#include "RLEShape.h"
#include "RSSound.h"
#include "SCButton.h"
#include "SCMouse.h"
#include "ShapeIDs.h"
#include "Texture.h"

#include "ShapeIDs.h"

class SCButton;

using GTime = double;
constexpr GTime TimeToMSec = 1000.0;
constexpr GTime FrameMul = 15.0;

class IActivity {
public:
	struct SceneSchape {
		std::vector<std::unique_ptr<RLEShape>> frames;
		AnimMode am { AnimMode::Cutscene };
		GTime timeOffset {};
		const std::vector<uint16_t>* anim { nullptr };
	};

	struct FrameParams {
		GTime totalTime {};
		GTime activityTime {};
		GTime deltaTime {};
		float fade {};
		int ScWidth {};
		int ScHeight {};
	};

	using SceneSchapes = std::vector<SceneSchape>;

	virtual ~IActivity();

	virtual void Focus() { focused = true; }
	virtual void UnFocus() { focused = false; }
	virtual void Start(GTime startTime)
	{
		this->startTime = startTime;
		this->running = true;
	}

	// virtual void Init() = 0;
	virtual void RunFrame(const FrameParams& p) = 0;

	GTime GetStartTime() const { return startTime; }
	void SetStartTime(GTime t) { startTime = t; }
	void Stop() { running = false; }
	bool IsRunning() const { return running; }
	void SetTitle(const char* title);
	bool Frame2D(const FrameParams& p, SceneSchapes& shapes, std::function<void()> userDraw = {});

protected:
	SceneSchapes shapes;
	VGAPalette palette;
	std::vector<std::unique_ptr<SCButton>> buttons;
	std::vector<RSSoundInstance> sounds;
	GTime startTime {};
	bool running{};
	bool focused{};

	IActivity();

	SCButton* MakeButton(Point2D pos, Point2D size, PakArchive& subPak, size_t upEntry, size_t downEntry, SCButton::ActionFunction&& fn);
	SCButton* CheckButtons();
	void DrawButtons();
	void ReadPatch(VGAPalette& pal, const ByteSlice& bytes);
	void ReadPatch(const ByteSlice& bytes);
	bool ReadPatches(std::initializer_list<int> patches, const char* pak = TRE_DATA_GAMEFLOW "OPTPALS.PAK");
	std::unique_ptr<PakArchive> GetPak(const char* label, const ByteSlice& bs);
	SceneSchape& AddShape();
	RLEShape& AddSingleShape();
	bool InitShapes(std::initializer_list<PalBg> ids);
	bool InitShape(RLEShape& shp, const char* label, const ByteSlice& entry);
	bool InitShape(SceneSchape& shp, const char* label, const ByteSlice& entry);
	void InitShapeAt(RLEShape& shp, const Point2D& position, const char* label, const ByteSlice& entry);

};
