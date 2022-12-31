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
	Game.SetMouseLock(true);

	auto AddJet = [&] (TreArchive& tre, const char* name, RSQuaternion* orientation, RSVector3* position)
	{
		TreEntry* jetEntry = tre.GetEntryByName(name);
		auto entity = RSEntity::LoadFromRAM(*jetEntry);
		jets.push_back({ std::move(entity), *orientation, *position });
	};

	auto& treObjects = Assets.tres[AssetManager::TRE_OBJECTS];
	area.InitFromPAKFileName("ARENA.PAK", treObjects, Assets.tres[AssetManager::TRE_TEXTURES]);

	TreEntry* cockpit = Assets.tres[AssetManager::TRE_OBJECTS].GetEntryByName(TRE_DATA_OBJECTS "F16PITXP.IFF");
	_cockpit = RSEntity::LoadFromRAM(*cockpit);

	plane.pos = { 4100, 100, 3000 };

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

void SCStrike::ComputeMove(const RSMatrix& transform, GTime dt)
{
	const float mQuick = Game.IsKeyPressed(GLFW_KEY_LEFT_SHIFT) ? 20.0f : 2.0f;

	// rotate
	const float mLeft = Game.IsKeyPressed(GLFW_KEY_A) ? -1.0f : 0.0f;
	const float mRight = Game.IsKeyPressed(GLFW_KEY_D) ? 1.0f : 0.0f;
	const float mUp = Game.IsKeyPressed(GLFW_KEY_W) ? -1.0f : 0.0f;
	const float mDown = Game.IsKeyPressed(GLFW_KEY_S) ? 1.0f : 0.0f;
	const float mPanL = Game.IsKeyPressed(GLFW_KEY_Q) ? -1.0f : 0.0f;
	const float mPanR = Game.IsKeyPressed(GLFW_KEY_E) ? 1.0f : 0.0f;

	// move
	const float rLeft = Game.IsKeyPressed(GLFW_KEY_LEFT) ? -1.0f : 0.0f;
	const float rRight = Game.IsKeyPressed(GLFW_KEY_RIGHT) ? 1.0f : 0.0f;
	const float rUp = Game.IsKeyPressed(GLFW_KEY_UP) ? -1.0f : 0.0f;
	const float rDown = Game.IsKeyPressed(GLFW_KEY_DOWN) ? 1.0f : 0.0f;

	RSVector3 d = plane.dir;
	RSVector3 u = plane.up;
	RSVector3 n = HMM_Cross(u, d);

	d = HMM_NormalizeVec3(d - dt * (rLeft + rRight) * n - dt * (rUp + rDown) * u);
	u = HMM_NormalizeVec3(HMM_Cross(d, n));
	u = HMM_NormalizeVec3(u + dt * (mPanL + mPanR) * n);
	n = HMM_Cross(u, d);
	d = HMM_NormalizeVec3(HMM_Cross(n, u));

	plane.dir = d;
	plane.up = u;
	plane.pos += 2.0f * dt* mQuick * ((mUp + mDown) * d + (mLeft + mRight) * n);
}

RSMatrix SCStrike::ComputeTransform(bool cockpit, bool lookAt)
{
	RSVector3 d, u, n;
	if (lookAt) {
		d = pilot.lookAt;
		n = HMM_NormalizeVec3(HMM_Cross(plane.up, d));
		u = HMM_Cross(d, n);
	} else {
		d = plane.dir;
		u = plane.up;
		n = HMM_Cross(u, d);
	}

	const float cT = cockpit ? -1.0f : 1.0f;
	const RSMatrix t = HMM_Translate(cT * -plane.pos);

	if (cockpit) {
		const RSMatrix r = { .Elements = {
				{ n.X, n.Y, n.Z, 0 },
				{ u.X, u.Y, u.Z, 0 },
				{ d.X, d.Y, d.Z, 0 },
				{   0,   0,   0, 1 },
			}
		};
		return t * r;
	} else {
		const RSMatrix r = { .Elements = {
				{ n.X, u.X, d.X, 0 },
				{ n.Y, u.Y, d.Y, 0 },
				{ n.Z, u.Z, d.Z, 0 },
				{   0,   0,   0, 1 },
			}
		};
		return r * t;
	}
}

void SCStrike::RunFrame(const FrameParams& p)
{
	// look at target
	const float lookAtTarget = Game.IsKeyPressed(GLFW_KEY_TAB);
	const bool usePlaneDirLookAt = jets.empty() || !lookAtTarget;
	const RSVector3 lookAt = usePlaneDirLookAt ? plane.dir : HMM_NormalizeVec3(plane.pos - jets[0].position);
	pilot.lookAt = HMM_NormalizeVec3(0.3f * lookAt + 0.7f * pilot.lookAt);

	const RSMatrix viewPilot = ComputeTransform(false, true);
	const RSMatrix viewPlane = ComputeTransform(false, false);
	const RSMatrix cockpit = ComputeTransform(true, false);
	ComputeMove(viewPlane, p.deltaTime);

	auto& cam = Renderer.GetCamera();
	cam.SetView(viewPilot);

	const auto& props = UserProperties::Get();

	const RSVector3 light = HMM_NormalizeVec3(props.Vectors3.Get("LightDir", { 2, 3, 2 }));
	Renderer.SetLight(light);

	//pilot._a = 0.09f * cosf(p.activityTime);
	//pilot._b = 0.09f * sinf(p.activityTime);

	Renderer.Draw3D({ R3Dp::SKY | R3Dp::CLOUDS }, [&] () {
		// world
		Renderer.RenderWorldSolid(area, BLOCK_LOD_MAX, p.totalTime);

		// jets
		for(auto&& jet : jets) {
			RSMatrix world = HMM_QuaternionToMat4(jet.orientation) * HMM_Scale({ OBJECT_SCALE, OBJECT_SCALE, OBJECT_SCALE });
			world.Elements[3][0] = jet.position.X;
			world.Elements[3][1] = jet.position.Y;
			world.Elements[3][2] = jet.position.Z;
			Renderer.DrawModel(jet.entity.get(), LOD_LEVEL_MAX, world);
		}

		// cockpit
		if (1) {
			const float sc = props.Floats.Get("CockpitScale", 0.05f);
			const RSMatrix mdl = HMM_Scale({ sc, sc, sc }) * HMM_Rotate(90.0f, { 0, 1, 0 }) * HMM_Translate({ 0, -3, 0 });
			Renderer.DrawModel(_cockpit.get(), LOD_LEVEL_MAX, cockpit * mdl);
		}
	});

	if (p.pressed.contains(GLFW_KEY_ESCAPE)) {
		Renderer.ClearCache();
		Stop();
	}
}
