//
//  ConAssetManager.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 2/1/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <map>
#include <vector>

#include "PakArchive.h"
#include "TreArchive.h"
#include "RSImageSet.h"

class IffChunk;
//class RLEShape;

#define CONV_TOP_BAR_HEIGHT 23
#define CONV_BOTTOM_BAR_HEIGHT 48

#define CONV_BORDER_MARGIN  69
#define CONV_INTERLETTER_SPACE  3
#define CONV_SPACE_SIZE  5

struct CharFace
{
	char name[9];
	RSImageSet appearances{};
	// size_t paletteID;
};

struct FacePalette
{
	char name[9];
	uint8_t index;
};

struct CharFigure
{
	char name[9];
	RLEShape* appearance{ nullptr };
	size_t paletteID;
};

struct ConvBackGround
{
	std::vector<RLEShape*> layers;
	std::vector<uint8_t*> palettes;
	char name[9];
};

class ConvAssetManager
{
public:
	ConvAssetManager();
	~ConvAssetManager();

	void Init();
	void Release();

	CharFace* GetCharFace(char* name);
	ConvBackGround* GetBackGround(char* name);
	CharFigure* GetFigure(char* name);

	uint8_t GetFacePaletteID(const char* name);

private:
	void BuildDB();
	void ReadBackGrounds(const IffChunk* chunk);
	void ReadFaces(const IffChunk* chunk);
	void ReadFigures(const IffChunk* chunk);
	void ReadPFigures(const IffChunk* chunk);
	//I have no idea what is in there.
	void ReadFCPL(const IffChunk* chunk);
	//I have no idea what is in there.
	void ReadFGPL(const IffChunk* chunk);

	std::map<std::string, std::unique_ptr<CharFace>> faces;
	std::map<std::string, std::unique_ptr<FacePalette>> facePalettes;
	std::map<std::string, std::unique_ptr<ConvBackGround>> backgrounds;
	std::map<std::string, std::unique_ptr<CharFigure>> figures;

	PakArchive _convShps;
	PakArchive _convPals;
	PakArchive _optShps;
	PakArchive _optPals;

	void ParseBGLayer(uint8_t* data, size_t layerID,ConvBackGround* back);
};
