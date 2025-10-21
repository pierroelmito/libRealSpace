
#pragma once

#include <cstddef>
#include <cstdint>

#include <memory>
#include <optional>
#include <vector>

using DataBuffer = std::vector<uint8_t>;
using DataBufferPtr = std::shared_ptr<DataBuffer>;

struct ByteSlice {
	uint8_t* data { nullptr };
	size_t size { 0 };

	static DataBufferPtr ReadFile(const char* fullPath);
	static ByteSlice Get(DataBufferPtr buffer, size_t start = 0, std::optional<size_t> size = {});
};
