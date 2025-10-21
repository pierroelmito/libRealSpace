//
//  SCRegister.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "SCRegister.h"

#include "main.h"

#include "SCGenericScene.h"

SCRegister::SCRegister() { }

SCRegister::~SCRegister() { }

void SCRegister::Init() { InitShapes({ OptRegistration }); }

void SCRegister::RunFrame(const FrameParams& p)
{
	if (IsKeyPressed(KEY_ENTER)) {
		Stop();
		Game.MakeActivity<SCGenericScene>(Scene::WildcatBaseHangar);
		Game.MakeActivity<SCCutScene>(5);
	}

	Frame2D(p, shapes);
}
