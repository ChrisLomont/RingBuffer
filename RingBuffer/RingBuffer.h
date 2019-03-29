#pragma once
#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <cstdint>
#include <cassert>

// Single producer, single-consumer ring buffer
// Doesn't leave any cells empty when full, unlike many implementations.
// MORE THREADS THAN THAT WILL NOT WORK!
// Chris Lomont, 2019, chris@lomont.org


// include RELACY thread ordering header before the ringbuffer include to enable relacy testing
#ifndef RL_RELACY_HPP
#include <atomic>
#define NM std
#define ACCESS(a) a
#else
#define NM rl
#define ACCESS(a) a($)
#endif

namespace Lomont {

#define LARGE_RING_BLOCKS  // define for larger block sizes - adds two counters, faster put/get

/************************** mod function variations ******************************/

// usual modulus % operator in C/C++
template<std::size_t N, typename IndexType>
struct SlowRingMod
{
	// given integer in [0,2N-1], return mod N in [0,N-1]        
	static inline IndexType Mod1N(const IndexType & index)
	{
		assert(0 <= index && index < 2 * N);
		return index % N;
	}

	// given integer in [0,4N-1], return mod M in [0,2N-1]
	static inline IndexType Mod2N(const IndexType & index)
	{
		assert(0 <= index && index < 4 * N);
		return index % (2 * N);
	}
};

// instead of % for mod, utilize fact can map [0,2N-1] to [0,N] with one 'if'
// for our use, the 'if' is true most of the time, helping prediction where it matters
template<std::size_t N, typename IndexType>
struct MidRingMod
{

	// given integer in [0,2N-1], return mod N in [0,N-1]        
	static inline IndexType Mod1N(const IndexType & index)
	{
		assert(0 <= index && index < 2 * N);
		if (index < N)
			return index;
		return index - N;
	}

	// given integer in [0,4N-1], return mod M in [0,2N-1]
	static inline IndexType Mod2N(const IndexType & index)
	{
		assert(0 <= index && index < 4 * N);
		if (index < 2 * N)
			return index;
		return index - 2 * N;
	}
};

// FastRingMod is same as MidRingMod, but with specialized power of 2 case replaced with a mask

// template to detect if N is power of 2
template<std::size_t N>
struct is_power_of_two
{
	static const bool value = N && ((N & (N - 1)) == 0);
};


// same as MidRingMod, but with specialized power of 2 case replaced with a mask
template<std::size_t N, typename IndexType>
struct FastRingModPowerOfTwo
{
	// given N power of two, integer in [0,2N-1], return mod N in [0,N-1]        
	static inline IndexType Mod1N(const IndexType & index)
	{
		assert(0 <= index && index < 2 * N);
		return index & (N - 1);
	}

	// given N power of two, integer in [0,4N-1], return mod N in [0,2N-1]        
	static inline IndexType Mod2N(const IndexType & index)
	{
		assert(0 <= index && index < 4 * N);
		return index & (2 * N - 1);
	}
};

// now pick based on power of 2 or not
template<std::size_t N, typename IndexType>
using FastRingMod = std::conditional_t<is_power_of_two<N>::value, FastRingModPowerOfTwo<N, IndexType>, MidRingMod<N, IndexType>>;

template<std::size_t N, typename DataType = char, typename IndexType = int32_t, typename RingMod = FastRingMod<N, IndexType>>
class RingBuffer
{
	// C++ doesn't yet support static_assert of is_always_lock_free, but may soon
	//static_assert(std::atomic<IndexType>::is_always_lock_free, "RingbufferIndexType should be lockfree for performance");

public:

	// how many items available to read in [0,N]
	// if called from consumer, true size may be more since producer can be adding
	// if called from producer, true size may be less since consumer may be removing
	// undefined to call from any other thread
	std::size_t AvailableToRead() const
	{
		return RingMod::Mod2N(2 * N + writeIndex_.load(NM::memory_order_acquire) - readIndex_.load(NM::memory_order_acquire));
	}

	// how many items available to write in [0,N]
	// if called from consumer, true size may be less since producer can be adding
	// if called from producer, true size may be more since consumer may be removing
	// undefined to call from any other thread
	std::size_t AvailableToWrite() const
	{
		return Size() - AvailableToRead();
	}

	bool IsEmpty() const { return AvailableToRead() == 0; }

