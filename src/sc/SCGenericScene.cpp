//
//  WildCatBase.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "SCGenericScene.h"

#include "precomp.h"

#include "SCConvPlayer.h"

SCGenericScene::SCGenericScene()
{
}

SCGenericScene::~SCGenericScene()
{
}

void SCGenericScene::Init(Scene sc, std::optional<Scene> next)
{
	_next = next;
	_font = FontManager.GetFont("");

	switch (sc) {
	case Scene::WildcatBaseHangar:
		InitShapes({
			OptHangar,
			OptHangarDoor0,
			OptHangarDoor1,
			//OptF16,
			OptHangarTruck,
			OptHangarChar0,
			OptHangarChar2,
			OptHangarChar3,
		});
		AddInteraction({ 17, 78, 54, 110 }, Scene::WildcatBaseOffice);
		//AddInteraction({ 17, 78, 54, 110 }, Scene::Bar);
		AddInteraction({ 275, 84, 307, 137 }, Scene::WildcatBaseChangeroom);
		//AddInteraction({ 103, 21, 237, 76 }, Scene::WildcatTentInside);
		AddInteraction({ 103, 21, 237, 76 }, Scene::CutsceneMoveA, Scene::WildcatTentInside);
		AddInteraction({ 189, 102, 198, 124 }, Character::Janet);
		break;
	case Scene::WildcatBaseOffice:
		InitShapes({
			OptDesk,
			OptHangarChar1,
		});
		AddInteraction({ 0, 180, 320, 200 }, Scene::WildcatBaseHangar);
		AddInteraction({ 74, 75, 122, 110 }, Character::Janet);
		break;
	case Scene::WildcatBaseChangeroom:
		InitShapes({
			OptChangeRoomBg,
			OptChangeRoomFg,
		});
		AddInteraction({ 56, 75, 92, 135 }, Scene::WildcatBaseHangar);
		AddInteraction({ 274, 85, 289, 107 }, Scene::WildcatBasePinupF);
		AddInteraction({ 143, 88, 156, 106 }, Scene::WildcatBasePinupM);
		AddInteraction({ 99, 123, 170, 145 }, Scene::Exit);
		break;
	case Scene::WildcatBasePinupF:
		InitShapes({
			OptPinupF,
		});
		AddInteraction({ 0, 0, 320, 200 }, Scene::WildcatBaseChangeroom);
		break;
	case Scene::WildcatBasePinupM:
		InitShapes({
			OptPinupM,
		});
		AddInteraction({ 0, 0, 320, 200 }, Scene::WildcatBaseChangeroom);
		break;
	case Scene::WildcatTentInside:
		InitShapes({
			OptTentInsideBg,
			OptTentInsideFg,
			OptTentInsideChar0,
			//OptTentInsideChar1,
		});
		AddInteraction({ 86, 95, 117, 131 }, Scene::WildcatTentOutside);
		AddInteraction({ 4, 129, 94, 159 }, Scene::Exit);
		AddInteraction({ 230, 92, 282, 126 }, Scene::WildcatBaseHangar);
		break;
	case Scene::WildcatTentOutside:
		InitShapes({
			OptTentOutsideBg,
			OptTentOutPlane,
		});
		AddInteraction({ 6, 120, 190, 200 }, Scene::WildcatTentInside);
		AddInteraction({ 110, 0, 320, 107 }, Scene::WildcatTentWeapons);
		break;
	case Scene::WildcatTentWeapons:
		InitShapes({
			OptOutsideHanger,
			OptTentOutside00,
		});
		AddInteraction({ 203, 28, 268, 67 }, Scene::WildcatTentOutside);
		break;
	case Scene::Bar:
		InitShapes({
			OptBarBg,
			OptBarFg,
			OptBarChar0,
			OptBarChar1,
			OptBarChar2,
		});
		AddInteraction({ 0, 0, 320, 200 }, Scene::WildcatBaseHangar);
		break;
	case Scene::BarTables:
		InitShapes({
			OptBarTables,
			OptBarTablesChar0,
			OptBarTablesChar1,
			OptBarTablesChar2,
		});
		AddInteraction({ 0, 0, 320, 200 }, Scene::WildcatBaseHangar);
		break;
	case Scene::CutsceneMoveA:
		InitShapes({
			OptCutsceneMoveA0,
			OptCutsceneMoveA1,
		});
		break;
	}
}

void SCGenericScene::AddInteraction(Area area, Scene sc, std::optional<Scene> next)
{
	_interactions.emplace_back(area, [sc, next] (SCGenericScene* current) {
		current->Stop();
		if (sc != Scene::Exit)
			Game.MakeActivity<SCGenericScene>(sc, next);
	});
}

void SCGenericScene::AddInteraction(Area area, Character ch)
{
	_interactions.emplace_back(area, [ch] (SCGenericScene* current) {
		current->Stop();
		Game.MakeActivity<SCConvPlayer>().SetID(14);
	});
}

void SCGenericScene::RunFrame(const FrameParams& p)
{
	const double t = p.currentTime - startTime;
	Mouse.SetMode(SCMouse::CURSOR);

	const Point2D mpos = Mouse.GetPosition();

	const double fspeed = 3.0;
	double fade = 0.0;
	if (!_activated) {
		fade = 1.0 - std::min(1.0, fspeed * t);
		for (const auto& interation : _interactions) {
			const auto area = interation.first;
			if (mpos.x < area.x0 || mpos.x > area.x1 || mpos.y < area.y0 || mpos.y > area.y1)
				continue;
			Mouse.SetMode(SCMouse::VISOR);
			if (Mouse.buttons[SCMouseButton::LEFT].event == SCMouseButton::RELEASED) {
				_activated = { t, interation.second };
			}
		}
	} else {
		fade = std::min(1.0, fspeed * (t - _activated->first));
		if (fade == 1.0) {
			auto action = std::move(_activated->second);
			_activated = {};
			action(this);
		}
	}

	FrameParams np = p;
	np.currentTime = t;
	np.fade = fade;
	const bool running = Frame2D(np, shapes, [&] {
		VGA.PrintText(_font, { 10, 10 }, 1, 3, 5, "%d,%d", mpos.x, mpos.y);
	});

	if (!running && _next && !_activated) {
		_activated = { t, [&] (SCGenericScene*) {
			Stop();
			Game.MakeActivity<SCGenericScene>(*_next);
		}};
	}
}
