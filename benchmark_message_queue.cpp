#include "RingBuffer_v1.hpp"
#include "RingBuffer_v2.hpp"
#include "BlockingQueue.hpp"
#include "spmc.hpp"
#include <iostream>
#include <thread>
#include "defs.hpp"

void producer_spmc(SPMC<int>& queue, std::atomic<bool>& running)
{
  u64 id = 0;
  while (running) {
    if (queue.tryPush(id))
        id++;
  }
}

void consumer_spmc(SPMC<int>& queue, std::atomic<bool>& running, std::atomic<u64> & messageCount) {
  int value;
  while (running) {
    if (queue.tryPop(value)) {
      messageCount.fetch_add(1, std::memory_order_relaxed);
    }
  }
}

template<typename Q>
void producer_ring_buffer(Q& queue, std::atomic<bool>& running) {
  u64 id = 0;
  while (running) {
    queue.write(id++);
  }
}

template<typename Q>
void consumer_ring_buffer(Q& queue, std::atomic<bool>& running, std::atomic<u64> & messageCount) {
  u64 id = queue.getIdx();
  // u64 id = 0;
  int value;
  while (running) {
    if (queue.read(id, value)) {
      id = queue.advance(id);
      messageCount.fetch_add(1, std::memory_order_relaxed);
    }
  }
}


void blockingProducer(BlockingQueue<int>& queue, std::atomic<bool> & running) {
  u64 id = 0;
  while (running) {
    queue.push(id);
    id++;
  }
}

void blockingConsumer(BlockingQueue<int>& queue, std::atomic<bool> & running, std::atomic<u64> & messageCount) {
  while (running) {
    int value;
    auto result = queue.pop(value);
    if (result) {
      messageCount.fetch_add(1, std::memory_order_relaxed);
    }
  }
}

template <typename Q, typename P, typename C>
void runBenchmark(const std::string& name, Q& queue, P producerFunc, C consumerFunc,
                  int numProducers, int numConsumers, int duration, bool normalize)
{
  std::atomic<u64> messageCount{0};
  auto producerWrapper = [&] (std::atomic<bool>&running) {
    producerFunc(queue, running);
  };

  auto consumerWrapper = [&] (std::atomic<bool> & running) {
    consumerFunc(queue, running, messageCount);
  };

  std::atomic<bool> running(true);
  std::vector<std::thread> producerThreads;
  std::vector<std::thread> consumerThreads;

  for (int i = 0; i < numProducers; ++i) {
    producerThreads.emplace_back(producerWrapper, std::ref(running));
  }

  for (int i = 0; i < numConsumers; ++i) {
    consumerThreads.emplace_back(consumerWrapper, std::ref(running));
  }

  std::this_thread::sleep_for(std::chrono::seconds(duration));
  running = false;

  for (auto& t : producerThreads) {
    t.join();
  }
  for (auto& t : consumerThreads) {
    t.join();
  }
  if (!normalize)
    std::cout << name << ": Total messages/μs processed: " << messageCount.load() / double(1'000'000*duration) << std::endl;
  else {
    std::cout << name <<  ": Avg messages/μs per consumer: " << (messageCount.load() / (double)numConsumers/(duration*1'000'000)) << std::endl;
  }
}

void test_blocking(size_t queue_size, int numConsumers, int duration) {
  BlockingQueue<int> q(queue_size);
  runBenchmark<>("Blocking", q, blockingProducer, blockingConsumer, 1, numConsumers, duration, /*normalize*/ false);
}

void test_spmc(size_t queue_size, int numConsumers, int duration) {
  SPMC<int> q(queue_size);
  runBenchmark<>("SPMC", q, producer_spmc, consumer_spmc, 1, numConsumers, duration, /*normalize*/ false);
}

void test_v1(size_t queue_size, int numConsumers, int duration) {
  v1::RingBuffer<int> q(queue_size);
  runBenchmark<>("Ring Buffer v1", q, producer_ring_buffer<v1::RingBuffer<int>>,
                 consumer_ring_buffer<v1::RingBuffer<int>>, 1, numConsumers, duration, /*normalize*/  true);
}

void test_v2(size_t queue_size, int numConsumers, int duration) {
  v2::RingBuffer<int> q(queue_size);
  runBenchmark<>("Ring Buffer v2", q, producer_ring_buffer<v2::RingBuffer<int>>,
                 consumer_ring_buffer<v2::RingBuffer<int>>, 1, numConsumers, duration, /*normalize*/  true);
}

auto main() -> int {
  int duration = 20;  // seconds
  int queue_size = 1024;
  int num_consumers = 4;
  std::cout << "Capacity = " << queue_size << " Consumers = " << std::to_string(num_consumers) << " duration = " << duration << " s" << std::endl;
  test_blocking(queue_size, num_consumers, duration);
  test_spmc(queue_size, num_consumers, duration);
  test_v1(queue_size, num_consumers, duration);
  test_v2(queue_size, num_consumers, duration);

  return 0;
}
