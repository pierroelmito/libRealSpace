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
	void SetCam(const RSVector3& position, const RSVector3& lookAtv);
	void SetView(const RSMatrix& view) { viewChanged = true; this->view = view; }
	const RSMatrix& getView(bool* changed = nullptr) const;
	const RSMatrix& getProj() const { return proj; }
	const RSVector3 getPosition() const { return { view.Elements[3][0], view.Elements[3][1], view.Elements[3][2] }; }

protected:
	RSMatrix view;
	RSMatrix proj;
	mutable bool viewChanged{ false };
};
