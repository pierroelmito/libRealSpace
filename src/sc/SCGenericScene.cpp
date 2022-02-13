//
//  WildCatBase.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "SCGenericScene.h"

#include "precomp.h"

#include <sstream>
#include <iomanip>

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

	struct Spr {
		int shp{};
		int target{};
		std::string label;
		std::vector<uint16_t> seq;
		std::vector<Area> rects;
		std::vector<Quad> quads;
	};

	struct Scene {
		uint16_t info{};
		uint16_t colr{};
		uint16_t tune{};
		std::vector<int> bgPals;
		std::vector<int> fgPals;
		std::vector<int> bgShps;
		std::vector<Spr> sprs;
	};

	std::vector<Scene> scenes;
	std::map<int, int> sceneIdToIndex;

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
	void ReadShots(IffChunk* chkShots)
	{
		for (IffChunk* child : chkShots->children) {
			if (child->children.size() != 3)
				continue;
			Shot& shot = shots.emplace_back();
			auto& info = *child->children[0];
			auto& shapes = *child->children[1];
			auto& pals = *child->children[2];
			for (IffChunk* shp : shapes.children) {
				shot.shps.push_back(shp->data[0]);
			}
			for (IffChunk* pal : pals.children) {
				shot.pals.push_back(pal->data[0]);
			}
		}
	}
	void ReadSceneSprite(IffChunk* chk, Spr& spr)
	{
		for (IffChunk* i : chk->children) {
			ByteStream bs(i->data);
			if (i->id == IdToUInt("LABL")) {
				spr.label = (const char*)i->data;
			} else if (i->id == IdToUInt("SHAP")) {
				const uint8_t b0 = bs.ReadByte();
				const uint8_t b1 = bs.ReadByte();
				spr.target = b0;
				spr.shp = b1;
			} else if (i->id == IdToUInt("SEQU")) {
				const int count = i->size / 2;
				spr.seq.reserve(count);
				for (int s = 0; s < count; ++s)
					spr.seq.push_back(bs.ReadShort());
			} else if (i->id == IdToUInt("RECT")) {
				const auto x0 = bs.ReadShort();
				const auto y0 = bs.ReadShort();
				const auto x1 = bs.ReadShort();
				const auto y1 = bs.ReadShort();
				spr.rects.push_back({ x0, y0, x1, y1 });
			} else if (i->id == IdToUInt("QUAD")) {
				auto& quad = spr.quads.emplace_back();
				for (int q = 0; q < 4; ++q)
					quad[q] = { bs.ReadShort(), bs.ReadShort() };
				std::swap(quad[2], quad[3]); // ABDC --> ABCD
			}
		}
	}
	void ReadScenes(IffChunk* chkScenes)
	{
		for (IffChunk* child : chkScenes->children) {
			Scene& sc = scenes.emplace_back();
			for (IffChunk* i : child->children) {
				ByteStream bs(i->data);
				if (i->id == IdToUInt("INFO")) {
					sc.info = bs.ReadShort();
				} else if (i->id == IdToUInt("COLR")) {
					sc.colr = bs.ReadShort();
				} else if (i->id == IdToUInt("TUNE")) {
					sc.tune = bs.ReadShort();
				} else if (i->subId == IdToUInt("BACK")) {
					for (IffChunk* j : i->children) {
						if (j->id == IdToUInt("PALT")) {
							sc.bgPals.push_back(j->data[0]);
						} else if (j->id == IdToUInt("SHAP")) {
							sc.bgShps.push_back(j->data[0]);
						}
					}
				} else if (i->subId == IdToUInt("FORE")) {
					for (IffChunk* j : i->children) {
						if (j->id == IdToUInt("PALT")) {
							sc.fgPals.push_back(j->data[0]);
						} else if (j->subId == IdToUInt("SPRT")) {
							Spr& spr = sc.sprs.emplace_back();
							ReadSceneSprite(j, spr);
						}
					}
				}
			}
		}
	}
	void ReadScenes()
	{
		auto& treGameFlow = Assets.tres[AssetManager::TRE_GAMEFLOW];
		TreEntry* iffScenes = treGameFlow.GetEntryByName(TRE_DATA_GAMEFLOW "OPTIONS.IFF");
		IffLexer lexer;
		lexer.InitFromRAM(*iffScenes);
		lexer.List(stdout);
		ReadScenes(lexer.GetChunkByID("OPTS"));
		ReadShots(lexer.GetChunkByID("ESTB"));
		for (int i = 0; i < scenes.size(); ++i)
			sceneIdToIndex[scenes[i].info] = i;
	}
};

class GameFlow
{
public:
	void ApplyInteraction(IActivity& activity, SceneID from, int to);
};

