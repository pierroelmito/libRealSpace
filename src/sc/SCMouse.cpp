//
//  SCMouse.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "SCMouse.h"

#include "precomp.h"

SCMouse::SCMouse()
{
}

SCMouse::~SCMouse()
{
}

void SCMouse::Init()
{
	TreEntry* cursorShape = Assets.tres[AssetManager::TRE_MISC].GetEntryByName(TRE_DATA "MOUSE.SHP");
	PakArchive cursors;
	cursors.InitFromRAM("MOUSE.SHP",*cursorShape);

	for (int i = 0 ; i < 4; i++) {
		appearances[i] = std::make_unique<RLEShape>();
		appearances[i]->Init(cursors.GetEntry(i));
	}
}

void SCMouse::Draw()
{
	if (! IsVisible())
		return;

	// We need to draw the cursor a little bit higher left than the mouse
	// position so it is centered.
	Point2D cursorPos = this->position;
	cursorPos.x -= 4;
	cursorPos.y -= 4;

	//If the mouse is over a clickable button, the current appearance has already been selected.
	if (mode == CURSOR) {
		appearances[1]->SetPosition(cursorPos);
		VGA.DrawShape(*appearances[1]);
	}

	if (mode == VISOR) {
		appearances[0]->SetPosition(cursorPos);
		VGA.DrawShape(*appearances[0]);
	}
}

void SCMouse::FlushEvents(void)
{
	for (size_t i = 0 ; i < 3 ; i++)
		buttons[i].event = SCMouseButton::NONE;
}
