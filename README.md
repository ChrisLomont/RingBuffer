# Ring Buffer

Chris Lomont 2023

This is code and slides from a talk I gave describing the design and evolution of a high-performance single producer, single consumer ring buffer. I made the talk for programmers I was teaching about nuances of memory models and threading. The ring buffer I needed for an embedded project I was doing.

Ten ring buffers are explained, each better than the last, to eke out every ounce of performance. 

[Here](Making a Ringbuffer.pptx) are PowerPoint slides for the talk. Here are results. [Excel doc with the data](Data.xlsx).

<img src="images\Results.png" alt="Results" style="zoom:67%;" />

List of the 10 improvement ideas:

1. Initial implementation
2. Made parameters generic to gain compiler optimizations
3. Made threaded with **lock**s
4. Replaced locks with careful **atomic**s
5. Replaced modulus with **if** and bitmask and template tricks
6. Replaced **seq_cst** atomics with careful load/store semantics
7. Replaced N-1 buffer with full N buffer via mod 2N math
8. Tried to arrange data for better cache and non-false sharing
9. Changed API to handle blocks instead of single put/get
10. Removed some cache sharing via predictive size buffering



Some notes

1. to help ensure correctness, I used the [Relacy race detector](https://github.com/dvyukov/relacy)
2. If you want a really good book (which I highly recommend) on multiprocessor programming, look at "[The Art of Multiprocessor Programming 2nd Edition](https://www.amazon.com/Art-Multiprocessor-Programming-Maurice-Herlihy/dp/0124159508/)," by Herlihy, Shavit, Luchangco, and Spear 
3. A good concurrency book in C++ is  "[C++ Concurrency in Action 2nd Edition](https://www.amazon.com/C-Concurrency-Action-Practical-Multithreading/dp/1933988770/)," by Anthony Williams



<img src="images\ArtOfMultiprocessorProgramming.jpg" alt="ArtOfMultiprocessorProgramming" style="zoom:50%;" />

<img src="images\ConcurrencyInAction.jpg" alt="ConcurrencyInAction" style="zoom:67%;" />