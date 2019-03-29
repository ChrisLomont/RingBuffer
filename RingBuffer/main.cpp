#if 0
#include <iostream>
#include <vector>
#include <thread>

#ifdef __SAMD21G18A__
#define SAMD21_BUILD
#endif

// Ring buffer - 
#include "SimpleRingBuffer.h"  // single thread, simple
#include "GenericRingBuffer.h" // template things, including size
#include "LockedRingBuffer.h"  // add locks to make threaded
#include "AtomicsRingBuffer.h" // for SPSC, can replace locks with atomics
#include "ModulusRingBuffer.h" // remove modulus, use if, or power of 2 tricks
#include "RelaxedRingBuffer.h" // relax the atomics load/store memory ordering
#include "FullRingBuffer.h"    // replace mod N with mod 2N, uses all N buffer slots
#include "CacheRingBuffer.h"   // move read/write to other locations for cache help
#include "BlocksRingBuffer.h"  // add read/write in blocks
#include "RingBuffer.h"        // add predictive read/write locations to loosen false sharing

// send output here
void WriteLine(const char * line);
void Write(const char * line);
void Error(const char * line);

#include "Tests.h"

using namespace Lomont;
using namespace std;

void Write(const char * line)
{
#ifndef SAMD21_BUILD
    cout << line;
#endif
}

void WriteLine(const char * line)
{
#ifndef SAMD21_BUILD
    cout << line << endl;
#endif
}
void Error(const char * line)
{
    #ifndef SAMD21_BUILD
    WriteLine(line);
    #endif
}


void PerformanceI(int bytes)
{ // Simple,Generic,Locked, all single threaded

	WriteLine("Performance I");
	ThroughputSingle<29,  16, SimpleRingBuffer <29>>(bytes);
	ThroughputSingle<29,  16, GenericRingBuffer<29>>(bytes);
	ThroughputSingle<29,  16, LockedRingBuffer <29>>(bytes/5);
						  
	ThroughputSingle<32,  16, SimpleRingBuffer <32>>(bytes);
	ThroughputSingle<32,  16, GenericRingBuffer<32>>(bytes);  
	ThroughputSingle<32,  16, LockedRingBuffer <32>>(bytes/5);  
						  
	ThroughputSingle<50,  16, SimpleRingBuffer <50>>(bytes);
	ThroughputSingle<50,  16, GenericRingBuffer<50>>(bytes);
	ThroughputSingle<50,  16, LockedRingBuffer <50>>(bytes/5);

	ThroughputSingle<128, 16, SimpleRingBuffer <128>>(bytes);
	ThroughputSingle<128, 16, GenericRingBuffer<128>>(bytes);
	ThroughputSingle<128, 16, LockedRingBuffer <128>>(bytes/5);

	WriteLine("Performance I - near power of 2");

	ThroughputSingle<126, 19, SimpleRingBuffer <126>>(bytes);
	ThroughputSingle<126, 19, GenericRingBuffer<126>>(bytes);
	ThroughputSingle<127, 19, SimpleRingBuffer <127>>(bytes);
	ThroughputSingle<127, 19, GenericRingBuffer<127>>(bytes);
	ThroughputSingle<128, 19, SimpleRingBuffer <128>>(bytes);
	ThroughputSingle<128, 19, GenericRingBuffer<128>>(bytes);
	ThroughputSingle<129, 19, SimpleRingBuffer <129>>(bytes);
	ThroughputSingle<129, 19, GenericRingBuffer<129>>(bytes);
	ThroughputSingle<130, 19, SimpleRingBuffer <130>>(bytes);
	ThroughputSingle<130, 19, GenericRingBuffer<130>>(bytes);
}

void PerformanceII(int bytes)
{   // Generic, Locked, Atomics, single and double threaded

	WriteLine("Performance II");
	ThroughputSingle<29, 16, GenericRingBuffer<29>>(bytes);
	ThroughputSingle<29, 16, LockedRingBuffer <29>>(bytes/5);
	ThroughputSingle<29, 16, AtomicsRingBuffer<29>>(bytes);
						 
	ThroughputSingle<32, 16, GenericRingBuffer<32>>(bytes);
	ThroughputSingle<32, 16, LockedRingBuffer <32>>(bytes/5);
	ThroughputSingle<32, 16, AtomicsRingBuffer<32>>(bytes);

	ThroughputSingle<50 ,16, GenericRingBuffer<50>>(bytes);
	ThroughputSingle<50, 16, LockedRingBuffer <50>>(bytes/5);
	ThroughputSingle<50, 16, AtomicsRingBuffer<50>>(bytes);

	ThroughputSingle<128, 16, GenericRingBuffer<128>>(bytes);
	ThroughputSingle<128, 16, LockedRingBuffer <128>>(bytes/5);
	ThroughputSingle<128, 16, AtomicsRingBuffer<128>>(bytes);


	WriteLine("Performance II - throughput 2 threads");

	ThroughputDouble<128, 16, LockedRingBuffer <128>>(bytes / 500);
	ThroughputDouble<128, 16, AtomicsRingBuffer<128>>(bytes / 5);
}

