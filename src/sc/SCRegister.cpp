//
//  SCRegister.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "SCRegister.h"

#include "precomp.h"

#include "SCWildCatBase.h"
#include "SCAnimationPlayer.h"

SCRegister::SCRegister()
{
}

SCRegister::~SCRegister()
{
}

void SCRegister::Init()
{
	auto& treGameFlow = Assets.tres[AssetManager::TRE_GAMEFLOW];

	//Load book
	TreEntry* entryMountain = treGameFlow.GetEntryByName(TRE_DATA_GAMEFLOW "OPTSHPS.PAK");
	PakArchive pak;
	pak.InitFromRAM("", *entryMountain);

	PakArchive bookPak;
	bookPak.InitFromRAM("subPak board", pak.GetEntry(OptionShapeID::START_GAME_REGISTRATION));
	book.Init(bookPak.GetEntry(0));

	//Load palette
	this->palette = VGA.GetPalette();

	TreEntry* palettesEntry = treGameFlow.GetEntryByName(TRE_DATA_GAMEFLOW "OPTPALS.PAK");
	PakArchive palettesPak;
	palettesPak.InitFromRAM("OPTSHPS.PAK", *palettesEntry);

	ByteStream paletteReader;
	paletteReader.Set(palettesPak.GetEntry(OPTPALS_PAK_STARTGAME_REGISTRATION).data);
	this->palette.ReadPatch(&paletteReader);
}

void SCRegister::RunFrame(const FrameParams& p)
{
	if (Game.IsKeyPressed(257 /*'\r'*/)) {
		Stop();
		//Add both animation and next location on the stack.
		Game.MakeActivity<SCWildCatBase>();
		Game.MakeActivity<SCAnimationPlayer>(0, 0);
	}

	Frame2D({ &book });
}
