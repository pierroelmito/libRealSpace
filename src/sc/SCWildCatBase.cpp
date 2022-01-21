//
//  WildCatBase.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "SCWildCatBase.h"

#include "precomp.h"

#include "SCConvPlayer.h"

SCWildCatBase::SCWildCatBase()
{
}

SCWildCatBase::~SCWildCatBase()
{
}

void SCWildCatBase::Init()
{
	InitShapes({ OptHangar, OptHangarDoor0, OptHangarDoor1, OptF16 });
}

void SCWildCatBase::RunFrame(const FrameParams& p)
{
	if (p.pressed.contains(GLFW_KEY_ENTER)) {
		Stop();
		Game.MakeActivity<SCConvPlayer>().SetID(14);
	}

	Frame2D(p, shapes);
}
