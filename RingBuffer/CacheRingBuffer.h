#pragma once

#include <cstdint>
#include <cassert>

#include "RingBuffer.h" // for mod

template<size_t N, typename DataType = char, typename IndexType = uint32_t, typename RingMod = Lomont::FastRingMod<N, IndexType>>
class CacheRingBuffer
{
public:

	// how many items available to read in [0,Size]
	// if called from consumer, true size may be more since producer can be adding
	// if called from producer, true size may be less since consumer may be removing
	// undefined to call from any other thread
	size_t AvailableToRead() const
	{
		return RingMod::Mod2N(2 * N + writeIndex_.load(std::memory_order_acquire) - readIndex_.load(std::memory_order_acquire));
	}

	// how many items available to write in [0,Size]
	// if called from consumer, true size may be less since producer can be adding
	// if called from producer, true size may be more since consumer may be removing
	// undefined to call from any other thread
	size_t AvailableToWrite() const
	{
		return Size() - AvailableToRead();
	}

	bool IsEmpty() const { return AvailableToRead() == 0; }

	bool IsFull()  const { return AvailableToRead() == Size(); }

	// size of buffer, can hold exactly this many
	size_t Size() const { return N; }

	// try to write an element, fails if no space available
	bool Put(const DataType & datum)
	{ // paper above has ability to write bigger blocks, is faster
		const auto w = writeIndex_.load(std::memory_order_relaxed);
		const auto nextWrite = RingMod::Mod2N(w + 1);
		if (nextWrite != readIndex_.load(std::memory_order_acquire))
		{
			buffer_[RingMod::Mod1N(w)] = datum;
			writeIndex_.store(nextWrite, std::memory_order_release);
			return true;
		}
		// buffer full
		return false;
	}

	// try to get an element, fails if none available
	bool Get(DataType & data)
	{
		const auto r = readIndex_.load(std::memory_order_relaxed);
		if (r != writeIndex_.load(std::memory_order_acquire))
		{
			data = buffer_[RingMod::Mod1N(r)];
			readIndex_.store(RingMod::Mod2N(r + 1), std::memory_order_release);
			return true;
		}
		return false; // buffer empty
	}
private:
	// Taking counters mod N leaves one cell unused without additional fields to track, 
	// but then atomic operations harder to check.
	// Taking counters mod 2N makes it possible to use all cells in the buffer when full, 
	// at additional cost of bound enforcement on buffer access.

	// try to cache separate read and write. Can do better, but at increased space
	std::atomic<IndexType> writeIndex_{ 0 };
	std::array<DataType, N> buffer_;
	std::atomic<IndexType> readIndex_{ 0 };

};