void GameFlow::ApplyInteraction(IActivity& activity, SceneID from, int to)
{
	const std::map<std::pair<int, int>, int> cinematics = {
		{ { 0xb, 0x6 }, 0 },
		{ { 0x6, 0xb }, 1 },
	};

	const auto& sd = SceneData::Get();
	const bool isScene = sd.sceneIdToIndex.find(to) != sd.sceneIdToIndex.end();
	if (isScene) {
		activity.Stop();
		Game.MakeActivity<SCGenericScene>(SceneID{ to });
	} else if (to == 0x129) {
		Game.MakeActivity<SCStrike>();
	} else if (to == 0x67 || to == 0x68) {
		activity.Stop();
	} else if (to == 0x12) {
		activity.Stop();
		Game.MakeActivity<SCGenericScene>(Scene::WildcatTentInside);
	} else if (to == 0xc || to == 0x81) {
		activity.Stop();
		Game.MakeActivity<SCStrike>();
	} else if (to == 0x1c) {
		Game.MakeActivity<SCConvPlayer>().SetID(14);
	};

	const auto itf = cinematics.find({ from.id, to });
	if (itf != cinematics.end())
		Game.MakeActivity<SCCutScene>(itf->second);
}

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

void SCGenericScene::InitFromScene(SceneID id, uint32_t sprMask)
{
	printf("init scene 0x%02d\n", id);

	const auto& sd = SceneData::Get();
	auto itf = sd.sceneIdToIndex.find(id.id);
	if (itf == sd.sceneIdToIndex.end())
		return;

	const auto& sc = sd.scenes[itf->second];
	_textColor = sc.colr;

	shapes.clear();

	auto& treGameFlow = Assets.tres[AssetManager::TRE_GAMEFLOW];
	auto pakShps = GetPak(OPTSHPS, *treGameFlow.GetEntryByName(OPTSHPS));
	auto pakPals = GetPak(OPTPALS, *treGameFlow.GetEntryByName(OPTPALS));

	for (const auto& palId : sc.bgPals) {
		ReadPatch(pakPals->GetEntry(palId));
	}

	for (const auto& palId : sc.fgPals) {
		ReadPatch(pakPals->GetEntry(palId));
	}

	for (const auto& shpId : sc.bgShps) {
		if (shpId < pakShps->GetNumEntries()) {
			auto& shp = AddShape();
			shp.am = AnimMode::Character;
			if (!InitShape(shp, "", pakShps->GetEntry(shpId)))
				printf("titi\n");
		}
	}

	const std::map<int, std::pair<AnimMode, AnimMode>> specialAnimModes = {
		{ 0x14, { AnimMode::First, AnimMode::Character } },
		{ 0x11,  { AnimMode::Second, AnimMode::Character } },
	};

	for (const auto& spr : sc.sprs) {
		const bool use = sprMask & 1;
		sprMask >>= 1;

		const bool isScene = sd.sceneIdToIndex.contains(spr.target);
		const char* vl = use ? "[x] " : "[ ] ";
		if (isScene) {
			printf("\t%sinteraction - go to scene 0x%02x '%s'\n", vl, spr.target, spr.label.c_str());
		} else {
			printf("\t%snot in declared scenes: 0x%02x '%s'\n", vl, spr.target, spr.label.c_str());
		}

		if (!use)
			continue;

		char buffer[512]{};
		sprintf(buffer, "0x%02x - %s - %s - 0x%02x", spr.shp, spr.label.c_str(), isScene ? "scene" : "??", spr.target);

		Interaction& interaction = _interactions.emplace_back();
		interaction.label = buffer;
		interaction.areas = spr.rects;
		interaction.quads = spr.quads;

		const int target{ spr.target };
		interaction.action = [target] (SCGenericScene* current) {
			GameFlow flow;
			flow.ApplyInteraction(*current, current->_currentScene, target);
		};

		auto itf = specialAnimModes.find(spr.shp);
		const AnimMode amIdle = itf != specialAnimModes.end() ? itf->second.first : AnimMode::Character;
		const AnimMode amClick = itf != specialAnimModes.end() ? itf->second.second : AnimMode::Character;
		interaction.am = amClick;

		if (spr.shp < pakShps->GetNumEntries()) {
			interaction.shpIndex = shapes.size();
			auto& shp = AddShape();
			shp.am = amIdle;
			if (!spr.seq.empty())
				shp.anim = &spr.seq;
			if (!InitShape(shp, "", pakShps->GetEntry(spr.shp)))
				printf("titi\n");
		}
	}
}

