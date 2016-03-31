#ifndef _UINT64_H
#define _UINT64_H

#include <nan.h>

#define UINT64_UNARY_OPS \
  X(neg,    { lhs = -lhs; }) \
  X(not,    { lhs = ~lhs; }) \
  X(abs,    { if (lhs>>63) lhs = -lhs; }) \
                             \
  X(clz,    RET(clz64(lhs))) \
  X(ctz,    RET(ctz64(lhs))) \
                             \
  X(isZero, RET(lhs == 0))

#define UINT64_BINARY_OPS \
  X(add, { lhs += rhs; })    \
  X(sub, { lhs -= rhs; })    \
  X(rsub,{ lhs = rhs-lhs; }) \
  X(add2,RET(adc64(lhs, lhs, rhs, info[1]->BooleanValue()))) \
  X(sub2,RET(sbb64(lhs, lhs, rhs, info[1]->BooleanValue()))) \
                             \
  X(and, { lhs &= rhs; })    \
  X(or,  { lhs |= rhs; })    \
  X(xor, { lhs ^= rhs; })    \
                             \
  X(eq,  RET(lhs == rhs))    \
  X(lt,  RET(lhs < rhs))     \
  X(gt,  RET(lhs > rhs))     \
  X(ilt, RET((int64_t)lhs < (int64_t)rhs)) \
  X(igt, RET((int64_t)lhs > (int64_t)rhs))

#define UINT64_UINT_OPS \
  X(shl, { lhs = shl64(lhs, rhs); }) \
  X(shr, { lhs = shr64(lhs, rhs); }) \
  X(sar, { lhs = sar64(lhs, rhs); }) \
  X(rol, { lhs = rol64(lhs, rhs); }) \
  X(ror, { lhs = ror64(lhs, rhs); })

class UInt64 : public Nan::ObjectWrap {
  static inline UInt64 *Unwrap(v8::Local<v8::Object> obj) {
    return Nan::ObjectWrap::Unwrap<UInt64>(obj);
  }
public:
  UInt64() : value(0) {}
  UInt64(const uint64_t value) : value(value) {}

  uint64_t Value() const { return value; }

  static bool HasInstance(v8::Local<v8::Value> value);
  static uint64_t Value(v8::Local<v8::Value> value);
  static v8::Local<v8::Object> NewInstance(uint64_t value,bool asSigned=false);

  static NAN_MODULE_INIT(Init);
private:
  uint64_t value;

  static UInt64 *This(Nan::NAN_METHOD_ARGS_TYPE info);
  static bool FromArgument(v8::Local<v8::Value> arg,uint64_t &ret,bool withSign=false);

  static void New(Nan::NAN_METHOD_ARGS_TYPE info,bool asSigned);
  static NAN_METHOD(NewUInt64);
  static NAN_METHOD(NewInt64);
  static NAN_METHOD(Compare);
  static NAN_METHOD(SignedCompare);

  static NAN_GETTER(GetSign);
  static NAN_SETTER(SetSign);
  static NAN_GETTER(GetHi32);
  static NAN_SETTER(SetHi32);
  static NAN_GETTER(GetLo32);
  static NAN_SETTER(SetLo32);

  static NAN_METHOD(ToString);

#define X(name,code) static NAN_METHOD(op_ ## name);
  UINT64_UNARY_OPS
  UINT64_BINARY_OPS
  UINT64_UINT_OPS
#undef X

  static Nan::Persistent<v8::Function> constructor;
  static Nan::Persistent<v8::Function> constructorSigned;
  static Nan::Persistent<v8::FunctionTemplate> tmpl;
};

#endif
