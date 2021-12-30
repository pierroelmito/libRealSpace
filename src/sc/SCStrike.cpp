//
//  SCStrike.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/28/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "SCStrike.h"

#include "precomp.h"

SCStrike::SCStrike()
{
}

SCStrike::~SCStrike()
{
}

void SCStrike::Init(void )
{
	area.InitFromPAKFileName("ARENA.PAK");
}

void SCStrike::RunFrame(const FrameParams& p)
{
	GTime currentTime = p.currentTime * TimeToMSec;
	const RSVector3 lookAt{ 3856, 0, 2856};
	RSVector3 newPosition{ 4100, 100, 3000 };
	newPosition.X =  lookAt.X + 300 * cos(currentTime/2000.0f);
	newPosition.Z =  lookAt.Z + 300 * sin(currentTime/2000.0f);
	auto& cam = Renderer.GetCamera();
	cam.SetPosition(newPosition);
	cam.LookAt(lookAt);

	const RSVector3 light = HMM_NormalizeVec3({ 1, 4, 1 });

	Renderer.SetLight(light);
	Renderer.Draw3D({}, [&] () {
		Renderer.RenderWorldSolid(area, BLOCK_LOD_MAX, 400);
	});
}
