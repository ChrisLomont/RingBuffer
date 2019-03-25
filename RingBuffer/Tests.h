#pragma once

#include <cstdint>
#include <string>
#include <thread>

#include "RingBuffer.h"
#include "Stopwatch.h"
#include "Rand32.h"

// stats holds per test statistics
struct Stats
{
	Stats(const char * testName, const char * typeName, size_t bufferSize, size_t blockSize, long size)
	{
		passCount = 100;
		total = 0;
		min = 1L << 30;
		max = -min;
		this->testName = testName;
		this->size = size;
		this->typeString = typeName;
		this->bufferSize = bufferSize;
		this->blockSize = blockSize;
		success = true;
	}
	void Add(size_t elapsedMs1)
	{
		auto elapsedMs = (long)elapsedMs1;
		min = min < elapsedMs ? min : elapsedMs;
		max = elapsedMs < max ? max : elapsedMs;
		total += elapsedMs;
	}
	int passCount;
	int64_t total;
	int64_t min;
	int64_t max;
	int64_t size; // # bytes processed
	const char * testName;
	const char * typeString;
	size_t bufferSize, blockSize;
	bool success;
};

inline void ShowLogFormat()
{
	printf("Test, ring size, block transfer size, avg MB/s, max MB/s, min MB/s, success/fail, buffer name, avgMs, \n");
}

long ZeroToOne(long v) { if (v != 0) return v; return 1; }

inline void Log(const Stats & stats)
{
	char name[1000];
	sprintf(name, "%s", stats.typeString);
	// excel having a mess importing names with ',' even when escaped, so...
	char * p = name;
	while (*p != 0)
	{
		if (*p == ',')
			*p = ':';
		p++;
	}
	char buffer[1000];
	auto avgElapsedMs = stats.total / stats.passCount; // ms/pass
	auto avgMB10 = (10000 * stats.size / ZeroToOne(avgElapsedMs)) / (1UL << 20);
	auto minMB10 = (10000 * stats.size / ZeroToOne(stats.max)) / (1UL << 20);
	auto maxMB10 = (10000 * stats.size / ZeroToOne(stats.min)) / (1UL << 20);
	sprintf(buffer, "%s, %ld, %ld, %lld.%lld, %lld.%lld, %lld.%lld, %d, %s, %ld, ",
		stats.testName,
		(long)stats.bufferSize, (long)stats.blockSize,
		avgMB10/10, avgMB10%10,
		minMB10/10, minMB10%10,
		maxMB10/10, maxMB10%10,
		stats.success, name,
		avgElapsedMs
	);
	std::cout << buffer << std::endl;
}

// buffer size, read / write size
template<size_t N, size_t M, typename RingType = Lomont::RingBuffer<N>>
uint32_t ThroughputSingleBlock(long size)
{
	StopWatch sw;
	Stats stats("SingleBlock", typeid(RingType).name(),N,M,size);

	for (int pass = 0; pass < stats.passCount; ++pass)
	{
		RingType rb;
		char buffer[1024];

		// fill buffer
		Rand32 rnd;
		rnd.seed = 0x12345;
		for (auto i = 0; i < sizeof(buffer); ++i)
			buffer[i] = rnd.Next();

		uint32_t reader = 0, writer = 0;

		sw.Reset();
		sw.Start();
		long processed = 0;
		while (processed < size)
		{
			rb.Put(buffer + writer, M);
			writer = (writer + M) & 1023;
			rb.Get(buffer + reader, M);
			reader = (reader + M) & 1023;
			processed += M;
		}

		sw.Stop();
		stats.Add(sw.ElapsedMs());

		// check matches
		rnd.seed = 0x12345;
		for (auto i = 0; i < sizeof(buffer); ++i)
			stats.success &= ((uint8_t)buffer[i]) == (rnd.Next() & 255);
		if (!stats.success)
			std::cout << "Error: mismatch!" << std::endl;
	}

	Log(stats);
	return stats.success;
}

