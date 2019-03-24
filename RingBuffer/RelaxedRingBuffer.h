#pragma once

#include <array>
#include <type_traits>
#include <atomic>

#include "RingBuffer.h" // import modulus types, Relacy defines

template <size_t N = 512, typename DataType = char, typename  IndexType = uint32_t, typename RingMod = Lomont::FastRingMod<N, IndexType>>
class RelaxedRingBuffer
{
	// basic Lamport Proving the Correctness of Multiprocess Programs (1977) 
	// wait-free SPSC queue
	// stop using % for modulus
public:

	static_assert(std::is_unsigned<IndexType>::value, "Ringbuffer IndexType should be unsigned for numerics");

	// how many items available to read in [0,Size]
	// if called from consumer, true size may be more since producer can be adding
	// if called from producer, true size may be less since consumer may be removing
	// undefined to call from any other thread
	size_t AvailableToRead() const
	{
		return RingMod::Mod1N(N + writeIndex_.load(std::memory_order_acquire) - readIndex_.load(std::memory_order_acquire));
	}

	// how many items available to write in [0,N]
	// if called from consumer, true size may be less since producer can be adding
	// if called from producer, true size may be more since consumer may be removing
	// undefined to call from any other thread
	size_t AvailableToWrite() const { return Size() - AvailableToRead(); }

	bool IsEmpty() const { return AvailableToRead() == 0; }

	bool IsFull() const { return AvailableToRead() == Size(); }

	// Max number the buffer can hold
	// sadly, this design is N-1, not N
	size_t Size() const { return N - 1; }

	// try to put the item in, return false if full
	bool Put(const DataType & datum)
	{
		IndexType w = writeIndex_.load(std::memory_order_relaxed);
		const IndexType nextWrite = RingMod::Mod1N(w + 1);
		if (nextWrite != readIndex_.load(std::memory_order_acquire))
		{
			buffer_[w] = datum;
			writeIndex_.store(nextWrite, std::memory_order_release);
			return true;
		}
		// buffer full
		return false;
	}

	// try to get an item, return false if none available
	bool Get(DataType & datum)
	{
			
		const auto r = readIndex_.load(std::memory_order_relaxed);
		if (r != writeIndex_.load(std::memory_order_acquire))
		{
			datum = buffer_[r];
			readIndex_.store(RingMod::Mod1N(r + 1), std::memory_order_release);
			return true;
		}
		return false; // buffer empty
	}
private:
	std::atomic<IndexType> writeIndex_{ 0 };
	std::atomic<IndexType> readIndex_{ 0 };
	std::array<DataType, N> buffer_;
};


