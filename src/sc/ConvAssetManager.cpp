//
//  ConAssetManager.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 2/1/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

#include "RSImageSet.h"

ConvAssetManager::ConvAssetManager()
{
}

ConvAssetManager::~ConvAssetManager()
{
	Game.Log("We are not freeing the RAM from all the RLEs !!!\n");
}

void ConvAssetManager::Init(void)
{
	BuildDB();
}

CharFace* ConvAssetManager::GetCharFace(char* name)
{
	CharFace* npc = this->faces[name];

	if (npc == NULL){
		static CharFace dummy;
		RSImageSet* set = new RSImageSet();
		set->Add(RLEShape::GetEmptyShape());
		dummy.appearances = set;
		npc = &dummy;
		Game.Log("ConvAssetManager: Cannot find npc '%s', returning dummy npc instead.\n",name);
		this->faces[name] = npc;
	}

	return npc;
}

ConvBackGround* ConvAssetManager::GetBackGround(char* name)
{
	ConvBackGround* shape = this->backgrounds[name];

	if (shape == NULL){
		Game.Log("ConvAssetManager: Cannot find loc '%s', returning dummy loc instead.\n",name);
		static ConvBackGround dummy;
		uint8_t dummyPalettePatch[5] = { 0, 0, 0 ,0 , 0};
		dummy.palettes.push_back(dummyPalettePatch);
		dummy.layers.push_back(RLEShape::GetEmptyShape());
		shape = &dummy;
		this->backgrounds[name] = shape;
	}

	return shape;
}

CharFigure* ConvAssetManager::GetFigure(char* name)
{
	return NULL;
}

uint8_t ConvAssetManager::GetFacePaletteID(const char* name)
{
	return this->facePalettes[name]->index;
}

void ConvAssetManager::ParseBGLayer(uint8_t* data, size_t layerID,ConvBackGround* back)
{
	ByteStream dataReader ;
	dataReader.Set(data + 5 * layerID);

	uint8_t type = dataReader.ReadByte();
	uint8_t shapeID = dataReader.ReadByte();
	uint8_t paletteID = dataReader.ReadByte();

	PakArchive* shapeArchive = NULL;
	PakArchive* paletteArchive = NULL;

	if (type == 0x00){
		// RLEShape is in CONVSHPS.PAK and Palette is in CONVPALS.PAK
		shapeArchive = & this->convShps;
		paletteArchive = &this->convPals;
	}

	if (type == 0x01){
		// RLEShape is in OPTSHPS.PAK and Palette is in OPTPALS.PAK
		shapeArchive = & this->optShps;
		paletteArchive = &this->optPals;
	}

	//Debug Display
	/*
		printf("\n%8s layer %lu :",back->name,layerID);
		for (size_t x=0; x < 5 ; x++) {
			printf("%3d ",*(data + 5 * layerID+x));
		}
	*/

	/*
	// The pack features some duplicate entries.
	while(convShapeArchive.GetEntry(shapeID)->size == 0)
		shapeID--;
	*/

	RLEShape* s = new RLEShape();

	const PakEntry& shapeEntry = shapeArchive->GetEntry(shapeID);
	PakArchive subPAK;
	subPAK.InitFromRAM("", shapeEntry);

	if (!subPAK.IsReady()){

		//Sometimes the image is not in a PAK but as raw data.
		Game.Log("Error on Pak %d for layer %d in loc %8s => Using dummy instead\n",shapeID,layerID,back->name);

		//Using an empty shape for now...
		*s = *RLEShape::GetEmptyShape();
		return;
	}
	else{
		s->Init(subPAK.GetEntry(0));
		if (s->GetHeight() < 199){                  //  If this is not a background, we need to move down
			Point2D pos = {0,CONV_TOP_BAR_HEIGHT+1};  //  to allow the black band on top of the screen
			s->SetPosition(&pos);
		}
	}

	back->layers.push_back(s);
	back->palettes.push_back(paletteArchive->GetEntry(paletteID).data);
}

void ConvAssetManager::ReadBackGrounds(const IffChunk* chunkRoot)
{
	for(size_t i = 0 ; i < chunkRoot->childs.size() ; i ++){
		IffChunk* chunk = chunkRoot->childs[i];
		if (chunk->id != IdToUInt("FORM")){
			Game.Log("ConvAssetManager::ReadBackGrounds => Unexpected chunk (%s).\n",chunk->GetName());
			Game.Terminate("Unable to build CONV database.\n");
		}

		IffChunk* info = chunk->childs[0];

		ConvBackGround* back = new ConvBackGround();

		//Get the name
		memset(back->name, 0, 9);
		memcpy(back->name, info->data, info->size);

		//Parse layers and associated bgs.
		size_t numLayers = chunk->childs[1]->size / 5 ; //A layer entry is 5 bytes wide
		for (size_t layerID=0; layerID < numLayers; layerID++)
			ParseBGLayer(chunk->childs[1]->data,layerID,back);

		this->backgrounds[back->name] = back;
		//Game.Log("  Able to reach shape in CONVSHPS.PAK entry %d from background '%s'.\n",shapeID,back->name);
	}
}

