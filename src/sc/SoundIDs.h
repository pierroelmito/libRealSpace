
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
#define MID16VOC "..\\..\\DATA\\MIDGAMES\\MID16VOC.PAK"
#define MID17VOC "..\\..\\DATA\\MIDGAMES\\MID17VOC.PAK"
#define MID20VOC "..\\..\\DATA\\MIDGAMES\\MID20VOC.PAK"
#define MID2VOC "..\\..\\DATA\\MIDGAMES\\MID2VOC.PAK"
#define MID33VOC "..\\..\\DATA\\MIDGAMES\\MID33VOC.PAK"
#define MID34VOC "..\\..\\DATA\\MIDGAMES\\MID34VOC.PAK"
#define MID36VOC "..\\..\\DATA\\MIDGAMES\\MID36VOC.PAK"
#define MID5VOC "..\\..\\DATA\\MIDGAMES\\MID5VOC.PAK"

constexpr SoundID SndYolo = { MID1VOC, 0 };
