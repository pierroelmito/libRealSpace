//
//  RSImageSet.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 2/3/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "RSImageSet.h"

#include "precomp.h"

RSImageSet::RSImageSet()
{
}

RSImageSet::~RSImageSet()
{
}

void RSImageSet::InitFromRAM(const ByteSlice& entry)
{
	uint8_t* end = entry.data + entry.size;
	ByteStream index(entry.data);

	uint32_t nextImage = index.ReadUInt32LE();
	//uint8_t flag  = (nextImage & 0xFF000000) >> 24;
	//printf("flag = %2X\n",flag);
	nextImage = nextImage & 0x00FFFFFF;

	const uint32_t numImages = nextImage / 4;
	for(size_t i = 0 ; i < numImages && (entry.data + nextImage < end); i++) {
		uint8_t* currImage = entry.data+nextImage;

		nextImage = index.ReadUInt32LE();
		nextImage = nextImage & 0x00FFFFFF;
		//flag  = (nextImage & 0xFF000000) >> 24;
		//printf("flag = %2X\n",flag);

		size_t size = 0;
		if (i == numImages - 1){
		} else {
		}

		auto& shape = shapes.emplace_back(std::make_unique<RLEShape>());
		shape->Init(currImage, size);
	}
}

void RSImageSet::Add(RLEShape* shape)
{
	shapes.emplace_back(shape);
}
