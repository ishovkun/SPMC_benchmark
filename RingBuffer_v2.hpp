#pragma once
#include <atomic>
#include <memory>
#include "defs.hpp"
#include <iostream>

namespace v2 {

template <typename T>
struct Block
{
  alignas(cache_line_size) std::atomic<u32> version;
  alignas(cache_line_size) T data;
};

template <typename T>
class RingBuffer
{
  alignas(cache_line_size) u64 _blockIdx{0};
  alignas(cache_line_size) std::unique_ptr<Block<T>[]> _blocks;
  size_t _capacity;


 public:
  auto advance(u64 idx) -> u64 {
    return (idx == _capacity - 1) ? 0 : (idx + 1);
  }

  RingBuffer(size_t size)
      : _capacity(size)
  {
    _blocks = std::make_unique<Block<T>[]>(size);
    for (size_t i = 0; i < size; i++)
    {
      _blocks[i].version.store(0, std::memory_order_relaxed);
    }
  }

  auto getIdx() const -> u64 {
    return _blockIdx;
  }

  void write(T item) {

    /*  If it is the first write, just write the data, and after the write is complete,
     *  increment the version to 1. The odd version means a read is allowed.
     * If it is a rewrite, then the version is already odd, so before writing,
     * increment the version so it is even (no reads allowed). Perform the write.
     * Once writing is done, increment the version again to make it odd (reading is allowed).
    */

    auto blockIdx = advance(_blockIdx);
    auto & block = _blocks[blockIdx];
    auto version = block.version.load(std::memory_order_acquire);
    // if ((version & 2) - 1 != 0) {
    if (version % 2) {  // TODO: use bitwse comparison
      version++;             // make even
      block.version.store(version, std::memory_order_release);
    }
    block.data = item;
    // store the new odd version
    version++;
    block.version.store(version, std::memory_order_release);
    // upgrade the counter
    _blockIdx = blockIdx;
  }

  bool read(u64 blockIdx, T &item) const {
    auto & block = _blocks[blockIdx];
    auto version = block.version.load(std::memory_order_acquire);
    // read only when version is odd
    if (version % 2 == 0) {  // TODO: use bitwse comparison
    // if ((version & 2) - 1 == 0 ) {  // TODO: use bitwse comparison
      return false;
    }

    // read the data
    item =  block.data;
    // indicate that the block is read but don't change evenness
    // TODO: That might not be necessary
    // block.version.store(version + 2, std::memory_order_release);
    return true;
  }

  size_t size() const {
    return _capacity;
  }
};



}  // end namespace v2
