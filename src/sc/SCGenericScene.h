//
//  WildCatBase.h
//  libRealSpace
//
//  Created by fabien sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include "IActivity.h"

#include <optional>

enum class Scene {
	WildcatBaseHangar,
	WildcatBaseOffice,
	WildcatBaseChangeroom,
	WildcatBasePinupF,
	WildcatBasePinupM,
	WildcatTentInside,
	WildcatTentOutside,
	WildcatTentWeapons,
	Bar,
	BarTables,
	CutsceneMoveA,
	CutsceneMoveB,
	Exit,
};

enum class Character {
	Janet,
};

enum class Mission {
	M00,
};

class RSFont;

class SCCutScene : public IActivity
{
public:
	SCCutScene();
	virtual ~SCCutScene();

	static void PushAll();

	virtual void Init(int id);
	virtual void RunFrame(const FrameParams& p) override;
};

class SCGenericScene : public IActivity
{
public:
	SCGenericScene();
	virtual ~SCGenericScene();

	struct Area
	{
		int x0, y0, x1, y1;
	};

	void AddInteraction(Area area, Scene sc, std::optional<Scene> next = {});
	void AddInteraction(Area area, Character ch);
	void AddInteraction(Area area, Mission m);

	virtual void Init(Scene sc, std::optional<Scene> next = {});
	virtual void RunFrame(const FrameParams& p) override;

protected:
	using AreaAction = std::function<void(SCGenericScene*)>;
	std::vector<std::pair<Area, AreaAction>> _interactions;
	std::optional<std::pair<double, AreaAction>> _activated;
	std::optional<Scene> _next{};
	RSFont* _font;
};
