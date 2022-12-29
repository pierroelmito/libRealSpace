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

void SCObjectViewer::InitFromExtractedFiles()
{
	constexpr size_t count = 221;
	constexpr const char* iffFiles[count] =  {
		"extracted_OBJECTS.TRE/DATA/OBJECTS/HANGAR3.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/ABRAMS-G.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/FAC10.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/GEAR4.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/RDRSTL2.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/MIG21AB.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/EJECSEAT.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/MIG29.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/SFA46.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/CHUTE.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/CASC84D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/MIRAGEAB.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/F-22.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/OILF23D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/EASTE30D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/REFA10D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/NUC28.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/POD.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/IRS12D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/GEAR1.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/DRUMC24.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/CASKPCD.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/REFB10.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/FORTW2D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/SMOKEGEN.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/F16PITXP.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/DMOBSAM.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/F22GEAR4.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/OFFB12D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/DJEEP.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/MILC22D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/NBLDG2D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/D40MMAA.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/C130DES.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/FORTT12D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/MILD32D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/DRADARST.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/SILO24D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/CASTLE10.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/DSPGUN.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/FACB10D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/AGM-65D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/OFFG10D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/MILE34D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/MILF22D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/YF23.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/PBLDGR.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/M261.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/PBLDGT.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/OFFD10.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/CASTW13.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/EJGUY.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/PYRAMID4.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/MIG29AB.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/F22GEAR2.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/ZSU-23.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/EASTC10D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/CASWA4.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/REFB12D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/EASTB10D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/HANGB22.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/REFA10.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/DCARRIER.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/CASKPC22.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/GEAR0.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/BARN22D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/GBU-15G.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/DCHUTE.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/DESTROYS.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/CASWA4D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/FORTT10.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/STASAMLN.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/SFGGA.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/LOWDCHUT.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/REFC10.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/CHURCH1.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/LOWEJECT.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/CARRIERW.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/F-18AB.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/OFFE10.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/40MMAA.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/FHOUS14.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/SFB20.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/GEAR2.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/TOWA24.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/FAC14D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/F22GEAR0.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/FORTW2.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/SFTA16.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/TORNCG.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/A-10.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/TRUCK.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/DBMP-2.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/747.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/KHAFJIGT.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/OFFF10.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/F16-AB.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/TORNCGAB.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/MIG29GRA.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/AMRAAM.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/DRUMA24.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/DBOAT.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/MILB22D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/OFFD10D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/DRUM1D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/OFFA12D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/DTRUCK2.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/OILOF23.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/LOWCHUTE.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/DABRMS-G.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/MIRAGE.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/TU-20.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/BRIGC14D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/SU-27AB.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/EASTA10D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/C130GRN.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/PBLDGP.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/WCB78D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/SWINDERJ.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/SWINDERM.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/OFFC10.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/SFTA4D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/OILTANKR.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/OFFG10.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/KHAFJIGD.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/DURANDAL.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/SPARTGUN.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/NUC28D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/F-15.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/HANGAR3D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/MK82.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/SFGGC.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/MK20.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/SA6.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/OFFC10D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/REFE10.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/F-16GUN.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/DRUMB16.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/JEEP.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/F22GEAR3.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/OFFB10.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/EASTC10.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/FORT32D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/MILE30.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/MILB20.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/BARN22.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/DDESTROY.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/SFGGB.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/MILG2O.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/EASTA10.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/AWACS.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/WCB78.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/OILA8.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/DZSU-23.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/F-22AB.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/DOME30D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/OFFA10.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/DT-72.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/F-18.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/DOILTANK.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/EASTD10D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/CASTW6D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/MOBSAMLN.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/OILA8D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/LEARJET.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/PYRAMD4D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/SFA46D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/F-16GRAY.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/PBLDGB.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/FORT30.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/MILG22D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/SPHINX55.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/BRIGC10.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/DOME28.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/SPHINX-D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/REFC8D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/MILA12D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/BUILD1D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/MILF20.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/STRIBASE.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/EASTE28.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/CASC101.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/HANGB22D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/EASTB10.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/CASB90D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/FHOUS14D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/MIG21.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/SFGGD.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/SFB20D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/F-15AB.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/FAC14.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/DRUMD24.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/TOWB24D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/NBLDG2.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/OFFE10D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/BMP-2.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/FORTWB2D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/MILC20.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/EASTD10.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/BOATX.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/SA2.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/F-16DES.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/PBLDGS.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/OFFF10D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/SILO24.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/IRS10.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/F22GEAR1.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/F22PIT.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/GEAR3.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/TOWA24D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/SU27.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/TANKGUN1.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/CHURCH14.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/MILD30.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/REFE10D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/MILA10.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/T-72.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/HANGBD.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/BUILD2D.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/FORTWB2.IFF",
		"extracted_OBJECTS.TRE/DATA/OBJECTS/TOWB24.IFF",
	};

	showCases.reserve(count);
	for(int objIndex = 0; objIndex < count; objIndex++) {
		RSShowCase& showCase = showCases.emplace_back();

		strncpy(showCase.displayName, iffFiles[objIndex] + 35, 20);

		IffLexer lexer;
		lexer.InitFromFile(iffFiles[objIndex]);
		showCase.entity = std::make_unique<RSEntity>();
		showCase.entity->InitFromIFF(&lexer);

		auto& bbox = showCase.entity->GetBoudingBpx();
		const auto sz = HMM_Length(bbox.max - bbox.min);
		showCase.cameraDist = 200.0f * sz;
	}
}

