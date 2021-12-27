//
//  WildCatBase.h
//  libRealSpace
//
//  Created by fabien sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include "IActivity.h"

class SCWildCatBase : public IActivity
{
public:
	SCWildCatBase();
	~SCWildCatBase();

	void Init( ) override;
	void RunFrame(void) override;
	void CheckKeyboard(void);

	RLEShape hangar;
	RLEShape vehicule;
};
