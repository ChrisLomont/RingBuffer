#pragma once

#include <cstdint>
#include <cassert>

#include "RingBuffer.h" // for mod

template<size_t N, typename DataType = char, typename IndexType = uint32_t, typename RingMod = Lomont::FastRingMod<N, IndexType>>
class BlocksRingBuffer
{
public:

	// how many items available to read in [0,N]
	// if called from consumer, true size may be more since producer can be adding
	// if called from producer, true size may be less since consumer may be removing
	// undefined to call from any other thread
	size_t AvailableToRead() const
	{
		auto w = writeIndex_.load(std::memory_order_acquire);
		auto r = readIndex_.load(std::memory_order_acquire);
		return RingMod::Mod2N(2 * N + w - r);
	}

	// how many items available to write in [0,N]
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

	// try to write n elements, fails if no space available
	bool Put(const DataType * data, size_t n)
	{
		auto w = writeIndex_.load(std::memory_order_relaxed);
		auto r = readIndex_.load(std::memory_order_acquire);
		if (Size() - RingMod::Mod2N(2 * N + w - r) < n)
			return false; // does not fit
		auto t = RingMod::Mod1N(w);
		for (auto i = 0; i < n; ++i)
			buffer_[RingMod::Mod1N(t+i)] = data[i];
		w = RingMod::Mod2N(w + n);
		writeIndex_.store(w, std::memory_order_release);
		return true;
	}

	// try to get n elements, fails if not available
	bool Get(DataType * data, size_t n)
	{
		auto w = writeIndex_.load(std::memory_order_relaxed);
		auto r = readIndex_.load(std::memory_order_acquire);
		if (RingMod::Mod2N(2 * N + w - r) < n) // predicted available to read
			return false; // not available
		auto t = RingMod::Mod1N(r);
		for (auto i = 0; i < n; ++i)
			data[i] = buffer_[RingMod::Mod1N(t+i)];
		r = RingMod::Mod2N(r + n);
		readIndex_.store(r, std::memory_order_release);
		return true;
	}
private:
	// Taking counters mod N leaves one cell unused without additional fields to track, 
	// but then atomic operations harder to check.
	// Taking counters mod 2N makes it possible to use all cells in the buffer when full, 
	// at additional cost of bound enforcement on buffer access.

	// try to cache separate read and write. Can do better, but at increased space
	std::atomic<IndexType> writeIndex_{ 0 };
	DataType buffer_[N];
	std::atomic<IndexType> readIndex_{ 0 };

};


