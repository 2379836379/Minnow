#include "wrapping_integers.hh"
#include <algorithm>
#include <cstdint>
#include <type_traits>

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  // 溢出时相当于取模
  return Wrap32 { static_cast<uint32_t>(n) + zero_point.raw_value_ };
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // 计算 offset = (raw_value_ - zero_point) mod 2^32
  // 向下溢出则得到对应的正值
  // uint32_t offset = raw_value_ - zero_point.raw_value_;
  // 计算 checkpoint 对应的低 32 位应该是什么
  uint64_t checkpoint_offset = wrap(checkpoint, zero_point).raw_value_;
    
  // 计算差值（模 2^32 意义下）
  int32_t diff = static_cast<int32_t>(raw_value_ - checkpoint_offset);
    
  // 绝对序列号 = checkpoint + diff
  // 但要处理溢出情况
  int64_t result = static_cast<int64_t>(checkpoint) + diff;
    
  // 确保结果非负且正确
  if (result < 0) {
    result += (1ULL << 32);
  }
  return static_cast<uint64_t>(result);
}
