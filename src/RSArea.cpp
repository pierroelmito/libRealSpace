//
//  RSMap.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

#include <cassert>

#include "AssetManager.h"

RSArea::RSArea()
{
}

RSArea::~RSArea()
{
	delete archive;
}

void RSArea::ParseMetadata()
{
	//Meta are in the first PAK file.

	printf("Parsing file[0] (Metadatas)\n");

	const PakEntry& entry = archive->GetEntry(0);

	IffLexer lexer;
	lexer.InitFromRAM(entry);
	lexer.List(stdout);

	IffChunk* tera = lexer.GetChunkByID("TERA");
	if (tera == NULL) {
		printf("Error while parsing Metadata: Cannot find a TERA chunk in first file: Is this really an AREA PAK ?!?!?\n");
		return;
	}

	//Parse BLOX containing the Elevation, Triangles and OBJS.
	IffChunk* blox = lexer.GetChunkByID("BLOX");
	if (blox == NULL) {
		printf("Error while parsing Metadata: Cannot find a BLOX chunk.\n");
		return;
	}

	/*
	 The content of the ELEV chunk is the same across ALL MAPS
	 Only RHODEI.ELV    RHODEI.AVG
		  RHODEI.MED    RHODEI.AVG
		  RHODEI.LOW    RHODEI.AVG

	 At the end change. RHODEI / CANYON / QUEBEC / MAURITAN etc.....
	 */

	//Elevation format entry is 46 bytes long:

	// 2 bytes 0F 00
	// 2 bytes
	// 4 bytes 08 00 00 00
	// 2 bytes
	// 4 bytes 00 00 12 00
	// 1 byte
	// 4 bytes 00 20 4e 00 00

	//13 bytes: A filename
	//13 bytes: An other filename

	IffChunk* elev = lexer.GetChunkByID("ELEV");
	printf("Content of elevation chunk:\n");
	size_t numEleRecords = elev->size / 46;
	ByteStream elevStream(elev->data);
	for(size_t e=0 ; e < numEleRecords ; e++)
	{

		printf("elev record [%zu] ",e);
		uint8_t unknownsElev[20];
		for(int i=0; i < 20 ; i++)
			unknownsElev[i] = elevStream.ReadByte();

		char elevName[14];
		for(int i=0; i < 13 ; i++)
			elevName[i] = elevStream.ReadByte();
		elevName[13]  = 0;

		char elevOtherName[14];
		for(int i=0; i < 13 ; i++)
			elevOtherName[i] = elevStream.ReadByte();
		elevOtherName[13]  = 0;

		for (int i=0; i<20 ; i++){
			printf("%2X ",unknownsElev[i]);
		}

		printf("%-13s %-13s \n",elevName,elevOtherName);
	}

	IffChunk* atri = lexer.GetChunkByID("ATRI");
	printf("Content of trigo chunk:\n");

	ByteStream triStream(atri->data);
	for (int i=0; i < 40; i++) {
		printf(" %2X",triStream.ReadByte());
	}
	char triFileName[13];
	for (int i=0; i < 13; i++)
		triFileName[i] = triStream.ReadByte();
	printf(" '%-13s' \n",triFileName);

	//IffChunk* objs = lexer.GetChunkByID("OBJS");
	/*
	 OBJS format:
	  4 bytes unknown :
	  4 bytes unknown :
	  4 bytes unknown :
	  14 bytes OBJ file name
	 */

	IffChunk* txms = lexer.GetChunkByID("TXMS");
	if (txms == NULL) {
		printf("Error while parsing Metadata: Cannot find a TXMS chunk.\n");
		return;
	}

	IffChunk* txmsInfo = txms->children[0];
	if (txmsInfo->id != IdToUInt("INFO")) {
		printf("Error: First child in TXMS is not an INFO chunk ?!\n");
		return;
	}

	IffChunk* txmsMaps = txms->children[1];
	if (txmsMaps->id != IdToUInt("MAPS")) {
		printf("Error: Second child in TXMS is not an MAP chunk ?!\n");
		return;
	}

	//Num texture sets
	size_t numTexturesSets = txmsMaps->size/12;
	//printf("This area features %lu textureSets references.\n",numTexturesSets);

	ByteStream textureRefStrean(txmsMaps->data);

	for (size_t i=0; i < numTexturesSets ; i++) {
		/*uint16_t fastID =*/ textureRefStrean.ReadUShort();
		char setName[8];
		for(int n=0; n < 8 ; n++){
			setName[n] = textureRefStrean.ReadByte();
		}
		/*uint8_t unknown =*/ textureRefStrean.ReadByte();
		/*uint8_t numImages =*/ textureRefStrean.ReadByte();

	   // printf("Texture Set Ref [%3lu] 0x%2X[%-8s] %02X (%2u files).\n",i,fastID,setName,unknown,numImages);
	}

	/*
		TXMS format:
			One INFO chunk
			X MAPS textures entries
					2 byte  : fast lookup ID (from TXM pack);
					8 bytes : Set Name (from TXM pack);
					1 byte  : unknown
					1 byte  : num textures in that set

	 */
}

