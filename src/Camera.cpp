//
//  Camera.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

void RSCamera::SetPersective(float fovy, float aspect, float zNear, float zFar)
{
	proj = HMM_Perspective(fovy, aspect, zNear, zFar);
}

void RSCamera::SetPosition(const RSVector3& position)
{
	this->position = position;
	dirtyView = true;
}

void RSCamera::LookAt(const RSVector3& lookAt)
{
	this->lookAt = lookAt;
	dirtyView = true;
}

const RSMatrix& RSCamera::getView()
{
	if (dirtyView)
		view = HMM_LookAt(position, lookAt, { 0, 1, 0 });
	dirtyView = false;
	return view;
};