void PerformanceIII(int bytes)
{   // Atomics, modulus, relaxed, single and double threaded

	WriteLine("Performance III - modulus");
	ThroughputSingle<127, 16, AtomicsRingBuffer <127>>(bytes);
	ThroughputSingle<127, 16, ModulusRingBuffer <127, char, uint32_t, SlowRingMod<127, uint32_t>>>(bytes);
	ThroughputSingle<127, 16, ModulusRingBuffer <127, char, uint32_t, MidRingMod<127, uint32_t>>>(bytes);
	ThroughputSingle<127, 16, ModulusRingBuffer <127, char, uint32_t, FastRingMod<127, uint32_t>>>(bytes);

	ThroughputSingle<128, 16, AtomicsRingBuffer <128>>(bytes);
	ThroughputSingle<128, 16, ModulusRingBuffer <128, char, uint32_t, SlowRingMod<128, uint32_t>>>(bytes);
	ThroughputSingle<128, 16, ModulusRingBuffer <128, char, uint32_t, MidRingMod<128, uint32_t>>>(bytes);
	ThroughputSingle<128, 16, ModulusRingBuffer <128, char, uint32_t, FastRingMod<128, uint32_t>>>(bytes);

	ThroughputSingle<129, 16, AtomicsRingBuffer <129>>(bytes);
	ThroughputSingle<129, 16, ModulusRingBuffer <129, char, uint32_t, SlowRingMod<129, uint32_t>>>(bytes);
	ThroughputSingle<129, 16, ModulusRingBuffer <129, char, uint32_t, MidRingMod<129, uint32_t>>>(bytes);
	ThroughputSingle<129, 16, ModulusRingBuffer <129, char, uint32_t, FastRingMod<129, uint32_t>>>(bytes);

	WriteLine("Performance III - all single");
	ThroughputSingle<29, 16, AtomicsRingBuffer<29>>(bytes);
	ThroughputSingle<29, 16, ModulusRingBuffer<29>>(bytes);
	ThroughputSingle<29, 16, RelaxedRingBuffer<29>>(bytes);

	ThroughputSingle<32, 16, AtomicsRingBuffer<32>>(bytes);
	ThroughputSingle<32, 16, ModulusRingBuffer <32>>(bytes);
	ThroughputSingle<32, 16, RelaxedRingBuffer<32>>(bytes);

	ThroughputSingle<50, 16, AtomicsRingBuffer<50>>(bytes);
	ThroughputSingle<50, 16, ModulusRingBuffer <50>>(bytes);
	ThroughputSingle<50, 16, RelaxedRingBuffer<50>>(bytes);

	ThroughputSingle<128, 16, AtomicsRingBuffer<128>>(bytes);
	ThroughputSingle<128, 16, ModulusRingBuffer <128>>(bytes);
	ThroughputSingle<128, 16, RelaxedRingBuffer<128>>(bytes);

	WriteLine("Performance III - all double");
	bytes /= 4;
	ThroughputDouble<29, 16, AtomicsRingBuffer<29>>(bytes/2);
	ThroughputDouble<29, 16, ModulusRingBuffer<29>>(bytes/2);
	ThroughputDouble<29, 16, RelaxedRingBuffer<29>>(bytes);
	ThroughputDouble<32, 16, AtomicsRingBuffer<32>>(bytes/2);
	ThroughputDouble<32, 16, ModulusRingBuffer <32>>(bytes/2);
	ThroughputDouble<32, 16, RelaxedRingBuffer<32>>(bytes);
	ThroughputDouble<50, 16, AtomicsRingBuffer<50>>(bytes/2);
	ThroughputDouble<50, 16, ModulusRingBuffer <50>>(bytes/2);
	ThroughputDouble<50, 16, RelaxedRingBuffer<50>>(bytes);
	ThroughputDouble<128, 16, AtomicsRingBuffer<128>>(bytes/2);
	ThroughputDouble<128, 16, ModulusRingBuffer <128>>(bytes/2);
	ThroughputDouble<128, 16, RelaxedRingBuffer<128>>(bytes);
}