#define OBJ_ENTRY_SIZE 0x46
#define OBJ_ENTRY_NUM_OBJECTS_FIELD 0x2
void RSArea::ParseObjects()
{
	printf("Parsing file[5] (Objects)\n");

	/*
	 The OBJ file seems to have a pattern:
	 It is a PAK archive
	 For each entry in the PAK
		short: num records
			X records of length 0x46
			Record format :
				7 bytes for name
				1 bytes unknown (sometimes 0x00 sometimes 0xC3
				4 bytes unknown
				12 bytes for coordinates ?
	*/

	const PakEntry& objectsFilesLocation = archive->GetEntry(5);

	PakArchive objectFiles;
	objectFiles.InitFromRAM("PAK Objects from RAM",objectsFilesLocation);

	printf("This .OBJ features %lu entries.\n",objectFiles.GetNumEntries());

	for(size_t i = 0 ; i < objectFiles.GetNumEntries() ; i++){
		const PakEntry& entry = objectFiles.GetEntry(i);

		if (entry.size == 0)
			continue;

		ByteStream sizeGetter(entry.data);
		uint16_t numObjs = sizeGetter.ReadUShort();
		printf("OBJ files %lu features %d objects.\n",i,numObjs);

		//if (i != 97)
		//    continue;

		for(int j = 0 ; j < numObjs; j++) {
			ByteStream reader(entry.data + OBJ_ENTRY_NUM_OBJECTS_FIELD + OBJ_ENTRY_SIZE * j);

			MapObject mapObject;

			for(int k=0 ; k <8 ; k++)
				mapObject.name[k] = reader.ReadByte();
			mapObject.name[8] = 0;

			const uint8_t unknown09 = reader.ReadByte();
			const uint8_t unknown10 = reader.ReadByte();
			const uint8_t unknown11 = reader.ReadByte();
			const uint8_t unknown12 = reader.ReadByte();
			const uint8_t unknown13 = reader.ReadByte();

			for(int k=0 ; k <8 ; k++)
				mapObject.destroyedName[k] = reader.ReadByte();
			mapObject.destroyedName[8] = 0;

			// decoding is incorrect...
			const auto Read32bitPos = [&reader] (const char* lbl) {
#if 1
				int32_t v[4]{};
				v[0] = reader.ReadByte();
				v[1] = reader.ReadByte();
				v[2] = reader.ReadByte();
				v[3] = reader.ReadByte();
				int32_t r = (v[2] << 8) | (v[3] << 0);
				printf("%s: %02X %02X %02X %02X -> %d\n", lbl, v[0], v[1], v[2], v[3], r);
				return float(r) / float(1 << 8);
#else
				int32_t r = reader.ReadInt32BE();
				float f = r / float(1 << 16);
				return f;
#endif
			};

			// work for 0, 1, -1 but is probably wrong for other values :-/
			const auto Read32bitTransform = [&reader] (const char* lbl) {
				int32_t v[4]{};
				v[0] = reader.ReadByte();
				v[1] = reader.ReadByte();
				v[2] = reader.ReadByte();
				v[3] = reader.ReadByte();
				assert(v[0] == 0);
				int32_t r = (v[3] << 16) | (v[2] << 8) | (v[1]);
				if ((r & (1 << 23)) != 0) {
					r = -((~(r | 0xff000000)) + 1);
				}
				//printf("%s: %02X %02X %02X %02X -> %d\n", lbl, v[0], v[1], v[2], v[3], r);
				return float(r);
			};

			// read translate
			const auto p0 = Read32bitPos("p0");
			const auto p1 = Read32bitPos("p1");
			const auto p2 = Read32bitPos("p2");
			mapObject.position[0] = p1;
			mapObject.position[1] = p2;
			mapObject.position[2] = p0;

			// zero separator??
			const int count = 1;
			uint8_t unknowns[count];
			for(int k=0 ; k <count; k++) {
				unknowns[k] = reader.ReadByte();
				assert(unknowns[k] == 0);
			}

			// looks like a 3x3 transform matrix
			float transform[3][3]{};
			for (int k = 0; k < 3; ++k) {
				for (int l = 0; l < 3; ++l) {
					char buffer[4] = "tXY";
					buffer[1] = '0' + k;
					buffer[2] = '0' + l;
					transform[k][l] = Read32bitTransform(buffer);
				}
			}

#if 1
			// swap axis
			mapObject.transform[0][0] = transform[0][0];
			mapObject.transform[0][1] = transform[0][2];
			mapObject.transform[0][2] = transform[0][1];
			mapObject.transform[1][0] = transform[2][0];
			mapObject.transform[1][1] = transform[2][2];
			mapObject.transform[1][2] = transform[2][1];
			mapObject.transform[2][0] = transform[1][0];
			mapObject.transform[2][1] = transform[1][2];
			mapObject.transform[2][2] = transform[1][1];
#else
			for (int k = 0; k < 3; ++k) {
				for (int l = 0; l < 3; ++l) {
					mapObject.transform[k][l] = transform[k][l];
				}
			}
#endif

			printf(
				"object set [%3lu] obj [%2d] - '%-8s' %02X %02X %02X %02X %02X '%-8s'\n",
				i,
				j,
				mapObject.name,
				unknown09,
				unknown10,
				unknown11,
				unknown12,
				unknown13,
				mapObject.destroyedName
			);

			objects[i].push_back(mapObject);
		}
	}
}

