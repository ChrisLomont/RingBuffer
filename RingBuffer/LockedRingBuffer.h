#pragma once

#include <array>
#include <type_traits>
#include <mutex>
// Implement a simple ring buffer
// put/get threadsafe, other functions questionable, cannot be mixed
// NOTE: these APIs are now poorly designed. Should remove all except Put and Get
template <size_t N = 512, typename DataType = char, typename  IndexType = uint32_t>
class LockedRingBuffer
{
public:
#ifndef SAMD21_BUILD
	static_assert(std::is_unsigned<IndexType>::value, "Ringbuffer IndexType should be unsigned for numerics");
	using Scoped = std::lock_guard<std::recursive_mutex>;
    #define GUARD() Scoped guard(lock_)
#else
    #define GUARD() 
#endif    
	// how many items available to read in [0,Size]
	size_t AvailableToRead() const
	{
		GUARD(); 
		// see comment in simple on why this cast is needed
		return (static_cast<size_t>(writeIndex_) - readIndex_ + N) % N;
	}

	// how many items available to write in [0,Size]
	size_t AvailableToWrite() const { GUARD(); return Size() - AvailableToRead(); }

	bool IsEmpty() const { GUARD(); return AvailableToRead() == 0; }

	bool IsFull() const { GUARD(); return AvailableToRead() == Size(); }

	// Max number the buffer can hold
	// sadly, this design is N-1, not N
	size_t Size() const { return N - 1; }

	// try to put the item in, return false if full
	bool Put(const DataType & datum)
	{   GUARD();
		if (IsFull())
			return false;
		buffer_[writeIndex_] = datum;
		writeIndex_ = (writeIndex_ + 1) % N;
		return true;
	}

	// try to get an item, return false if none available
	bool Get(DataType & datum)
	{   GUARD();
		if (IsEmpty())
			return false;
		datum = buffer_[readIndex_];
		readIndex_ = (readIndex_ + 1) % N;
		return true;
	}
private:
	IndexType readIndex_{ 0 };
	IndexType writeIndex_{ 0 };
	std::array<DataType, N> buffer_;   
#ifndef SAMD21_BUILD
	mutable std::recursive_mutex lock_;
#endif    
};


