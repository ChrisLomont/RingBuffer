
#include <iostream>
#include <vector>
#include <thread>

// relacy thread testing
// put relacy wherever you like, point this at it
#include "../relacy_2_3/relacy/relacy.hpp"

#include "RingBuffer.h"

#ifdef RL_RELACY_HPP

// unit test, 2 is # threads for SPSC ring buffer
template<size_t N>
struct ring_test : rl::test_suite<ring_test<N>, 2>
{
	Lomont::RingBuffer<N, char, int32_t> buffer;

	rl::var<int> nextWritten_;
	rl::var<int> nextRead_;

	ring_test() {}

	int pos = 0;

	// executed first, single thread
	void before()
	{
		nextWritten_($) = 0;
		nextRead_($) = 0;
	}

	// main thread function
	void thread(unsigned int threadIndex)
	{
		if (threadIndex == 0)
		{
			//cout << "0";
			//cout << ++pos << ' ';
			auto available = buffer.Size() - buffer.AvailableToRead();
			if (!available)
				return;
			auto toWrite = rl::rand(available + 1);
			while (toWrite--)
			{
				RL_ASSERT(buffer.Put(nextWritten_($)));
				nextWritten_($)++;
			}
			//cout << buffer.AvailableToRead() << " ";
		}
		else if (threadIndex == 1)
		{
			char v;
			//cout << "1";
			//cout << --pos << ' ';
			auto available = buffer.AvailableToRead();
			if (!available)
				return;
			auto toRead = rl::rand(available + 1);
			while (toRead--)
			{
				RL_ASSERT(buffer.Get(v));
				RL_ASSERT(v == nextRead_($));
				nextRead_($)++;
			}
			//cout << buffer.AvailableToRead() << " ";
		}
	}
	// end of thread
	void after()
	{
	}
};

template<size_t N>
void Test(bool full)
{
	rl::test_params params;

	params.iteration_count = 1'000'000;

	params.search_type = rl::fair_context_bound_scheduler_type;
	rl::simulate<ring_test<N>>(params);

	params.search_type = rl::random_scheduler_type;
	rl::simulate<ring_test<N>>(params);

	if (full)
	{
		params.search_type = rl::fair_full_search_scheduler_type;
		rl::simulate<ring_test<N>>(params);
	}

}
#endif


void TestRelacy()
{
    Test<3>(true);
	Test<4>(true);
	Test<5>(true);

	Test<10>(false);
	
	Test<31>(false);
	Test<32>(false);
	Test<33>(false);
	Test<34>(false);

	Test<63>(false);
	Test<64>(false);
	Test<65>(false);

	Test<126>(false);
	Test<127>(false);
	Test<128>(false);
	Test<129>(false);
	Test<130>(false);

	Test<254>(false);
	Test<255>(false);
	Test<256>(false);
	Test<257>(false);
	Test<258>(false);

	Test<505>(false);
	Test<511>(false);
	Test<512>(false);
	Test<517>(false);
	Test<525>(false);
}