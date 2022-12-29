//
//  File.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/28/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

#include "SCTrainingMenu.h"
#include "SCStrike.h"
#include "SCSelectWeaponsF16.h"

SCTrainingMenu::SCTrainingMenu()
{
}

SCTrainingMenu::~SCTrainingMenu()
{
}

void SCTrainingMenu::Init()
{
	InitShapes({ ShpTraingBg0, ShpTrainingTitle });

	/*
	PakEntry* ue = assets.GetEntry(8);
	PakArchive up;
	up.InitFromRAM("OBJVIEW.PAK: file 5",ue->data, ue->size);
	up.List(stdout);
	title.Init(up.GetEntry(0)->data, up.GetEntry(0)->size);
	 */

	const Point2D positionBoard = { 6, 150 };

	//InitShapeAt(*s, { 4, 0 }, "", assets->GetEntry(1));
	auto& treGameFlow = Assets.tres[AssetManager::TRE_GAMEFLOW];
	auto trButtonsPack = GetPak("TM.SHP", *treGameFlow.GetEntryByName(TRE_DATA_GAMEFLOW "TM.SHP"));
	auto& board = AddSingleShape();
	board.Init(trButtonsPack->GetEntry(0));
	board.SetPosition(positionBoard);

	const Point2D sandDDimension = { 130, 15 };
	const Point2D sanDPosition = { positionBoard.x + 16, positionBoard.y + 9 };
	const Point2D dogDDimension = { 130, 15 } ;
	const Point2D dogDPosition = { positionBoard.x + 155, positionBoard.y + 9 };
	const Point2D exitDDimension = { 60, 15 } ;
	const Point2D exitDPosition = { positionBoard.x + 155, positionBoard.y + 23 };

	MakeButton(sanDPosition, sandDDimension, *trButtonsPack, 1, 2, [&] {
		Stop();
		//Game.MakeActivity<SCSelectWeaponF16>();
		Game.MakeActivity<SCStrike>();
	});

	MakeButton(dogDPosition, dogDDimension, *trButtonsPack, 3, 4, [] {
	})->SetEnable(false);

	MakeButton(exitDPosition, exitDDimension, *trButtonsPack, 5, 6, [] {
		Game.StopTopActivity();
	});
}

void SCTrainingMenu::RunFrame(const FrameParams& p)
{
	Frame2D(p, shapes);
}
