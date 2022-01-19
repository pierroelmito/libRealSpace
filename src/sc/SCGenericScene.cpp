//
//  WildCatBase.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "SCGenericScene.h"

#include "precomp.h"

SCGenericScene::SCGenericScene()
{
}

SCGenericScene::~SCGenericScene()
{
}

void SCGenericScene::Init(Scene sc)
{
	_font = FontManager.GetFont("");

	switch (sc) {
	case Scene::WildcatBaseHangar:
		InitShapes({ OptHangar, OptHangarDoor0, OptHangarDoor1, OptF16 });
		AddInteraction({ 17, 78, 54, 110 }, Scene::WildcatBaseOffice);
		AddInteraction({ 275, 84, 307, 137 }, Scene::WildcatBaseChangeroom);
		break;
	case Scene::WildcatBaseOffice:
		InitShapes({ OptDesk });
		AddInteraction({ 0, 0, 320, 200 }, Scene::WildcatBaseHangar);
		break;
	case Scene::WildcatBaseChangeroom:
		InitShapes({ OptChangeRoomBg, OptChangeRoomFg });
		AddInteraction({ 56, 75, 92, 135 }, Scene::WildcatBaseHangar);
		AddInteraction({ 274, 85, 289, 107 }, Scene::WildcatBasePinupF);
		AddInteraction({ 143, 88, 156, 106 }, Scene::WildcatBasePinupM);
		break;
	case Scene::WildcatBasePinupF:
		InitShapes({ OptPinupF });
		AddInteraction({ 0, 0, 320, 200 }, Scene::WildcatBaseChangeroom);
		break;
	case Scene::WildcatBasePinupM:
		InitShapes({ OptPinupM });
		AddInteraction({ 0, 0, 320, 200 }, Scene::WildcatBaseChangeroom);
		break;
	}
}

void SCGenericScene::AddInteraction(Area area, Scene sc)
{
	_interactions.emplace_back(area, [sc] (SCGenericScene* current) {
		current->Stop();
		Game.MakeActivity<SCGenericScene>(sc);
	});
}

void SCGenericScene::RunFrame(const FrameParams& p)
{
	Mouse.SetMode(SCMouse::CURSOR);

	const Point2D mpos = Mouse.GetPosition();
	for (const auto& interation : _interactions) {
		const auto area = interation.first;
		if (mpos.x < area.x0 ||
			mpos.x > area.x1 ||
			mpos.y < area.y0 ||
			mpos.y > area.y1
			)
			continue;
		Mouse.SetMode(SCMouse::VISOR);
		if (Mouse.buttons[SCMouseButton::LEFT].event == SCMouseButton::RELEASED)
			interation.second(this);
	}

	char buffer[512]{};
	auto sz = snprintf(buffer, sizeof(buffer), "%d,%d", mpos.x, mpos.y);

	Frame2D(shapes, [&] {
		VGA.DrawText(_font, { 10, 10 }, buffer, 1, 0, sz, 3, 5);
	});
}
