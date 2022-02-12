//
//  main.c
//  tre
//
//  Created by fabien sanglard on 12/7/2013.
//  Copyright (c) 2013 fabien sanglard. All rights reserved.
//

#include "precomp.h"

TreArchive::TreArchive()
{
	this->path[0] = '\0';
}

TreArchive::~TreArchive()
{
}

void TreArchive::Release()
{
	_dataBuffer = nullptr;
	initalizedFromFile = false;
	data = nullptr;
	size = 0;
	entries.clear();
	mappedEntries.clear();
}

bool TreArchive::InitFromFile(const char* filepath)
{
	char fullPath[512] ;
	fullPath[0] = '\0';

	strcat(fullPath, GetBase());
	strcat(fullPath, filepath);

	_dataBuffer = ByteSlice::ReadFile(fullPath);
	if (!_dataBuffer)
		return false;

	InitFromRAM(filepath, ByteSlice::Get(_dataBuffer));

	return true;
}

void TreArchive::InitFromRAM(const char* name, const ByteSlice& bs)
{
	strcpy(this->path, name);

	data = bs.data;
	size = bs.size;

	Parse();
}

void TreArchive::ReadEntry(ByteStream* stream, TreEntry* entry)
{
	/*The format of a TRE entry is (see doc/p1_tre_format.txt):

	 data type    |     size      |       description
	 ------------------------------------------------------------------------------
	 byte        |       1       |  always 0x1, it's meaning is not fully
	 |               | understood.
	 ----------------|---------------|---------------------------------------------
	 string      |      65       |  The name of the file as the game reads it.
	 |               | Privateer's internal file structure demands
	 |               | that every file name starts with ..\..
	 ----------------|---------------|---------------------------------------------
	 dword       |       4       |  The offset on the TRE where the file starts.
	 ----------------|---------------|---------------------------------------------
	 dword       |       4       |  The size of the file.
	 ----------------|---------------|---------------------------------------------

	 */

	entry->unknownFlag = stream->ReadByte();
	for(int i=0 ; i < 65 ; i++)
		entry->name[i] = stream->ReadByte();
	entry->data = this->data + stream->ReadUInt32LE();
	entry->size = stream->ReadUInt32LE();
}

void TreArchive::Parse(void)
{
	ByteStream stream(this->data);

	const size_t numEntries = stream.ReadUInt32LE() ;

	//The pointer to the start of the data. We are not using it.
	stream.ReadUInt32LE() ;

	//Now read all entries
	entries.reserve(numEntries);
	for(size_t i =0; i < numEntries; i++){
		TreEntry entry{};
		ReadEntry(&stream, &entry);

		size_t index = entries.size();
		mappedEntries[entry.name] = index;

		// We use a vector so we can return the list of files in the order
		// they were listed in the TRE index.
		entries.push_back(entry);
	}

	// std::sort(entries.begin(), entries.end(),TreArchive::Compare);

	valid = true;
}

void TreArchive::List(FILE* output)
{
	fprintf(output,"Listing content of TRE archive '%s'.\n",this->path);
	fprintf(output,"    %lu entrie(s) found.\n",entries.size());

	for (size_t i=0 ; i < entries.size() ; i++){
		TreEntry& entry = entries[i];
		fprintf(output,"    Entry [%3lu] offset[0x%8lX]'%s' size: %lu bytes.\n",i,entry.data-this->data,entry.name,entry.size);
	}
}

//Direct access to a TRE entry.
TreEntry* TreArchive::GetEntryByName(const char* entryName)
{
	auto it = mappedEntries.find(entryName);
	if (it == mappedEntries.end())
		return nullptr;
	return &entries[it->second];
}

bool TreArchive::GetPAKByName(const char* entryName,PakArchive* pak)
{
	TreEntry* entry = GetEntryByName(entryName);
	if (entry == NULL)
		return false;
	pak->InitFromRAM(entryName, *entry);
	if (!pak->IsReady())
		return false;
	return true;
}

TreEntry* TreArchive::GetEntryByID(size_t entryID)
{
	return &entries[entryID];
}

size_t TreArchive::GetNumEntries(void)
{
	return entries.size();
}

bool TreArchive::Decompress(const char* dstDirectory)
{
	for(size_t i = 0; i < mappedEntries.size(); i++) {
		TreEntry& entry = entries[i];

		char fullPath[512];
		fullPath[0] = '\0';
		strcat(fullPath,dstDirectory);

		//Make sure the dstDirectory end with a /
		size_t dstSize = strlen(fullPath);
		if (fullPath[dstSize-1] != '/')
			strcat(fullPath,"/");

		//Remove the leading . and .. and /
		char* cursor = entry.name;
		while(*cursor == '.' ||
			  *cursor == '/' ||
			  *cursor == '\\')
			cursor++;
		strcat(fullPath, cursor);

		//Convert '\\' to '/'
		size_t sizeFullPath = strlen(fullPath);
		for (int i =0 ; i < sizeFullPath ; i++){
			if (fullPath[i] =='\\')
				fullPath[i] = '/';
		}

		//Recursively create the directories
		CreateDirectories(fullPath);

		//Write file !
		printf("Decompressing TRE file: %lu '%s' %lu (bytes).\n",i,fullPath,entry.size);
		FILE* file = fopen(fullPath,"wb");
		fwrite(entry.data, 1, entry.size, file);
		fclose(file);

		const char* ext = fullPath + strlen(fullPath) - 4;
		if (strcmp(ext, ".IFF") == 0) {
			strcat(fullPath, ".dump");
			FILE* iffFile = fopen(fullPath,"w");
			IffLexer lexer;
			lexer.InitFromRAM(entry);
			lexer.List(iffFile);
			fclose(iffFile);
		} else if (strcmp(ext, ".PAK") == 0) {
			PakArchive pak;
			pak.InitFromRAM(fullPath, entry);
			strcat(fullPath, ".dump");
			pak.Decompress(fullPath, "bin");
		}
	}

	return true;
}
