//
//  tre.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/28/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <cstring>
#include <cstdint>
#include <cstdio>

#include <vector>
#include <string>
#include <map>

#include "ByteSlice.h"

class PakArchive;
class ByteStream;

struct Char_String_Comparator
{
	bool operator()(char const *a, char const *b) const
	{
		return strcmp(a, b) < 0;
	}
};

struct TreEntry : public ByteSlice
{
	uint8_t unknownFlag{ 0 };
	char name[65]{};
};

class TreArchive
{
public:
	 TreArchive();
	~TreArchive();

	bool InitFromFile(const char* filepath);
	void InitFromRAM(const char* name, uint8_t* data, size_t size);
	void Release();

	char* GetPath(void);

	void List(FILE* output);

	//Direct access to a TRE entry.
	TreEntry* GetEntryByName(const char* entryName);

	//Build a pak directly
	bool GetPAKByName(const char* entryName,PakArchive* pak);

	//A way to iterate through all entries in the TRE.
	TreEntry* GetEntryByID(size_t entryID);
	size_t GetNumEntries(void);

	bool Decompress(const char* dstDirectory);

	static inline bool Compare(TreEntry* any, TreEntry* other) {
		return any->data < other->data;
	}

	inline uint8_t* GetData() const { return data; }
	inline bool IsValid() const { return this->valid;}

private:
	void ReadEntry(ByteStream* stream, TreEntry* entry);
	void Parse(void);

	char path[512];
	std::vector<TreEntry> entries;
	std::map<std::string, size_t> mappedEntries;
	uint8_t* data{ nullptr };
	size_t size{ 0 };
	bool valid{ false };
	bool initalizedFromFile{ false }; // allows to know if we should free the TRE data
};
