//
//  Camera.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <cmath>

#include "HandmadeMath.h"

#include "Matrix.h"

class RSCamera
{
public:
	void SetPersective(float fovy, float aspect, float zNear, float zFar);
	void SetPosition(const RSVector3& position);
	void LookAt(const RSVector3& lookAt);
	const RSMatrix& getView();
	const RSMatrix& getProj() const { return proj; };
	const RSVector3& getPosition() const { return position; }
	const RSVector3& getLookAt() const { return lookAt; }

protected:
	RSMatrix view;
	RSMatrix proj;
	RSVector3 position;
	RSVector3 lookAt;
	bool dirtyView{ true };
};