void PerformanceIV(int bytes)
{   // Relaxed, full size, cache, blocks, predictive
	WriteLine("Performance IV - single");
	bytes *= 8;
	bytes /= 3;
	ThroughputSingle<29, 16, RelaxedRingBuffer<29>>(bytes);
	ThroughputSingle<29, 16, FullRingBuffer   <29>>(bytes);
	ThroughputSingle<29, 16, CacheRingBuffer  <29>>(bytes);
	ThroughputSingle<29, 16, BlocksRingBuffer <29>>(bytes);
	ThroughputSingle<29, 16, RingBuffer       <29>>(bytes);

	ThroughputSingle<32, 16, RelaxedRingBuffer<32>>(bytes);
	ThroughputSingle<32, 16, FullRingBuffer   <32>>(bytes);
	ThroughputSingle<32, 16, CacheRingBuffer  <32>>(bytes);
	ThroughputSingle<32, 16, BlocksRingBuffer <32>>(bytes);
	ThroughputSingle<32, 16, RingBuffer       <32>>(bytes);

	ThroughputSingle<50, 16, RelaxedRingBuffer<50>>(bytes);
	ThroughputSingle<50, 16, FullRingBuffer   <50>>(bytes);
	ThroughputSingle<50, 16, CacheRingBuffer  <50>>(bytes);
	ThroughputSingle<50, 16, BlocksRingBuffer <50>>(bytes);
	ThroughputSingle<50, 16, RingBuffer       <50>>(bytes);

	ThroughputSingle<128, 16, RelaxedRingBuffer<128>>(bytes);
	ThroughputSingle<128, 16, FullRingBuffer   <128>>(bytes);
	ThroughputSingle<128, 16, CacheRingBuffer  <128>>(bytes);
	ThroughputSingle<128, 16, BlocksRingBuffer <128>>(bytes);
	ThroughputSingle<128, 16, RingBuffer       <128>>(bytes);

	WriteLine("Performance IV - double");
	bytes /= 10;

	ThroughputDouble<29, 16, RelaxedRingBuffer<29>>(bytes);
	ThroughputDouble<29, 16, FullRingBuffer   <29>>(bytes);
	ThroughputDouble<29, 16, CacheRingBuffer  <29>>(bytes);
	ThroughputDouble<29, 16, BlocksRingBuffer <29>>(bytes);
	ThroughputDouble<29, 16, RingBuffer       <29>>(bytes);

	ThroughputDouble<32, 16, RelaxedRingBuffer<32>>(bytes);
	ThroughputDouble<32, 16, FullRingBuffer   <32>>(bytes);
	ThroughputDouble<32, 16, CacheRingBuffer  <32>>(bytes);
	ThroughputDouble<32, 16, BlocksRingBuffer <32>>(bytes);
	ThroughputDouble<32, 16, RingBuffer       <32>>(bytes);

	ThroughputDouble<50, 16, RelaxedRingBuffer<50>>(bytes);
	ThroughputDouble<50, 16, FullRingBuffer   <50>>(bytes);
	ThroughputDouble<50, 16, CacheRingBuffer  <50>>(bytes);
	ThroughputDouble<50, 16, BlocksRingBuffer <50>>(bytes);
	ThroughputDouble<50, 16, RingBuffer       <50>>(bytes);

	ThroughputDouble<128, 16, RelaxedRingBuffer<128>>(bytes);
	ThroughputDouble<128, 16, FullRingBuffer   <128>>(bytes);
	ThroughputDouble<128, 16, CacheRingBuffer  <128>>(bytes);
	ThroughputDouble<128, 16, BlocksRingBuffer <128>>(bytes);
	ThroughputDouble<128, 16, RingBuffer       <128>>(bytes);
}

