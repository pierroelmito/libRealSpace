//
//  IActivity.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "IActivity.h"

#include "precomp.h"

#include "main.h"

#include "SCButton.h"

IActivity::IActivity()
{
}

IActivity::~IActivity()
{
}

void IActivity::SetTitle(const char* title){
	Screen.SetTitle(title);
}

bool IActivity::Frame2D(const FrameParams& p, SceneSchapes& shapes, std::function<void()> userDraw)
{
	CheckButtons();

	VGA.Clear();
	VGA.SetPalette(this->palette);

	bool running = false;

	for (auto& shape : shapes) {
		if (shape.frames.empty())
			continue;
		const GTime t = p.activityTime - shape.timeOffset;
		if (shape.frames.size() > 1) {
			if (shape.anim != nullptr) {
				VGA.DrawShape(*shape.frames[0]);
				const GTime speed = 8;
				const uint32_t idx = uint32_t(t * speed) % shape.anim->size();
				VGA.DrawShape(*shape.frames[1 + (*shape.anim)[idx]]);
			} else {
				switch (shape.am) {
				case AnimMode::Character:
					{
						VGA.DrawShape(*shape.frames[0]);
						uint32_t idx = uint32_t(t * FrameMul) % (shape.frames.size() - 1);
						VGA.DrawShape(*shape.frames[1 + idx]);
					}
					break;
				case AnimMode::Cutscene:
					{
						uint32_t maxIdx = shape.frames.size() - 1;
						uint32_t idx = std::min(maxIdx, uint32_t(t * FrameMul));
						if (idx != maxIdx)
							running = true;
						VGA.DrawShape(*shape.frames[idx]);
					}
					break;
				case AnimMode::First:
					{
						uint32_t idx = 0;
						VGA.DrawShape(*shape.frames[idx]);
					}
					break;
				case AnimMode::Second:
					{
						uint32_t idx = 1;
						VGA.DrawShape(*shape.frames[idx]);
					}
					break;
				}
			}
		} else {
			VGA.DrawShape(*shape.frames[0]);
		}
	}

	if (userDraw)
		userDraw();

	DrawButtons();
	Mouse.Draw();
	VGA.VSync(p.fade);

	return running;
}

SCButton* IActivity::CheckButtons(void)
{
	if (buttons.empty())
		return nullptr;

	for(auto&& button : buttons) {
		if (!button->IsEnabled())
			continue;

		const Point2D mpos = Mouse.GetPosition();
		if (mpos.x < button->position.x ||
			mpos.x > button->position.x + button->dimension.x ||
			mpos.y < button->position.y ||
			mpos.y > button->position.y + button->dimension.y
			)
		{
			button->SetAppearance(SCButton::APR_UP);
			continue;
		}

		//HIT !
		Mouse.SetMode(SCMouse::VISOR);

		if (Mouse.buttons[SCMouseButton::LEFT].event == SCMouseButton::PRESSED)
			button->SetAppearance(SCButton::APR_DOWN);

		//If the mouse button has just been released: trigger action.
		if (Mouse.buttons[SCMouseButton::LEFT].event == SCMouseButton::RELEASED) {
			Mouse.SetMode(SCMouse::CURSOR);
			button->OnAction();
		}

		return button.get();
	}

	Mouse.SetMode(SCMouse::CURSOR);
	return NULL;
}

void IActivity::ReadPatch(VGAPalette& pal, const ByteSlice& bytes)
{
	ByteStream paletteReader;
	paletteReader.Set(bytes.data);
	pal.ReadPatch(&paletteReader, 0);
}

void IActivity::ReadPatch(const ByteSlice& bytes)
{
	ReadPatch(palette, bytes);
}

bool IActivity::ReadPatches(std::initializer_list<int> patches, const char* pak)
{
	palette = VGA.GetPalette();
	auto& treGameFlow = Assets.tres[AssetManager::TRE_GAMEFLOW];
	auto palettesPak = GetPak(pak, *treGameFlow.GetEntryByName(pak));
	for (int p : patches) {
		if (p < 0 || p >= palettesPak->GetNumEntries())
			return false;
		ReadPatch(palettesPak->GetEntry(p));
	}
	return true;
}

