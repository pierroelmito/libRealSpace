//
//  PakArchive.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 12/29/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

#include <cerrno>

PakArchive::PakArchive()
{
}

PakArchive::~PakArchive()
{
	if (initalizedFromFile)
		delete[] this->data;
}

void PakArchive::Parse(void)
{
	uint32_t advertisedSize = stream.ReadUInt32LE();

	//if (advertisedSize != this->size) {
	//if (advertisedSize != this->size)
	//	printf("expected size: %d instead of %d\n", this->size, advertisedSize);
	if (advertisedSize > this->size)
		return;

	ready = true;

	ByteStream peek(this->stream);
	uint32_t offset = peek.ReadUInt32LE();
	offset &= 0x00FFFFFF ; //Remove the leading 0xE0

	size_t numEntries = (offset-4)/4;

	//Hashmap to keep track of duplicates
	std::map<uint32_t, size_t> uniqueOffsets;

	//First to read all the offsets
	for(int i =0 ; i < numEntries ; i ++) {
		offset = stream.ReadUInt32LE();
		PakEntry entry{};
		entry.type = (offset & 0xFF000000) >> 24;
		offset &= 0x00FFFFFF ; //Remove the leading 0xE0 or 0xFF
		entry.data = this->data + offset;
		//if (uniqueOffsets[offset] == NULL){
			size_t index = entries.size();
			entries.push_back(entry);
			uniqueOffsets[offset] = index;
		//}
	}

	//numEntries = uniqueOffsets.size();

	//Second pass to calculate the sizes.
	int i =0;
	for( ; i < numEntries-1 ; i ++){
		PakEntry& entry = entries[i];
		entry.size = entries[i+1].data - entry.data;
	}

	PakEntry& entry = entries[i];
	entry.size = (this->data + this->size) - entries[i].data;

	//std::sort(entries.begin(), entries.end(),PakEntry::Compare);
}

bool PakArchive::InitFromFile(const char* filepath)
{
	char fullPath[512] ;
	fullPath[0] = '\0';

	strcat(fullPath, GetBase());
	strcat(fullPath, filepath);

	FILE* file = fopen(fullPath, "rb");

	if (!file){
		printf("Unable to open PAK archive: '%s'.\n",filepath);
		return false;
	}

	fseek(file, 0,SEEK_END);
	size_t fileSize = ftell(file);
	fseek(file,0 ,SEEK_SET);

	uint8_t* fileData = new uint8_t[fileSize];
	fread(fileData, 1, fileSize, file);

	initalizedFromFile = true;

	InitFromRAM(filepath,{ fileData, fileSize });

	fclose(file);

	return true;
}

void PakArchive::InitFromRAM(const char* name, const ByteSlice& bs)
{
	strcpy(this->path,name);

	this->data = bs.data;
	this->size = bs.size;

	stream.Set(this->data);

	ready = false;

	Parse();

	/*
	int i = -1;
	for (const auto& e : entries) {
		++i;
		RSVocSoundData snd;
		if (snd.InitFromRAM(e))
			printf("sound: %d\n", i);
	}
	*/
}

bool PakArchive::Decompress(const char* dstDirectory, const char* unkExtension)
{
	const char* suffix = ".CONTENT/";
	const char* filePattern = "FILE%04d.%s";
	char fullDstPath[512];

	printf("Decompressing PAK %s (size: %lu bytes)\n.",this->path,this->size);

	for( size_t idx = 0 ; idx < this->entries.size(); idx++) {
		const PakEntry& entry = entries[idx];
		const char* data = (const char*)entry.data;

		if(entry.size == 0)
			continue;

		const char vocHeader[] = "Creative Voice File";
		const int szVoc = sizeof(vocHeader) - 1;
		const bool isVoc = entry.size > sizeof(vocHeader) && strncmp(data, vocHeader, szVoc) == 0;
		const bool isIff = entry.size > 4 && strncmp(data, "FORM", 4) == 0;
		const char* extension = isVoc ? "voc" : (isIff ? "iff" : unkExtension);
		if (isVoc)
			printf("voc found!\n");
		else if (isIff)
			printf("iff found!\n");

		//Build dst path
		fullDstPath[0] = '\0';
		strcat(fullDstPath, dstDirectory);
		//Make sure we have a slash at the end of the dst.
		size_t pathLength = strlen(fullDstPath);
		if (fullDstPath[pathLength-1] != '/')
			strcat(fullDstPath,"/");
		//Append the PAK archive name
		strcat(fullDstPath, this->path);
		//Append the subdirectory name.
		strcat(fullDstPath, suffix);

		sprintf(fullDstPath + strlen(fullDstPath), filePattern, idx, extension);

		//Convert '\\' to '/'
		size_t sizeFullPath = strlen(fullDstPath);
		for (int i =0 ; i < sizeFullPath ; i++){
			if (fullDstPath[i] =='\\')
				fullDstPath[i] = '/';
		}

		//Make sure we have all the directories
		CreateDirectories(fullDstPath);

		//Write content.
		FILE* dstFile = fopen(fullDstPath,"w");

		if (dstFile == NULL){
			printf("Unable to create destination file: '%s'.\n",fullDstPath);
			continue;
		}

		size_t byteWritten = fwrite(entry.data,1, entry.size, dstFile);

		if (byteWritten != entry.size)
			printf("*Error while writing entry (errono: %s) size(size: %lu).\n",strerror(errno),entry.size);
		else
			printf("Extracted file: '%s. (size: %lu).'\n",fullDstPath,entry.size);

		fclose(dstFile);
	}

	return true;
}

size_t PakArchive::GetNumEntries(void) const
{
	return this->entries.size();
}

const PakEntry& PakArchive::GetEntry(size_t index) const
{
	return entries[index];
}

void PakArchive::List(FILE* output)
{
	fprintf(output,"Listing content of PAK archives '%s'\n",this->path);
	for(size_t i =0; i < GetNumEntries() ; i++){
		PakEntry& entry = entries[i];
		if (entry.size != 0) {
			fprintf(output,"    Entry [%3lu] offset[0x%8lX] size: %7lu bytes, type: %X.",i,entry.data-this->data, entry.size,entry.type);
			ByteStream::PrintBufStart(output, entry.data, entry.size, 16);
		} else {
			fprintf(output,"    Entry [%3lu] offset[0x%8lX] size: %7lu bytes, type: %X (DUPLICATE).",i,entry.data-this->data, entry.size,entry.type);
		}
		fprintf(output, "\n");
	}
}

void PakArchive::GuessPakEntryContent(PakEntry& entry)
{
}

void PakArchive::GuessContent(FILE* output)
{
	for(size_t i=0 ; i < entries.size() ; i++)
		GuessPakEntryContent(entries[i]);
}

