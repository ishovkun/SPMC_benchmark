#pragma once
#include <cstdint>
#include <functional>

using u8 = uint8_t;
using u32 = uint32_t;
using i32 = int32_t;
using u64 = uint64_t;

using message_size_t = u32;
constexpr u32 maximum_message_size = 64;
using callback_t = std::function<void(uint8_t* data)>;
// constexpr u32 cache_line_size = 128;
constexpr u32 cache_line_size = 64;
