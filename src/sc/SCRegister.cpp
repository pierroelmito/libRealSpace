//
//  SCRegister.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "SCRegister.h"

#include "precomp.h"

#include "SCGenericScene.h"
#include "SCAnimationPlayer.h"
#include "SCConvPlayer.h"

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
	if (p.pressed.contains(GLFW_KEY_ENTER)) {
		Stop();
		Game.MakeActivity<SCGenericScene>(Scene::CutsceneMoveA, Scene::WildcatBaseHangar);
	}

	Frame2D(p, shapes);
}