// buffer size, read/write size
// two threads
// return true on matches
template<size_t N, size_t M, typename RingType = Lomont::RingBuffer<N>>
bool ThroughputDoubleBlock(long size)
{
	StopWatch sw;
	Stats stats("DoubleBlock", typeid(RingType).name(), N, M, size);

	for (int pass = 0; pass < stats.passCount; ++pass)
	{
		RingType rb;
		char buffer[1024];

		// fill buffer
		Rand32 rnd;
		rnd.seed = 0x12345;
		for (auto i = 0; i < sizeof(buffer); ++i)
			buffer[i] = rnd.Next();

		long errors1 = 0, errors2 = 0;

		sw.Reset();
		sw.Start();


		std::thread t1(
			[&]()
		{
			uint32_t writer = 0;
			long processed = 0;

			while (processed < size)
			{
				// todo - reinstate others?!
				while (rb.AvailableToWrite() < M)
				{ // spin 
				}
				errors1 += !rb.Put(buffer + writer, M);
				writer = (writer + M) & 1023;
				processed += M;
			}
		}
		);

		std::thread t2(
			[&]()
		{
			uint32_t reader = 0;
			long processed = 0;

			while (processed < size)
			{
				while (rb.AvailableToRead() < M)
				{ // spin 
				}
				errors2 += !rb.Get(buffer + reader, M);
				reader = (reader + M) & 1023;
				processed += M;
			}
		}
		);

		t1.join();
		t2.join();

		stats.success &= errors1 + errors2 == 0;
		if (!stats.success)
			std::cout << "Error: get/put errors" << std::endl;

		sw.Stop();
		stats.Add(sw.ElapsedMs());

		// check matches
		rnd.seed = 0x12345;
		for (auto i = 0; i < sizeof(buffer); ++i)
			stats.success &= ((uint8_t)buffer[i]) == (rnd.Next() & 255);
		if (!stats.success)
			std::cout << "Error: mismatch!" << std::endl;
	}
	Log(stats);
	return stats.success;
}

// buffer size, read/write size
// return true on matches
template<size_t N, size_t M, typename RingType = Lomont::RingBuffer<N>>
bool ThroughputSingle(long size)
{
	StopWatch sw;
	Stats stats("Single", typeid(RingType).name(), N, M, size);

	for (int pass = 0; pass < stats.passCount; ++pass)
	{
		RingType rb;
		char buffer[1024];

		// fill buffer
		Rand32 rnd;
		rnd.seed = 0x12345;
		for (auto i = 0; i < sizeof(buffer); ++i)
			buffer[i] = rnd.Next();

		uint32_t reader = 0, writer = 0;

		sw.Reset();
		sw.Start();
		long processed = 0;
		while (processed < size)
		{
			for (auto i = 0; i < M; ++i)
			{
				rb.Put(buffer[writer]);
				writer = (writer + 1) & 1023;
			}
			for (auto i = 0; i < M; ++i)
			{
				rb.Get(buffer[reader]);
				reader = (reader + 1) & 1023;
			}
			processed += M;
		}

		sw.Stop();
		stats.Add(sw.ElapsedMs());

		// check matches
		rnd.seed = 0x12345;
		for (auto i = 0; i < sizeof(buffer); ++i)
			stats.success &= ((uint8_t)buffer[i]) == (rnd.Next() & 255);
		if (!stats.success)
			std::cout << "Error: mismatch!" << std::endl;
	}
	Log(stats);
	return stats.success;
}



// buffer size, read/write size
// two threads
// return true on matches
template<size_t N, size_t M, typename RingType = Lomont::RingBuffer<N>>
bool ThroughputDouble(long size)
{
	StopWatch sw;
	Stats stats("Double", typeid(RingType).name(), N, M, size);

	for (int pass = 0; pass < stats.passCount; ++pass)
	{
		RingType rb;
		char buffer[1024];

		// fill buffer
		Rand32 rnd;
		rnd.seed = 0x12345;
		for (auto i = 0; i < sizeof(buffer); ++i)
			buffer[i] = rnd.Next();

		long errors1 = 0, errors2 = 0;

		sw.Reset();
		sw.Start();

		std::thread t1(
			[&]()
		{
			uint32_t writer = 0;
			long processed = 0;

			while (processed < size)
			{
				for (auto i = 0; i < M; ++i)
				{
					while (rb.AvailableToWrite() < 1)
					{ // spin 
					}
					errors1 += !rb.Put(buffer[writer]);
					// if (errors1) std::cout << "ERRORS1\n"; // todo - remove
					writer = (writer + 1) & 1023;
				}
				processed += M;
			}
		}
		);

		std::thread t2(
			[&]()
		{
			uint32_t reader = 0;
			long processed = 0;

			while (processed < size)
			{
				for (auto i = 0; i < M; ++i)
				{
					while (rb.AvailableToRead() < 1)
					{ // spin 
					}
					errors2 += !rb.Get(buffer[reader]);
					// if (errors2) std::cout << "ERRORS2\n"; // todo - remove
					reader = (reader + 1) & 1023;
				}
				processed += M;
			}
		}
		);

		t1.join();
		t2.join();


		sw.Stop();
		stats.Add(sw.ElapsedMs());

		stats.success = errors1 + errors2 == 0;
		if (!stats.success)
			std::cout << "ERROR: thread r/w errors" << std::endl;

		// check matches
		rnd.seed = 0x12345;
		for (auto i = 0; i < sizeof(buffer); ++i)
			stats.success &= ((uint8_t)buffer[i]) == (rnd.Next() & 255);
		if (!stats.success)
			std::cout << "Error: mismatch!" << std::endl;
	}

	Log(stats);
	return stats.success;
}

