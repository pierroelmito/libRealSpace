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

class SCStrike : public IActivity
{
public:
	SCStrike();
	~SCStrike();

	void Init( ) override;
	void RunFrame(void) override;

private:
	RSArea area;
};

