//
//  AssetManager.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

AssetManager::AssetManager()
{
}

AssetManager::~AssetManager()
{
}

void AssetManager::SetBase(const char* newBase)
{
	::SetBase(newBase);
}

void AssetManager::Init()
{
	struct TreNameID
	{
		AssetManager::TreID id;
		const char* filename;
	};

	TreNameID nameIds[NUM_TRES] = {
		{AssetManager::TRE_GAMEFLOW,"GAMEFLOW.TRE"},
		{AssetManager::TRE_OBJECTS,"OBJECTS.TRE"},
		{AssetManager::TRE_MISC,"MISC.TRE"},
		{AssetManager::TRE_SOUND,"SOUND.TRE"},
		{AssetManager::TRE_MISSIONS,"MISSIONS.TRE"},
		{AssetManager::TRE_TEXTURES,"TEXTURES.TRE"},
		{AssetManager::TRE_MGSPEECH,"MGSPEECH.TRE"},
		{AssetManager::TRE_MSSPEECH,"MSSPEECH.TRE"},
	};

	//Load all TRE in RAM and store them.
	for (size_t i =0 ; i < NUM_TRES; i++) {
		TreArchive& tre = tres[i];
		tre.InitFromFile(nameIds[i].filename);
		if (!tre.IsValid())
			Game.Terminate("Unable to load asset '%s' (Did you set the SC base folder ?).", nameIds[i].filename);
		tre.List(stdout);
		//tre.Decompress((std::string("extracted_") + nameIds[i].filename).c_str());
	}
}

void AssetManager::Release()
{
	for (TreArchive& t : tres)
		t.Release();
}