void PerformanceV(int bytes)
{   // Blocks, predictive
	bytes *= 8;
	bytes /= 2;

	WriteLine("Performance IV - one item vs blocks");

	ThroughputSingle<128, 16, BlocksRingBuffer <128>>(bytes);
	ThroughputSingle<128, 16, RingBuffer       <128>>(bytes);
	ThroughputSingleBlock<128, 16, BlocksRingBuffer <128>>(bytes*3);
	ThroughputSingleBlock<128, 16, RingBuffer       <128>>(bytes*3);

	bytes /= 6;
	ThroughputDouble<128, 16, BlocksRingBuffer <128>>(bytes/3);
	ThroughputDouble<128, 16, RingBuffer       <128>>(bytes/3);
	ThroughputDoubleBlock<128, 16, BlocksRingBuffer <128>>(bytes*4);
	ThroughputDoubleBlock<128, 16, RingBuffer       <128>>(bytes*4);
}

void PerformanceVI(int bytes)
{   // final all
	
	constexpr size_t N = 128;
	constexpr size_t M = 16;
	bytes /= 3;

	WriteLine("Performance VI - single all");
	ThroughputSingle<N, M, SimpleRingBuffer <N>>(bytes * 3);    // single thread, simple to implement
	ThroughputSingle<N, M, GenericRingBuffer<N>>(bytes * 10);   // Templatized things									  
	ThroughputSingle<N, M, LockedRingBuffer <N>>(bytes / 5);    // added locks, API now bad
	ThroughputSingle<N, M, AtomicsRingBuffer<N>>(bytes);        // SPSC using atomics
	ThroughputSingle<N, M, ModulusRingBuffer<N>>(bytes);        // power of 2 specialized
	ThroughputSingle<N, M, RelaxedRingBuffer<N>>(bytes * 20);   // relaxed atomic memory model
	ThroughputSingle<N, M, FullRingBuffer   <N>>(bytes * 20);   // use all items
	ThroughputSingle<N, M, CacheRingBuffer  <N>>(bytes * 20);   // cache lines
	ThroughputSingleBlock<N, M, BlocksRingBuffer <N>>(bytes * 20);   // read/write blocks
	ThroughputSingleBlock<N, M, RingBuffer       <N>>(bytes * 20);   // added predictive read/write to avoid false sharing

	WriteLine("Performance VI - double all");
	bytes /= 5;
	ThroughputDouble<N, M, LockedRingBuffer <N>>(bytes / 30);   // added locks, API now bad
	ThroughputDouble<N, M, AtomicsRingBuffer<N>>(bytes);        // SPSC using atomics
	ThroughputDouble<N, M, ModulusRingBuffer<N>>(bytes);        // power of 2 specialized
	ThroughputDouble<N, M, RelaxedRingBuffer<N>>(bytes * 2);   // relaxed atomic memory model
	ThroughputDouble<N, M, FullRingBuffer   <N>>(bytes * 2);   // use all items
	ThroughputDouble<N, M, CacheRingBuffer  <N>>(bytes * 2);   // cache lines
	ThroughputDoubleBlock<N, M, BlocksRingBuffer <N>>(bytes * 20);   // read/write blocks
	ThroughputDoubleBlock<N, M, RingBuffer       <N>>(bytes * 20);   // added predictive read/write to avoid false sharing


}


bool TestSanity(long size)
{
	auto success = true;
	success &= SanityCheck<30, 3, RingBuffer<30>>(size);
	success &= SanityCheck<31, 3, RingBuffer<31>>(size);
	success &= SanityCheck<32, 3, RingBuffer<32>>(size);
	success &= SanityCheck<33, 3, RingBuffer<33>>(size);
	if (success)
		WriteLine("Sanity passed");
	else
		WriteLine("ERROR: Sanity failed");
	return success;
}

// test all ringbuffers single threaded
void TestAllSingle()
{
	constexpr size_t N = 256;
	constexpr size_t M = 16;
	long size = 1'000'000; // pick sizes so that each sample around 25 ms
	ThroughputSingle<N, M, SimpleRingBuffer <N>>(size*3);   // single thread, simple to implement
	ThroughputSingle<N, M, GenericRingBuffer<N>>(size*10);   // Templatized things									  
	ThroughputSingle<N, M, LockedRingBuffer <N>>(size/5);   // added locks, API now bad
	ThroughputSingle<N, M, AtomicsRingBuffer<N>>(size);   // SPSC using atomics
	ThroughputSingle<N, M, ModulusRingBuffer<N, char, uint32_t, SlowRingMod<N, uint32_t>>>(size); // replaced modulus, old method
	ThroughputSingle<N, M, ModulusRingBuffer<N, char, uint32_t, MidRingMod<N, uint32_t>>>(size);  // mod replace with 'if'
	ThroughputSingle<N, M, ModulusRingBuffer<N, char, uint32_t, FastRingMod<N, uint32_t>>>(size); // power of 2 specialized
	ThroughputSingle<N, M, RelaxedRingBuffer<N>>(size*20);   // relaxed atomic memory model
	ThroughputSingle<N, M, FullRingBuffer   <N>>(size*20);   // use all items
	ThroughputSingle<N, M, CacheRingBuffer  <N>>(size*20);   // cache lines
	ThroughputSingle<N, M, BlocksRingBuffer <N>>(size*20);   // read/write blocks
	ThroughputSingle<N, M, RingBuffer       <N>>(size*20);   // added predictive read/write to avoid false sharing
}

