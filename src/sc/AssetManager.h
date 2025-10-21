//
//  AssetManager.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <array>
#include <vector>

#include "ShapeIDs.h"
#include "TreArchive.h"

class PakArchive;

class AssetManager {
public:
	enum TreID {
		TRE_GAMEFLOW,
		TRE_OBJECTS,
		TRE_MISC,
		TRE_SOUND,
		TRE_MISSIONS,
		TRE_TEXTURES,
		TRE_MGSPEECH,
		TRE_MSSPEECH,
		NUM_TRES
	};
	static_assert(NUM_TRES == 8);

	AssetManager();
	~AssetManager();

	void SetBase(const char* base);
	void Init(void);
	void Release();

	std::array<TreArchive, NUM_TRES> tres;

private:
	TreArchive* LoadTRE(const char* name);
	PakArchive* LoadPAK(const char* name);
};
