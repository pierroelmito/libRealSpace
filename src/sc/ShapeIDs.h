//
//  Backgrounds.h
//  libRealSpace
//
//  Created by fabien sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include "PaletteIDs.h"

#include <cstring>
#include <utility>

enum class AnimMode
{
	First, // display only first frame animation
	Second, // display only second frame animation
	Character, //display frame 0 + loop over remaining frames
	Cutscene, // display each frame sequentially
};

struct PalBg {
	const char* pakShp{};
	const char* pakPal{};
	int pal{};
	int shp{};
	AnimMode am{ AnimMode::Character };
	bool operator !=(const PalBg& o) {
		if (o.pal != pal)
			return true;
		if (o.shp != shp)
			return true;
		if (strcmp(o.pakShp, pakShp) != 0)
			return true;
		if (strcmp(o.pakPal, pakPal) != 0)
			return true;
		return false;
	}
};

#define TRE_DATA "..\\..\\DATA\\"
#define TRE_DATA_GAMEFLOW TRE_DATA "GAMEFLOW\\"
#define TRE_DATA_OBJECTS TRE_DATA "OBJECTS\\"
#define TRE_DATA_TXM TRE_DATA "TXM\\"
#define OPTSHPS TRE_DATA_GAMEFLOW "OPTSHPS.PAK"
#define OPTPALS TRE_DATA_GAMEFLOW "OPTPALS.PAK"
#define CONVSHPS TRE_DATA_GAMEFLOW "CONVSHPS.PAK"
#define CONVPALS TRE_DATA_GAMEFLOW "CONVPALS.PAK"
#define MIDGAMES TRE_DATA "MIDGAMES\\MIDGAMES.PAK"
#define MAINMENU TRE_DATA_GAMEFLOW "MAINMENU.PAK"
#define OBJVIEW TRE_DATA_GAMEFLOW "OBJVIEW.PAK"

constexpr PalBg OptSky =             { OPTSHPS,  OPTPALS,  21, 116 };
constexpr PalBg OptMountain	=        { OPTSHPS,  OPTPALS,  24,  44 };
constexpr PalBg OptRegistration =    { OPTSHPS,  OPTPALS,  30, 160 };
constexpr PalBg ShpClouds =          { MIDGAMES, OPTPALS,  21,  20 };
constexpr PalBg ShpBoard =           { MAINMENU, MAINMENU,  2,   1 };
constexpr PalBg ShpBlueprint =       { OBJVIEW, nullptr,    0,   8 };
constexpr PalBg ShpTitle =           { OBJVIEW, nullptr,    0,   0 };
constexpr PalBg ShpTrainingTitle =   { OBJVIEW, OBJVIEW,    7,   1 };
constexpr PalBg ShpTraingBg0 =       { OBJVIEW, OBJVIEW,    7,   6 };

