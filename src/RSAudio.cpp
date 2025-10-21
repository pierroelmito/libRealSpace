
#include "RSAudio.h"

RSAudio::RSAudio() { }

RSAudio::~RSAudio() { }

bool RSAudio::Init()
{
	return true;
}

void RSAudio::Release()
{
}

bool RSAudio::Update(const SamplerCB& cb)
{
	/*
	const int num_frames = saudio_expect();
	std::vector<float> buffer(num_frames, 0.0f);
	cb(saudio_sample_rate(), saudio_channels(), buffer);
	for (int i = 0; i < num_frames; i++) {
		// simple square wave generator
		// const float volume = 0.1f;
		buf[bufPos++] = buffer[i];
		if (bufPos == BufSize) {
			bufPos = 0;
			saudio_push(buf, BufSize);
		}
	}
	*/
	return true;
}