#if USE_SHADER_PIPELINE != 1

void RSArea::ParseTriFile(const PakEntry* entry)
{
	const auto readCoord = [] (int32_t coo) -> float {
		return (coo>>8) + (coo&0x000000FF)/255.0;
	};

	RSVector3* vertices = new RSVector3[300];

	ByteStream stream(entry->data);
	/*const auto v0 =*/ stream.ReadInt32LE();
	/*const auto v1 =*/ stream.ReadInt32LE();
	const float scale = 1.0f / 2000.0f;
	for (int i=0 ; i < 300; i++) {
		const float x = scale * readCoord(stream.ReadInt32LE());
		const float z = scale * readCoord(stream.ReadInt32LE());
		const float y = scale * readCoord(stream.ReadInt32LE());
		vertices[i] = { x, y, z };
	}

	//Render them
	Renderer.RenderVerticeField(vertices, 300);

	delete[] vertices;
}

void RSArea::ParseTrigo()
{
	Renderer.Init(2);

	const PakEntry& entry = archive->GetEntry(4);

	printf(".TRI file is %lu bytes.\n",entry.size);
	// .TRI is a PAK
	PakArchive triFiles;
	triFiles.InitFromRAM(".TRI",entry);
	triFiles.List(stdout);
	//triFiles.Decompress("/Users/fabiensanglard/Desktop/MAURITAN.TRIS/","TRI");

	printf("Found %zu .TRI files.\n",triFiles.GetNumEntries());

	for(size_t i=0 ; i < triFiles.GetNumEntries() ; i++) {
		const PakEntry& entry  = triFiles.GetEntry(i);
		if (entry.size != 0)
			ParseTriFile(&entry);
	}
}

#endif

enum LAND_TYPE {
	LAND_TYPE_SEA,
	LAND_TYPE_DESERT,
	LAND_TYPE_GROUND,
	LAND_TYPE_SAVANNAH,
	LAND_TYPE_TAIGA,
	LAND_TYPE_TUNDRA,
	LAND_TYPE_SNOW
};

