#pragma once
#include <cstdint>
// Simple 32 bit Marsaglia xorshift rnd
// not super strong, good enough for us
struct Rand32
{
	uint32_t seed = 0x12345;
	uint32_t Next()
	{
		uint32_t x = seed;
		x ^= x << 13;
		x ^= x >> 17;
		x ^= x << 5;
		seed = x;
		return seed;
	}
};

