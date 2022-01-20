//
//  NSObjectViewer.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/3/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "SCObjectViewer.h"

#include "precomp.h"

#include <cctype>

static const uint8_t PAK_ID_MENU_STATIC       = 0;
static const uint8_t PAK_ID_MENU_DYNAMC       = 4;

static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_BACKGROUND_RELEASED   = 0;
static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_BACKGROUND_PRESSED    = 1;

static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_UPARROW_RELEASED      = 2;
static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_UPARROW_PRESSED       = 3;

static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_DOWNARROW_RELEASED    = 4;
static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_DOWNARROW_PRESSED     = 5;

static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_UPARROW2_RELEASED     = 6;
static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_UPARROW2_PRESSED      = 7;

static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_DOWN2ARROW_RELEASED   = 8;
static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_DOWN2ARROW_PRESSED    = 9;

static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_LEFTARROW_RELEASED    =10;
static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_LEFTARROW_PRESSED     =11;

static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_RIGHTARROW_RELEASED   =12;
static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_RIGHTARROW_PRESSED    =13;

static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_EXIT_RELEASED         =14;
static const uint8_t PAK_ID_MENU_DYNAMC_JETNAME_EXIT_PRESSED          =15;

static const uint8_t PAK_ID_BACKGROUND  = 8;

SCObjectViewer::SCObjectViewer()
{
}

SCObjectViewer::~SCObjectViewer()
{
}

void ConvertToUpperCase(char *sPtr)
{
	while(*sPtr != '\0')
	{
		if (islower(*sPtr))
			*sPtr = toupper(*sPtr);
		sPtr++;
	}
}

void SCObjectViewer::ParseObjList(IffLexer* lexer)
{
	//The objects referenced are within this TRE archive
	TreArchive tre ;
	tre.InitFromFile("OBJECTS.TRE");

	//The object all follow the same path:
	const char* OBJ_PATH = TRE_DATA_OBJECTS;
	const char* OBJ_EXTENSION = ".IFF";

	IffChunk* chunk = lexer->GetChunkByID("OBJS");
	if (chunk == NULL){
		printf("**Error: Cannot parse Object List (Missing OBJS chunk).\n");
		return;
	}

	ByteStream stream(chunk->data);

	size_t numObjectInList = chunk->size / 33;

	showCases.reserve(numObjectInList);
	for(int objIndex = 0; objIndex < numObjectInList; objIndex++) {
		RSShowCase& showCase = showCases.emplace_back();

		char objName[9];
		for(int k = 0 ; k < 9 ; k++)
			objName[k] = stream.ReadByte();
		ConvertToUpperCase(objName);

		for(int k = 0 ; k < 20 ; k++)
			showCase.displayName[k] = stream.ReadByte();

		char modelPath[512];
		strcpy(modelPath,OBJ_PATH);
		strcat(modelPath,objName);
		strcat(modelPath, OBJ_EXTENSION);
		TreEntry* entry = tre.GetEntryByName(modelPath);

		if (entry == NULL){
			printf("Object reference '%s' not found in TRE.\n",modelPath);
			continue;
		}

		showCase.entity = new RSEntity();
		showCase.entity->InitFromRAM(*entry);

		uint32_t fixedPointDist = stream.ReadInt32LE();
		showCase.cameraDist = (fixedPointDist >> 8) + (fixedPointDist & 0xFF)/255.0f ;
		//showCase.cameraDist = 200000;
	}
}

void SCObjectViewer::NextObject()
{
	currentObject = (currentObject + 1) % showCases.size();
}