#define BYTETOBINARYPATTERN "%d%d%d%d%d%d%d%d"
#define BYTETOBINARY(byte)  \
(byte & 0x80 ? 1 : 0), \
(byte & 0x40 ? 1 : 0), \
(byte & 0x20 ? 1 : 0), \
(byte & 0x10 ? 1 : 0), \
(byte & 0x08 ? 1 : 0), \
(byte & 0x04 ? 1 : 0), \
(byte & 0x02 ? 1 : 0), \
(byte & 0x01 ? 1 : 0)


//A lod features
//A block features either 25, 100 or 400 vertex
void RSArea::ParseBlocks(size_t lod,const PakEntry* entry, size_t blockDim)
{
	const auto typeToPal = [] (uint8_t t) -> uint8_t {
		// Hardcoding the values since I have no idea where those
		// are coming from. Maybe it was hard-coded in STRIKE.EXE ?
		switch (t) {
			case LAND_TYPE_SEA:
				return 0xA;
			case LAND_TYPE_DESERT:
				return 0x3;
			case LAND_TYPE_GROUND:
				return 0x7;
			case LAND_TYPE_SAVANNAH:
				return 0x5;
			case LAND_TYPE_TAIGA:
				return 0x9;
			case LAND_TYPE_TUNDRA:
				return 0x1;
			case LAND_TYPE_SNOW:
				return 0xC;
			default:
				printf("No color for type %d\n", t);
		}
		return 0;
	};

	PakArchive blocksPAL;
	blocksPAL.InitFromRAM("BLOCKS", *entry);

	const int bcount = BLOCK_PER_MAP_SIDE;
	for (size_t i = 0; i < blocksPAL.GetNumEntries(); i++) { // Iterate over the BLOCKS_PER_MAP block entries.
		const int bx = i % bcount;
		const int by = i / bcount;

		//SRC Asset Block
		const PakEntry& blockEntry = blocksPAL.GetEntry(i);

		//DST custom block
		AreaBlock* block = &blocks[lod][i];

		block->sideSize = static_cast<int32_t>(blockDim);

		ByteStream vertStream(blockEntry.data);
		for(size_t vertexID = 0; vertexID < blockDim*blockDim; vertexID++) {
			const int vx = vertexID % blockDim;
			const int vy = vertexID / blockDim;

			MapVertex* vertex = &block->vertice[vertexID];

			const int16_t height = vertStream.ReadShort();

			vertex->flag = vertStream.ReadByte();
			vertex->type = vertStream.ReadByte();


			uint8_t shade = (vertex->flag & 0x0F);
			shade = shade >> 1;
			/*
			if (shade & 1)
				shade = 0;
			*/

			int16_t unknown = (vertex->flag & 0xF0);
			unknown = unknown >> 8;

			vertex->upperImageID = vertStream.ReadByte();
			vertex->lowerImageID = vertStream.ReadByte();

			/*
			//City block
			if (blockDim == 20 && i==97){
				printf("%2X (%2X) ",vertex->lowerImageID,vertex->upperImageID );

				if (vertexID % 20 == 19)
					printf("\n");
			}
			*/


			//Texel* t = renderer.GetDefaultPalette()->GetRGBColor(vertex->text);
			/*
			  TODO: Figure out what are:
					- flag high 4 bits.
					- flag low  1 bits.
					- textSet
					- text
			*/

			const float offset = 0.0f;
			const float rx = offset + (1.0f - 2.0f * offset) * float(vx) / (float)(blockDim);
			const float ry = offset + (1.0f - 2.0f * offset) * float(vy) / (float)(blockDim);
			const float bsz = BLOCK_WIDTH;

			vertex->v.X = (bx + rx)* bsz;
			vertex->v.Y = (float)height / (float)HEIGHT_DIVIDER; //-vertex->text * 10;//height ;
			vertex->v.Z = (by + ry) * bsz;

			// need to compute normals
			vertex->n = { 0, -1, 0 };

			const uint8_t paletteColor = 16 * typeToPal(vertex->type);
			Texel* t = Renderer.GetPalette().GetRGBColor(paletteColor + shade);
			vertex->color[0] = t->r/255.0f;//*1-(vertex->z/(float)(BLOCK_WIDTH*blockDim))/2;
			vertex->color[1] = t->g/255.0f;;//*1-(vertex->z/(float)(BLOCK_WIDTH*blockDim))/2;
			vertex->color[2] = t->b/255.0f;;//*1-(vertex->z/(float)(BLOCK_WIDTH*blockDim))/2;
			vertex->color[3] = paletteColor / 255.0f;
		}
	}

	// compute normal
	for (size_t i = 0; i < blocksPAL.GetNumEntries(); i++) {
		const int bx = i % bcount;
		const int by = i / bcount;

		AreaBlock* const block00 = &blocks[lod][i];
		AreaBlock* const block10 = bx < bcount - 1 ? &blocks[lod][i + 1] : block00;
		AreaBlock* const block01 = by < bcount - 1 ? &blocks[lod][i + bcount] : block00;
		AreaBlock* const block11 = bx < bcount - 1 && by < bcount - 1 ? &blocks[lod][i + bcount + 1] : block00;

		for(size_t vertexID = 0; vertexID < blockDim * blockDim; vertexID++) {
			const int vx = vertexID % blockDim;
			const int vy = vertexID / blockDim;

			MapVertex* const vertex00 = &block00->vertice[vertexID];
			MapVertex* const vertex10 = vx < blockDim - 1 ? &block00->vertice[vertexID + 1] : &block10->vertice[vy * blockDim];
			MapVertex* const vertex01 = vy < blockDim - 1 ? &block00->vertice[vertexID + blockDim] : &block01->vertice[vx];
			MapVertex* const vertex11 = vx < blockDim - 1 && vy < blockDim - 1 ? &block00->vertice[vertexID + blockDim + 1] : &block11->vertice[0];

			const RSVector3 v0 = vertex11->v - vertex00->v;
			const RSVector3 v1 = vertex01->v - vertex10->v;

			vertex00->n = HMM_Normalize(HMM_Cross(v0, v1));
		}
	}
}

