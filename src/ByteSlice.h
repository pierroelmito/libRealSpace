
#pragma once

#include <cstdint>
#include <cstddef>

struct ByteSlice
{
	uint8_t* data{ nullptr };
	size_t size{ 0 };
};
