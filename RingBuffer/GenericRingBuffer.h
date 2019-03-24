#pragma once

#include <array>
#include <type_traits>


// Implement a single threaded simple ring buffer
// templated to make items generic
template <size_t N = 512, typename DataType = char, typename  IndexType = uint32_t>
class GenericRingBuffer
{
public:
	static_assert(std::is_unsigned<IndexType>::value, "Ringbuffer IndexType should be unsigned for numerics");

	// how many items available to read in [0,Size]
	size_t AvailableToRead() const
	{
		// see comment on simple ring buffer
		return (N + writeIndex_ - readIndex_) % N;
	}

	// how many items available to write in [0,Size]
	size_t AvailableToWrite() const { return Size() - AvailableToRead(); }

	bool IsEmpty() const { return AvailableToRead() == 0; }

	bool IsFull() const { return AvailableToRead() == Size(); }

	// Max number the buffer can hold
	// sadly, this design is N-1, not N
	size_t Size() const { return N - 1; }

	// try to put the item in, return false if full
	bool Put(const DataType & datum)
	{
		if (IsFull())
			return false;
		buffer_[writeIndex_] = datum;
		writeIndex_ = (writeIndex_ + 1) % N;
		return true;
	}

	// try to get an item, return false if none available
	bool Get(DataType & datum)
	{
		if (IsEmpty())
			return false;
		datum = buffer_[readIndex_];
		readIndex_ = (readIndex_ + 1) % buffer_.size();
		return true;
	}
private:
	IndexType readIndex_{ 0 };
	IndexType writeIndex_{ 0 };
	std::array<DataType,N> buffer_;
};



