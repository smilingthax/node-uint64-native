#ifndef _BINARY64UTIL_H
#define _BINARY64UTIL_H

#include <stdint.h>

/* Provides:

double buildBinary64Dbl(const int sign,const uint64_t mantissa,const int exponent)
- sign=+1, mantissa=0x0010000000000000, exponent=1 -> 2e0
- will not check mantissa for bit 53 to be set (resp. cleared for exceptional exponents)

int splitBinary64Dbl(const double d,int *sign,uint64_t *mantissa,int *exponent)
- returns !=0 on NaN/Inf/Denormal

Notes:
- Encoding for 0 is: exponent==-1023, mantissa==0

*/

// ONLY for ieee754 / binary64 doubles

#include <limits.h>
#if !defined(CHAR_BIT) || (CHAR_BIT!=8)
#error Only CHAR_BITS==8 is supported
#endif

#include <float.h>
#if !defined(FLT_RADIX) || (FLT_RADIX!=2)
#error FLT_RADIX!=2 is not supported here
#endif
#if !defined(DBL_MANT_DIG) || (DBL_MANT_DIG!=53)
#error DBL_MANT_DIG!=53 is not supported here
#endif

// requires sizeof(double)==sizeof(uint64_t)
typedef char _dblu64_h_check_equal[2*(sizeof(double)==sizeof(uint64_t))-1];

// if __FLOAT_WORD_ORDER != __BYTE_ORDER   #error ?

#define DBLU64_USE_FAST_IEEE754

#ifdef DBLU64_USE_FAST_IEEE754
#include "bitcount.h"
#include <string.h> // memcpy
#else
#include <math.h>  // C99, requires -lm, ...
#endif

#ifdef __cplusplus
extern "C" {
#endif

// sign=+1, mantissa=0x0010000000000000, exponent=1 -> 2e0
// note: 0 must be encoded as exponent==-1023, mantissa==0 !
static inline double buildBinary64Dbl(const int sign,const uint64_t mantissa,const int exponent)
{
#ifdef DBLU64_USE_FAST_IEEE754
  const uint64_t mant=mantissa&0xfffffffffffff;
  const uint64_t exp=(exponent+1023)&0x7ff;
  const uint64_t ieee=((uint64_t)(sign<0)<<63) | (exp<<52) | mant;

  double ret;
// #if defined(__FLOAT_WORD_ORDER) && (__FLOAT_WORD_ORDER != __BYTE_ORDER) ... swap instead of plain memcpy  (#include <endian.h> on __GNUC__ ??)
  memcpy(&ret,&ieee,sizeof(double));
  return ret;
#else
  // CAVE: handles denormals, etc. differently!
  const double ret=ldexp(mantissa,exponent-52);
  return (sign<0) ? -ret : ret;
#endif
}

// returns !=0 on NaN/Inf/Denormal
// note: 0 is: exponent==-1023, mantissa==0
static inline int splitBinary64Dbl(const double d,int *sign,uint64_t *mantissa,int *exponent)
{
#ifdef DBLU64_USE_FAST_IEEE754
  uint64_t ieee;
// #if defined(__FLOAT_WORD_ORDER) && (__FLOAT_WORD_ORDER != __BYTE_ORDER) ... swap instead of plain memcpy  (#include <endian.h> on __GNUC__ ??)
  memcpy(&ieee,&d,sizeof(double));

  *sign=(ieee>>63) ? -1 : 1;
  const unsigned int exp=(ieee>>52)&0x7ff;
  const uint64_t isNormal=(uint64_t)(exp-1<2046)<<52; // TRICK: exp is unsigned, exp-1 wraps!

  *exponent=exp-1023;
  *mantissa=(ieee&0xfffffffffffff) | isNormal;
  return !isNormal;
#else
  // handles denormals, etc. differently!
  *sign=signbit(d) ? -1 : 1;
  if ( (isnan(d))||(isinf(d)) ) {
    *mantissa=(isnan(d)) ? 1 : 0; // FIXME? type of nan?
    *exponent=1024;
    return 1;
  } else if (!d) {
    *mantissa=0;
    *exponent=-1023;
    return 0;
  } // (!isnormal(d)): exponent=-1023: subnormals are handled differently!
  const int lb=ilogb(d);
  *mantissa=ldexp(fabs(d),52-lb);
  *exponent=lb;
  return 0;
#endif
}

#ifdef __cplusplus
}
#endif

#endif
