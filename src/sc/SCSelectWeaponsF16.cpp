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

void SCSelectWeaponF16::Init()
{
	_font = FontManager.GetFont("");

	//const char* pakPath = "..\\..\\DATA\\MIDGAMES\\RSOUNDFX.PAK";
	const char* pakPath = "..\\..\\DATA\\MIDGAMES\\MID1VOC.PAK";
	auto& treGameFlow = Assets.tres[AssetManager::TRE_GAMEFLOW];
	auto pak = GetPak(pakPath, *treGameFlow.GetEntryByName(pakPath));
	for (int i = 0; i < pak->GetNumEntries(); ++i) {
		const PakEntry& pe = pak->GetEntry(i);
		printf("==== %d ====\n", pe.size);
		RSVocSound snd;
		snd.InitFromRAM(pe);
	}

	wantedBg = OptHangarTruck;
	wantedBg.am = AnimMode::Cutscene;
}

void SCSelectWeaponF16::RunFrame(const FrameParams& p)
{
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

	FrameParams np = p;
	np.currentTime -= startTime;
	Frame2D(np, shapes, [&] {
		VGA.PrintText(_font, { 10, 10 }, uint8_t(p.currentTime * 40), 3, 5, "pal:%d - shp:%d - ofs:%d", currentBg.pal, currentBg.shp, colOfs);
	});
}
