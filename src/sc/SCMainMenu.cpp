//
//  SCMainMenu.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/3/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "SCMainMenu.h"
#include "SCTrainingMenu.h"
#include "SCObjectViewer.h"
#include "SCRegister.h"

#include "precomp.h"

#define MAINMENU_PAK_PATH "..\\..\\DATA\\GAMEFLOW\\MAINMENU.PAK"

#define MAINMENU_PAK_BUTTONS_INDICE 0
#define MAINMENU_PAK_BOARD_INDICE   1
#define MAINMENU_PAK_BOARD_PALETTE  2

/*
 3 eagle
 9 letters
 */

SCMainMenu::SCMainMenu(){
    
}

SCMainMenu::~SCMainMenu(){
    
}

void SCMainMenu::Init(void)
{
	TreArchive& gameFlow = Assets.tres[AssetManager::TRE_GAMEFLOW];
	TreEntry* entry = gameFlow.GetEntryByName(MAINMENU_PAK_PATH);
	mainMenupak.InitFromRAM("MAINMENU.PAK",*entry);

	LoadPalette();
	LoadButtons();
	LoadBoard();
	LoadBackgrounds();

	SetTitle("Neo Strike Commander");
}

/*
void SCMainMenu::LoadButton(const char* name, PakArchive* subPak, size_t upIndice, size_t downIndice, Point2D dimensions, ActionFunction onClick){
    
}
 */


Point2D boardPosition = {44,25};

enum ButtonIDS {
	BUTTON_CONTINUE,
	BUTTON_LOADGAME,
	BUTTON_STARTNEWGAME,
	BUTTON_TRAINING,
	BUTTON_OBJVIEWER
};

void SCMainMenu::LoadButtons()
{
	const PakEntry& boardEntry = mainMenupak.GetEntry(MAINMENU_PAK_BUTTONS_INDICE);

	//The buttons are within an other pak within MAINMENU.PAK !!!!
	PakArchive subPak;
	subPak.InitFromRAM("subPak Buttons",boardEntry);

	SCButton* button{};
	Point2D buttonDimension = {211, 15} ;

	button = new SCButton();
	Point2D continuePosition = {boardPosition.x+11,boardPosition.y+10};
	button->InitBehavior(continuePosition,buttonDimension, [] { printf("OnContinue\n"); });
	button->appearance[SCButton::APR_UP]  .InitWithPosition(subPak.GetEntry(0),&continuePosition);
	button->appearance[SCButton::APR_DOWN].InitWithPosition(subPak.GetEntry(5),&continuePosition);
	button->SetEnable(false);
	buttons.push_back(button);

	button = new SCButton();
	Point2D loadGamePosition = {boardPosition.x+11,continuePosition.y+buttonDimension.y+2};
	button->InitBehavior(loadGamePosition,buttonDimension, [] { printf("OnLoadGame\n"); });
	button->appearance[SCButton::APR_UP]  .InitWithPosition(subPak.GetEntry(1),&loadGamePosition);
	button->appearance[SCButton::APR_DOWN].InitWithPosition(subPak.GetEntry(6),&loadGamePosition);
	button->SetEnable(false);
	buttons.push_back(button);

	button = new SCButton();
	Point2D startNewGamePosition = {boardPosition.x+11,loadGamePosition.y+buttonDimension.y+2};
	button->InitBehavior(startNewGamePosition,buttonDimension,[this] {
		Stop();
		Game.MakeActivity<SCRegister>();
	});
	button->appearance[SCButton::APR_UP]  .InitWithPosition(subPak.GetEntry(2),&startNewGamePosition);
	button->appearance[SCButton::APR_DOWN].InitWithPosition(subPak.GetEntry(7),&startNewGamePosition);
	buttons.push_back(button);

	button = new SCButton();
	Point2D trainingPosition = {boardPosition.x+11,startNewGamePosition.y+buttonDimension.y+2};
	button->InitBehavior(trainingPosition,buttonDimension,[] {
		Game.MakeActivity<SCTrainingMenu>();
	});
	button->appearance[SCButton::APR_UP]  .InitWithPosition(subPak.GetEntry(3),&trainingPosition);
	button->appearance[SCButton::APR_DOWN].InitWithPosition(subPak.GetEntry(8),&trainingPosition);
	buttons.push_back(button);

	button = new SCButton();
	Point2D viewObjectPosition = {boardPosition.x+11,trainingPosition.y+buttonDimension.y+2};
	button->InitBehavior(viewObjectPosition,buttonDimension,[] {
		Game.MakeActivity<SCObjectViewer>();
	});
	button->appearance[SCButton::APR_UP]  .InitWithPosition(subPak.GetEntry(4),&viewObjectPosition);
	button->appearance[SCButton::APR_DOWN].InitWithPosition(subPak.GetEntry(9),&viewObjectPosition);
	buttons.push_back(button);
}

