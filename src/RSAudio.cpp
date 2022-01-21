
#include "RSAudio.h"

#define SOKOL_AUDIO_IMPL
#include "sokol_audio.h"

RSAudio::RSAudio()
{
}

RSAudio::~RSAudio()
{
}

bool RSAudio::Init()
{
	// init sokol-audio with default params, no callback
	saudio_setup(saudio_desc());
	assert(saudio_channels() == 1);

	//saudio_shutdown();
	return true;
}

bool RSAudio::Update(const std::function<void(std::vector<float>&)>& cb)
{
	const int num_frames = saudio_expect();
	std::vector<float> buffer(num_frames, 0.0f);
	cb(buffer);
	for (int i = 0; i < num_frames; i++) {
		// simple square wave generator
		//const float volume = 0.1f;
		buf[bufPos++] = buffer[i];
		if (bufPos == BufSize) {
			bufPos = 0;
			saudio_push(buf, BufSize);
		}
	}
	return true;
}
