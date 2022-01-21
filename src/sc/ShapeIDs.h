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

#if 0

//CONVSHPS.PAK
enum ConvShapeID
{
	DESERT_LANDSTRIP,
	HACEIENDA,
	wc_hangar=7,
	tent=9,
	rich_mansion_entry=12,
	wc_changeroom=13,
	office=15,
	office2=16,
	wc_hangar2=19, //Wildcat hangar conv
	wc_hangar3=20, //Wildcat hangar conv
	tent2=21,
	tent3=22,
	virgil_office_chair=23,
	virgil_office_wall=24,
	wc_changeroom2=25,
	wc_changeroom_fullscreen=26, //full screen
	prideauxCorner_chair = 29,
	prideauxCorner_wall = 30,
	desert_truck = 31,
	desert_truck2 =32,
	shelves = 33,
	shelves2= 34,
	backalley=35,
	windows = 36,
	base_cu = 38,
	ba2_cu= 39,
	set_windows2=40,
	set_map=41,
	bar_fs=42,
	bar2_fs=43,
	bar_counter=44,
	bar_counter2=45,

	//FACE
	AIR, //46    NOt a PAK, a sequence of 62 entries.
		  /*
		 HAIRS
		 ANIMS
		 cloths
		 sunclass
		 helmet
		 visor
		 //palette ?
		   */
	BETA,     // 47
		  //last entry = accept_full screen
	BILLY,  //48
	FARID, //49
		  //not last entry = accept_full screen
	GWEN,  //50
	JANET, //51 betray me girl
	LYLE,
	MELE = 53,
	MIGUEL =54,
	MULSTAFA =55,
	prideau =56,
	stern =57,
	tex =58,
	virgil =59,
	walt =60,
	tweedly =61,
	goon =62,


	//FIGURE
	FIGURE_AIR = 63,

	FIGURE_X = 63,

	FIGURE_STERN = 68,

	FIGURE_MIGUEL = 70,

	FIGURE_VIRGIL = 71,

	FIGURE_GWEN = 72,

	FIGURE_GENERAK = 74,
	FIGURE_LYLE = 75,
	FIGURE_TEX = 76,
	FIGURE_VIRGIL2 = 77,

	FACE_MUGGER = 78,

	WILD_CAT_BASE_DESTROYED= 79, //full screen
	DONNO= 80, //full screen

	ALL_FIGURES = 81,
	TEX_FIGURE = 82,
	GWEN_FIGURE = 83,
	 X_FIGURE = 84,
	 Y_FIGURE = 85,
	 Z_FIGURE = 86,

	AIRPLANE_WILDCAT_BACKGROUND = 87,
	AIRPLACE_OUTDOOR_BACKGROUND = 88,

	FIGURE_AIR_POINTING = 89, // animation

	AIRPLANE_WILDCAT_BACKGROUND2 = 90,
	AIRPLANE_OUTDOOR_BACKGROUND2 = 91,

	BANKRUPT_VIRGIL_OFFICE = 92,
};

//OPTSHPS.PAK
enum OptionShapeID{
	BAR_ROOM_PRUDEAUX = 0,
	BAR_LOBBY_BILLY = 1,
	BAR_ROOM_FAN = 2, // animation 6 images
	BAR_LOBBY_JANE = 4, // animation 8 images
	BAR_LOBBY_MIGUEL = 5, // animation 12 images
	BAR_ROOM_QUATARIS = 6, // animation 14 images
	BAR_LOBBY_WC_MEMBER = 7, // animation 11 images
	BAR_BACKROOM_BG = 8,
	BAR_LOBBY_BG = 9,
	BAR_ROOM_BG = 10,
	BAR_ROOM_BURRITO_MAN = 11, // animation 10 images
	BAR_ROOM_UNKNOWN_MAN = 12, // animation 13 images
	WILDCAT_CHANGEROOM_BG = 13,

	WILDCAT_HANGAR_VEHICULE_F16 = 16,
	SOMETHING = 17,

	WILDCAT_HANGAR_VEHICULE_JEEP = 17, //also contains the animation, not in a pak

	WILDCAT_HANGAR = 18,
	WILDCAT_HANGAR_DAMAGE_DECAL = 19,
	WILDCAT_HANGAR_VEHICULE_TRUCK = 20, //also contains the animation, not in a pak
	WILDCAT_VIRGIL_OFFICE = 21,
	WILDCAT_VIRGIL_OFFICE_VIRGIL = 22, // animation 35 images
	ISTANBUL_BG = 23,
	BAR_FROM_WINDOW_BG = 24,
	MISSION_TENT_BG = 25,

