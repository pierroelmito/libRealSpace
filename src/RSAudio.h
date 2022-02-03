
#pragma once

#include <functional>

class RSAudio
{
public:
	using SamplerCB = std::function<void(int, int, std::vector<float>&)>;

	RSAudio();
	~RSAudio();
	bool Init();
	void Release();
	bool Update(const SamplerCB& cb);

protected:
	static constexpr int BufSize = 128;
	int bufPos{ 0 };
	int count{ 0 };
	float buf[BufSize];
};
