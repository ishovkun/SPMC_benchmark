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
  std::cout << name << " benchmark completed." << std::endl;

  std::cout << name << ": " << " blocks " << std::to_string(numProducers) << " producer  " << std::to_string(numConsumers) << " consumers  " << std::to_string(duration) << " seconds" << std::endl;
  if (!normalize)
    std::cout << "Total messages/μs processed: " << messageCount.load() / double(1'000'000*duration) << "\n" << std::endl;
  else {
    std::cout << "Avg messages/μs per consumer: " << (messageCount.load() / (double)numConsumers/(duration*1'000'000))  << "\n" << std::endl;
  }
}

void test_blocking(size_t queue_size, std::vector<int> const & numConsumers, int duration) {
  for (size_t i = 0; i < numConsumers.size(); i++) {
    BlockingQueue<int> q(queue_size);
    runBenchmark<>("Blocking", q, blockingProducer, blockingConsumer, 1, numConsumers[i], duration, /*normalize*/ false);
  }
}

void test_spmc(size_t queue_size, std::vector<int> const & numConsumers, int duration) {
  for (size_t i = 0; i < numConsumers.size(); i++) {
    SPMC<int> q(queue_size);
    runBenchmark<>("SPMC", q, producer_spmc, consumer_spmc, 1, numConsumers[i], duration, /*normalize*/ false);
  }
}

void test_v1(size_t queue_size, std::vector<int> const & numConsumers, int duration) {
  for (size_t i = 0; i < numConsumers.size(); i++) {
    v1::RingBuffer<int> q(queue_size);
    runBenchmark<>("Version 1", q, producer_ring_buffer<v1::RingBuffer<int>>,
                   consumer_ring_buffer<v1::RingBuffer<int>>, 1, numConsumers[i], duration, /*normalize*/  true);
  }
}

void test_v2(size_t queue_size, std::vector<int> const & numConsumers, int duration) {
  for (size_t i = 0; i < numConsumers.size(); i++) {
    v2::RingBuffer<int> q(queue_size);
    runBenchmark<>("Version 1", q, producer_ring_buffer<v2::RingBuffer<int>>,
                   consumer_ring_buffer<v2::RingBuffer<int>>, 1, numConsumers[i], duration, /*normalize*/  true);
  }
}

// auto main(int argc, char *argv[]) -> int {
auto main() -> int {
  int duration = 15;  // seconds
  int queue_size = 1024;
  // std::vector<int> num_consumers{1, 2};
  std::vector<int> num_consumers{4};
  // test_blocking(queue_size, num_consumers, duration);
  // test_spmc(queue_size, num_consumers, duration);
  test_v1(queue_size, num_consumers, duration);
  test_v2(queue_size, num_consumers, duration);


  return 0;
}
