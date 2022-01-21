//
//  RSSound.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "RSSound.h"

#include "precomp.h"

bool RSVocSoundData::InitFromRAM(const ByteSlice& bs)
{
	ByteStream stream(bs.data);
	const auto header = stream.ReadBytes<0x13>();
	const uint8_t b0 = stream.ReadByte();
	const uint16_t sz = stream.ReadShort();
	const uint16_t version0 = stream.ReadShort();
	const uint16_t version1 = stream.ReadShort();

	if (sz != 26)
		return false;

	while (true) {
		const uint8_t bt = stream.ReadByte();
		if (bt == BlockType::Terminator)
			break;

		const auto bsz = stream.ReadBytes<3>();
		const uint32_t szBlock = (bsz[2] << 16) | (bsz[1] << 8) | (bsz[0] << 0);

		//printf("\t[%d] : %d bytes\n", bt, szBlock);

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
