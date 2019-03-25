#pragma once

#include <array>
#include <type_traits>
#include <atomic>

// Implement a single producer, single consumer (SPSC) ring buffer
// basic Lamport Proving the Correctness of Multiprocess Programs (1977) design
// is wait-free
template <size_t N = 512, typename DataType = char, typename  IndexType = uint32_t>
class AtomicsRingBuffer
{
public:
	static_assert(std::is_unsigned<IndexType>::value, "Ringbuffer IndexType should be unsigned for numerics");

	// how many items available to read in [0,Size]
	// if called from consumer, true size may be more since producer can be adding
	// if called from producer, true size may be less since consumer may be removing
	// undefined to call from any other thread
	size_t AvailableToRead() const
	{
		// see comment in simple on why this cast is needed
		return (static_cast<size_t>(writeIndex_) - readIndex_ + N) % N;
	}



	// how many items available to write in [0,Size]
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
		IndexType w = writeIndex_;
		IndexType r = readIndex_;

		if ((w+1)%N == r)
			return false;

		buffer_[w] = datum;
		writeIndex_ = (w + 1) % N;
		return true;
	}

	// try to get an item, return false if none available
	bool Get(DataType & datum)
	{
		IndexType w = writeIndex_;
		IndexType r = readIndex_;

		if (w == r)
			return false;
		datum = buffer_[r];
		readIndex_ = (r + 1) % N;
		return true;
	}
private:
	std::atomic<IndexType> readIndex_{ 0 };
	std::atomic<IndexType> writeIndex_{ 0 };
	std::array<DataType, N> buffer_;
};

