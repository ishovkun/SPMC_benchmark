# Benchmarking Single Producer Multiple Consumer (SPMC) queues

The purpose of the repository is to compare the throughput of implementations of
SPMC queues. Let's meet the contenders:

- Blocking queue (std::mutex, std::unique_lock)
- SPMC lock-free queue
- SPMC wait free ring buffer (v1). Not strictly a queue but a ring buffer where readers do not block one another
- SPMC wait free ring buffer (v2). Same interface as above but a different synchronization mechanism.

## Results
This is the results of my benchmark measured on Intel Xeon Gold 5418Y.

![Intel XEON Gold 5418Y](img/benchmark_intel.png)


This is the measurement on my Mac M2.
![Apple M2](img/benchmark_mac.png)

## References
Inspired by
![rezabrizi's repo](https://youtu.be/8uAW5FQtcvE) and 
![David Gross' talk](https://github.com/rezabrizi/SPMC-Queue/blob/main/README.md).
