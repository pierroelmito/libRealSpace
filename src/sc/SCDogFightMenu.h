//
//  SCDogFightMenu.h
//  libRealSpace
//
//  Created by fabien sanglard on 1/3/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include "IActivity.h"

class SCDogFightMenu : public IActivity
{
public:
	SCDogFightMenu();
	~SCDogFightMenu();

	void Init( ) override;
	void RunFrame(const FrameParams& p) override;

protected:
};

