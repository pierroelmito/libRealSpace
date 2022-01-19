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

void IActivity::Frame2D(std::initializer_list<RLEShape*> shapes)
{
	CheckButtons();
	VGA.Clear();
	VGA.SetPalette(this->palette);
	for (RLEShape* shape : shapes)
		VGA.DrawShape(*shape);
	DrawButtons();
	Mouse.Draw();
	VGA.VSync();
}

void IActivity::Frame2D(std::vector<std::unique_ptr<RLEShape>>& shapes)
{
	CheckButtons();
	VGA.Clear();
	VGA.SetPalette(this->palette);
	for (auto& shape : shapes)
		VGA.DrawShape(*shape);
	DrawButtons();
	Mouse.Draw();
	VGA.VSync();
}

SCButton* IActivity::CheckButtons(void)
{
	for(SCButton* button : buttons) {
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
		if (Mouse.buttons[SCMouseButton::LEFT].event == SCMouseButton::RELEASED)
			button->OnAction();

		return button;
	}

	Mouse.SetMode(SCMouse::CURSOR);
	return NULL;
}

void IActivity::ReadPatch(VGAPalette& pal, const ByteSlice& bytes)
{
	ByteStream paletteReader;
	paletteReader.Set(bytes.data);
	pal.ReadPatch(&paletteReader);
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

bool IActivity::InitShapes(std::initializer_list<PalBg> ids)
{
	shapes.clear();

	auto& treGameFlow = Assets.tres[AssetManager::TRE_GAMEFLOW];
	std::map<std::string, std::unique_ptr<PakArchive>> paks;
	auto getPak = [&] (const char* path) -> PakArchive& {
		auto& pak = paks[path];
		if (!pak)
			pak = GetPak(path, *treGameFlow.GetEntryByName(path));
		return *pak;
	};

	for (const PalBg& id : ids) {
		printf("pal : %d / bg : %d\n", id.pal, id.shp);
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
		if (id.shp >= 0 && id.shp < pak.GetNumEntries())
			InitShape(AddShape(), "", pak.GetEntry(id.shp));
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

void IActivity::InitShapeAt(RLEShape& shp, const Point2D& position, const char* label, const ByteSlice& entry)
{
	PakArchive pak;
	pak.InitFromRAM(label, entry);
	shp.InitWithPosition(pak.GetEntry(0), position);
}

SCButton* IActivity::MakeButton(Point2D pos, Point2D size, PakArchive& subPak, size_t upEntry, size_t downEntry, SCButton::ActionFunction&& fn)
{
	SCButton* button = new SCButton();
	button->InitBehavior(pos, size, std::move(fn));
	button->appearance[SCButton::APR_UP]  .InitWithPosition(subPak.GetEntry(upEntry), pos);
	button->appearance[SCButton::APR_DOWN].InitWithPosition(subPak.GetEntry(downEntry), pos);
	buttons.push_back(button);
	return button;
}

void IActivity::DrawButtons(void)
{
	for(SCButton* button : buttons) {
		if (button->IsEnabled())
			VGA.DrawShape(button->appearance[button->GetAppearance()]);
		else
			VGA.DrawShape(button->appearance[SCButton::Appearance::APR_DOWN]);
	}
}
