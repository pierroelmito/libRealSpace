//
//  SCAnimationPlayer.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <cstdint>

#include "IActivity.h"

class SCAnimationPlayer: public IActivity
{
	public :
	SCAnimationPlayer(int32_t animationID, int32_t backgroundID);
	~SCAnimationPlayer();

	void Init() override;
	void RunFrame(void) override;

private:
};
