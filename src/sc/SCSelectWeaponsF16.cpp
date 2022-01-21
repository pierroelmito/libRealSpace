//
//  SCSelectWeaponsF16.cpp
//  libRealSpace
//
//  Created by fabien sanglard on 1/26/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "SCSelectWeaponsF16.h"

#include "precomp.h"

#include "SCStrike.h"

SCSelectWeaponF16::SCSelectWeaponF16()
{
}

SCSelectWeaponF16::~SCSelectWeaponF16()
{
}

#if 0
class RSVocSound
{
public:
	struct BlockType {
		enum Type {
			Terminator,
			SoundData,
			SoundContinue,
			Silence,
			Marker,
			ASCII,
			Repeat,
			EndRepeat,
			Extended
		};
	};
	bool InitFromRAM(const ByteSlice& bs);
protected:
	struct SoundData
	{
		uint32_t sampelRate{};
		uint32_t sz{};
		const uint8_t* data{};;
	};
	SoundData data;
};

bool RSVocSound::InitFromRAM(const ByteSlice& bs)
{
	ByteStream stream(bs.data);
	const auto header = stream.ReadBytes<0x13>();
	const uint8_t b0 = stream.ReadByte();
	const uint16_t sz = stream.ReadShort();
	const uint16_t version0 = stream.ReadShort();
	const uint16_t version1 = stream.ReadShort();

	if (sz != 26)
		return false;

	printf("VOC:\n");

	while (true) {
		const uint8_t bt = stream.ReadByte();
		if (bt == BlockType::Terminator)
			break;

		const auto bsz = stream.ReadBytes<3>();
		const uint32_t szBlock = (bsz[2] << 16) | (bsz[1] << 8) | (bsz[0] << 0);

		printf("\t[%d] : %d bytes\n", bt, szBlock);

		switch (bt) {
		case BlockType::SoundData:
			{
				const uint8_t bsampleRate = stream.Cursor()[0];
				const uint8_t compType = stream.Cursor()[1];
				const uint32_t sampleRate = 256 - (1000000 / bsampleRate);
				data = {
					sampleRate,
					szBlock - 2,
					stream.Cursor() + 2
				};
			}
			break;
		/*
		case BlockType::SoundContinue:
			break;
		case BlockType::Silence:
			break;
		case BlockType::Marker:
			break;
		case BlockType::ASCII:
			break;
		case BlockType::Repeat:
			break;
		case BlockType::EndRepeat:
			break;
		*/
		default:
			return false;
		}

		stream.MoveForward(szBlock);
	}

	printf("ok\n");

	return true;
}
#endif

void SCSelectWeaponF16::Init()
{
	_font = FontManager.GetFont("");

	wantedBg = OptHangarTruck;
	wantedBg.am = AnimMode::Cutscene;

	//auto& tre = Assets.tres[AssetManager::TRE_SOUND];
	//tre.Decompress("tre_sound/");

#if 0
	const std::vector<std::string> items = {
	#if 1
		"..\\..\\DATA\\GAMEFLOW\\CONV.PAK",
		"..\\..\\DATA\\GAMEFLOW\\CONVPALS.PAK",
		"..\\..\\DATA\\GAMEFLOW\\CONVSHPS.PAK",
		"..\\..\\DATA\\GAMEFLOW\\MAINMENU.PAK",
		"..\\..\\DATA\\GAMEFLOW\\OBJVIEW.PAK",
		"..\\..\\DATA\\GAMEFLOW\\OPTPALS.PAK",
		"..\\..\\DATA\\GAMEFLOW\\OPTSHPS.PAK",
		"..\\..\\DATA\\MIDGAMES\AMUSIC.PAK",
		"..\\..\\DATA\\MIDGAMES\\ASOUNDFX.PAK",
		"..\\..\\DATA\\MIDGAMES\\MID1.PAK",
		"..\\..\\DATA\\MIDGAMES\\MID12.PAK",
		"..\\..\\DATA\\MIDGAMES\\MID14.PAK",
		"..\\..\\DATA\\MIDGAMES\\MID15.PAK",
		"..\\..\\DATA\\MIDGAMES\\MID16.PAK",
		"..\\..\\DATA\\MIDGAMES\\MID17.PAK",
		"..\\..\\DATA\\MIDGAMES\\MID1VOC.PAK",
		"..\\..\\DATA\\MIDGAMES\\MID2.PAK",
		"..\\..\\DATA\\MIDGAMES\\MID20.PAK",
		"..\\..\\DATA\\MIDGAMES\\MID3.PAK",
		"..\\..\\DATA\\MIDGAMES\\MID36.PAK",
		"..\\..\\DATA\\MIDGAMES\\MID5.PAK",
		"..\\..\\DATA\\MIDGAMES\\MIDGAMES.PAK",
		"..\\..\\DATA\\MIDGAMES\\RMUSIC.PAK",
		"..\\..\\DATA\\MIDGAMES\\RSOUNDFX.PAK",
	#endif
	#if 0
		//"..\\..\\DATA\\FONTS\\NEWBUTNS.SHP",
		//"..\\..\\DATA\\FONTS\\NODISK3.SHP",
		  "..\\..\\DATA\\MIDGAMES\\MMUSIC.PAK",
		//"..\\..\\DATA\\OBJECTS\\F-16DESH.IFF",
		//"..\\..\\DATA\\SOUND\\ADLIB.DRV",
		//"..\\..\\DATA\\SOUND\\COMBAT.ADL",
		//"..\\..\\DATA\\SOUND\\COMBAT.DAT",
		//"..\\..\\DATA\\SOUND\\COMBAT.MID",
		//"..\\..\\DATA\\SOUND\\COMBAT.ROL",
		  "..\\..\\DATA\\SOUND\\DSOUNDFX.PAK",
		//"..\\..\\DATA\\SOUND\\GAMEFLOW.ADL",
		//"..\\..\\DATA\\SOUND\\GAMEFLOW.DAT",
		//"..\\..\\DATA\\SOUND\\GAMEFLOW.MID",
		//"..\\..\\DATA\\SOUND\\GAMEFLOW.ROL",
		//"..\\..\\DATA\\SOUND\\MIDI.DRV",
		//"..\\..\\DATA\\SOUND\\PAS.DRV",
		//"..\\..\\DATA\\SOUND\\ROLAND.DRV",
		//"..\\..\\DATA\\SOUND\\SB.DRV",
		//"..\\..\\DATA\\SOUND\\SOUNDFX.ADL",
		//"..\\..\\DATA\\SOUND\\SOUNDFX.ROL",
		//"..\\..\\DATA\\SOUND\\STRIKE.AD",
		//"..\\..\\DATA\\SOUND\\STRIKE.MT",
	#endif
	};

	auto& treGameFlow = Assets.tres[AssetManager::TRE_GAMEFLOW];
	for (const auto& i : items) {
		auto* entry = treGameFlow.GetEntryByName(i.c_str());
		if (entry) {
			auto pak = GetPak(i.c_str(), *entry);
		}
	}
#endif
}