	bool IsFull()  const { return AvailableToRead() == Size(); }

	// size of buffer, can hold exactly this many
	std::size_t Size() const { return N; }

	// try to write an element, fails if no space available
	bool Put(const DataType & datum)
	{ // paper above has ability to write bigger blocks, is faster
		const auto w = writeIndex_.load(NM::memory_order_relaxed);
		const auto nextWrite = RingMod::Mod2N(w + 1);
		if (nextWrite != readIndex_.load(NM::memory_order_acquire))
		{
			buffer_[RingMod::Mod1N(w)] = datum;
			writeIndex_.store(nextWrite, NM::memory_order_release);
			return true;
		}
		// buffer full
		return false;
	}

	// try to get an element, fails if none available
	bool Get(DataType & data)
	{
		const auto r = readIndex_.load(NM::memory_order_relaxed);
		if (r != writeIndex_.load(NM::memory_order_acquire))
		{
			data = buffer_[RingMod::Mod1N(r)];
			readIndex_.store(RingMod::Mod2N(r + 1), NM::memory_order_release);
			return true;
		}
		return false; // buffer empty
	}

#ifdef LARGE_RING_BLOCKS
	// try to write n elements, fails if no space available
	bool Put(const DataType * data, std::size_t n)
	{
		auto w = writeIndex_.load(NM::memory_order_relaxed);
		if (Size() - RingMod::Mod2N(2 * N + w - pReadIndex_) < n) // predicted available to write
		{ // may not fit, check more exactly, costing an atomic read
			pReadIndex_ = readIndex_.load(NM::memory_order_acquire);
			if (Size() - RingMod::Mod2N(2 * N + w - pReadIndex_) < n) // current available to write
				return false; // does not fit
		}
		auto t = RingMod::Mod1N(w);
		for (auto i = 0; i < n; ++i)
			buffer_[RingMod::Mod1N(t+i)] = data[i];   
		w = RingMod::Mod2N(w + n);
		writeIndex_.store(w, NM::memory_order_release);
		return true;
	}

	// try to get n elements, fails if not available
	bool Get(DataType * data, std::size_t n)
	{
		auto r = readIndex_.load(NM::memory_order_relaxed);
		if (RingMod::Mod2N(2 * N + pWriteIndex_ - r) < n) // predicted available to read
		{ // may not fit, check more exactly, costing an atomic read
			pWriteIndex_ = writeIndex_.load(NM::memory_order_acquire);
			if (RingMod::Mod2N(2 * N + pWriteIndex_ - r) < n) // current available to read
				return false; // not available
		}
		auto t = RingMod::Mod1N(r);
		for (auto i = 0; i < n; ++i)
			data[i] = buffer_[RingMod::Mod1N(t+i)];
		r = RingMod::Mod2N(r + n);
		readIndex_.store(r, NM::memory_order_release);
		return true;
	}
#endif
private:
	// Taking counters mod N leaves one cell unused without additional fields to track, 
	// but then atomic operations harder to check.
	// Taking counters mod 2N makes it possible to use all cells in the buffer when full, 
	// at additional cost of bound enforcement on buffer access.

	// try to cache separate read and write. Can do better, but at increased space
	NM::atomic<IndexType> writeIndex_{ 0 };
#ifdef LARGE_RING_BLOCKS
	IndexType pReadIndex_{ 0 }; // predictive read index, cache neighbors
#endif
	DataType buffer_[N];
	NM::atomic<IndexType> readIndex_{ 0 };
#ifdef LARGE_RING_BLOCKS
	IndexType pWriteIndex_{ 0 }; // predictive write index, cache neighbors
#endif

};

#undef NM
#undef ACCESS

/* Notes:
	// todo - wrap with C++ 17 ifdef
	// todo - perhaps check if integer is unsigned? std::is_unsigned, needed to prevent subtraction errors want wraparound
	// todo - check if index is integer?
	// todo - asserts throughout
	// see Correct and Efficient Bounded FIFO Queues, https://www.irif.fr/~guatto/papers/sbac13.pdf
	// and https://www.snellman.net/blog/archive/2016-12-13-ring-buffers/
	// also Facebook Folly threading lib example https://github.com/facebook/folly/blob/master/folly/ProducerConsumerQueue.h
*/

} // namespace Lomont

#endif // RING_BUFFER_H

