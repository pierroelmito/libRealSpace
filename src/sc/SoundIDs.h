
#pragma once

#include <cstring>

struct SoundID {
	const char* pak{};
	int index{};
	bool operator !=(const SoundID& o) {
		if (o.index != index)
			return true;
		if (strcmp(o.pak, pak) != 0)
			return true;
		return false;
	}
};

#define MID1VOC "..\\..\\DATA\\MIDGAMES\\MID1VOC.PAK"

constexpr SoundID SndYolo = { MID1VOC, 0 };
