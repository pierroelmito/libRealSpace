//
//  SCFont.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 2/5/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "RSFont.h"

#include "precomp.h"

RSFont::RSFont()
{
}

RSFont::~RSFont()
{
}

RLEShape* RSFont::GetShapeForChar(char c)
{
	return letters[c].get();
}

void RSFont::InitFromPAK(const PakArchive& fontArchive)
{
	for (size_t i = 0; i < fontArchive.GetNumEntries(); i++) {
		const PakEntry& entry = fontArchive.GetEntry(i);
		auto& s = letters.emplace_back(std::make_unique<RLEShape>());
		s->Init(entry);
	}
}
