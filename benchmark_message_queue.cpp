#include <unordered_map>
#include <vector>
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <boost/lockfree/queue.hpp>
#include "defs.hpp"
#include "RingBuffer_v1.hpp"
#include "RingBuffer_v2.hpp"
#include "BlockingQueue.hpp"
#include "spmc.hpp"

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
  u64 msg_count_local = 0;
  while (running) {
    if (queue.tryPop(value)) {
      msg_count_local++;
    }
  }
  messageCount.fetch_add(msg_count_local, std::memory_order_relaxed);
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
  u64 msg_count_local = 0;
  int value;
  while (running) {
    if (queue.read(id, value)) {
      id = queue.advance(id);
      msg_count_local++;
    }
  }
  messageCount.fetch_add(msg_count_local, std::memory_order_relaxed);
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

void boost_producer(boost::lockfree::queue<int>& queue, std::atomic<bool>& running) {
    int id = 0;
    while (running) {
        while (!queue.push(id)) {
            std::this_thread::yield();
        }
        id++;
    }
}

void boost_consumer(boost::lockfree::queue<int>& queue, std::atomic<bool>& running, std::atomic<u64>& messageCount) {
    int data;
    u64 msg_count_local = 0;
    while (running) {
        if (queue.pop(data)) {
            msg_count_local++;
        } else {
            std::this_thread::yield();
        }
    }
  messageCount.fetch_add(msg_count_local, std::memory_order_relaxed);
}

template <typename Q, typename P, typename C>
double runBenchmark(const std::string& name, Q& queue, P producerFunc, C consumerFunc,
                    int numConsumers, float duration, bool normalize)
{
  std::atomic<u64> messageCount{0};
  std::chrono::high_resolution_clock::time_point start;
  auto producerWrapper = [&] (std::atomic<bool>&running) {
    start = std::chrono::high_resolution_clock::now();
    producerFunc(queue, running);
  };

  auto consumerWrapper = [&] (std::atomic<bool> & running) {
    consumerFunc(queue, running, messageCount);
  };

  std::atomic<bool> running(true);
  std::vector<std::thread> producerThreads;
  std::vector<std::thread> consumerThreads;

  producerThreads.emplace_back(producerWrapper, std::ref(running));

  for (int i = 0; i < numConsumers; ++i) {
    consumerThreads.emplace_back(consumerWrapper, std::ref(running));
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(int(duration*1000)));
  running = false;
  auto end = std::chrono::high_resolution_clock::now();
  auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

  for (auto& t : producerThreads) {
    t.join();
  }
  for (auto& t : consumerThreads) {
    t.join();
  }
  double bandwidth = messageCount.load() / double(duration_us);
  if (!normalize)
    std::cout << name << ": reads/μs (total per all consumers) " << bandwidth << std::endl;
  else {
    std::cout << name <<  ": reads/μs (avg per consumer): " << bandwidth / numConsumers << std::endl;
  }
  return bandwidth;
}

auto test_blocking(size_t queue_size, int numConsumers, float duration) {
  BlockingQueue<int> q(queue_size);
  return runBenchmark<>("Blocking", q, blockingProducer, blockingConsumer, numConsumers, duration, /*normalize*/ false);
}

auto test_spmc(size_t queue_size, int numConsumers, float duration) {
  SPMC<int> q(queue_size);
  return runBenchmark<>("SPMC", q, producer_spmc, consumer_spmc, numConsumers, duration, /*normalize*/ false);
}

auto test_v1(size_t queue_size, int numConsumers, float duration) {
  v1::RingBuffer<int> q(queue_size);
  return runBenchmark<>("Ring Buffer v1", q, producer_ring_buffer<v1::RingBuffer<int>>,
                        consumer_ring_buffer<v1::RingBuffer<int>>, numConsumers, duration, /*normalize*/  true);
}

auto test_v2(size_t queue_size, int numConsumers, float duration) {
  v2::RingBuffer<int> q(queue_size);
  return runBenchmark<>("Ring Buffer v2", q, producer_ring_buffer<v2::RingBuffer<int>>,
                        consumer_ring_buffer<v2::RingBuffer<int>>, numConsumers, duration, /*normalize*/  true);
}

auto test_boost(size_t queue_size, int numConsumers, float duration) {
  boost::lockfree::queue<int> q(queue_size);
  return runBenchmark<>("Boost", q, boost_producer, boost_consumer, numConsumers, duration, /*normalize*/  false);
}

auto print_summary(std::unordered_map<std::string, std::vector<double>>& summary) {
  std::cout << "\nSummary:\n";
  std::vector<std::string> labels {"Num Consumers", "Blocking Queue", "Custom SPMC Queue", "Boost Lock-Free", "Ring Buffer v1", "Ring Buffer v2"};

  for (auto label : labels) {
    if (!summary.count(label)) continue;
    std::cout << std::left << std::setw(20) << label << ": ";
    for (auto v : summary[label]) {
      std::cout << std::left << std::setw(10) << std::setprecision(2) << std::fixed << v << " ";
    }
    std::cout << std::endl;
  }
}

auto main(int argc, char *argv[]) -> int {

  float duration = 20;  // seconds
  if (argc > 1) {
    duration = std::stof(argv[1]);
  }

  int queue_size = 1024;

  std::unordered_map<std::string, std::vector<double>> summary;
  if (argc <= 2)
  {
    auto max_threads = std::thread::hardware_concurrency();
    std::cout << "Hardware concurrency: " << max_threads << std::endl;

    for (u64 i = 1; i + 2 <= max_threads; i++) {
      summary["Num Consumers"].push_back(i);
    }
  }
  else {
    summary["Num Consumers"].push_back(std::stoi(argv[2]));
  }

  std::cout << "This program will run " << duration << " s benchmarks "
            << "Queue capacity = " << queue_size << "\n"
            << "for the following number of consumers: ";
  for (auto i : summary["Num Consumers"]) {
    std::cout << i << " ";
  }
  std::cout << std::endl;

  for (int num_consumers : summary["Num Consumers"]) {
    std::cout << "\nConsumers = " << std::to_string(num_consumers) << std::endl;
    summary["Blocking Queue"] .push_back( test_blocking(queue_size, num_consumers, duration) );
    summary["Custom SPMC Queue"] .push_back( test_spmc(queue_size, num_consumers, duration));
    summary["Boost Lock-Free"] .push_back( test_boost(queue_size, num_consumers, duration));
    summary["Ring Buffer v1"] .push_back( test_v1(queue_size, num_consumers, duration));
    summary["Ring Buffer v2"] .push_back( test_v2(queue_size, num_consumers, duration));
  }

  print_summary(summary);

  return EXIT_SUCCESS;
}
