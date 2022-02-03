//
//  SCStrike.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/28/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "SCStrike.h"

#include "precomp.h"

#include "UserProperties.h"

SCStrike::SCStrike()
{
}

SCStrike::~SCStrike()
{
}

void SCStrike::Init(void )
{
	area.InitFromPAKFileName("ARENA.PAK", Assets.tres[AssetManager::TRE_OBJECTS], Assets.tres[AssetManager::TRE_TEXTURES]);

	camPos = { 4100, 100, 3000 };
	angleV = 0.0f;
	angleH = 0.0f;
}

void SCStrike::RunFrame(const FrameParams& p)
{
#if 0
	auto& cam = Renderer.GetCamera();
	GTime currentTime = p.currentTime * TimeToMSec;
	const RSVector3 lookAt{ 3856, 0, 2856};
	RSVector3 newPosition{ 4100, 100, 3000 };
	newPosition.X =  lookAt.X + 300 * cos(currentTime/2000.0f);
	newPosition.Z =  lookAt.Z + 300 * sin(currentTime/2000.0f);
	cam.SetPosition(newPosition);
	cam.LookAt(lookAt);
#else
	const float mLeft = Game.IsKeyPressed(GLFW_KEY_A) ? -1.0f : 0.0f;
	const float mRight = Game.IsKeyPressed(GLFW_KEY_D) ? 1.0f : 0.0f;
	const float mUp = Game.IsKeyPressed(GLFW_KEY_W) ? -1.0f : 0.0f;
	const float mDown = Game.IsKeyPressed(GLFW_KEY_S) ? 1.0f : 0.0f;
	const float mQuick = Game.IsKeyPressed(GLFW_KEY_LEFT_SHIFT) ? 20.0f : 2.0f;
	const float rLeft = Game.IsKeyPressed(GLFW_KEY_LEFT) ? -1.0f : 0.0f;
	const float rRight = Game.IsKeyPressed(GLFW_KEY_RIGHT) ? 1.0f : 0.0f;
	const float rUp = Game.IsKeyPressed(GLFW_KEY_UP) ? -1.0f : 0.0f;
	const float rDown = Game.IsKeyPressed(GLFW_KEY_DOWN) ? 1.0f : 0.0f;

	const float maxA = 1.4;
	angleV += 0.1f * (rLeft + rRight);
	angleH = std::min(maxA, std::max(-maxA, angleH + 0.1f * (rUp + rDown)));

	auto& cam = Renderer.GetCamera();
	const RSVector3 camDir = {
		cosf(angleV) * cosf(angleH),
		sinf(angleH),
		sinf(angleV) * cosf(angleH)
	};
	const RSVector3 strafe = HMM_Cross(camDir, { 0, 1, 0 });

	camPos += -0.1f * mQuick * ((mUp + mDown) * camDir - (mLeft + mRight) * strafe);

	cam.SetPosition(camPos);
	cam.LookAt(camPos + camDir);
#endif

	const RSVector3 light = HMM_NormalizeVec3(UserProperties::Get().Vectors3.Get("LightDir", { 2, 3, 2 }));

	Renderer.SetLight(light);
	Renderer.Draw3D({ R3Dp::SKY | R3Dp::CLOUDS }, [&] () {
		Renderer.RenderWorldSolid(area, BLOCK_LOD_MAX, p.totalTime);
	});

	if (p.pressed.contains(GLFW_KEY_ESCAPE)) {
		Renderer.ClearCache();
		Stop();
	}
}