// simple checks
// return true on success
// error msg  and false on error
template<size_t N, size_t M, typename RingType = Lomont::RingBuffer<N>>
bool SanityCheck(long size)
{
	RingType rb;
	char buffer[1024];
	auto success = true;
	std::cout << "Sanity check " << typeid(RingType).name() << std::endl;

	// fill buffer
	Rand32 rnd;
	rnd.seed = 0x12345;
	for (auto i = 0; i < sizeof(buffer); ++i)
		buffer[i] = rnd.Next();

	uint32_t reader = 0, writer = 0;

	// fill buffer
	success &= rb.IsEmpty() == true;
	success &= rb.IsFull() == false;
	auto sz = rb.Size();
	for (auto i = 0; i < sz - 1; ++i)
	{
		success &= rb.AvailableToRead() == i;
		success &= rb.AvailableToWrite() == sz - i;
		success &= rb.Put(buffer[writer]) == true;
		writer = (writer + 1) & 1023;
		success &= rb.IsEmpty() == false;
		success &= rb.IsFull() == false;
	}

	success &= rb.Put(buffer[writer]) == true;
	writer = (writer + 1) & 1023;
	success &= rb.AvailableToRead() == sz;
	success &= rb.AvailableToWrite() == 0;
	success &= rb.IsEmpty() == false;
	success &= rb.IsFull() == true;

	if (!success)
		std::cout << "Error: " << typeid(RingType).name() << " failed write tests" << std::endl;

	reader = writer = 0;
	long processed = 0;
	while (processed < size)
	{
		for (auto i = 0; i < M; ++i)
		{
			rb.Put(buffer[writer]);
			writer = (writer + 1) & 1023;
		}
		for (auto i = 0; i < M; ++i)
		{
			auto b = buffer[reader];
			if (!rb.Get(buffer[reader]))
				throw std::logic_error("");
			if (b != buffer[reader])
				throw std::logic_error("");
			reader = (reader + 1) & 1023;
		}
		processed += M;
	}

	// check matches
	rnd.seed = 0x12345;
	for (auto i = 0; i < sizeof(buffer); ++i)
		success &= ((uint8_t)buffer[i]) == (rnd.Next() & 255);
	if (!success)
		std::cout << "Error: mismatch!" << std::endl;

	return success;
}

template<
	// sizes of ring buffers
	size_t Nmin, size_t Nmax, size_t Ndelta, 
	size_t Mmin, size_t Mmax, size_t Mdelta,
	typename RingType>
class SanityHelper
{
public:
	bool Run(long size)
	{
		if (Mmin < Nmin)
		{
			auto success = SanityCheck<Nmin,Mmin,RingType>(size);
			if (success && Nmin + Ndelta <= Nmax)
			{
				SanityHelper<
					Nmin + Ndelta, Nmax, Ndelta,
					Mmin, Mmax, Mdelta,
					RingType> 
					s;
				success &= s.Run(size);
			}
			if (success && Mmin + Mdelta <= Mmax)
			{
				SanityHelper<
					Nmin, Nmax, Ndelta,
					Mmin + Mdelta, Mmax, Mdelta,
					RingType>
					s;
				success &= s.Run(size);
			}
			return success;
		}
		return true;
	}
};

template<
	// sizes of ring buffers
	size_t Nmin, size_t Nmax, size_t Ndelta,
	size_t Mmin, size_t Mmax, size_t Mdelta,
	typename RingType>
bool RunSanity(long size)
{
	SanityHelper<
		Nmin,Nmax,Ndelta,
		Mmin,Mmax,Mdelta,
		RingType> 
	helper;
	return helper.Run(size);
}
