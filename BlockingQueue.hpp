#pragma once
#include <mutex>
#include <condition_variable>
#include <queue>
#include <iostream>

template <typename T, int timeout_ms = 16>
class BlockingQueue {
  std::queue<T> _queue;
  std::mutex _mutex;
  std::condition_variable _cv;
  size_t _capacity;
  static constexpr std::chrono::milliseconds timeout{timeout_ms};

 public:
  BlockingQueue(size_t cap)
      : _capacity{cap}
  {}

  void push(const T & item) {
    std::unique_lock<std::mutex> lock(_mutex);
    _cv.wait(lock, [this]() { return _queue.size() < _capacity;});
    _queue.push(item);
    lock.unlock();
    _cv.notify_one();
  }

  bool pop(T & value) {
    std::unique_lock<std::mutex> lock(_mutex);
    _cv.wait_for(lock, timeout, [this]() { return !_queue.empty();});
    if (_queue.empty()) {
      return false;
    }
    value = _queue.front();
    _queue.pop();
    lock.unlock();
    _cv.notify_all();
    return true;
  }
};
