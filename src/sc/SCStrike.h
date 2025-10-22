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

class Plane {
public:
	Vector3 pos {};
	Vector3 dir { 0, 0, 1 };
	Vector3 up { 0, 1, 0 };
	RSEntity* _entity {};
};

class Pilot {
public:
	Vector3 lookAt {};
};

struct Jet {
	std::unique_ptr<RSEntity> entity;
	Quaternion orientation;
	Vector3 position;
};

class SCStrike : public IActivity {
public:
	SCStrike();
	virtual ~SCStrike();

	void Init();
	void RunFrame(const FrameParams& p) override;
	void ComputeMove(const Matrix& transform, GTime dt);
	Matrix ComputeTransform(bool cockpit, bool lookAt);

protected:
	Pilot pilot;
	Plane plane;

	RSArea area {};
	std::unique_ptr<RSEntity> _cockpit {};
	std::vector<Jet> jets;
};
