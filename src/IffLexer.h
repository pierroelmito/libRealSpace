//
//  IffLexer.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/28/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <vector>
#include <map>

#include "ByteStream.h"
#include "ByteSlice.h"

constexpr uint32_t IdToUInt(const char id[5])
{
	uint8_t a = id[0];
	uint8_t b = id[1];
	uint8_t c = id[2];
	uint8_t d = id[3];
	return (a << 24) | (b << 16) | (c << 8) | d;
}

class IffChunk
{
public:
	IffChunk();
	~IffChunk();

	uint32_t id{};
	uint8_t* data{};
	size_t size{};
	uint32_t subId{}; //In the case of FORM,CAT  and LIST
	std::vector<IffChunk*> children;
	char name[5];

	char* GetName();
	char* GetChunkTextID(uint32_t id);
	void List(FILE* output, int level = 0);
};

class IffLexer
{
public:
	IffLexer();
	~IffLexer();

	bool InitFromFile(const char* filepath);
	bool InitFromRAM(const ByteSlice& bytes);
	void Release();

	void List(FILE* output);
	IffChunk* GetChunkByID(const char id[5]);
	inline const char* GetName(){ return this->path;}
	DataBufferPtr Buffer() { return _buffer; }

private:
	size_t ParseChunk(IffChunk* child);
	size_t ParseFORM(IffChunk* child);
	size_t ParseCAT(IffChunk* child);
	size_t ParseLIST(IffChunk* child);

	void Parse(void);
	std::map<uint32_t,IffChunk*> chunksHashTable;

	DataBufferPtr _buffer;
	ByteStream stream;
	uint8_t* data{ nullptr };
	size_t size{};

	IffChunk topChunk;

	char path[512];
};

