//
//  WildCatBase.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "SCWildCatBase.h"
#include "SCConvPlayer.h"

#include "precomp.h"

#include <SDL2/SDL.h>

SCWildCatBase::SCWildCatBase()
{
}

SCWildCatBase::~SCWildCatBase()
{
}

void SCWildCatBase::CheckKeyboard(void)
{
	if (Game.IsKeyPressed(SDLK_RETURN)) {
		Stop();
		SCConvPlayer* conv = new SCConvPlayer();
		conv->Init();
		conv->SetID(14);
		Game.AddActivity(conv);
	}
}

void SCWildCatBase::Init()
{
	//Load book
	TreEntry* entryMountain = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTSHPS.PAK");
	PakArchive pak;
	pak.InitFromRAM("", *entryMountain);

	PakArchive bookPak;
	bookPak.InitFromRAM("subPak board", pak.GetEntry(OptionShapeID::WILDCAT_HANGAR));
	hangar.Init(bookPak.GetEntry(0));

	//Load vehicule
	PakArchive vehiculePak;
	vehiculePak.InitFromRAM("subPak board", pak.GetEntry(OptionShapeID::WILDCAT_HANGAR_VEHICULE_F16));
	vehicule.Init(vehiculePak.GetEntry(0));

	//Load palette
	this->palette = VGA.GetPalette();

	TreEntry* palettesEntry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTPALS.PAK");
	PakArchive palettesPak;
	palettesPak.InitFromRAM("OPTSHPS.PAK",*palettesEntry);

	ByteStream paletteReader;
	paletteReader.Set(palettesPak.GetEntry(OPTPALS_PAK_WILD_CAT_HANGAR).data);
	this->palette.ReadPatch(&paletteReader);
}

void SCWildCatBase::RunFrame(const FrameParams& p)
{
	CheckKeyboard();
	Frame2D({ &hangar, &vehicule });
}

