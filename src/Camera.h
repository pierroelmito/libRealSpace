//
//  Camera.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <cmath>

#include "Math.h"

class RSCamera {
public:
	void SetFov(float fovy);
	void SetCam(const Vector3& position, const Vector3& lookAtv);
};