void SCMainMenu::LoadBoard(void)
{
	const PakEntry& boardEntry = mainMenupak.GetEntry(MAINMENU_PAK_BOARD_INDICE);
	//The board is within an other pak within MAINMENU.PAK !!!!
	PakArchive subPak;
	subPak.InitFromRAM("subPak board", boardEntry);
	board.InitWithPosition(subPak.GetEntry(0), &boardPosition);
}

void SCMainMenu::LoadPalette(void)
{
	ByteStream paletteReader;

	this->palette = VGA.GetPalette();
	//Load the default palette

	TreEntry* palettesEntry = Assets.tres[AssetManager::TRE_GAMEFLOW].GetEntryByName(OPTPALS_PAK_PATH);
	PakArchive palettesPak;
	palettesPak.InitFromRAM("OPTSHPS.PAK",*palettesEntry);
	//palettesPak.List(stdout);

	paletteReader.Set(palettesPak.GetEntry(OPTPALS_PAK_MOUTAIN_PALETTE_PATCH_ID).data); //mountains Good but not sky
	this->palette.ReadPatch(&paletteReader);
	paletteReader.Set(palettesPak.GetEntry(OPTPALS_PAK_SKY_PALETTE_PATCH_ID).data); //Sky Good but not mountains
	this->palette.ReadPatch(&paletteReader);

	//Third palette patch (for silver board and buttons)
	const PakEntry& palettePatchEntry = mainMenupak.GetEntry(MAINMENU_PAK_BOARD_PALETTE);
	paletteReader.Set(palettePatchEntry.data);
	this->palette.ReadPatch(&paletteReader);
}

void SCMainMenu::LoadBackgrounds(void)
{
	auto& treGameFlow = Assets.tres[AssetManager::TRE_GAMEFLOW];

	TreEntry* entryMountain = treGameFlow.GetEntryByName(TRE_DATA "GAMEFLOW\\OPTSHPS.PAK");
	PakArchive pak;
	pak.InitFromRAM("",*entryMountain);

	//The board is within an other pak within MAINMENU.PAK !!!!
	PakArchive mountainPak;
	mountainPak.InitFromRAM("subPak board", pak.GetEntry(OptionShapeID::MOUTAINS_BG));
	mountain.Init(mountainPak.GetEntry(0));

	PakArchive skyPak;
	skyPak.InitFromRAM("subPak sky", pak.GetEntry(OptionShapeID::SKY));
	sky.Init(skyPak.GetEntry(0));

	TreEntry* entryCloud = treGameFlow.GetEntryByName(TRE_DATA "MIDGAMES\\MIDGAMES.PAK");
	PakArchive subcloudPak;
	subcloudPak.InitFromRAM("cloud oak entry", *entryCloud);

	PakArchive cloudPak;
	cloudPak.InitFromRAM("subPak cloud",subcloudPak.GetEntry(20));
	cloud.Init(cloudPak.GetEntry(0));
}

void SCMainMenu::RunFrame(const FrameParams& p)
{
	Frame2D({ &sky, &mountain, &cloud, &board });
}
