//
//  RSSound.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <cstdint>

#include "ByteSlice.h"

class RSVocSoundData
{
public:
	struct SoundData
	{
		uint32_t sampelRate{};
		uint32_t sz{};
		const uint8_t* data{};;
	};
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
	const SoundData& Data() const { return data; }
protected:
	SoundData data;
};

class RSSoundInstance
{
public:
	RSVocSoundData* data{};
	double startTime{};
};
