//
//  RSEntity.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 12/29/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "RSEntity.h"

#include <cfloat>

#include "RSImage.h"
#include "IffLexer.h"

RSEntity::RSEntity()
{
}

RSEntity::~RSEntity()
{
	while(!images.empty()) {
		RSImage* image = images.back();
		images.pop_back();
		delete image;
	}
}

void RSEntity::ParseTXMP(IffChunk* chunk)
{
	ByteStream stream(chunk->data);

	RSImage* image = new RSImage();

	char name[8];
	for(int i=0; i < 8 ; i++)
		name[i] = stream.ReadByte();

	uint32_t width = stream.ReadShort();
	uint32_t height= stream.ReadShort();

	image->Create(name, width, height, IMAGE_FLAG_COPY_PALINDEX_TO_ALPHA);
	image->UpdateContent(stream.GetPosition());

	AddImage(image);
}

void RSEntity::ParseTXMS(IffChunk* chunk)
{
	if (chunk==NULL)
		return;

	if (chunk->childs.size() <1 ){
		//That is not expected, at minimun we should have an INFO chunk.
		printf("Error: This TXMS doesn't even have an INFO chunk.");
		return;
	}

	ByteStream stream(chunk->childs[0]->data);
	/*uint32_t version =*/ stream.ReadUInt32LE();
	//printf("TXMS Version: %u.\n",);

	for(int i =1; i < chunk->childs.size() ; i++) {
		IffChunk* maybeTXMS = chunk->childs[i];
		if (maybeTXMS->id == IdToUInt("TXMP"))
			ParseTXMP(maybeTXMS);
	}
}

void RSEntity::ParseVERT(IffChunk* chunk)
{
	if (chunk==NULL)
		return;

	const auto readCoord = [] (int32_t coo) -> float {
		return (coo>>8) + (coo&0x000000FF)/255.0;
	};

	ByteStream stream(chunk->data);
	const size_t numVertice = chunk->size/12;
	for (int i = 0; i < numVertice ; i++) {
		const float x = readCoord(stream.ReadInt32LE());
		const float y = readCoord(stream.ReadInt32LE());
		const float z = readCoord(stream.ReadInt32LE());
		AddVertex({ y, z, x }); // Pierro: need to swizzle coord, why?
	}
}

void RSEntity::ParseLVL(IffChunk* chunk)
{
	if (chunk==NULL)
		return;

	ByteStream stream(chunk->data);

	Lod lod;
	lod.numTriangles = (chunk->size - 4) / 2;
	lod.dist = stream.ReadUInt32LE();

	for (int i = 0 ; i < lod.numTriangles ; i++)
		lod.triangleIDs[i] = stream.ReadUShort();

	AddLod(&lod);
}

void RSEntity::ParseVTRI(IffChunk* chunk)
{
	if (chunk==NULL)
		return;

	size_t numTriangle= chunk->size / 8;
	ByteStream stream(chunk->data);

	Triangle triangle ;
	for (int i = 0; i < numTriangle ; i++) {
		triangle.property = stream.ReadByte();

		triangle.ids[0] = stream.ReadByte();
		triangle.ids[1] = stream.ReadByte();
		triangle.ids[2] = stream.ReadByte();

		triangle.color = stream.ReadByte();

		triangle.flags[0] = stream.ReadByte();
		triangle.flags[1] = stream.ReadByte();
		triangle.flags[2] = stream.ReadByte();

		AddTriangle(&triangle);
	}
}

void RSEntity::ParseUVXY(IffChunk* chunk)
{
	if (chunk==NULL)
		return;

	ByteStream stream(chunk->data);
	const size_t numEntries = chunk->size/8;

	for (size_t i=0; i < numEntries; i++) {
		uvxyEntry uvEntry;

		uvEntry.triangleID = stream.ReadByte();
		uvEntry.textureID =  stream.ReadByte();

		uvEntry.uvs[0].u = stream.ReadByte();
		uvEntry.uvs[0].v = stream.ReadByte();

		uvEntry.uvs[1].u = stream.ReadByte();
		uvEntry.uvs[1].v = stream.ReadByte();

		uvEntry.uvs[2].u = stream.ReadByte();
		uvEntry.uvs[2].v = stream.ReadByte();

		AddUV(&uvEntry);
	}
}

void RSEntity::InitFromRAM(const ByteSlice& bytes)
{
	return InitFromRAM(bytes.data, bytes.size);
}

void RSEntity::InitFromRAM(uint8_t* data, size_t size)
{
	IffLexer lexer;
	lexer.InitFromRAM(data, size);
	InitFromIFF(&lexer);
}

void RSEntity::InitFromIFF(IffLexer* lexer)
{
	IffChunk* chunk;

	chunk = lexer->GetChunkByID("UVXY");
	ParseUVXY(chunk);

	chunk = lexer->GetChunkByID("VTRI");
	ParseVTRI(chunk);

	chunk = lexer->GetChunkByID("VERT");
	ParseVERT(chunk);

	chunk = lexer->GetChunkByID("TXMS");
	ParseTXMS(chunk);

	chunk = lexer->GetChunkByID("LVL0");
	if (chunk!= NULL)
		ParseLVL(chunk);

	chunk = lexer->GetChunkByID("LVL1");
	if (chunk!= NULL)
		ParseLVL(chunk);

	chunk = lexer->GetChunkByID("LVL2");
	if (chunk!= NULL)
		ParseLVL(chunk);

	CalcBoundingBox();
}

void RSEntity::CalcBoundingBox(void)
{
	this->bb.min.X = FLT_MAX;
	this->bb.min.Y = FLT_MAX;
	this->bb.min.Z = FLT_MAX;

	this->bb.max.X = FLT_MIN;
	this->bb.max.Y = FLT_MIN;
	this->bb.max.Z = FLT_MIN;

	for(size_t i =0; i < this->vertices.size() ; i++) {
		const RSVector3& vertex = vertices[i];

		if (bb.min.X > vertex.X)
			bb.min.X = vertex.X;
		if (bb.min.Y > vertex.Y)
			bb.min.Y = vertex.Y;
		if (bb.min.Z > vertex.Z)
			bb.min.Z = vertex.Z;

		if (bb.max.X < vertex.X)
			bb.max.X = vertex.X;
		if (bb.max.Y < vertex.Y)
			bb.max.Y = vertex.Y;
		if (bb.max.Z < vertex.Z)
			bb.max.Z = vertex.Z;
	}
}

void RSEntity::AddImage(RSImage* image)
{
	this->images.push_back(image);
}

void RSEntity::AddVertex(const RSVector3& vertex)
{
	this->vertices.push_back(vertex);
}

void RSEntity::AddUV(uvxyEntry* uv)
{
	this->uvs.push_back(*uv);
}

void RSEntity::AddLod(Lod* lod)
{
	this->lods.push_back(*lod);
}

void RSEntity::AddTriangle(Triangle* triangle)
{
	this->triangles.push_back(*triangle);
}