	WILDCAT_CHANGEROOM_ANIM = 26, // animation 10  images
	MISSION_TENT_LEFT_WOMAN = 27, // animation 12  images
	BAR_LOBBY_GWEN = 28, // animation 9 images
	MISSION_TENT_MAN_MAP = 29, // animation 6 images
	MISSION_TENT_STERN_SEAT = 30, // animation 5 images
	MISSION_TENT_MIGUEL_SEAT = 31, // animation 5 images
	MISSION_TENT_BILLY = 32, // animation 6 images

	WILDCAT_HANGAR_GWEN = 35, // animation 5 images
	WILDCAT_HANGAR_JANE = 36, // animation 9 images
	WILDCAT_HANGAR_MIGUEL = 37, // animation 13 images

	WILDCAT_HANGAR_LYLE = 39, // animation 7 images

	SELECT_WEAPON_F16_IN_DESERT_BG = 40,

	ACCOUNTING_BOOK = 41, //animation (not in a pak) 5 images

	DESERT_TAKE_OFF_BG = 42,

	ISTANBUL_TO_BAR_ROAD_CITY_BG = 43,

	MOUTAINS_BG = 44,

	LAMDSTRIP_BG = 45,

	SELECT_WEAPON_F16_DECAL = 47,

	SELECT_WEAPON_F16_AT_WILDCAT_BG = 91,

	ACCOUNTING_BOOK_STATIC = 92,
	ACCOUNTING_DECALS = 93, // UP TO 114

	MISSION_DESERT_OUTSIDE_BG = 115,

	SKY = 116,
	BAR_SALIMS_OUTSIDE = 117,


	WILDCAT_CHANGEROOM_PINUP_W = 118,
	WC_BASE_OUTSIDE_BG = 119,
	LAMDSTRIP_BG2 = 120,

	FROM_SALIM_BAR_CAR_DEPART_ANIM = 121, //animation (not in a pak) 21 images
	FROM_SALIM_FRONT_CAR_DEPART_ANIM = 122, //animation (not in a pak) 21 images

	MAP_NORTH_AMERICA = 128,
	MAP_SOUTH_AMERICA = 129,
	MAP_EUROPE = 130,
	MAP_SOUTH_AFRICA = 131,
	MAP_ISLAND = 132,

	MAP_ALASKA = 134,

	ANIM_PLANE = 135,


	FROM_SALIM_CAR_DEPART_ANIM = 136, //animation (not in a pak) 20 images

	TO_SALIM_BAR_CAR_GOING_ANIM = 137, //animation (not in a pak)
	TO_SALIM_FRONT_CAR_GOING_ANIM = 138, //animation (not in a pak)
	TO_WILCAT_BASE_CAR_GOING_ANIM = 139, //animation (not in a pak)

	NOTE_BOOK = 140,

	JET_SIDE_PAINT_SHOTS = 141,
	JET_SIDE_PAINT_SHOTS_ITEMS= 142, //not a pak
							//planes
							//planes5
							//planes10
							//ground
							//ground5
							//ground10

	JET_SIDE_PAINT_SHOTS_ANIMATION= 143, //(not in a pak)

	JUNGLE_TREES_BG = 144,

	TUNDRA_TREES_BG = 145,
	MISSION_OUTSIDE_VEHICULE_TRUCK = 146,
	MISSION_OUTSIDE_VEHICULE_F16 = 147,
	MISSION_OUTSIDE_VEHICULE_F22 = 148,
	DESERT_LANDSCAPE_OUTSIDE_BG = 149,

	START_GAME_REGISTRATION = 160,

	BAR_ROOM_SUNGLASSES_MAN = 161,

	LANDING_TRIP_SUMMER = 169,
	LANDING_TRIP_WINTER = 170,
	LANDING_TRIP_SPRING = 171,

	MOUNTAIN_ANIM_BG = 171,
	C130_WITH_ONE_f16 = 172,
	WILDCAT_CHANGEROOM_PINUP_M = 174,
	//NO MORE IMAGES AFTER THAT
};

#endif

