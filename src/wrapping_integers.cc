#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  return zero_point+(uint32_t)n;
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  uint32_t offset = raw_value_-zero_point.raw_value_;
  uint64_t u1 = (uint64_t)offset | (checkpoint & 0xFFFFFFFF00000000U);
  if(u1<checkpoint){
    uint64_t u2 = u1 + 0x100000000U;
    return (checkpoint-u1)<(max(u2,checkpoint)-min(u2,checkpoint))? u1:u2;
  }
  else{
    uint64_t u2 = u1 - 0x100000000U;
    return (u1-checkpoint)<(max(u2,checkpoint)-min(u2,checkpoint))? u1:u2;
  }
}
