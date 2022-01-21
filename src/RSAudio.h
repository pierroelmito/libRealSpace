
#pragma once

#include <functional>

class RSAudio
{
public:
	RSAudio();
	~RSAudio();
	bool Init();
	bool Update(const std::function<void(std::vector<float>&)>& cb);

protected:
	static constexpr int BufSize = 128;
	int bufPos{ 0 };
	int count{ 0 };
	float buf[BufSize];
};
