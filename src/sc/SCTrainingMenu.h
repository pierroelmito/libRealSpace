//
//  File.h
//  libRealSpace
//
//  Created by fabien sanglard on 1/28/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include "IActivity.h"

class SCTrainingMenu : public IActivity
{
public:
	SCTrainingMenu();
	~SCTrainingMenu();

	void Init( ) override;
	void RunFrame(const FrameParams& p) override;

protected:
	RLEShape title;
	RLEShape background;
	RLEShape board;
};

