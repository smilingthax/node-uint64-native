#ifndef _U64STR_H
#define _U64STR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint64_t u64FromString(const char *s,int len);

// returns NULL on bad radix or missing scratch, else pointer to result
// scratch space must be at least 65 bytes
char *u64ToString(uint64_t val,int radix,char *scratch65);

#ifdef __cplusplus
}
#endif

#endif
