#include "wrapping_integers.hh"
#include <algorithm>
#include <cstdint>

using namespace std;
// 期待一行进行包装
Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return zero_point + static_cast<uint32_t>(n);
}
//这个方法用于将相对序列号转换为绝对序列号。它接受一个相对序列号（Wrap32对象）、初始序列号（zero point）以及一个绝对检查点序列号（checkpoint），并找到与检查点最接近的绝对序列号。这个方法的目的是解决在循环序列号中可能存在的多个绝对序列号之间的歧义。通过提供检查点，可以确定哪个绝对序列号在正确的范围内。

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  uint64_t offset = raw_value_ - zero_point.raw_value_;
  uint64_t ret = offset + checkpoint + 1; 
  (void)zero_point;
  (void)checkpoint;
  return ret;
}