void SCObjectViewer::ParseObjList(IffLexer* lexer)
{
	//The objects referenced are within this TRE archive
	TreArchive& tre = Assets.tres[AssetManager::TRE_OBJECTS];

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

		showCase.entity = RSEntity::LoadFromRAM(*entry);

		const uint32_t fixedPointDist = stream.ReadInt32LE();
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

#if 0
	IffLexer lexer ;
	lexer.InitFromFile("PALETTE.IFF");
	//lexer.List(stdout);
	RSPalette palette;
	palette.InitFromIFF(&lexer);
	this->palette = *palette.GetColorPalette();
	lexer.Release();
#else
	palette = *RSPalette::LoadFromFile("PALETTE.IFF").GetColorPalette();
#endif
}

void SCObjectViewer::Init(void)
{
	_font = FontManager.GetFont("");

	ParseAssets();

#if 1
	auto& treGameFlow = Assets.tres[AssetManager::TRE_GAMEFLOW];
	TreEntry* objViewIFF = treGameFlow.GetEntryByName(TRE_DATA_GAMEFLOW "OBJVIEW.IFF");
	IffLexer objToDisplay;
	objToDisplay.InitFromRAM(*objViewIFF);
	//objToDisplay.List(stdout);
	ParseObjList(&objToDisplay);
#else
	InitFromExtractedFiles();
#endif

	// ListObjects();

	SetTitle("Neo Object Viewer");

	currentObject = 0;
}

void SCObjectViewer::RunFrame(const FrameParams& p)
{
	const RSShowCase& showCase = showCases[currentObject];

	Frame2D(p, shapes, [&] {
		VGA.PrintText(_font, { 10, 50 }, 255, 3, 5, "%s", showCase.displayName);
	});

	const double totalTime = TimeToMSec * p.activityTime ;
	const double camTime = totalTime / 2000.0;
	const double lightTime = totalTime / 8000.0;

	const BoudingBox bbox = showCases[currentObject].entity->GetBoudingBpx();

	const RSVector3 newPosition{
		showCase.cameraDist / 150 * cosf(camTime),
		showCase.cameraDist / 350,
		showCase.cameraDist / 150 * sinf(camTime)
	};

	auto& cam = Renderer.GetCamera();
	cam.SetCam(newPosition, { 0, 0.2f * (bbox.min.Y + bbox.max.Y), 0 });

	const RSVector3 light = HMM_NormalizeVec3({ 4.0f * cosf(lightTime), 4.0f, 4.0f * sinf(lightTime) });
	const RSMatrix id = HMM_Mat4d(1);

	Renderer.SetLight(light);
	Renderer.Draw3D({ R3Dp::CLEAR_COLORS }, [&] () {
		Renderer.DrawModel(showCases[currentObject].entity.get(), LOD_LEVEL_MAX, id);
	});

	if (!running)
		Renderer.ClearCache();
}
