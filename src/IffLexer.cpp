//
//  IffLexer.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/28/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "IffLexer.h"

#include <cstring>

#include "Base.h"

IffChunk::IffChunk()
: subId(0)
{
}

IffChunk::~IffChunk()
{
	while (!children.empty()){
		IffChunk* chunk = children.back();
		children.pop_back();
		delete chunk;
	}
}

char* IffChunk::GetName()
{
	name[0] = (id & 0xFF000000) >> 24;
	name[1] = (id & 0x00FF0000) >> 16;
	name[2] = (id & 0x0000FF00) >> 8;
	name[3] = (id & 0x000000FF) >> 0;
	name[4] = 0;
	return name;
}

char* IffChunk::GetChunkTextID(uint32_t id)
{
	static char textIDs[5];
	char* cursor = (char*)&id;
	for (int i=3 ; i >=0 ; i--)
		textIDs[i] = cursor[3-i];
	return textIDs;
}

void IffChunk::List(FILE* output, int level)
{
	if (level >= 0) {
		const std::string tabs(level * 4, ' ');
		const std::string prefix = subId != 0 ? " " : tabs;
		if (subId != 0) {
			fprintf(output, "%s[%s]", tabs.c_str(), GetChunkTextID(subId));
		}
		if (size != 0) {
			const char eol = '\n';
			if (data != nullptr) {
				fprintf(output, "%s%s %d -", prefix.c_str(), GetChunkTextID(id), size);
				ByteStream::PrintBufStart(output, data, int(size), 16);
				fprintf(output, "\n");
			} else {
				fprintf(output, "%s%s %d\n", prefix.c_str(), GetChunkTextID(id), size);
			}
		}
	}
	for(IffChunk* child : children) {
		child->List(output, level + 1);
	}
}

//A CHUNK_HEADER_SIZE features a 4 bytes ID and a 4 bytes size;
#define CHUNK_HEADER_SIZE 8

IffLexer::IffLexer()
{
	this->path[0] = '\0';
}

IffLexer::~IffLexer()
{
	/*
	for(std::vector<IffChunk*>::iterator i = topChunk.childs.begin();
		i != topChunk.childs.end();
		++i)
		delete *i;
	*/

	while (!topChunk.children.empty()){
		IffChunk* chunk = topChunk.children.back();
		topChunk.children.pop_back();
		delete chunk;
	}

	/*
	for(int i=0 ; i < this->topChunk.childs.size() ; i++){
		printf("~IffLexer Feeing topChunk childs  %d. (size=%lu)\n",i,this->topChunk.childs.size());
		IffChunk* chunk = topChunk.childs[i];

		delete chunk;
	}
	 */
}

bool IffLexer::InitFromFile(const char* filepath)
{
	char fullPath[512] ;
	fullPath[0] = '\0';

	strcat(fullPath, GetBase());
	strcat(fullPath, filepath);

	FILE* file = fopen(fullPath, "rb");

	if (!file){
		printf("Unable to open IFF archive: '%s'.\n",filepath);
		return false;
	}

	fseek(file, 0,SEEK_END);
	size_t fileSize = ftell(file);
	fseek(file,0 ,SEEK_SET);

	uint8_t* fileData = new uint8_t[fileSize];
	fread(fileData, 1, fileSize, file);

	strcpy(this->path, filepath);

	return InitFromRAM({ fileData, fileSize });
}

bool IffLexer::InitFromRAM(const ByteSlice& bytes)
{
	this->data = bytes.data;
	this->size = bytes.size;

	stream.Set(this->data);

	if (this->path[0] == '\0')
		strcpy(this->path, "PALETTE FROM RAM");

	Parse();

	return true;
}

void IffLexer::Release()
{
	delete[] data;
	data = nullptr;
	size = {};
}

size_t IffLexer::ParseFORM(IffChunk* chunk)
{
	//FORM id
	chunk->id = stream.ReadUInt32BE();

	chunk->size = stream.ReadUInt32BE();

	if (chunk->size % 2 != 0)
		chunk->size++;
	size_t bytesToParse = chunk->size;

	//Form subtype
	chunk->subId = stream.ReadUInt32BE();
	chunksHashTable[chunk->subId] = chunk;

	bytesToParse-=4;

	while (bytesToParse > 0) {
		IffChunk* child = new IffChunk();
		size_t byteParsed = ParseChunk(child);
		chunk->children.push_back(child);
		chunksHashTable[child->id] = child;
		bytesToParse -= byteParsed;
	}
	return chunk->size+CHUNK_HEADER_SIZE;
}

size_t IffLexer::ParseChunk(IffChunk* chunk)
{
	ByteStream peek(stream);
	chunk->id = peek.ReadUInt32BE();
	switch (chunk->id) {
		case IdToUInt("FORM"):
			return ParseFORM(chunk);
			break;
		case IdToUInt("CAT "):
			return ParseFORM(chunk);
			break;
		case IdToUInt("LIST"):
			return ParseFORM(chunk);
			break;
		default:
		{
			chunk->id = stream.ReadUInt32BE();
			chunk->size = stream.ReadUInt32BE();
			if (chunk->size % 2 != 0)
				chunk->size++;
			//That this chunk
			chunk->data = stream.GetPosition();
			stream.MoveForward(chunk->size);
			chunksHashTable[chunk->id] = chunk;
			return chunk->size+CHUNK_HEADER_SIZE;
		}
		break;
	}
}

size_t IffLexer::ParseCAT(IffChunk* chunk)
{
	return ParseFORM(chunk);
}

//Return how many bytes were moved forward
size_t IffLexer::ParseLIST(IffChunk* chunk)
{
	return ParseFORM(chunk);
}

void IffLexer::Parse(void)
{
	size_t bytesToParse = this->size;

	ByteStream peek(stream);
	uint32_t header = peek.ReadUInt32BE();

	switch (header) {
		case IdToUInt("FORM"):
		case IdToUInt("CAR "):
		case IdToUInt("LIST"):
			break;
		default:
		{
			printf("ERROR, this is not an IFF file.\n");
			return;
			break;
		}
	}

	while(bytesToParse > 0) {
		IffChunk* child = new IffChunk();
		size_t byteParsed =ParseChunk(child);
		topChunk.children.push_back(child);
		chunksHashTable[child->id] = child;
		bytesToParse -= byteParsed;
	}
}

IffChunk* IffLexer::GetChunkByID(const char id[5])
{
	auto it = chunksHashTable.find(IdToUInt(id));
	if (it == chunksHashTable.end())
		return  nullptr;
	return it->second;
}

void IffLexer::List(FILE* output)
{
	topChunk.List(output, -1);
}
