//
//  SCDogFightMenu.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/3/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "SCDogFightMenu.h"

#include "precomp.h"

constexpr uint8_t PAK_ID_PALETTE    = 7;
constexpr uint8_t PAK_ID_BACKGROUND = 6;
constexpr uint8_t PAK_ID_TITLE      = 1 ;
constexpr uint8_t PAK_ID_BUTTONS    = 3 ;

SCDogFightMenu::SCDogFightMenu()
{
}

SCDogFightMenu::~SCDogFightMenu()
{
}

void SCDogFightMenu::Init()
{
	TreArchive tre ;
	tre.InitFromFile("GAMEFLOW.TRE");

	TreEntry* objViewIFF = NULL;//Assets.tres[]->GetEntryByName(TRE_DATA_GAMEFLOW "OBJVIEW.IFF");
	TreEntry* objViewPAK = NULL;//tre.GetEntryByName(TRE_DATA_GAMEFLOW "OBJVIEW.PAK");

	IffLexer objToDisplay;
	objToDisplay.InitFromRAM(*objViewIFF);
	objToDisplay.List(stdout);

	auto assets = GetPak("OBJVIEW.PAK", *objViewPAK);
	assets->List(stdout);

	palette.Diff(Renderer.GetPalette());

	auto file0 = GetPak("OBJVIEW.PAK: file PAK_ID_MENU_DYNAMC", assets->GetEntry(PAK_ID_BACKGROUND));
	file0->List(stdout);
	//showAllImage(&file0);
}

void SCDogFightMenu::RunFrame(const FrameParams& p)
{
}
