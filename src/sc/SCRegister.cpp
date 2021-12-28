//
//  SCRegister.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "SCRegister.h"

#include "precomp.h"

#include <SDL2/SDL.h>

#include "SCWildCatBase.h"
#include "SCAnimationPlayer.h"

SCRegister::SCRegister()
{
}

SCRegister::~SCRegister()
{
}

void SCRegister::CheckKeyboard(void)
{
	//Keyboard
	SDL_Event keybEvents[5];
	int numKeybEvents = SDL_PeepEvents(keybEvents,5,SDL_PEEKEVENT,SDL_KEYDOWN,SDL_KEYDOWN);
	for(int i= 0 ; i < numKeybEvents ; i++){
		SDL_Event* event = &keybEvents[i];
		switch (event->key.keysym.sym) {
			case SDLK_RETURN :{
				Stop();
				//Add both animation and next location on the stack.
				Game.MakeActivity<SCWildCatBase>();
				Game.MakeActivity<SCAnimationPlayer>(0, 0);
				break;
			}
			default:
				break;
		}
	}
}

void SCRegister::Init()
{
	//Load book
	TreEntry* entryMountain = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTSHPS.PAK");
	PakArchive pak;
	pak.InitFromRAM("", *entryMountain);

	PakArchive bookPak;
	bookPak.InitFromRAM("subPak board", pak.GetEntry(OptionShapeID::START_GAME_REGISTRATION));
	book.Init(bookPak.GetEntry(0));

	//Load palette
	this->palette = VGA.GetPalette();

	TreEntry* palettesEntry = Assets.tres[AssetManager::TRE_GAMEFLOW]->GetEntryByName("..\\..\\DATA\\GAMEFLOW\\OPTPALS.PAK");
	PakArchive palettesPak;
	palettesPak.InitFromRAM("OPTSHPS.PAK", *palettesEntry);

	ByteStream paletteReader;
	paletteReader.Set(palettesPak.GetEntry(OPTPALS_PAK_STARTGAME_REGISTRATION).data);
	this->palette.ReadPatch(&paletteReader);
}

void SCRegister::RunFrame(const FrameParams& p)
{
	CheckKeyboard();
	Frame2D({ &book });
}
