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
	Exit,
};

enum class Character {
	Janet,
};

class RSFont;

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

	virtual void Init(Scene sc, std::optional<Scene> next = {});
	virtual void RunFrame(const FrameParams& p) override;

protected:
	std::vector<std::pair<Area, std::function<void(SCGenericScene*)>>> _interactions;
	std::optional<std::pair<double, std::function<void(SCGenericScene*)>>> _activated;
	std::optional<Scene> _next{};
	RSFont* _font;
};
