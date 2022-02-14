//
//  SCStrike.h
//  libRealSpace
//
//  Created by fabien sanglard on 1/28/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include "IActivity.h"
#include "RSArea.h"

class SCStrike : public IActivity
{
public:
	SCStrike();
	~SCStrike();

	void Init();
	void RunFrame(const FrameParams& p) override;

protected:
	RSVector3 camPos{};
	float angleV{};
	float angleH{};

	RSArea area{};
	std::unique_ptr<RSEntity> _cockpit{};
	std::vector<std::unique_ptr<RSEntity>> jets;
};

