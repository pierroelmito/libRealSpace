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

struct SceneID { int id{}; };

struct Scene {
	static constexpr SceneID WildcatBaseHangar = { 0x0b };
	static constexpr SceneID WildcatBaseOffice = { 0x0d };
	static constexpr SceneID WildcatBaseChangeroom = { 0x0e };
	static constexpr SceneID WildcatBasePinupF = { 0x6b };
	static constexpr SceneID WildcatBasePinupM = { 0x87 };
	static constexpr SceneID WildcatTentInside = { 0x1d };
	static constexpr SceneID Bar = { 0x06 };
	static constexpr SceneID WildcatTentOutside = { 0x14 };
	static constexpr SceneID WildcatTentWeapons = { 0xf01 };
	static constexpr SceneID BarTables = { 0x01 };
	static constexpr SceneID Exit = { 0xf03 };
};

enum class Character {
	Janet,
};

enum class Mission {
	M00,
};

class RSFont;

struct Area
{
	int x0, y0, x1, y1;
};

using Quad = std::array<Point2D, 4>;

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

	void AddInteraction(Area area, SceneID sc, std::optional<int> cutscene = {});
	void AddInteraction(Area area, Character ch);
	void AddInteraction(Area area, Mission m);

	virtual void Init(SceneID sc);
	void InitFromScene(SceneID id, uint32_t sprMask);
	virtual void RunFrame(const FrameParams& p) override;

protected:
	using AreaAction = std::function<void(SCGenericScene*)>;
	struct Interaction
	{
		std::vector<Area> areas{};
		std::vector<Quad> quads{};
		std::string label{};
		AreaAction action{};
	};
	std::vector<Interaction> _interactions;
	std::optional<std::pair<double, AreaAction>> _activated;
	RSFont* _font;
	uint8_t _textColor{ 1 };

	bool IsHovered(const Interaction& interaction) const;
};
