#pragma once
#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <chrono>
#include <cstdint>

class StopWatch 
{
	static_assert(std::chrono::steady_clock::is_steady, "Fatal clock: C++ steady_clock not steady");
public:
	// using clock = std::chrono::steady_clock;
	using clock = std::chrono::high_resolution_clock; // todo - make clock selection by traits
	using nanoseconds = std::chrono::nanoseconds;
	using microseconds = std::chrono::microseconds;
	using milliseconds = std::chrono::milliseconds;
	using seconds = std::chrono::seconds;
	using duration = nanoseconds;

	// start, resets clock to zero, unless requested to keep elapsed
	void Start(bool keepElapsed = false)
	{
		isRunning_ = true;
		if (!keepElapsed)
			Reset();
		start_ = clock::now();
	}

	void Stop()
	{
		auto diff = clock::now() - start_;
		elapsed_ += clock::now() - start_;
		isRunning_ = false;
	}

	void Reset()
	{
		elapsed_ = duration::zero();
	}

	bool IsRunning() const { return isRunning_;  }


	// return elapsed nanoseconds since start
	uint64_t ElapsedNs() const
	{
		return std::chrono::duration_cast<nanoseconds>(Elapsed()).count();
	}

	// return elapsed microseconds since start
	uint64_t ElapsedUs() const 
	{
		return std::chrono::duration_cast<microseconds>(Elapsed()).count();
	}

	// return the elapsed milliseconds since start
	uint64_t ElapsedMs() const 
	{
		return std::chrono::duration_cast<milliseconds>(Elapsed()).count();
	}

	// return the elapsed seconds since start
	uint64_t ElapsedSec() const 
	{
		return std::chrono::duration_cast<seconds>(Elapsed()).count();
	}

	duration Elapsed() const
	{
		auto e = elapsed_;
		if (isRunning_)
			e += std::chrono::duration_cast<duration>(clock::now() - start_);
		return e;
	}
	
private:
	clock::time_point start_;
	duration elapsed_{ 0 };
	bool isRunning_;
};



#endif // STOPWATCH_H