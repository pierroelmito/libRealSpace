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

void RSCamera::SetCam(const RSVector3& position, const RSVector3& lookAt)
{
	viewChanged = true;
	view = HMM_LookAt(position, lookAt, { 0, 1, 0 });
}

const RSMatrix&
RSCamera::getView(bool* changed) const
{
	if (changed && viewChanged)
		*changed = true;
	viewChanged = false;
	return view;
}