void RSArea::ParseElevations()
{
	const PakEntry& entry = archive->GetEntry(6);
	ByteStream stream(entry.data);
	for (size_t i = 0 ; i < BLOCKS_PER_MAP; i++) {
		elevation[i] = stream.ReadUShort() ;
	}
}

void RSArea::ParseHeightMap(void)
{
	//char title[512];

	const PakEntry& entry0 = archive->GetEntry(1);
	PakArchive fullPak;
	fullPak.InitFromRAM("FULLSIZE", entry0);
	// fullPak.List(stdout);
	ParseBlocks(BLOCK_LOD_MAX,&entry0,20);

	//renderer.RenderWorldPoints(this,BLOCK_LOD_MAX,400);

	const PakEntry& entry1 = archive->GetEntry(2);
	PakArchive medPak;
	medPak.InitFromRAM("MED SIZE",entry1);
	// medPak.List(stdout);
	ParseBlocks(BLOCK_LOD_MED,&entry1,10);

	//renderer.RenderWorldSolid(this,BLOCK_LOD_MED,100);

	const PakEntry& entry2 = archive->GetEntry(3);
	PakArchive smallPak;
	smallPak.InitFromRAM("SMALSIZE",entry2);
	// smallPak.List(stdout);
	ParseBlocks(BLOCK_LOD_MIN,&entry2,5);

	//renderer.RenderWorldSolid(this,BLOCK_LOD_MIN,25);
}

RSImage* RSArea::GetImageByID(size_t ID) const
{
	return textures[0]->GetImageById(ID);
}

void RSArea::AddJet(TreArchive& tre, const char* name, RSQuaternion* orientation, RSVector3* position)
{
	TreEntry* jetEntry = tre.GetEntryByName(name);
	RSEntity* entity = new RSEntity();
	IffLexer lexer;
	lexer.InitFromRAM(*jetEntry);
	entity->InitFromIFF(&lexer);

	entity->orientation = *orientation;
	entity->position = *position;

	jets.emplace_back(entity);
}

