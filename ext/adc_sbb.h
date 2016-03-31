#ifndef _ADC_SBB_H
#define _ADC_SBB_H

#include <stdint.h>
// TODO? C++ only - or #include <stdbool.h> ?

// TODO? more efficient implementation / x86 intrinsic _addcarry_u64 ?
static inline bool adc64(uint64_t &ret,uint64_t a,uint64_t b,bool carry)
{
  ret = a + b + carry;
  return (ret < b) || (!b && ret == b);
}

static inline bool sbb64(uint64_t &ret,uint64_t a,uint64_t b,bool carry)
{
  ret = a - b - carry;
  return (a < ret) || (b && a == ret);
}

#endif
