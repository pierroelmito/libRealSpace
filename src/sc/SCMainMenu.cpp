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

/*
 3 eagle
 9 letters
 */

SCMainMenu::SCMainMenu()
{
}

SCMainMenu::~SCMainMenu()
{
}

void SCMainMenu::Init(void)
{
	SetTitle("Neo Strike Commander");

	const Point2D boardPosition = { 44, 25 };

	InitShapes({ OptSky, OptMountain, ShpClouds, ShpBoard });
	shapes.back()->SetPosition(boardPosition);

	{
		const Point2D buttonDimension = { 211, 15 };
		const Point2D continuePosition = { boardPosition.x + 11, boardPosition.y + 10 };
		const Point2D loadGamePosition = { boardPosition.x + 11, continuePosition.y + buttonDimension.y + 2 };
		const Point2D startNewGamePosition = { boardPosition.x + 11, loadGamePosition.y + buttonDimension.y + 2 };
		const Point2D trainingPosition = { boardPosition.x + 11, startNewGamePosition.y + buttonDimension.y + 2 };
		const Point2D viewObjectPosition = { boardPosition.x + 11, trainingPosition.y + buttonDimension.y + 2 };

		//The buttons are within an other pak within MAINMENU.PAK !!!!
		TreArchive& treGameFlow = Assets.tres[AssetManager::TRE_GAMEFLOW];
		auto mainMenupak = GetPak("MAINMENU.PAK", *treGameFlow.GetEntryByName(TRE_DATA_GAMEFLOW "MAINMENU.PAK"));
		auto subPak = GetPak("subPak Buttons", mainMenupak->GetEntry(0));

		MakeButton(continuePosition, buttonDimension, *subPak, 0, 5, [] {})->SetEnable(false);
		MakeButton(loadGamePosition, buttonDimension, *subPak, 1, 6, [] {})->SetEnable(false);
		MakeButton(startNewGamePosition, buttonDimension, *subPak, 2, 7, [this] { Game.MakeActivity<SCRegister>(); });
		MakeButton(trainingPosition, buttonDimension, *subPak, 3, 8, [] { Game.MakeActivity<SCTrainingMenu>(); });
		MakeButton(viewObjectPosition, buttonDimension, *subPak, 4, 9, [] { Game.MakeActivity<SCObjectViewer>(); });
	}
}

void SCMainMenu::RunFrame(const FrameParams& p)
{
	Frame2D(shapes);
}
