//
//  RSMap.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once

#include "RSEntity.h"

class RSMapTextureSet;
class TreArchive;
class PakEntry;
class PakArchive;

//DIRTY HACK !!! DELETE ME ASAP
//#define HEIGHT_DIVIDER 17
//#define BLOCK_WIDTH 512
#define HEIGHT_DIVIDER 32
#define BLOCK_WIDTH 512
#define OBJECT_SCALE 0.05f

struct MapObject
{
	char name[9]{};
	char destroyedName[9]{};
	float position[3]{};
	RSEntity* entity{ nullptr };
	float transform[3][3]{};
};

struct MapVertex
{
	RSVector3 v;
	RSVector3 n;
	uint8_t flag;
	uint8_t type;
	uint8_t lowerImageID;
	uint8_t upperImageID;
	float color[4];
};

struct AreaBlock
{
	size_t width;
	size_t height;
	int sideSize;

	//To be delete later when we can parse it properly
	MapVertex vertice[400];

	inline const MapVertex* GetVertice(int x, int y) const {
		return &vertice[x + y * this->sideSize];
	}
};

#define BLOCK_LOD_MAX 0
#define BLOCK_LOD_MED 1
#define BLOCK_LOD_MIN 2

#define NUM_LODS 3

#define BLOCK_PER_MAP_SIDE 18
#define BLOCKS_PER_MAP (BLOCK_PER_MAP_SIDE*BLOCK_PER_MAP_SIDE)

class RSArea
{
public:
	RSArea();
	~RSArea();

	void InitFromPAKFileName(const char* pakFilename);

	inline const AreaBlock& GetAreaBlockByID(int lod,int blockID) const {
		return this->blocks[lod][blockID];
	}

	inline const AreaBlock* GetAreaBlockByCoo(int lod, int x, int y) const {
		return &this->blocks[lod][x + y * BLOCK_PER_MAP_SIDE];
	}

	RSImage* GetImageByID(size_t ID) const;

	//Per block objects list
	std::vector<MapObject> objects[BLOCKS_PER_MAP];
	float elevation[BLOCKS_PER_MAP];

	const std::vector<RSEntity*>& GetJets() const { return jets; }

private:
    
    void ParseMetadata(void );
    void ParseObjects(void );
    
#if USE_SHADER_PIPELINE != 1
	void ParseTrigo(void );
	void ParseTriFile(const PakEntry* entry);
#endif

    //Temporary name: I don't know yet what is in there.
    void ParseHeightMap(void);
	void ParseBlocks(size_t lod,const PakEntry* entry,size_t verticePerBlock);
    
	void ParseElevations();
    
    std::vector<RSMapTextureSet*> textures;
    PakArchive* archive;
    
    // An area is made of 18*18 (324) blocks each block has 3 levels of details
    // Level 0 blocks are 20*20;
    // Level 1 blocks are 10*10;
    // Level 0 blocks are  5* 5;
    AreaBlock blocks[NUM_LODS][BLOCKS_PER_MAP];
    
    
    char name[16];
    
	void AddJet(TreArchive* tre, const char* name, RSQuaternion* orientation, RSVector3* position);
	void AddJets();

	std::vector<RSEntity*> jets;
};
