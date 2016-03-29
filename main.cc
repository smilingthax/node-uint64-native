#include <nan.h>
#include <math.h> // cmath?
#include "uint64.h"
#include "ext/binary64util.h"
#include "ext/bitcount.h"

/* Provides:

u64.clz32
u64.ctz32
u64.popcnt32

u64.buildDouble(sign[+/-1],mantissa:UInt64,exponent:int):Number
* (-1023 <= exponent <= 1024); -1023 and 1024 are special
* 0 must be encoded with mantissa==0, exponent==-1023
* mantissa must have bit 53 set, iff resulting double is normal,!=0

u64.splitDouble(d) -> {sign,mantissa,exponent,isNormal:bool}

*/

static NAN_METHOD(Clz32)
{
  if (!info[0]->IsNumber()) {
    Nan::ThrowTypeError("Expected Number as argument");
    return;
  }
  info.GetReturnValue().Set(clz32(info[0]->Uint32Value()));
}

static NAN_METHOD(Ctz32)
{
  if (!info[0]->IsNumber()) {
    Nan::ThrowTypeError("Expected Number as argument");
    return;
  }
  info.GetReturnValue().Set(ctz32(info[0]->Uint32Value()));
}

static NAN_METHOD(Popcnt32)
{
  if (!info[0]->IsNumber()) {
    Nan::ThrowTypeError("Expected Number as argument");
    return;
  }
  info.GetReturnValue().Set(popcnt32(info[0]->Uint32Value()));
}

static NAN_METHOD(buildDouble)
{
  if (info.Length() != 3) {
    Nan::ThrowTypeError("Wrong number of arguments");
    return;
  }
  if (!info[0]->IsNumber()) { // sign
    Nan::ThrowTypeError("Expected Number as first argument");
    return;
  } else if (!UInt64::HasInstance(info[1])) { // mantissa
    Nan::ThrowTypeError("Expected UInt64 as second argument");
    return;
  } else if (!info[2]->IsInt32()) { // exponent
    Nan::ThrowTypeError("Expected Integer as third argument");
    return;
  }

  const int sign = signbit(info[0]->NumberValue()) ? -1 : +1; // std::signbit ?
  const uint64_t mantissa = UInt64::Value(info[1]);
  const int exp = info[2]->Int32Value();

  if ( (exp<-1023)||(exp>1024) ) {
    Nan::ThrowRangeError("Exponent must be between -1023 and 1024");
    return;
  } else if ( (exp==-1023)||(exp==1024) ) {
    if (mantissa>>52) {
      Nan::ThrowError("Mantissa for Denormals/Nan/Infinity cannot have bit 53 set");
      return;
    }
  } else if ((mantissa>>52)!=1) {
    Nan::ThrowError("Mantissa (of normal double) must have bit 53 set");
    return;
  }

  const double ret = buildBinary64Dbl(sign,mantissa,exp);
  info.GetReturnValue().Set(Nan::New(ret)); // or: v8::Number::New(ret)
}

static NAN_METHOD(splitDouble)
{
  if (info.Length() != 1) {
    Nan::ThrowTypeError("Wrong number of arguments");
    return;
  }
  if (!info[0]->IsNumber()) {
    Nan::ThrowTypeError("Expected Number as first argument");
    return;
  }

  const double d=info[0]->NumberValue();

  int sign,exponent;
  uint64_t mantissa;
  const bool isNormal=(splitBinary64Dbl(d,&sign,&mantissa,&exponent)==0);

  v8::Local<v8::Object> ret = Nan::New<v8::Object>();
  ret->Set(Nan::New("sign").ToLocalChecked(),Nan::New((int32_t)sign));
  ret->Set(Nan::New("mantissa").ToLocalChecked(),UInt64::NewInstance(mantissa));
  ret->Set(Nan::New("exponent").ToLocalChecked(),Nan::New((int32_t)exponent));
  ret->Set(Nan::New("isNormal").ToLocalChecked(),Nan::New(isNormal));

  info.GetReturnValue().Set(ret);
}

static NAN_MODULE_INIT(init)
{
  UInt64::Init(target);

  Nan::SetMethod(target, "clz32", Clz32);
  Nan::SetMethod(target, "ctz32", Ctz32);
  Nan::SetMethod(target, "popcnt32", Popcnt32);

  Nan::SetMethod(target, "buildDouble", buildDouble);
  Nan::SetMethod(target, "splitDouble", splitDouble);
}

NODE_MODULE(u64, init)

