//
//  SCSelectWeaponsF16.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/26/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

#include "SCSelectWeaponsF16.h"

SCSelectWeaponF16::SCSelectWeaponF16()
{
}

SCSelectWeaponF16::~SCSelectWeaponF16()
{
}

void SCSelectWeaponF16::Init( )
{
	//Palette
	this->palette = VGA.GetPalette();

	auto& treGameFlow = Assets.tres[AssetManager::TRE_GAMEFLOW];

	//Patch palette
	ByteStream paletteReader;
	TreEntry* palettesEntry = treGameFlow.GetEntryByName(TRE_DATA_GAMEFLOW "OPTPALS.PAK");
	PakArchive palettesPak;
	palettesPak.InitFromRAM("OPTSHPS.PAK", *palettesEntry);
	paletteReader.Set(palettesPak.GetEntry(12).data);
	this->palette.ReadPatch(&paletteReader);

	TreEntry* optionShapesEntry = treGameFlow.GetEntryByName(TRE_DATA_GAMEFLOW "OPTSHPS.PAK");

	PakArchive optionShapes;
	optionShapes.InitFromRAM("", *optionShapesEntry);

	PakArchive backgroundPak;
	backgroundPak.InitFromRAM("", optionShapes.GetEntry(91));
	background.Init(backgroundPak.GetEntry(0));
}

void SCSelectWeaponF16::RunFrame(const FrameParams& p)
{
	if (Game.IsKeyPressed('\r'))
		Stop();
	Frame2D({ &background });
}
