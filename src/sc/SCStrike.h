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

class Plane
{
public:
	RSVector3 pos{};
	RSVector3 dir{ 0, 0, 1 };
	RSVector3 up{ 0, 1, 0 };
	RSEntity* _entity{};
};

class Pilot
{
public:
	std::optional<RSVector3> lookAt{};
};

struct Jet
{
	std::unique_ptr<RSEntity> entity;
	RSQuaternion orientation;
	RSVector3 position;
};

class SCStrike : public IActivity
{
public:
	SCStrike();
	~SCStrike();

	void Init();
	void RunFrame(const FrameParams& p) override;
	void ComputeMove(const RSMatrix& transform, GTime dt);
	RSMatrix ComputeTransform(bool cockpit);

protected:
	Pilot pilot;
	Plane plane;

	RSArea area{};
	std::unique_ptr<RSEntity> _cockpit{};
	std::vector<Jet> jets;
};

