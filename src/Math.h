//
//  Math.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/13/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include <cmath>
#include <cstdint>
#include <cassert>

#include <array>

#include "HandmadeMath.h"

struct Point2D
{
	int32_t x;
	int32_t y;
};

using RSVector3 = hmm_vec3;
using RSMatrix = hmm_mat4;

inline constexpr float Ratio(float a, float b, float x)
{
	if (x < a)
		return 0.0f;
	if (x > b)
		return 1.0f;
	return (x - a) / (b - a);
}

inline constexpr float SmoothStep(float x)
{
	return x * x * (3 - 2 * x);
}

inline constexpr std::pair<int, float> GetBilinear(float v, int sz)
{
	const int ix = v > 0.0f ? (int(sz * v) % sz) & (sz - 1) : (sz - 1) - (int(sz * -v) % sz) & (sz - 1);
	const float av = abs(v);
	const float sr = av * sz - truncf(av * sz);
	if (v < 0.0f)
		return { ix, 1.0f - sr };
	return { ix, sr };
}

template <size_t N>
inline constexpr float PerlinNoise(float x, float y, const std::array<hmm_vec2, N * N>& gradients)
{
	const auto [ ix0, rx1 ] = GetBilinear(x, N);
	const auto [ iy0, ry1 ] = GetBilinear(y, N);
	const float rx0 = 1.0f - rx1;
	const float ry0 = 1.0f - ry1;
	const float srx0 = SmoothStep(rx0);
	const float srx1 = SmoothStep(rx1);
	const float sry0 = SmoothStep(ry0);
	const float sry1 = SmoothStep(ry1);
	const int ix1 = (ix0 + 1) % N;
	const int iy1 = (iy0 + 1) % N;
	const float x0y0 = HMM_DotVec2(gradients[iy0 * N + ix0], { srx0, sry0 });
	const float x1y0 = HMM_DotVec2(gradients[iy0 * N + ix1], { srx1, sry0 });
	const float x0y1 = HMM_DotVec2(gradients[iy1 * N + ix0], { srx0, sry1 });
	const float x1y1 = HMM_DotVec2(gradients[iy1 * N + ix1], { srx1, sry1 });
	const float ir = srx0 * sry0 * x0y0 + srx1 * sry0 * x1y0 + srx0 * sry1 * x0y1 + srx1 * sry1 * x1y1;
	const float r = 0.5f * (1.0f + ir);
	assert(r >= 0.0f && r <= 1.0f);
	return r;
}
