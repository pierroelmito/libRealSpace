//
//  RSEntity.h
//  libRealSpace
//
//  Created by fabien sanglard on 12/29/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <vector>
#include <map>
#include <memory>

#include "Matrix.h"
#include "Quaternion.h"
#include "ByteSlice.h"

class RSImage;
class IffChunk;
class IffLexer;

enum LOD_LEVEL {
	LOD_LEVEL_MAX,
	LOD_LEVEL_MED,
	LOD_LEVEL_MIN,
};

struct BoudingBox
{
	RSVector3 min;
	RSVector3 max;
};

struct UV
{
	uint8_t u;
	uint8_t v;
};

struct uvxyEntry
{
	uint8_t triangleID;
	uint8_t textureID;
	UV uvs[3];
};

struct Triangle
{
	uint8_t property;
	uint8_t ids[3];
	uint8_t color;
	uint8_t flags[3];
};

struct Lod
{
	uint32_t dist;
	uint16_t numTriangles;
	uint16_t triangleIDs[256];
};

class RSEntity
{
public:
	RSEntity();
	~RSEntity();

	void InitFromRAM(const ByteSlice& bytes);
	void InitFromIFF(IffLexer* lexer);

	void AddImage(std::unique_ptr<RSImage>&& image);
	void AddVertex(const RSVector3& vertex);
	void AddUV(const uvxyEntry& uv);
	void AddLod(Lod* lod);
	void AddTriangle(Triangle* triangle);

	std::vector<std::unique_ptr<RSImage>> images;
	std::vector<RSVector3> vertices;
	std::vector<uvxyEntry> uvs;
	std::vector<Lod> lods;
	std::vector<Triangle> triangles;

	enum Property {
		TRANSPARENT = 0x02,
		COLORKEY = 0x06,
	};

	const BoudingBox& GetBoudingBpx() const { return bb; }

	//For rendering
	RSVector3 position;
	RSQuaternion orientation;

	bool IsPrepared() const { return prepared; }
	bool prepared{ false };

private:
	BoudingBox bb;
	void CalcBoundingBox(void);

	//Has the entity been sent to te GPU and is ready to be renderer.

	void ParseVERT(IffChunk* chunk);
	void ParseLVL(IffChunk* chunk);
	void ParseVTRI(IffChunk* chunk);
	void ParseTXMS(IffChunk* chunk);
	void ParseUVXY(IffChunk* chunk);
	void ParseTXMP(IffChunk* chunk);
};
