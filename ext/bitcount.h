#ifndef _BITCOUNT_H
#define _BITCOUNT_H

#include <stdint.h>

/* Provides:

Count leading zeros
- unsigned int clz32(uint32_t)    - 32 for x==0
- unsigned int clz64(uint64_t)
* floor(log2(x))/BitScanReverse is (31-clz32(x))

Count trailing zeros
- unsigned int ctz32(uint32_t)    - 32 for x==0
- unsigned int ctz64(uint64_t)
* ffs(FindFirstSet) is (ctz+1) [but usually 33/65->0]

- unsigned int popcnt32(uint32_t)  (no popcnt64 (yet))
*/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __has_builtin
#define __has_builtin(x) 0  // prevent non-clang preprocessor syntax error
#endif

#if defined(_MSC_VER) // since VS2005
#include <intrin.h>

#pragma intrinsic(_BitScanForward)
#pragma intrinsic(_BitScanReverse)

// MSVC > 1500, but only with certain processor features(?): __popcnt()

static inline unsigned int clz32(const uint32_t val)
{
  if (!val) {
    return 32;
  }
  int ret;
  _BitScanReverse(&ret,val);
  return 0x1f^ret; // = 32+~ret = 31+(~ret+1) = 31-ret
}

static inline unsigned int ctz32(const uint32_t val)
{
  if (!val) {
    return 32;
  }
  int ret;
  _BitScanForward(&ret,val);
  return ret;
}

#  if defined(_WIN64)  // TODO? (_M_AMD64 || _M_ARM64) ?
#define _BITCOUNT_H_HAS64
#pragma intrinsic(_BitScanForward64)
#pragma intrinsic(_BitScanReverse64)

static inline unsigned int clz64(const uint64_t val)
{
  if (!val) {
    return 64;
  }
  int ret;
  _BitScanReverse64(&ret,val);
  return 0x3f^ret; // = 64+~ret = 63+(~ret+1) = 63-ret
}

static inline unsigned int ctz32(const uint32_t val)
{
  if (!val) {
    return 32;
  }
  int ret;
  _BitScanForward(&ret,val);
  return ret;
}
#  endif

#elif (defined(__GNUC__) || defined(__clang__)) && (__SIZEOF_INT__ == 4)  // gcc since 3.4.0,  clang: __has_builtin(__builtin_clz) ...
#if defined(__clang__) && __has_builtin(__builtin_popcnt)
// Don't use __builtin_popcount on gcc (only since gcc 4.5; only calls __popcountsi2 - or even __popcountdi2)
#define _BITCOUNT_H_HASPOP

static inline unsigned int popcnt32(const uint32_t val)
{
  return __builtin_popcount(val);
}
#endif

static inline unsigned int clz32(const uint32_t val)
{
  if (!val) { // TODO?! unlikely ?
    return 32;
  }
  return __builtin_clz(val);
}

static inline unsigned int ctz32(const uint32_t val)
{
  if (!val) {
    return 32;
  }
  return __builtin_ctz(val);
}

// Note: Don't use __builtin_clzll/ctlll on 32bit, because gcc will not inline but emit call to __clzdi2/...
#  if __SIZEOF_LONG_INT__ == 8  // TODO? and LLP64 ?
#define _BITCOUNT_H_HAS64

static inline unsigned int clz64(const uint64_t val)
{
  if (!val) {
    return 64;
  }
  return __builtin_clzl(val);
}

static inline unsigned int ctz64(const uint64_t val)
{
  if (!val) {
    return 64;
  }
  return __builtin_ctzl(val);
}
#  endif

#else
// TODO? glibc/POSIX: ffs[0->0] (i.e. ctz+1; but no support for clz)

static inline unsigned int popcnt32(uint32_t val); // forward declaration needed, _BITCOUNT_H_HASPOP is NOT defined

static inline unsigned int clz32(uint32_t val)
{
  val |= val>>1;
  val |= val>>2;
  val |= val>>4;
  val |= val>>8;
  val |= val>>16;
  return 32-popcnt32(val);
}

static inline unsigned int ctz32(const uint32_t val)
{
  return popcnt32((val&-val) - 1);
}
#endif


#ifdef _BITCOUNT_H_HASPOP
#undef _BITCOUNT_H_HASPOP
#else
static inline unsigned int popcnt32(uint32_t val)
{
  val -= (val>>1) & 0x55555555;
  val = ((val>>2) & 0x33333333) + (val & 0x33333333);
  val = ((val>>4) + val) & 0x0f0f0f0f;
  // bits are now summed in the 4 bytes
  return (val*0x01010101) >> 24;
  // or: val += val>>16; return (val + (val>>8))&0x3f;
}
#endif


#ifdef _BITCOUNT_H_HAS64
#undef _BITCOUNT_H_HAS64
#else
static inline unsigned int clz64(uint64_t val)
{
  const uint32_t hi=val>>32;
  if (hi) { // could be eliminated (cf. ctz64, below)
    return clz32(hi);
  }
  return 32+clz32(val);
}

static inline unsigned int ctz64(uint64_t val)
{
  const uint32_t lo=val;
  if (lo) { // could be eliminated by smart(?) const uint32_t c=-(!lo); return ctz32(((val>>32)&c)|(lo&~c)) + (32&c);
    return ctz32(lo);
  }
  return 32+ctz32(val>>32);
}
#endif

#ifdef __cplusplus
}
#endif

#endif