void ConvAssetManager::ReadFaces(const IffChunk* root)
{
	for(size_t i=0 ; i < root->childs.size() ; i ++){
		IffChunk* chunk = root->childs[i];
		ByteStream s(chunk->data);

		CharFace* face = new CharFace();
		memcpy(face->name, chunk->data,8);
		face->name[8] = '\0';

		s.MoveForward(8);

		uint8_t pakID = s.ReadByte();

		RSImageSet* imageSet = new RSImageSet();
		imageSet->InitFromRAM(convShps.GetEntry(pakID));
		face->appearances = imageSet;

		const auto& shapes = imageSet->GetShapes();
		for (size_t fid=0; fid < shapes.size(); fid++) {
			RLEShape*s = shapes[fid];
				Point2D pos = {0,CONV_TOP_BAR_HEIGHT+1}; //  to allow the black band on top of the screen
				s->SetPosition(&pos);
		}

		//printf("Face '%s' features %lu images.\n",face->name,imageSet->GetNumImages());

		this->faces[face->name] = face;
	}
}

//FIGR
void ConvAssetManager::ReadFigures(const IffChunk* root){
	for(size_t i=0 ; i < root->childs.size() ; i ++){
	}
}

//PFIG
void ConvAssetManager::ReadPFigures(const IffChunk* root)
{
}

//Face palettes FCPL
void ConvAssetManager::ReadFCPL(const IffChunk* root)
{
	for(size_t i=0 ; i < root->childs.size() ; i ++){
		//Game.Log("FCPL %lu: %s %2X\n",root->childs[i]->size,root->childs[i]->data,*(root->childs[i]->data+8));
		FacePalette* pal = new FacePalette();
		memcpy(pal->name, root->childs[i]->data, 8);
		pal->name[8] = '\0';
		pal->index = *(root->childs[i]->data+8);
		this->facePalettes[pal->name] = pal;
	}
}

//FGPL I have no idea what is in there.
void ConvAssetManager::ReadFGPL(const IffChunk* root)
{
}

void ConvAssetManager::BuildDB()
{
	auto& treGameFlow = Assets.tres[AssetManager::TRE_GAMEFLOW];

	//This is were the background shapes are stored.
	TreEntry* convShapEntry = treGameFlow.GetEntryByName(TRE_DATA_GAMEFLOW "CONVSHPS.PAK");
	convShps.InitFromRAM("CONVSHPS.PAK", *convShapEntry);
	//convShapeArchive.List(stdout);

	//This is were the palette patches are stored
	TreEntry* convPalettesEntry = treGameFlow.GetEntryByName(TRE_DATA_GAMEFLOW "CONVPALS.PAK");
	convPals.InitFromRAM("CONVPALS.PAK", *convPalettesEntry);
	//convPalettePak.List(stdout);

	//This is were the background shapes are stored.
	TreEntry* optShapEntry = treGameFlow.GetEntryByName(TRE_DATA_GAMEFLOW "OPTSHPS.PAK");
	optShps.InitFromRAM("OPTSHPS.PAK", *optShapEntry);
	//optShps(stdout);

	//This is were the palette patches are stored
	TreEntry* optPalettesEntry = treGameFlow.GetEntryByName(TRE_DATA_GAMEFLOW "OPTPALS.PAK");
	optPals.InitFromRAM("OPTPALS.PAK", *optPalettesEntry);
	//optPals(stdout);

	//Open the metadata
	TreEntry* convDataEntry = treGameFlow.GetEntryByName(TRE_DATA_GAMEFLOW "CONVDATA.IFF");
	IffLexer convDataLexer;
	convDataLexer.InitFromRAM(*convDataEntry);
	//convDataLexer.List(stdout);

	ReadBackGrounds(convDataLexer.GetChunkByID("BCKS"));
	ReadFaces(convDataLexer.GetChunkByID("FACE"));  //PAK id for Face image collection
	ReadFigures(convDataLexer.GetChunkByID("FIGR")); //PAK id for Figures image
	ReadPFigures(convDataLexer.GetChunkByID("PFIG")); // ??!? Maybe Palette figure ???!?!
	//I have no idea what is in there.
	ReadFCPL(convDataLexer.GetChunkByID("FCPL"));  //Face Conv Palette normal and night
	//I have no idea what is in there.
	ReadFGPL(convDataLexer.GetChunkByID("FGPL"));  //Face Game palette normal
}
