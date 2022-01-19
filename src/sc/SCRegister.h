//
//  SCRegister.h
//  libRealSpace
//
//  Created by fabien sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include "IActivity.h"

class SCRegister : public IActivity
{
public:
	SCRegister();
	~SCRegister();

	void Init();
	void RunFrame(const FrameParams& p) override;
};

