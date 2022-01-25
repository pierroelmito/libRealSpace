//
//  SCFontManager.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 2/5/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "RSFontManager.h"

#include "precomp.h"

RSFontManager::RSFontManager()
{
}

RSFontManager::~RSFontManager()
{
}

void RSFontManager::Init(TreArchive& tre)
{
	const char* fontPath = "..\\..\\DATA\\FONTS\\CONVFONT.SHP";
	TreEntry* convFontEntry = tre.GetEntryByName(fontPath);

	PakArchive fontArch;
	fontArch.InitFromRAM("CONVFONT.SHP", *convFontEntry);
	//fontArch.List(stdout);

	font.InitFromPAK(fontArch);
}

RSFont* RSFontManager::GetFont(const char* name)
{
	return &font;
}
