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

void SCSelectWeaponF16::Init()
{
	wantedBg = OptHangar;
}

void SCSelectWeaponF16::RunFrame(const FrameParams& p)
{
	if (wantedBg != currentBg) {
		currentBg = wantedBg;
		printf("pal : %d / bg : %d", currentBg.pal, currentBg.shp);
		shapes.clear();
		//Patch palette
		if (ReadPatches({ currentBg.pal }, currentBg.pakPal)) {
			auto& treGameFlow = Assets.tres[AssetManager::TRE_GAMEFLOW];
			auto optionShapes = GetPak("", *treGameFlow.GetEntryByName(currentBg.pakShp));
			//auto optionShapes = GetPak("", *treGameFlow.GetEntryByName(TRE_DATA_GAMEFLOW "CONVSHPS.PAK"));
			//auto optionShapes = GetPak("", *treGameFlow.GetEntryByName(TRE_DATA "MIDGAMES\\MIDGAMES.PAK"));
			//auto optionShapes = GetPak("", *treGameFlow.GetEntryByName(TRE_DATA "MIDGAMES\\MID12.PAK"));
			if (currentBg.shp >= 0 && currentBg.shp < optionShapes->GetNumEntries()) {
				if (!InitShape(AddShape(), "", optionShapes->GetEntry(currentBg.shp))) {
					shapes.clear();
					printf(" - ERR");
				} else {
					printf(" - OK");
				}
			} else {
				printf(" - OOI");
			}
		} else {
			printf(" - no pal");
		}
		printf("\n");
	}

	if (p.pressed.contains(257))
		Stop();
	if (p.pressed.contains(GLFW_KEY_A))
		wantedBg.shp+= 1;
	if (p.pressed.contains(GLFW_KEY_Q))
		wantedBg.shp -= 1;
	if (p.pressed.contains(GLFW_KEY_Z))
		wantedBg.pal += 1;
	if (p.pressed.contains(GLFW_KEY_S))
		wantedBg.pal -= 1;

	Frame2D(shapes);
}
