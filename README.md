# Benchmarking Single Producer Multiple Consumer (SPMC) queues

The purpose of the repository is to compare the throughput of implementations of
SPMC queues. Let's meet the contenders:

- Blocking queue (std::mutex, std::unique_lock)
- SPMC lock-free queue
- SPMC wait free ring buffer (v1). Not strictly a queue but a ring buffer where readers do not block one another
- SPMC wait free ring buffer (v2). Same interface as above but a different synchronization mechanism.

Inspired by
https://github.com/rezabrizi/SPMC-Queue/blob/main/README.md
and
https://youtu.be/8uAW5FQtcvE
