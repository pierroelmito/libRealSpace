//
//  SCFont.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 2/5/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <vector>

#include "PakArchive.h"

class RLEShape;

class RSFont
{
public:
	RSFont();
	~RSFont();
	RLEShape* GetShapeForChar(char c);
	void InitFromPAK(const PakArchive& fontArchive);

private:
	std::vector<std::unique_ptr<RLEShape>> letters;
};

