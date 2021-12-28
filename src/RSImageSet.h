//
//  RSImageSet.h
//  libRealSpace
//
//  Created by fabien sanglard on 2/3/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <cstdint>
#include <vector>

#include "PakArchive.h"

class RLEShape;

class RSImageSet
{
public:
	RSImageSet();
	~RSImageSet();

	void InitFromRAM(const ByteSlice& entry);
	const std::vector<RLEShape*>& GetShapes() const { return shapes; }
	void Add(RLEShape* shape);

private:
	std::vector<RLEShape*> shapes;
};
