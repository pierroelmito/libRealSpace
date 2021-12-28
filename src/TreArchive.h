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
#include <map>

#include "ByteSlice.h"

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
    
    
    static inline bool Compare(TreEntry* any, TreEntry* other){
        return any->data < other->data;
    }
    
    inline uint8_t* GetData(void){ return data; }
    
    inline bool IsValid(void){ return this->valid;}
    
private:

	bool valid{ false };

	std::vector<TreEntry*> entries;

	void ReadEntry(ByteStream* stream, TreEntry* entry);
    void Parse(void);

	//
    char path[512];
    uint8_t* data;
    size_t   size;

	// allows to know if we should free the TRE data
	bool initalizedFromFile{ false };

    std::map<const char*,TreEntry*,Char_String_Comparator> mappedEntries;
};
