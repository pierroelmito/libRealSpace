//
//  File.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/28/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "SCTrainingMenu.h"
#include "SCStrike.h"
#include "SCSelectWeaponsF16.h"

#include "precomp.h"

SCTrainingMenu::SCTrainingMenu()
{
}

SCTrainingMenu::~SCTrainingMenu()
{
}

void SCTrainingMenu::Init()
{
	auto& treGameFlow = Assets.tres[AssetManager::TRE_GAMEFLOW];

	TreEntry* objViewPAK = treGameFlow.GetEntryByName(TRE_DATA_GAMEFLOW "OBJVIEW.PAK");
	PakArchive assets;
	assets.InitFromRAM("OBJVIEW.PAK", *objViewPAK);
	assets.List(stdout);

	//Load palette
	ByteStream paletteReader;
	paletteReader.Set(assets.GetEntry(7).data);
	this->palette.ReadPatch(&paletteReader);

	const PakEntry& backgroundPakEntry = assets.GetEntry(6);
	//Identified as Dodge Fight background
	PakArchive bgPack;
	bgPack.InitFromRAM("OBJVIEW.PAK: file 6",backgroundPakEntry);
	background.Init(bgPack.GetEntry(0));

	const PakEntry& titlePackEntry = assets.GetEntry(1);
	PakArchive titlePack;
	titlePack.InitFromRAM("", titlePackEntry);
	title.Init(titlePack.GetEntry(0));
	Point2D positionTitle = {4,0};
	title.SetPosition(&positionTitle);

	/*
	PakEntry* ue = assets.GetEntry(8);
	PakArchive up;
	up.InitFromRAM("OBJVIEW.PAK: file 5",ue->data, ue->size);
	up.List(stdout);

	title.Init(up.GetEntry(0)->data, up.GetEntry(0)->size);
	 */

	TreEntry* trButtonsEntry = treGameFlow.GetEntryByName(TRE_DATA_GAMEFLOW "TM.SHP");
	PakArchive trButtonsPack;
	trButtonsPack.InitFromRAM("TM.SHP", *trButtonsEntry);

	Point2D positionBoard = {6,150};
	board.Init(trButtonsPack.GetEntry(0));
	board.SetPosition(&positionBoard);

	//Search and destroy button
	SCButton* button;
	button = new SCButton();
	Point2D sandDDimension = {130, 15};
	Point2D sanDPosition = {positionBoard.x+16,positionBoard.y+9};
	button->InitBehavior(sanDPosition,sandDDimension, [] {
		Game.MakeActivity<SCStrike>();
		Game.MakeActivity<SCSelectWeaponF16>();
	});
	button->appearance[SCButton::APR_UP]  .InitWithPosition(trButtonsPack.GetEntry(1),&sanDPosition);
	button->appearance[SCButton::APR_DOWN].InitWithPosition(trButtonsPack.GetEntry(2),&sanDPosition);
	buttons.push_back(button);

	button = new SCButton();
	Point2D dogDDimension = {130, 15} ;
	Point2D dogDPosition = {positionBoard.x+155,positionBoard.y+9};
	button->InitBehavior(dogDPosition,dogDDimension, [] {});
	button->appearance[SCButton::APR_UP]  .InitWithPosition(trButtonsPack.GetEntry(3),&dogDPosition);
	button->appearance[SCButton::APR_DOWN].InitWithPosition(trButtonsPack.GetEntry(4),&dogDPosition);
	button->SetEnable(false);
	buttons.push_back(button);

	button = new SCButton();
	Point2D exitDDimension = {60, 15} ;
	Point2D exitDPosition = {positionBoard.x+155,positionBoard.y+23};
	button->InitBehavior(exitDPosition,exitDDimension, [] { Game.StopTopActivity(); });
	button->appearance[SCButton::APR_UP]  .InitWithPosition(trButtonsPack.GetEntry(5),&exitDPosition);
	button->appearance[SCButton::APR_DOWN].InitWithPosition(trButtonsPack.GetEntry(6),&exitDPosition);
	buttons.push_back(button);
}

void SCTrainingMenu::RunFrame(const FrameParams& p)
{
	Frame2D({ &background, &title, &board });
}
