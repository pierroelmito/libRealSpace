//
//  AssetManager.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <vector>
#include <array>

class TreArchive;
class PakArchive;

#define TRE_DATA "..\\..\\DATA\\"

class AssetManager
{
public:
	enum TreID {TRE_GAMEFLOW, TRE_OBJECTS, TRE_MISC, TRE_SOUND, TRE_MISSIONS,TRE_TEXTURES, NUM_TRES } ;
	static_assert(NUM_TRES == 6);

	AssetManager();
	~AssetManager();

	void SetBase(const char* base);
	void Init(void);

	std::array<TreArchive, NUM_TRES> tres;

private:
	TreArchive* LoadTRE(const char* name);
	PakArchive* LoadPAK(const char* name);
};
