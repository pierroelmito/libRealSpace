//
//  SCStrike.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/28/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "SCStrike.h"

#include "main.h"

#include <raymath.h>

#include "UserProperties.h"
#include "rltools.hpp"

SCStrike::SCStrike()
{
}

SCStrike::~SCStrike()
{
}

void SCStrike::Init(void)
{
	// Game.SetMouseLock(true);

	auto AddJet = [&](TreArchive& tre, const char* name, Quaternion* orientation, Vector3* position) {
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

	// TRE_DATA_GAMEFLOW "MIG29.IFF
	// TRE_DATA_GAMEFLOW "F-22.IFF"
	// TRE_DATA_GAMEFLOW "F-15.IFF"
	// TRE_DATA_GAMEFLOW "YF23.IFF"
	// TRE_DATA_GAMEFLOW "MIG21.IFF"
	// TRE_DATA_GAMEFLOW "MIG29.IFF"

	Quaternion rot0 = QuaternionIdentity(); // HMM_Mat4ToQuaternion(QuaternionRotate(angle, { 1, 0, 0 }));
	Vector3 pos0 = { mul * 4016, mul * 95, mul * 2980 };
	AddJet(treObjects, TRE_DATA_OBJECTS "F-16DES.IFF", &rot0, &pos0);

	Quaternion rot1 = QuaternionIdentity(); // HMM_Mat4ToQuaternion(HMM_Rotate(-angle, { 1, 0, 0 }));
	Vector3 pos1 = { mul * 4010, mul * 95, mul * 2980 };
	AddJet(treObjects, TRE_DATA_OBJECTS "F-22.IFF", &rot1, &pos1);
}

void SCStrike::ComputeMove(const Matrix& transform, GTime dt)
{
	const float mQuick = IsKeyDown(KEY_LEFT_SHIFT) ? 20.0f : 2.0f;

	// rotate
	const float mLeft = IsKeyDown(KEY_A) ? -1.0f : 0.0f;
	const float mRight = IsKeyDown(KEY_D) ? 1.0f : 0.0f;
	const float mUp = IsKeyDown(KEY_W) ? -1.0f : 0.0f;
	const float mDown = IsKeyDown(KEY_S) ? 1.0f : 0.0f;
	const float mPanL = IsKeyDown(KEY_Q) ? -1.0f : 0.0f;
	const float mPanR = IsKeyDown(KEY_E) ? 1.0f : 0.0f;

	// move
	const float rLeft = IsKeyDown(KEY_LEFT) ? -1.0f : 0.0f;
	const float rRight = IsKeyDown(KEY_RIGHT) ? 1.0f : 0.0f;
	const float rUp = IsKeyDown(KEY_UP) ? -1.0f : 0.0f;
	const float rDown = IsKeyDown(KEY_DOWN) ? 1.0f : 0.0f;

	Vector3 d = plane.dir;
	Vector3 u = plane.up;
	Vector3 n = Vector3CrossProduct(u, d);

	d = Vector3Normalize(d - n * (dt * (rLeft + rRight)) - u * (dt * (rUp + rDown)));
	u = Vector3Normalize(Vector3CrossProduct(d, n));
	u = Vector3Normalize(u + n * (dt * -(mPanL + mPanR)));
	n = Vector3CrossProduct(u, d);
	d = Vector3Normalize(Vector3CrossProduct(n, u));

	plane.dir = d;
	plane.up = u;
	plane.pos += (d * -(mUp + mDown) + n * -(mLeft + mRight)) * (2.0f * dt * mQuick);
}

Matrix SCStrike::ComputeTransform(bool cockpit, bool lookAt)
{
	Vector3 d, u, n;
	if (lookAt) {
		d = pilot.lookAt;
		n = Vector3Normalize(Vector3CrossProduct(plane.up, d));
		u = Vector3CrossProduct(d, n);
	} else {
		d = plane.dir;
		u = plane.up;
		n = Vector3CrossProduct(u, d);
	}

	const float cT = cockpit ? -1.0f : 1.0f;
	auto pos = plane.pos * -cT;
	const Matrix t = MatrixTranslate(pos.x, pos.y, pos.z);

	if (cockpit) {
		const Matrix r = rlt::MakeMat4({
			{ n.x, n.y, n.z, 0 },
			{ u.x, u.y, u.z, 0 },
			{ d.x, d.y, d.z, 0 },
			{ 0, 0, 0, 1 },
		});
		return t * r;
	} else {
		const Matrix r = rlt::MakeMat4({
			{ n.x, u.x, d.x, 0 },
			{ n.y, u.y, d.y, 0 },
			{ n.z, u.z, d.z, 0 },
			{ 0, 0, 0, 1 },
		});
		return r * t;
	}
}

void SCStrike::RunFrame(const FrameParams& p)
{
	// look at target
	const bool lookAtTarget = IsKeyDown(KEY_TAB);
	const bool usePlaneDirLookAt = jets.empty() || !lookAtTarget;
	const Vector3 lookAt = usePlaneDirLookAt ? plane.dir : Vector3Normalize(plane.pos - jets[0].position);
	pilot.lookAt = Vector3Normalize(lookAt * 0.3f + pilot.lookAt * 0.7f);

	const Matrix viewPilot = ComputeTransform(false, true);
	const Matrix viewPlane = ComputeTransform(false, false);
	const Matrix cockpit = ComputeTransform(true, false);
	ComputeMove(viewPlane, p.deltaTime);

	auto& cam = Renderer.GetCamera();
	// cam.SetView(viewPilot);

	const auto& props = UserProperties::Get();

	const Vector3 light = Vector3Normalize(props.Vectors3.Get("LightDir", { 2, 3, 2 }));
	Renderer.SetLight(light);

	// pilot._a = 0.09f * cosf(p.activityTime);
	// pilot._b = 0.09f * sinf(p.activityTime);

	Vector3 camPos = plane.pos;

	Camera rcam {};
	rcam.position = {};
	rcam.target = lookAt;
	rcam.up = plane.up;
	rcam.fovy = 45.0f;
	rcam.projection = CAMERA_PERSPECTIVE;

	Renderer.Draw3D({ camPos, rcam, R3Dp::CLEAR_COLORS | R3Dp::SKY | R3Dp::CLOUDS }, [&](const SCRenderer::Render3DParams& params) {
		// world
		Renderer.RenderWorldSolid(params, area, BLOCK_LOD_MAX, p.totalTime);
		// jets
		for (auto&& jet : jets) {
			Matrix world = QuaternionToMatrix(jet.orientation) * MatrixScale(OBJECT_SCALE, OBJECT_SCALE, OBJECT_SCALE);
			world.m12 = jet.position.x - camPos.x;
			world.m13 = jet.position.y - camPos.y;
			world.m14 = jet.position.z - camPos.z;
			Renderer.DrawModel(jet.entity.get(), LOD_LEVEL_MAX, world);
		}
		// cockpit
		if (0) {
			const float sc = props.Floats.Get("CockpitScale", 0.05f);
			const Matrix mdl = MatrixScale(sc, sc, sc) * MatrixRotate({ 0, 1, 0 }, 90.0f) * MatrixTranslate(0, -3, 0);
			Renderer.DrawModel(_cockpit.get(), LOD_LEVEL_MAX, cockpit * mdl);
		}
	});

#if 0
	int y = 10;
	y = rlt::MyDrawText(10, y, WHITE, 20, "a");
	y = rlt::MyDrawText(10, y, WHITE, 20, "b");
	y = rlt::MyDrawText(10, y, WHITE, 20, "c");
#endif

	if (IsKeyPressed(KEY_ESCAPE)) {
		Renderer.ClearCache();
		Stop();
	}
}
