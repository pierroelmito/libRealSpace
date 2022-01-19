//
//  SCSelectWeaponsF16.h
//  libRealSpace
//
//  Created by fabien sanglard on 1/26/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include "IActivity.h"

//GAMEFLOW
// 70 74
//Main image 91	

class SCSelectWeaponF16 : public IActivity
{
public:
	SCSelectWeaponF16();
	~SCSelectWeaponF16();

	void Init( ) override;
	void RunFrame(const FrameParams& p) override;

protected:
	PalBg wantedBg{};
	PalBg currentBg{};
};

