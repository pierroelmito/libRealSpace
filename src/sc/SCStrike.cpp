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
	auto AddJet = [&] (TreArchive& tre, const char* name, RSQuaternion* orientation, RSVector3* position)
	{
		TreEntry* jetEntry = tre.GetEntryByName(name);
		auto entity = RSEntity::LoadFromRAM(*jetEntry);
		entity->orientation = *orientation;
		entity->position = *position;
		jets.emplace_back(std::move(entity));
	};

	auto& treObjects = Assets.tres[AssetManager::TRE_OBJECTS];
	area.InitFromPAKFileName("ARENA.PAK", treObjects, Assets.tres[AssetManager::TRE_TEXTURES]);

	TreEntry* cockpit = Assets.tres[AssetManager::TRE_OBJECTS].GetEntryByName(TRE_DATA_OBJECTS "F16PITXP.IFF");
	_cockpit = RSEntity::LoadFromRAM(*cockpit);

	camPos = { 4100, 100, 3000 };
	angleV = 0.0f;
	angleH = 0.0f;

	const float angle = 25.0f;
	const float mul = 1.0f;

	//TRE_DATA_GAMEFLOW "MIG29.IFF
	//TRE_DATA_GAMEFLOW "F-22.IFF"
	//TRE_DATA_GAMEFLOW "F-15.IFF"
	//TRE_DATA_GAMEFLOW "YF23.IFF"
	//TRE_DATA_GAMEFLOW "MIG21.IFF"
	//TRE_DATA_GAMEFLOW "MIG29.IFF"

	RSQuaternion rot0 = HMM_Mat4ToQuaternion(HMM_Rotate(angle, { 1, 0, 0 }));
	RSVector3 pos0 = { mul * 4016, mul * 95, mul * 2980};
	AddJet(treObjects, TRE_DATA_OBJECTS "F-16DES.IFF", &rot0, &pos0);

	RSQuaternion rot1 = HMM_Mat4ToQuaternion(HMM_Rotate(-angle, { 1, 0, 0 }));
	RSVector3 pos1 = { mul * 4010, mul * 95, mul * 2980};
	AddJet(treObjects, TRE_DATA_OBJECTS "F-22.IFF", &rot1, &pos1);
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
		Renderer.RenderEntities(jets);
		const float sc = 0.1f;
		const RSVector3 headPos = { 0, sc * 3.3f / 1.2f, 0 };
		cam.SetPosition(headPos);
		cam.LookAt(headPos + RSVector3{ 1, 0, 0 });
		Renderer.DrawModel(_cockpit.get(), LOD_LEVEL_MAX, HMM_Scale({ sc, sc, sc }));
		cam.SetPosition(camPos);
		cam.LookAt(camPos + camDir);
	});

	if (p.pressed.contains(GLFW_KEY_ESCAPE)) {
		Renderer.ClearCache();
		Stop();
	}
}
