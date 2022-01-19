//
//  SCMainMenu.h
//  libRealSpace
//
//  Created by fabien sanglard on 1/3/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include "IActivity.h"

class SCMainMenu : public IActivity
{
public:
	SCMainMenu();
	~SCMainMenu();

	virtual void Init();
	virtual void RunFrame(const FrameParams& p) override;
};