void SCGenericScene::Init(SceneID sc)
{
	_currentScene = sc;
	auto& sd = SceneData::Get();

	_font = FontManager.GetFont("");

	uint32_t targetMask = ~0u;

	switch (sc.id) {
	case Scene::WildcatBaseHangar.id:
		targetMask = 0b10010100111100u;
		//AddInteraction({ 103, 21, 237, 76 }, Scene::WildcatTentInside);
		//AddInteraction({ 189, 102, 198, 124 }, Character::Janet);
		break;
	case Scene::WildcatBaseChangeroom.id:
		targetMask = 0b1101111u;
		AddInteraction({ 99, 123, 170, 145 }, Scene::Exit);
		break;
	case Scene::WildcatTentInside.id:
		targetMask = 0b11010100000111u;
		AddInteraction({ 4, 129, 94, 159 }, Scene::Exit);
		break;
	case Scene::WildcatTentOutside.id:
		targetMask = 0b1110010u;
		//AddInteraction({ 110, 0, 320, 107 }, Scene::WildcatTentWeapons);
		break;
	case Scene::WildcatTentWeapons.id:
		//InitShapes({
		//	OptOutsideHanger,
		//	OptTentOutside00,
		//});
		//AddInteraction({ 203, 28, 268, 67 }, Scene::WildcatTentOutside);
		//AddInteraction({ 122, 73, 198, 111 }, Mission::M00);
		break;
	case Scene::Bar.id:
		targetMask = 0b10111u;
		break;
	case Scene::BarTables.id:
		targetMask = 0b10111u;
		break;
	default:
		break;
	}

	InitFromScene(sc, targetMask);
}

void SCGenericScene::AddInteraction(Area area, SceneID sc, std::optional<int> cutscene)
{
	Interaction& interaction = _interactions.emplace_back();
	interaction.areas = { area };
	interaction.action = [sc, cutscene] (SCGenericScene* current) {
		current->Stop();
		if (sc.id != Scene::Exit.id) {
			Game.MakeActivity<SCGenericScene>(sc);
			if (cutscene)
				Game.MakeActivity<SCCutScene>(*cutscene);
		}
	};
}

void SCGenericScene::AddInteraction(Area area, Character ch)
{
	Interaction& interaction = _interactions.emplace_back();
	interaction.areas = { area };
	interaction.action = [ch] (SCGenericScene* current) {
		current->Stop();
		Game.MakeActivity<SCConvPlayer>().SetID(14);
	};
}

void SCGenericScene::AddInteraction(Area area, Mission m)
{
	Interaction& interaction = _interactions.emplace_back();
	interaction.areas = { area };
	interaction.action = [m] (SCGenericScene* current) {
		current->Stop();
		Game.MakeActivity<SCGenericScene>(Scene::WildcatBaseHangar); // place to go after mission end
		Game.MakeActivity<SCStrike>();
	};
}

bool InsideQuad(Point2D p, const std::array<Point2D, 4>& quad)
{
	int pos = 0;
	int neg = 0;
	for (int i = 0; i < 4; ++i) {
		const Point2D a = quad[i];
		const Point2D b = quad[(i + 1) % 4];
		const auto d  = (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x);
		if (d > 0)
			pos++;
		if (d < 0)
			neg++;
		if (pos > 0 && neg > 0)
			return false;
	}
	return true;
}

bool SCGenericScene::IsHovered(const Interaction& interaction) const
{
	const Point2D mpos = Mouse.GetPosition();
	for (const auto& area : interaction.areas) {
		if (mpos.x >= area.x0 && mpos.x <= area.x1 && mpos.y >= area.y0 && mpos.y <= area.y1)
			return true;
	}
	for (const auto& quad : interaction.quads) {
		if (InsideQuad(mpos, quad))
			return true;
	}
	return false;
}

void SCGenericScene::RunFrame(const FrameParams& p)
{
	if (p.pressed.contains(GLFW_KEY_F1))
		VGA.ShowPalette() = !VGA.ShowPalette();

	const double t = p.activityTime;
	Mouse.SetMode(SCMouse::CURSOR);

	const Point2D mpos = Mouse.GetPosition();

	std::optional<std::string> hoveredAction;

	const double fadeSpeed = 2.0;
	double fade = 0.0;
	if (!_activated) {
		fade = 1.0 - std::min(1.0, fadeSpeed * t);
		for (const auto& interaction : _interactions) {
			if (IsHovered(interaction)) {
				Mouse.SetMode(SCMouse::VISOR);
				hoveredAction = interaction.label;
				if (interaction.action && Mouse.buttons[SCMouseButton::LEFT].event == SCMouseButton::RELEASED) {
					if (interaction.shpIndex) {
						auto& shape = shapes[*interaction.shpIndex];
						if (shape.am != AnimMode::Character) {
							shape.timeOffset = p.activityTime - 1.0001 / FrameMul;
							shape.am = interaction.am;
						}
					}
					_activated = { t, interaction.action };
				}
			}
		}
	} else {
		fade = std::min(1.0, fadeSpeed * (t - _activated->first));
		if (fade == 1.0) {
			auto action = std::move(_activated->second);
			_activated = {};
			action(this);
		}
	}

	FrameParams np = p;
	np.fade = fade;
	const bool running = Frame2D(np, shapes, [&] {
		VGA.PrintText(_font, { 10, 10 }, _textColor, 3, 5, "%d,%d", mpos.x, mpos.y);
		VGA.PrintText(_font, { 10, 190 }, _textColor, 3, 5, "%s", hoveredAction.value_or("").c_str());
	});
}
