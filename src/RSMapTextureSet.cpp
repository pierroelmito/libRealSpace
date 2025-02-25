//
//  RSMapTextureSet.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

RSMapTextureSet::RSMapTextureSet()
{
}

RSMapTextureSet::~RSMapTextureSet()
{
}

void RSMapTextureSet::InitFromPAK(PakArchive* archive)
{
	strcpy(name,archive->GetName());
	Parse(archive);
	printf("%d images in set\n", int(images.size()));
	std::map<std::pair<int, int>, int> counts;
	for (auto&& img : images)
		counts[{ int(img->width), int(img->height) }] += 1;
	for (const auto& i : counts)
		printf("- %d x %d: %d images\n", int(i.first.first), int(i.first.second), i.second);
}

size_t RSMapTextureSet::GetNumImages(void)
{
	return images.size();
}

RSImage* RSMapTextureSet::GetImageById(size_t index)
{
	return images[index].get();
}

void RSMapTextureSet::Parse(PakArchive* archive)
{
	for(size_t i = 0 ; i < archive->GetNumEntries() ; i++){

		if (i == 229 || i ==  230)
			printf("Intersest !\n");

		const PakEntry& entry = archive->GetEntry(i);

		if (entry.size == 0)
			continue;

		ByteStream stream(entry.data);

		const uint16_t width = stream.ReadUShort();
		const uint16_t height = stream.ReadUShort();
		const uint32_t size = width*height;

		if (entry.size-4 == size){
			//That does look like a map texture !
			auto& image = images.emplace_back(std::make_unique<RSImage>());
			image->Create("MAP_TEXTURE",width,height, 0 /*IMAGE_FLAG_COPY_PALINDEX_TO_ALPHA*/);
			image->UpdateContent(stream.GetPosition());
			//printf("RSMapTextureSet img [%3zu] is %lux%lu.\n",i,image->width,image->height);
		} else {
			printf("Cannot make sense of entry %lu:\n REASON: (entry size is %lu but advertised is %d).\n",i,entry.size,size);
		}
	}

	printf("RSMapTextureSet found %lu textures in %s.\n",archive->GetNumEntries(),archive->GetName());
}

void RSMapTextureSet::List(FILE* output)
{
	printf("Listing all textures from '%s'.\n",name);

	for(size_t i = 0 ; i < images.size() ; i++){
		auto&& image = images[i];
		printf("Image [%3lu] width: %zu height: %zu size: %lu bytes.\n",i,image->width,image->height,image->width*image->height);
	}
}

