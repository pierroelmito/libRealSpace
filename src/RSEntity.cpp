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
}

std::unique_ptr<RSEntity> RSEntity::LoadFromRAM(const ByteSlice& bytes)
{
	auto entity = std::make_unique<RSEntity>();
	entity->InitFromRAM(bytes);
	return entity;
}

void RSEntity::ParseTXMP(IffChunk* chunk)
{
	ByteStream stream(chunk->data);

	auto& image = images.emplace_back(std::make_unique<RSImage>());

	char name[8];
	for(int i=0; i < 8 ; i++)
		name[i] = stream.ReadByte();

	uint32_t width = stream.ReadShort();
	uint32_t height= stream.ReadShort();

	image->Create(name, width, height, 0);
	image->UpdateContent(stream.GetPosition());
}

void RSEntity::ParseTXMS(IffChunk* chunk)
{
	if (chunk == nullptr)
		return;

	if (chunk->children.size() <1 ){
		//That is not expected, at minimun we should have an INFO chunk.
		printf("Error: This TXMS doesn't even have an INFO chunk.");
		return;
	}

	ByteStream stream(chunk->children[0]->data);
	/*uint32_t version =*/ stream.ReadUInt32LE();
	//printf("TXMS Version: %u.\n",);

	for(int i =1; i < chunk->children.size() ; i++) {
		IffChunk* maybeTXMS = chunk->children[i];
		if (maybeTXMS->id == IdToUInt("TXMP"))
			ParseTXMP(maybeTXMS);
	}
}

void RSEntity::ParseVERT(IffChunk* chunk)
{
	if (chunk == nullptr)
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
	if (chunk == nullptr)
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
	if (chunk == nullptr)
		return;

	size_t numTriangle= chunk->size / 8;
	ByteStream stream(chunk->data);

	Triangle triangle ;
	for (int i = 0; i < numTriangle ; i++) {
		triangle.property = stream.ReadByte();
		triangle.ids = stream.ReadBytes<3>();
		triangle.color = stream.ReadByte();
		triangle.flags = stream.ReadBytes<3>();
		AddTriangle(&triangle);
	}
}

void RSEntity::ParseUVXY(IffChunk* chunk)
{
	if (chunk == nullptr)
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

		AddUV(uvEntry);
	}
}

void RSEntity::InitFromRAM(const ByteSlice& bytes)
{
	IffLexer lexer;
	lexer.InitFromRAM(bytes);
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

void RSEntity::AddVertex(const RSVector3& vertex)
{
	this->vertices.push_back(vertex);
}

void RSEntity::AddUV(const uvxyEntry& uv)
{
	this->uvs.push_back(uv);
}

void RSEntity::AddLod(Lod* lod)
{
	this->lods.push_back(*lod);
}

void RSEntity::AddTriangle(Triangle* triangle)
{
	this->triangles.push_back(*triangle);
}