void SCSelectWeaponF16::RunFrame(const FrameParams& p)
{
	if (soundIndex != soundWanted) {
		soundIndex = soundWanted;
		testSound = {};
		//const char* pakPath = "..\\..\\DATA\\MIDGAMES\\RSOUNDFX.PAK";
		//const char* pakPath = "..\\..\\DATA\\MIDGAMES\\MID1VOC.PAK";
		//const char* pakPath = "..\\..\\DATA\\MIDGAMES\\MID1.PAK";
		//const char* pakPath = "..\\..\\DATA\\MIDGAMES\\MID1VOC.PAK";
		//auto& treGameFlow = Assets.tres[AssetManager::TRE_GAMEFLOW];
		//const char* pakPath = "..\\..\\DATA\\SOUND\\DSOUNDFX.PAK";
		//auto& treGameFlow = Assets.tres[AssetManager::TRE_SOUND];
		const char* pakPath = MID2VOC;
		auto& treGameFlow = Assets.tres[AssetManager::TRE_MGSPEECH];
		auto* entry = treGameFlow.GetEntryByName(pakPath);
		if (entry) {
			auto pak = GetPak(pakPath, *entry);
			testSound.InitFromRAM(pak->GetEntry(std::min(pak->GetNumEntries() - 1, soundIndex)));
		}
	}

	static int played = 0;
	if (testSound.Data().data != nullptr) {
		Audio.Update([&] (std::vector<float>& buffer) {
			for (float& f : buffer) {
				const uint8_t s = testSound.Data().data[((played++) / 4) % testSound.Data().sz];
				f = (s / 255.0f) - 0.5f;
			}
		});
	}

	if (wantedBg != currentBg) {
		currentBg = wantedBg;
		printf("pal : %d / bg : %d\n", currentBg.pal, currentBg.shp);
		for (int i = 0; i < 256; ++i)
			palette.SetColor(i, { 255u, 0, 255u, 255u });
		startTime = p.currentTime;
		InitShapes({ currentBg });
	}

	if (!shapes.empty()) {
		for (auto& s : shapes[0].frames)
			s->SetColorOffset(uint8_t(colOfs));
	}

	if (p.pressed.contains(GLFW_KEY_ENTER)) {
		VGA.ShowPalette() = false;
		Stop();
		Game.MakeActivity<SCStrike>();
	}

	if (p.pressed.contains(GLFW_KEY_F1))
		VGA.ShowPalette() = !VGA.ShowPalette();
	if (p.pressed.contains(GLFW_KEY_A))
		wantedBg.shp+= 1;
	if (p.pressed.contains(GLFW_KEY_Q))
		wantedBg.shp = std::max(0, wantedBg.shp - 1);
	if (p.pressed.contains(GLFW_KEY_W))
		wantedBg.pal += 1;
	if (p.pressed.contains(GLFW_KEY_S))
		wantedBg.pal = std::max(0, wantedBg.pal - 1);
	if (p.pressed.contains(GLFW_KEY_E))
		colOfs += 1;
	if (p.pressed.contains(GLFW_KEY_D))
		colOfs = std::max(0, colOfs - 1);
	if (p.pressed.contains(GLFW_KEY_R))
		soundWanted += 1;
	if (p.pressed.contains(GLFW_KEY_F))
		soundWanted = soundWanted == 0 ? soundWanted : soundWanted - 1;

	FrameParams np = p;
	np.currentTime -= startTime;
	Frame2D(np, shapes, [&] {
		VGA.PrintText(_font, { 10, 10 }, uint8_t(p.currentTime * 40), 3, 5, "pal:%d - shp:%d - ofs:%d - sound: %d", currentBg.pal, currentBg.shp, colOfs, soundIndex);
	});
}