void RSArea::AddJets(TreArchive& treObjects)
{
	const float angle = 25.0f;
	const float mul = 1.0f;

	RSQuaternion rot0 = HMM_Mat4ToQuaternion(HMM_Rotate(angle, { 1, 0, 0 }));
	RSVector3 pos0 = { mul * 4016, mul * 95, mul * 2980};
	AddJet(treObjects, TRE_DATA_OBJECTS "F-16DES.IFF", &rot0, &pos0);

	RSQuaternion rot1 = HMM_Mat4ToQuaternion(HMM_Rotate(-angle, { 1, 0, 0 }));
	RSVector3 pos1 = { mul * 4010, mul * 95, mul * 2980};
	AddJet(treObjects, TRE_DATA_OBJECTS "F-22.IFF", &rot1, &pos1);

	//pos = {3886,300,2886};
	//AddJet(&tre,TRE_DATA_GAMEFLOW "MIG29.IFF",&rot,&pos);

	//const char* jetPath = TRE_DATA_GAMEFLOW "F-22.IFF";
	//const char* jetPath = TRE_DATA_GAMEFLOW "F-15.IFF";
	//const char* jetPath = TRE_DATA_GAMEFLOW "YF23.IFF";
	//const char* jetPath = TRE_DATA_GAMEFLOW "MIG21.IFF";
	//const char* jetPath = TRE_DATA_GAMEFLOW "MIG29.IFF";

	for(int id = 0; id < BLOCKS_PER_MAP; id++) {
		std::vector<MapObject>& blockObjects = objects[id];
		for (MapObject& object : blockObjects) {
			if (object.entity == nullptr) {
				auto& e = entities[object.name];
				if (e == nullptr) {
					e = std::make_unique<RSEntity>();
					char buffer[512];
					snprintf(buffer, sizeof(buffer), "%s%s.IFF", TRE_DATA_OBJECTS, object.name);
					TreEntry* entry = treObjects.GetEntryByName(buffer);
					IffLexer lexer;
					lexer.InitFromRAM(*entry);
					e->InitFromIFF(&lexer);
				}
				if (e == nullptr) {
					printf("Unable to load area object '%s'\n", object.name);
				}
				object.entity = e.get();
			}
		}
	}
}

void RSArea::InitFromPAKFileName(const char* pakFilename, TreArchive& treObjects, TreArchive& treTextures)
{
	strcpy(name,pakFilename);

	//Check the PAK has 5 entries
	this->archive = new PakArchive();
	this->archive->InitFromFile(pakFilename);

	//Check that we have 6 entries.
	if (archive->GetNumEntries() != 7){
		printf("***Error: An area PAK file should have 7 files:\n");
		printf("        - IFF file.\n");
		printf("        - 1 file containing records of size 2400.\n");
		printf("        - 1 file containing records of size 600 (mipmaps of the 2400 records ?)\n");
		printf("        - 1  file containing records of size 150 (mipmaps of the 600  records ?)\n");
		printf("        - 1 file containing the map 3D data (MAURITAN.TRI).\n");
		printf("        - 1 file containing the objects locations on the map (MAURITAN.OBJ).\n");      //COMPLETELY REVERSE ENGINEERED !!!!
		printf("        - 1 file containing MAURITAN.AVG (I assume Average of something ?)\n");
		return;
	}

	//Load the textures from the PAKs (TXMPACK.PAK and ACCPACK.PAK) within TEXTURES.TRE.
	/*
		  Note: This is the bruteforce approach and not very good:
				I feel like the right way would be to be able to parse the AREA info
				(the first IFF entry which seems to contain textures references.)
				And load only those from the TXM PACK. This is probably how they did
				it on a machine with only 4MB of RAM.

	*/

	const char* pakNames[2] = {
		TRE_DATA_TXM "TXMPACK.PAK",
		TRE_DATA_TXM "ACCPACK.PAK",
	};
	for (int i = 0; i < 2; ++i) {
		const char* txmPakName = TRE_DATA_TXM "TXMPACK.PAK";
		TreEntry* treEntry = treTextures.GetEntryByName(txmPakName);
		PakArchive txmPakArchive;
		txmPakArchive.InitFromRAM(txmPakName,*treEntry);
		auto set = std::make_unique<RSMapTextureSet>();
		set->InitFromPAK(&txmPakArchive);
		textures.push_back(std::move(set));
	}

	//Parse the meta datas.
	ParseElevations();
	ParseMetadata();
	ParseObjects();
	// ParseTrigo();

	ParseHeightMap();

	AddJets(treObjects);
}
