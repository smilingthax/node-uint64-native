#include "u64str.h"

static char hexDigit(char c)
{
  switch (c) {
  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':
    return c-'0';
  case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
    return c-'A'+10;
  case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
    return c-'a'+10;
  default:
    return -1;
  }
}

static char decDigit(char c)
{
  if ( (c>='0')&&(c<='9') ) {
    return c-'0';
  }
  return -1;
}

uint64_t u64FromString(const char *s,const char *end)
{
  uint64_t ret=0;
  if ( (s+2<end)&&(s[0]=='0')&&(s[1]=='x') ) { // hex
    for (s+=2; s!=end; s++) {
      const char res=hexDigit(*s);
      if (res<0) {
        break;
      }
      ret=(ret<<4)|res;
    }
  } else { // assume decimal  // TODO?
    for (; s!=end; s++) {
      const char res=decDigit(*s);
      if (res<0) {
        break;
      }
      ret=(ret*10)+res;
    }
  }
  return ret;
}

// returns NULL on bad radix or missing scratch, else pointer to result
// scratch space must be at least 65 bytes
char *u64ToString(uint64_t val,int radix,char *scratch65)
{
  static const char digits[36]="0123456789abcdefghijklmnopqrstuvwxyz";
  if ( (radix<2)||(radix>36)||(!scratch65) ) {
    return 0;
  }
  char *pos=scratch65+65;
  *--pos=0;
  if (radix&(radix-1)) { // not power of two
    do {
      *--pos=digits[val%radix];
      val/=radix;
    } while (val>0);
  } else {
    const int shift=((0x24060008>>(33-radix))&0x7)+1;
    radix--; // use as mask
    do {
      *--pos=digits[val&radix];
      val>>=shift;
    } while (val>0);
  }
  return pos;
}

