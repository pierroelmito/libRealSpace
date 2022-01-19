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
public:
	SCAnimationPlayer();
	~SCAnimationPlayer();

	void Init(int32_t animationID, int32_t backgroundID);
	void RunFrame(const FrameParams& p) override;

protected:
};
