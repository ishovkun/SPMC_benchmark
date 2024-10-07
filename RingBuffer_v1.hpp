#pragma once
#include <memory>
#include <atomic>
#include <cassert>
#include <iostream>
#include "defs.hpp"

namespace v1 {

template <typename T>
class RingBuffer
{
  std::atomic<u32> _index;
  std::atomic<u32> _pending;
  std::unique_ptr<T[]> _data;
  u32 _index_writer;
  u32 _capacity;
public:
  RingBuffer(u32 size)
  {
    _index.store(0);
    _pending.store(0);
    _capacity = size;
    _index_writer = 0;
    _data = std::make_unique<T[]>(size);
  }

  auto advance(u64 idx) -> u64 {
    return (idx == _capacity - 1) ? 0 : (idx + 1);
  }

  void write(T&& data)
  {
    _pending.store(advance(_pending.load(std::memory_order_acquire)), std::memory_order_release);
    auto writeIdx = _index.load(std::memory_order_acquire);
    // auto writeIdx = _index_writer;
    _data[writeIdx] = std::move(data);
    // _index_writer = advance(_index_writer);
    // _index.store(_index_writer, std::memory_order_release);
    _index.store(advance(writeIdx), std::memory_order_release);
  }

  bool read(u32 readIdx, T& data)
  {
    if (readIdx == _index.load(std::memory_order_acquire) || readIdx == _pending.load(std::memory_order_acquire)) {
      return false;
    }
    {
      data = _data[readIdx];
      return true;
    }
    // else throw std::runtime_error("Reader too slow: aborting");
  }

  u32 getIdx() const
  {
    return _index.load(std::memory_order_acquire);
  }
};

}  // end namespace v1