// test all ringbuffers double threaded that can be
void TestAllDouble()
{
	constexpr size_t N = 256;
	constexpr size_t M = 16;
	long size = 500'000; // pick sizes so that each sample around 25 ms
	ThroughputDouble<N, M, LockedRingBuffer <N>>(size / 50);   // added locks, API now bad
	ThroughputDouble<N, M, AtomicsRingBuffer<N>>(size);   // SPSC using atomics
	ThroughputDouble<N, M, ModulusRingBuffer<N, char, uint32_t, SlowRingMod<N, uint32_t>>>(size); // replaced modulus, old method
	ThroughputDouble<N, M, ModulusRingBuffer<N, char, uint32_t, MidRingMod<N, uint32_t>>>(size);  // mod replace with 'if'
	ThroughputDouble<N, M, ModulusRingBuffer<N, char, uint32_t, FastRingMod<N, uint32_t>>>(size); // power of 2 specialized
	ThroughputDouble<N, M, RelaxedRingBuffer<N>>(size * 4);   // relaxed atomic memory model
	ThroughputDouble<N, M, FullRingBuffer   <N>>(size * 4);   // use all items
	ThroughputDouble<N, M, CacheRingBuffer  <N>>(size * 4);   // cache lines
	ThroughputDouble<N, M, BlocksRingBuffer <N>>(size * 4);   // read/write blocks
	ThroughputDouble<N, M, RingBuffer       <N>>(size * 4);   // added predictive read/write to avoid false sharing
}

void TestTimingBySize()
{
	constexpr size_t N = 256;
	constexpr size_t M = 140;
	ThroughputSingle<N, M, SimpleRingBuffer <N>>(100'000);   // single thread, simple to implement
	ThroughputSingle<N, M, SimpleRingBuffer <N>>(200'000);   // single thread, simple to implement
	ThroughputSingle<N, M, SimpleRingBuffer <N>>(400'000);   // single thread, simple to implement
	ThroughputSingle<N, M, SimpleRingBuffer <N>>(800'000);   // single thread, simple to implement
	ThroughputSingle<N, M, SimpleRingBuffer <N>>(1'600'000);   // single thread, simple to implement
	ThroughputSingle<N, M, SimpleRingBuffer <N>>(3'200'000);   // single thread, simple to implement
	ThroughputSingle<N, M, SimpleRingBuffer <N>>(6'400'000);   // single thread, simple to implement
	ThroughputSingle<N, M, SimpleRingBuffer <N>>(12'800'000);   // single thread, simple to implement
	ThroughputSingle<N, M, SimpleRingBuffer <N>>(25'600'000);   // single thread, simple to implement
	ThroughputSingle<N, M, SimpleRingBuffer <N>>(51'200'000);   // single thread, simple to implement
}

#ifndef SAMD21_BUILD // ARM board
int main()
{

	//todo - 
	//	TODO – can possibly change to not use 2 mod per loop – mod in array w + i, then out of loop mod2n final w + n – errors – why ? Possibly can split into values that stay in bounds, and do those – remove all per loop mod



	//FinalStressTest<126>();

    //extern void TestRelacy();
	//TestRelacy(); // do relacy thread checker testing
	//TestSanity(1000); // sanity check a buffer

	ShowLogFormat();
	
	// determined that avg ms should be 25-50 to get good samples
	// also fix sizes we test, say 256,16 in general
	// TestTimingBySize(); 

	//TestAllSingle();  // each through single threaded
	//TestAllDouble();  // each through single threaded

	// particular tests for talk slides
	int bytes = 3'000'000;
	//PerformanceI(bytes);
	//PerformanceII(bytes);
	//PerformanceIII(bytes);
	//PerformanceIV(bytes);
	//PerformanceV(bytes);
	PerformanceVI(bytes);
	return 0;
}
#endif // SAMD21_BUILD
// end of file
#endif