IActivity::SceneSchape& IActivity::AddShape()
{
	return shapes.emplace_back();
}

RLEShape& IActivity::AddSingleShape()
{
	auto& v = shapes.emplace_back();
	return *v.frames.emplace_back(std::make_unique<RLEShape>());
}

bool IActivity::InitShapes(std::initializer_list<PalBg> ids)
{
	shapes.clear();

	printf("init shapes\n");

	auto& treGameFlow = Assets.tres[AssetManager::TRE_GAMEFLOW];
	std::map<std::string, std::unique_ptr<PakArchive>> paks;
	auto getPak = [&] (const char* path) -> PakArchive& {
		auto& pak = paks[path];
		if (!pak) {
			pak = GetPak(path, *treGameFlow.GetEntryByName(path));
			printf("\tload pak %s\n", path);
			//pak->List(stdout);
		}
		return *pak;
	};

	for (const PalBg& id : ids) {
		printf("\tpal : %d / bg : %d\n", id.pal, id.shp);
	}

	for (const PalBg& id : ids) {
		if (id.pakPal == nullptr)
			continue;
		auto& pak = getPak(id.pakPal);
		if (id.pal >= 0 && id.pal < pak.GetNumEntries())
			ReadPatch(pak.GetEntry(id.pal));
	}

	for (const PalBg& id : ids) {
		if (id.pakShp == nullptr)
			continue;
		auto& pak = getPak(id.pakShp);
		if (id.shp >= 0 && id.shp < pak.GetNumEntries()) {
			auto& shp = AddShape();
			shp.am = id.am;
			if (!InitShape(shp, "", pak.GetEntry(id.shp)))
				printf("titi\n");
		} else {
			printf("toto\n");
		}
	}

	return true;
}

std::unique_ptr<PakArchive> IActivity::GetPak(const char* label, const ByteSlice& bs)
{
	std::unique_ptr<PakArchive> r = std::make_unique<PakArchive>();
	r->InitFromRAM(label, bs);
	return r;
}

bool IActivity::InitShape(RLEShape& shp, const char* label, const ByteSlice& entry)
{
	PakArchive pak;
	pak.InitFromRAM(label, entry);
	const int entries = pak.GetNumEntries();
	if (entries == 0)
		return false;
	if (entries != 1)
		printf("\t%d entries\n", entries);
	shp.Init(pak.GetEntry(0));
	return true;
}

bool IActivity::InitShape(SceneSchape& shp, const char* label, const ByteSlice& entry)
{
	PakArchive pak;
	pak.InitFromRAM(label, entry);
	const int entries = pak.GetNumEntries();
	for (int i = 0; i < entries; ++i) {
		auto& s = shp.frames.emplace_back(std::make_unique<RLEShape>());
		s->Init(pak.GetEntry(i));
	}
	return entries != 0;
}

void IActivity::InitShapeAt(RLEShape& shp, const Point2D& position, const char* label, const ByteSlice& entry)
{
	PakArchive pak;
	pak.InitFromRAM(label, entry);
	shp.InitWithPosition(pak.GetEntry(0), position);
}

SCButton* IActivity::MakeButton(Point2D pos, Point2D size, PakArchive& subPak, size_t upEntry, size_t downEntry, SCButton::ActionFunction&& fn)
{
	auto& button = buttons.emplace_back(std::make_unique<SCButton>());
	button->InitBehavior(pos, size, std::move(fn));
	button->appearance[SCButton::APR_UP]  .InitWithPosition(subPak.GetEntry(upEntry), pos);
	button->appearance[SCButton::APR_DOWN].InitWithPosition(subPak.GetEntry(downEntry), pos);
	return button.get();
}

void IActivity::DrawButtons(void)
{
	for(auto&& button : buttons) {
		if (button->IsEnabled())
			VGA.DrawShape(button->appearance[button->GetAppearance()]);
		else
			VGA.DrawShape(button->appearance[SCButton::Appearance::APR_DOWN]);
	}
}
