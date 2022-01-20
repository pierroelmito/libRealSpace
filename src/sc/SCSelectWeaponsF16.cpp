//
//  SCSelectWeaponsF16.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/26/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

#include "SCSelectWeaponsF16.h"

SCSelectWeaponF16::SCSelectWeaponF16()
{
}

SCSelectWeaponF16::~SCSelectWeaponF16()
{
}

void SCSelectWeaponF16::Init()
{
	_font = FontManager.GetFont("");

	wantedBg = OptHangarTruck;
}

void SCSelectWeaponF16::RunFrame(const FrameParams& p)
{
	if (wantedBg != currentBg) {
		currentBg = wantedBg;
		printf("pal : %d / bg : %d\n", currentBg.pal, currentBg.shp);
		for (int i = 0; i < 256; ++i)
			palette.SetColor(i, { 255u, 0, 255u, 255u });
		startTime = p.currentTime;
		InitShapes({ currentBg });
	}

	if (!shapes.empty()) {
		for (auto& s : shapes[0].frames)
			s->SetColorOffset(uint8_t(colOfs));
	}

	if (p.pressed.contains(257))
		Stop();
	if (p.pressed.contains(GLFW_KEY_A))
		wantedBg.shp+= 1;
	if (p.pressed.contains(GLFW_KEY_Q))
		wantedBg.shp = std::max(0, wantedBg.shp - 1);
	if (p.pressed.contains(GLFW_KEY_W))
		wantedBg.pal += 1;
	if (p.pressed.contains(GLFW_KEY_S))
		wantedBg.pal = std::max(0, wantedBg.pal - 1);
	if (p.pressed.contains(GLFW_KEY_E))
		colOfs += 1;
	if (p.pressed.contains(GLFW_KEY_D))
		colOfs = std::max(0, colOfs - 1);

	FrameParams np = p;
	np.currentTime -= startTime;
	Frame2D(np, shapes, [&] {
		VGA.PrintText(_font, { 10, 10 }, uint8_t(p.currentTime * 40), 3, 5, "pal:%d - shp:%d - ofs:%d", currentBg.pal, currentBg.shp, colOfs);
	});
}
