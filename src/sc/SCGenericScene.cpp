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
#include "SCStrike.h"

class SceneData
{
public:
	struct Shot {
		std::vector<int> pals;
		std::vector<int> shps;
	};
	std::vector<Shot> shots;
	static SceneData& Get()
	{
		static SceneData v;
		return v;
	}
protected:
	SceneData()
	{
		ReadScenes();
	}
	void ReadScenes()
	{
		auto& treGameFlow = Assets.tres[AssetManager::TRE_GAMEFLOW];
		TreEntry* scenes = treGameFlow.GetEntryByName(TRE_DATA_GAMEFLOW "OPTIONS.IFF");
		IffLexer lexer;
		lexer.InitFromRAM(*scenes);
		lexer.List(stdout);

		{
			IffChunk* chkScenes = lexer.GetChunkByID("OPTS");
			for (IffChunk* child : chkScenes->childs) {
				if (child->childs.size() != 5)
					continue;
				auto& info = *child->childs[0];
				auto& colr = *child->childs[1];
				auto& tune = *child->childs[2];
				auto& back = *child->childs[3];
				auto& fore = *child->childs[4];
				printf("scene...\n");
			}
		}

		{
			IffChunk* chkShots = lexer.GetChunkByID("ESTB");
			for (IffChunk* child : chkShots->childs) {
				if (child->childs.size() != 3)
					continue;
				Shot& shot = shots.emplace_back();
				auto& info = *child->childs[0];
				auto& shapes = *child->childs[1];
				auto& pals = *child->childs[2];
				for (IffChunk* shp : shapes.childs) {
					shot.shps.push_back(shp->data[0]);
				}
				for (IffChunk* pal : pals.childs) {
					shot.pals.push_back(pal->data[0]);
				}
			}
		}
	}
};

SCCutScene::SCCutScene()
{
}

SCCutScene::~SCCutScene()
{
}

void SCCutScene::PushAll()
{
	const auto& sd = SceneData::Get();
	for (int i = 0; i < sd.shots.size(); ++i)
		Game.MakeActivity<SCCutScene>(i);
}

void SCCutScene::Init(int id)
{
	const auto& sd = SceneData::Get();
	const auto& shot = sd.shots[id];

	shapes.clear();

	auto& treGameFlow = Assets.tres[AssetManager::TRE_GAMEFLOW];
	auto pakShps = GetPak(OPTSHPS, *treGameFlow.GetEntryByName(OPTSHPS));
	auto pakPals = GetPak(OPTPALS, *treGameFlow.GetEntryByName(OPTPALS));

	for (const auto& palId : shot.pals) {
		ReadPatch(pakPals->GetEntry(palId));
	}

	for (const auto& shpId : shot.shps) {
		auto& shp = AddShape();
		if (!InitShape(shp, "", pakShps->GetEntry(shpId)))
			printf("titi\n");
	}
}

void SCCutScene::RunFrame(const FrameParams& p)
{
	const bool running = Frame2D(p, shapes, [&] {});
	if (!running || p.pressed.contains(GLFW_KEY_ESCAPE)) {
		Stop();
	}
}

SCGenericScene::SCGenericScene()
{
}

SCGenericScene::~SCGenericScene()
{
}

void SCGenericScene::Init(Scene sc, std::optional<Scene> next)
{
	auto& sd = SceneData::Get();

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
			//OptHangarJeep,
			OptHangarChar0,
			OptHangarChar2,
			OptHangarChar3,
		});
		AddInteraction({ 17, 78, 54, 110 }, Scene::WildcatBaseOffice);
		//AddInteraction({ 17, 78, 54, 110 }, Scene::Bar);
		AddInteraction({ 275, 84, 307, 137 }, Scene::WildcatBaseChangeroom);
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
		AddInteraction({ 230, 92, 282, 126 }, Scene::CutsceneMoveB, Scene::WildcatBaseHangar);
		break;
	case Scene::WildcatTentOutside:
		InitShapes({
			OptTentOutsideBg,
			OptTentOutPlane00,
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
		AddInteraction({ 122, 73, 198, 111 }, Mission::M00);
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
	case Scene::CutsceneMoveB:
		InitShapes({
			OptLookOutside,
			OptJeep01,
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

void SCGenericScene::AddInteraction(Area area, Mission m)
{
	_interactions.emplace_back(area, [m] (SCGenericScene* current) {
		current->Stop();
		Game.MakeActivity<SCGenericScene>(Scene::WildcatBaseHangar); // place to go after mission end
		Game.MakeActivity<SCStrike>();
	});
}

void SCGenericScene::RunFrame(const FrameParams& p)
{
	const double t = p.activityTime;
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
	np.fade = fade;
	const bool running = Frame2D(np, shapes, [&] {
		VGA.PrintText(_font, { 10, 10 }, 1, 3, 5, "%d,%d", mpos.x, mpos.y);
	});

	if (_next && !_activated) {
		if (!running || p.pressed.contains(GLFW_KEY_ESCAPE)) {
			_activated = { t, [&] (SCGenericScene*) {
				Stop();
				Game.MakeActivity<SCGenericScene>(*_next);
			}};
		}
	}
}
