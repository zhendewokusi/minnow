#include "wrapping_integers.hh"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>

using namespace std;
// 期待一行进行包装
Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return zero_point + static_cast<uint32_t>( n );
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  uint32_t offset = this->raw_value_ - zero_point.raw_value_;

  if ( offset >= checkpoint )
    return offset;

  uint64_t MAX = 1LL << 32;
  uint64_t real_part = checkpoint - offset + ( MAX >> 1 );

  return real_part / MAX * MAX + offset;
}