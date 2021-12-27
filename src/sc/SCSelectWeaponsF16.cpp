//
//  SCSelectWeaponsF16.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/26/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

#include "SCSelectWeaponsF16.h"

SCSelectWeaponF16::SCSelectWeaponF16(){
    
}

SCSelectWeaponF16::~SCSelectWeaponF16(){
    
}

#define OPTPALS_PAK_PATH "..\\..\\DATA\\GAMEFLOW\\OPTPALS.PAK"
void SCSelectWeaponF16::Init( )
{
	//Palette
	this->palette = VGA.GetPalette();

	//Patch palette
	ByteStream paletteReader;
	TreEntry* palettesEntry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName(OPTPALS_PAK_PATH);
	PakArchive palettesPak;
	palettesPak.InitFromRAM("OPTSHPS.PAK",palettesEntry->data,palettesEntry->size);
	paletteReader.Set(palettesPak.GetEntry(12).data);
	this->palette.ReadPatch(&paletteReader);

	TreEntry* optionShapesEntry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTSHPS.PAK");

	PakArchive optionShapes;
	optionShapes.InitFromRAM("", optionShapesEntry->data,optionShapesEntry->size);

	PakArchive backgroundPak;
	backgroundPak.InitFromPakEntry("", optionShapes.GetEntry(91));
	background.Init(backgroundPak.GetEntry(0));
}

void SCSelectWeaponF16::RunFrame(void){

    CheckButtons();

    VGA.Activate();
    VGA.Clear();

	VGA.SetPalette(this->palette);

    //Draw static
	VGA.DrawShape(background);
    

    DrawButtons();

    //Draw Mouse
    Mouse.Draw();

    //Check Mouse state.

    VGA.VSync();
}
