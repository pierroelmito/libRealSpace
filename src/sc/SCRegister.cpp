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
	InitShapes({ OptRegistration });
}

void SCRegister::RunFrame(const FrameParams& p)
{
	if (p.pressed.contains(257)) {
		Stop();
		//Add both animation and next location on the stack.
		Game.MakeActivity<SCWildCatBase>();
		Game.MakeActivity<SCAnimationPlayer>(0, 0);
	}

	Frame2D(shapes);
}