enum class AnimMode
{
	First, // display only first frame animation
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

constexpr PalBg OptBarChar1 =        { OPTSHPS,  OPTPALS,   0,   5 };
constexpr PalBg OptBarTablesChar2 =  { OPTSHPS,  OPTPALS,   0,   6 };
constexpr PalBg OptBarChar2 =        { OPTSHPS,  OPTPALS,   0,   7 };
constexpr PalBg OptBarTablesChar0 =  { OPTSHPS,  OPTPALS,   0,  11 };
constexpr PalBg OptBarTablesChar1 =  { OPTSHPS,  OPTPALS,   0,  12 };
constexpr PalBg OptHangarChar1 =     { OPTSHPS,  OPTPALS,   0,  22 };
constexpr PalBg OptBarChar0 =        { OPTSHPS,  OPTPALS,   0,  46 };
constexpr PalBg OptTable =           { OPTSHPS,  OPTPALS,   1,   8 };
constexpr PalBg OptBarFg =           { OPTSHPS,  OPTPALS,   2,   2 };
constexpr PalBg OptBarBg =           { OPTSHPS,  OPTPALS,   2,   9 };
constexpr PalBg OptBarTables =       { OPTSHPS,  OPTPALS,   3,  10 };
constexpr PalBg OptChangeRoomBg =    { OPTSHPS,  OPTPALS,   4,  13 };
constexpr PalBg OptHangarDoor0 =     { OPTSHPS,  OPTPALS,   5,  14 };
constexpr PalBg OptHangarDoor1 =     { OPTSHPS,  OPTPALS,   5,  15 };
constexpr PalBg OptF16 =             { OPTSHPS,  OPTPALS,   5,  16 };
constexpr PalBg OptHangar =          { OPTSHPS,  OPTPALS,   5,  18 };
constexpr PalBg OptHangarTruck =     { OPTSHPS,  OPTPALS,   5,  20, AnimMode::First };
constexpr PalBg OptHangarJeep =      { OPTSHPS,  OPTPALS,   5,  17, AnimMode::Character };
constexpr PalBg OptHangarChar2 =     { OPTSHPS,  OPTPALS,   5,  35 };
constexpr PalBg OptHangarChar0 =     { OPTSHPS,  OPTPALS,   5,  37 };
constexpr PalBg OptHangarChar3 =     { OPTSHPS,  OPTPALS,   5, 166 };
constexpr PalBg OptDesk =            { OPTSHPS,  OPTPALS,   6,  21 };
constexpr PalBg OptDesertCity =      { OPTSHPS,  OPTPALS,   7,  43 };
constexpr PalBg OptJeep00 =          { OPTSHPS,  OPTPALS,   7, 121 };
constexpr PalBg OptJeep02 =          { OPTSHPS,  OPTPALS,   7, 137 };
constexpr PalBg OptLookOutside =     { OPTSHPS,  OPTPALS,   8,  24 };
constexpr PalBg OptJeep01 =          { OPTSHPS,  OPTPALS,   8, 122, AnimMode::Cutscene };
constexpr PalBg OptTentInsideBg =    { OPTSHPS,  OPTPALS,   9,  25 };
constexpr PalBg OptTentInsideFg =    { OPTSHPS,  OPTPALS,   9,  27 };
constexpr PalBg OptTentInsideChar0 = { OPTSHPS,  OPTPALS,   9,  30 };
constexpr PalBg OptTentInsideChar1 = { OPTSHPS,  OPTPALS,   9,  31 };
constexpr PalBg OptBaseOutside =     { OPTSHPS,  OPTPALS,  11, 119 };
constexpr PalBg OptPlaneTakeoff00 =  { OPTSHPS,  OPTPALS,  11, 136 };
constexpr PalBg OptTentOutside00 =   { OPTSHPS,  OPTPALS,  12,  40 };
constexpr PalBg OptOutsideHanger =   { OPTSHPS,  OPTPALS,  12,  91 };
constexpr PalBg OptCalculator =      { OPTSHPS,  OPTPALS,  13,  92 };
constexpr PalBg OptTentOutsideBg =   { OPTSHPS,  OPTPALS,  14, 115 };
constexpr PalBg OptTentOutPlane00 =  { OPTSHPS,  OPTPALS,  14, 147 };
constexpr PalBg OptTentOutPlane01 =  { OPTSHPS,  OPTPALS,  14, 162 };
constexpr PalBg OptDesertStreet =    { OPTSHPS,  OPTPALS,  15, 117 };
constexpr PalBg OptCutsceneMoveA0 =  { OPTSHPS,  OPTPALS,  16, 119 };
constexpr PalBg OptCutsceneMoveA1 =  { OPTSHPS,  OPTPALS,  16, 139, AnimMode::Cutscene };
constexpr PalBg OptPinupF =          { OPTSHPS,  OPTPALS,  17, 118 };
constexpr PalBg OptPlaneOutside =    { OPTSHPS,  OPTPALS,  19, 141 };
constexpr PalBg OptSky =             { OPTSHPS,  OPTPALS,  21, 116 };
constexpr PalBg OptMap00 =           { OPTSHPS,  OPTPALS,  23, 128 };
constexpr PalBg OptMap01 =           { OPTSHPS,  OPTPALS,  23, 129 };
constexpr PalBg OptMap02 =           { OPTSHPS,  OPTPALS,  23, 130 };
constexpr PalBg OptMap03 =           { OPTSHPS,  OPTPALS,  23, 131 };
constexpr PalBg OptMap04 =           { OPTSHPS,  OPTPALS,  23, 132 };
constexpr PalBg OptMap05 =           { OPTSHPS,  OPTPALS,  23, 133 };
constexpr PalBg OptMap06 =           { OPTSHPS,  OPTPALS,  23, 134 };
constexpr PalBg OptMountain	=        { OPTSHPS,  OPTPALS,  24,  44 };
constexpr PalBg OptDesert00 =        { OPTSHPS,  OPTPALS,  25,  42 };
constexpr PalBg OptDesert01 =        { OPTSHPS,  OPTPALS,  25, 149 };
constexpr PalBg OptDesert02 =        { OPTSHPS,  OPTPALS,  25, 168 };
constexpr PalBg OptCitySky =         { OPTSHPS,  OPTPALS,  26,  23 };
constexpr PalBg OptRegistration =    { OPTSHPS,  OPTPALS,  30, 160 };
constexpr PalBg OptChangeRoomFg =    { OPTSHPS,  OPTPALS,  31,  26 };
constexpr PalBg OptRoad00 =          { OPTSHPS,  OPTPALS,  32,  45 };
constexpr PalBg OptRoadDesert =      { OPTSHPS,  OPTPALS,  33, 120 };
constexpr PalBg OptJungle00 =        { OPTSHPS,  OPTPALS,  35, 169 };
constexpr PalBg OptForest00 =        { OPTSHPS,  OPTPALS,  36, 170 };
constexpr PalBg OptPlain00 =         { OPTSHPS,  OPTPALS,  36, 171 };
constexpr PalBg OptPinupM =          { OPTSHPS,  OPTPALS,  40, 174 };

constexpr PalBg ConvCamp =           { CONVSHPS, CONVPALS,  0,   0 };
constexpr PalBg ConvHouse =          { CONVSHPS, CONVPALS,  1,   1 };
constexpr PalBg ConvHangerInside0 =  { CONVSHPS, CONVPALS,  2,   7 };
constexpr PalBg ConvTentInside0 =    { CONVSHPS, CONVPALS,  8,   9 };
constexpr PalBg ConvPortrait =       { CONVSHPS, CONVPALS, 10,  12 };
constexpr PalBg ConvWall =           { CONVSHPS, CONVPALS, 13,  13 };
constexpr PalBg ConvDesk0 =          { CONVSHPS, CONVPALS, 14,  15 };
constexpr PalBg ConvDesk1 =          { CONVSHPS, CONVPALS, 16,  16 };
constexpr PalBg ConvHangerInside1 =  { CONVSHPS, CONVPALS, 17,  19 };
constexpr PalBg ConvElevator =       { CONVSHPS, CONVPALS, 20,  20 };
constexpr PalBg ConvTentInside1 =    { CONVSHPS, CONVPALS, 21,  21 };
constexpr PalBg ConvTentInside2 =    { CONVSHPS, CONVPALS, 22,  22 };
constexpr PalBg ConvChair =          { CONVSHPS, CONVPALS, 23,  23 };
constexpr PalBg ConvBooksStatue =    { CONVSHPS, CONVPALS, 24,  24 };

constexpr PalBg ShpClouds =          { MIDGAMES, OPTPALS,  21,  20 };

constexpr PalBg ShpBoard =           { MAINMENU, MAINMENU,  2,   1 };

constexpr PalBg ShpBlueprint =       { OBJVIEW, nullptr,    0,   8 };
constexpr PalBg ShpTitle =           { OBJVIEW, nullptr,    0,   0 };
constexpr PalBg ShpTrainingTitle =   { OBJVIEW, OBJVIEW,    7,   1 };
constexpr PalBg ShpTraingBg0 =       { OBJVIEW, OBJVIEW,    7,   6 };

