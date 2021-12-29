//
//  Camera.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

void Camera::SetPersective(float fovy, float aspect, float zNear, float zFar)
{
	proj = HMM_Perspective(fovy, aspect, zNear, zFar);
}

void Camera::SetPosition(const Point3D& position)
{
	this->position = position;
	dirtyView = true;
}

void Camera::LookAt(const Point3D& lookAt)
{
	this->lookAt = lookAt;
	dirtyView = true;
}

Matrix& Camera::getView()
{
	if (dirtyView)
		view = HMM_LookAt(position, lookAt, { 0, 1, 0 });
	dirtyView = false;
	return view;
};
