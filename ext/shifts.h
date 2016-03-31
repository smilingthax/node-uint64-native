#ifndef _SHIFTS_H
#define _SHIFTS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Enforce consistency: mask n by 31/63 as x86 does
// (ARM would happily shift until n==255)

static inline uint32_t shl32(uint32_t val, unsigned int n)
{
  return val << (n & 31);
}

static inline uint64_t shl64(uint64_t val, unsigned int n)
{
  return val << (n & 63);
}

static inline uint32_t shr32(uint32_t val, unsigned int n)
{
  return val >> (n & 31);
}

static inline uint64_t shr64(uint64_t val, unsigned int n)
{
  return val >> (n & 63);
}

static inline uint32_t rol32(uint32_t val, unsigned int n)
{
  n &= 31;
  return (val << n) | (val >> (32-n)); // also works for n==0
}

static inline uint64_t rol64(uint64_t val, unsigned int n)
{
  n &= 63;
  return (val << n) | (val >> (64-n));
}

static inline uint32_t ror32(uint32_t val, unsigned int n)
{
  n &= 31;
  return (val >> n) | (val << (32-n));
}

static inline uint64_t ror64(uint64_t val, unsigned int n)
{
  n &= 63;
  return (val >> n) | (val << (64-n));
}

#if (-1>>1) < 0   // compiler treats signed right shift as arithmetic shift.
static inline uint32_t sar32(uint32_t val, unsigned int n)
{
  return (int32_t)val >> (n & 31);
}

static inline uint64_t sar64(uint64_t val, unsigned int n)
{
  return (int64_t)val >> (n & 63);
}

#else   // portable arithmetic shift

static inline uint32_t sar32(uint32_t val, unsigned int n)
{
  n &= 31;
  if (!n) {
    return val;
  }
  return (val >> n) | (-(val>>31) << (32-n));
}

static inline uint64_t sar64(uint64_t val, unsigned int n)
{
  n &= 63;
  if (!n) {
    return val;
  }
  return (val >> n) | (-(val>>63) << (64-n));
}
#endif

#ifdef __cplusplus
}
#endif

#endif
