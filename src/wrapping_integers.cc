#include "wrapping_integers.hh"
#include <algorithm>
#include <cstdint>
#include <cmath>  
#include <cstdlib>

using namespace std;
// 期待一行进行包装
Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return zero_point + static_cast<uint32_t>(n);
}
// unwarp应该返回一个32位的绝对序列号，不知道为啥返回值是 uint64_t
uint64_t Wrap32::unwrap(Wrap32 zero_point, uint64_t checkpoint) const {
  uint64_t offset = raw_value_ - zero_point.raw_value_;
  uint64_t temp = (checkpoint & 0xffffffff00000000ULL) + offset;
  uint64_t ret = temp;
  int64_t diff1 = temp + (1ULL << 32) - checkpoint;
  int64_t diff2 = temp - checkpoint;
  if (std::abs(static_cast<int64_t>(diff1)) < std::abs(static_cast<int64_t>(diff2))) {
    ret += (1ULL << 32);
  }

  if (temp >= (1ULL << 32)) {
    if (std::abs(static_cast<int64_t>(temp - (1ULL << 32) - checkpoint)) < 
        std::abs(static_cast<int64_t>(ret - checkpoint))) {
      ret = temp - (1ULL << 32);
    }
  }

  return ret;
}
