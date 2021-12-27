//
//  Camera.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once

#include "HandmadeMath.h"

#include "Matrix.h"

class Camera
{
public:
	void SetPersective(float fovy, float aspect, float zNear, float zFar);
	void SetPosition(const Point3D& position);
	void LookAt(const Point3D& lookAt);
	Matrix& getView();

	Matrix view;
	Matrix proj;
	Vector3D position;
	Vector3D lookAt;
	bool dirtyView{ true };
};