void SCObjectViewer::ParseAssets()
{
	auto& treGameFlow = Assets.tres[AssetManager::TRE_GAMEFLOW];
	auto archive = GetPak("OBJVIEW.PAK", *treGameFlow.GetEntryByName(TRE_DATA_GAMEFLOW "OBJVIEW.PAK"));
	//assets->List(stdout);
	//assets->Decompress("/Users/fabiensanglard/Desktop/ObjViewer.PAK", "MEH");

	/*
	PakEntry* entry0 = archive->GetEntry(PAK_ID_MENU_DYNAMC); OBJ_VIEWER BOARD
	PakArchive file0;
	file0.InitFromRAM("OBJVIEW.PAK: file PAK_ID_MENU_DYNAMC",entry0->data, entry0->size);
	file0.List(stdout);
	showAllImage(&file0);
	*/

	//Identified as OBJECT VIEWER STATIC TITLE

	//Identified as TRAINING MISSION TITLE
	/*
	PakEntry* entry1 = archive->GetEntry(1);
	PakArchive file1;
	file1.InitFromRAM("OBJVIEW.PAK: file 1",entry1->data, entry1->size);
	file1.List(stdout);
	showAllImage(&file1);
	*/

	//Identified as DOGFIGHT SETUP FIGHT SETUP # enemies and stuff
	/*
	PakEntry* entry2 = archive->GetEntry(2);
	PakArchive file2;
	file2.InitFromRAM("OBJVIEW.PAK: file 2",entry2->data, entry2->size);
	file2.List(stdout);
	showAllImage(&file2);
	*/

	//Identified as BUTTONS DODGE AIR TO AIR button and AIR TO GROUND BUTTON
	/*
	PakEntry* entry3 = archive->GetEntry(3);
	PakArchive file3;
	file3.InitFromRAM("OBJVIEW.PAK: file 3",entry3->data, entry3->size);
	file3.List(stdout);
	showAllImage(&file3);
	*/

	//Identified as BUTTONS OBJ VIEWER
	auto objButtons = GetPak("OBJVIEW.PAK: file 4", archive->GetEntry(4));
	objButtons->List(stdout);

	const Point2D boardPosition = { 4, 155 } ;
	const Point2D exitDimension{ 30, 15 };
	const Point2D exitPosition = { boardPosition.x + 268, boardPosition.y + 15 };
	const Point2D rotRightButtonPosition = { boardPosition.x + 232, boardPosition.y + 12 };
	const Point2D rotLeftButtonPosition = { boardPosition.x + 174, boardPosition.y + 12 };
	const Point2D rotDownButtonPosition = { boardPosition.x + 198, boardPosition.y + 24 };
	const Point2D rotUpButtonPosition = { boardPosition.x + 198, boardPosition.y + 6 };
	const Point2D zoomOutButtonPosition = { boardPosition.x + 122, boardPosition.y + 25 };
	const Point2D zoomInButtonPosition = { boardPosition.x + 121, boardPosition.y + 7 };
	const Point2D nextButtonPosition = { boardPosition.x + 10, boardPosition.y + 15 };
	const Point2D arrowDimension{ 15, 15 };
	const Point2D nextDimension = { 75, 15 };

	MakeButton(exitPosition, exitDimension, *objButtons, 14, 15, [] { Game.StopTopActivity(); });
	MakeButton(rotRightButtonPosition, arrowDimension, *objButtons, 12, 13, [] { Game.StopTopActivity(); });
	MakeButton(rotLeftButtonPosition, arrowDimension, *objButtons, 10, 11, [] { Game.StopTopActivity(); });
	MakeButton(rotDownButtonPosition, arrowDimension, *objButtons, 8, 9, [] { Game.StopTopActivity(); });
	MakeButton(rotUpButtonPosition, arrowDimension, *objButtons, 6, 7, [] { Game.StopTopActivity(); });
	MakeButton(zoomOutButtonPosition, arrowDimension, *objButtons, 4, 5, [] { Game.StopTopActivity(); });
	MakeButton(zoomInButtonPosition, arrowDimension, *objButtons, 2, 3, [] { Game.StopTopActivity(); });
	MakeButton(nextButtonPosition, nextDimension, *objButtons, 0, 1, [this] { NextObject(); });

	//buttons.push_back(button);
	//showAllImage(&file4);

	//Identified as DODGE FIGHT MISSION BUILDER ACCEPT CANCEL 12:00 3:00   NUMBERS GOOD FAIR
	/*
	PakEntry* entry5 = archive->GetEntry(5);
	PakArchive file5;
	file5.InitFromRAM("OBJVIEW.PAK: file 5",entry5->data, entry5->size);
	file5.List(stdout);
	showAllImage(&file5);
	*/

	//Identified as Dodge Fight background
	/*
	PakEntry* entry6 = archive->GetEntry(6);
	PakArchive file6;
	file6.InitFromRAM("OBJVIEW.PAK: file 6",entry6->data, entry6->size);
	file6.List(stdout);
	showAllImage(&file6);
	*/

	//Identified as DOGFIGHT PALETTE
	/*
	PakEntry* entry7 = archive->GetEntry(7);
	PakArchive file7;
	file7.InitFromRAM("OBJVIEW.PAK: file 7",entry7->data, entry7->size);
	file7.List(stdout);
	//showAllImage(&file7);
	*/

	//Identified as blue background

	InitShapes({ ShpBlueprint, ShpTitle });

	//Unknown content
	/*
	PakEntry* entry9 = archive->GetEntry(9);
	PakArchive file9;
	file9.InitFromRAM("OBJVIEW.PAK: file 9",entry9->data, entry9->size);
	file9.List(stdout);
	showAllImage(&file9);
	*/

	IffLexer lexer ;
	lexer.InitFromFile("PALETTE.IFF");
	//lexer.List(stdout);

	RSPalette palette;
	palette.InitFromIFF(&lexer);
	this->palette = *palette.GetColorPalette();
}

void SCObjectViewer::Init(void)
{
	ParseAssets();

	auto& treGameFlow = Assets.tres[AssetManager::TRE_GAMEFLOW];
	TreEntry* objViewIFF = treGameFlow.GetEntryByName(TRE_DATA_GAMEFLOW "OBJVIEW.IFF");
	IffLexer objToDisplay;
	objToDisplay.InitFromRAM(*objViewIFF);
	//objToDisplay.List(stdout);
	ParseObjList(&objToDisplay);

	// ListObjects();

	SetTitle("Neo Object Viewer");

	currentObject = 0;
}

void SCObjectViewer::RunFrame(const FrameParams& p)
{
	Frame2D(p, shapes);

	const RSShowCase& showCase = showCases[currentObject];

	const double totalTime = TimeToMSec * (p.currentTime - startTime);
	const double camTime = totalTime / 2000.0;
	const double lightTime = totalTime / 8000.0;

	const BoudingBox bbox = showCases[currentObject].entity->GetBoudingBpx();

	const RSVector3 newPosition{
		showCase.cameraDist / 150 * cosf(camTime),
		showCase.cameraDist / 350,
		showCase.cameraDist / 150 * sinf(camTime)
	};

	auto& cam = Renderer.GetCamera();
	cam.SetPosition(newPosition);
	cam.LookAt({ 0, 0.2f * (bbox.min.Y + bbox.max.Y), 0 });

	const RSVector3 light = HMM_NormalizeVec3({ 4.0f * cosf(lightTime), 4.0f, 4.0f * sinf(lightTime) });
	const RSMatrix id = HMM_Mat4d(1);

	Renderer.SetLight(light);
	Renderer.Draw3D({ R3Dp::CLEAR_COLORS }, [&] () {
		Renderer.DrawModel(showCases[currentObject].entity, LOD_LEVEL_MAX, id);
	});
}
