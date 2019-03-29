#pragma once


#ifdef SAMD21_BUILD
#include <array>
#else
#include <vector>
#endif

// Implement a simple single threaded ring buffer


template <size_t N = 512>
class SimpleRingBuffer 
{
public:
	SimpleRingBuffer(int size = N) 
    #ifndef SAMD21_BUILD
    :buffer_(size) 
#endif    
    { }

	// how many items available to read in [0,Size]
	size_t AvailableToRead() const
	{
		// SUPER SUBTLE BUG HERE!
		// when both indices unsigned int (32 bits), and
		// size_t is 64 bits, then for size 513 buffer, 
		// write at 31, read at 512, math was:
		// 31-512 
		// adding 513 in 64 bit size_t expanded to 64 bits, but not sign extended!
		// then mod 513 was 0, instead of 32
		// return (writeIndex_ - readIndex_ + N) % N;
		// fix was to cast to size_t first, or move N to front!
		return (N + writeIndex_ - readIndex_) % N;
	}

	// how many items available to write in [0,Size]
	// was bug here: N - .. instead of Size. Also in IsFull
	size_t AvailableToWrite() const { return Size() - AvailableToRead();  }

	bool IsEmpty() const { return AvailableToRead() == 0; }
	
	bool IsFull() const { return AvailableToRead() == Size(); }

	// Max number the buffer can hold
	// sadly, this design is N-1, not N
	size_t Size() const { return N-1; }

	// try to put the item in, return false if full
	bool Put(char datum)
	{
		if (IsFull())
			return false;
		buffer_[writeIndex_] = datum;
		writeIndex_ = (writeIndex_ + 1) % N;
		return true;
	}
	
	// try to get an item, return false if none available
	bool Get(char & datum)
	{
		if (IsEmpty())
			return false;
		datum = buffer_[readIndex_];
		readIndex_ = (readIndex_ + 1) % buffer_.size();
		return true;
	}
private:
	unsigned readIndex_{ 0 };
	unsigned writeIndex_{ 0 };
#ifdef SAMD21_BUILD
    // default lib has no heap
    std::array<char,128> buffer_; // fix test size
#else        
	std::vector<char> buffer_;
#endif    
};

