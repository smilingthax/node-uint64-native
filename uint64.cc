#include "uint64.h"
#include "u64str.h"
#include "ext/bitcount.h"
#include "ext/shifts.h"
#include "ext/adc_sbb.h"

Nan::Persistent<v8::Function> UInt64::constructor;
Nan::Persistent<v8::Function> UInt64::constructorSigned;
Nan::Persistent<v8::FunctionTemplate> UInt64::tmpl;

NAN_MODULE_INIT(UInt64::Init)
{
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(UInt64::NewUInt64);
  tpl->SetClassName(Nan::New("UInt64").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tmpl.Reset(tpl);

  Nan::SetMethod(tpl, "Compare", Compare);

  Nan::SetAccessor(tpl->InstanceTemplate(),Nan::New("sign").ToLocalChecked(), GetSign, SetSign);
  Nan::SetAccessor(tpl->InstanceTemplate(),Nan::New("hi32").ToLocalChecked(), GetHi32, SetHi32);
  Nan::SetAccessor(tpl->InstanceTemplate(),Nan::New("lo32").ToLocalChecked(), GetLo32, SetLo32);

  Nan::SetPrototypeMethod(tpl, "toString", ToString);

#define X(name,code) Nan::SetPrototypeMethod(tpl, #name, op_ ## name);
  UINT64_UNARY_OPS
  UINT64_BINARY_OPS
  UINT64_UINT_OPS
#undef X

  constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
  Nan::Set(target, Nan::New("UInt64").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());

  // proper Int64 derivation can only be done on the native side...
  v8::Local<v8::FunctionTemplate> tpl2 = Nan::New<v8::FunctionTemplate>(UInt64::NewInt64);
  tpl2->SetClassName(Nan::New("Int64").ToLocalChecked());
  tpl2->Inherit(tpl);
  tpl2->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetMethod(tpl2, "Compare", SignedCompare);

  constructorSigned.Reset(Nan::GetFunction(tpl2).ToLocalChecked());
  Nan::Set(target, Nan::New("Int64").ToLocalChecked(), Nan::GetFunction(tpl2).ToLocalChecked());
}

static uint64_t u64FromString(v8::Local<v8::String> value,bool withSign)
{
  Nan::Utf8String str(value);
  const char *s=*str,
             *end=s+str.length();

  // TODO? skip whitespace?
  if ( (withSign)&&(*s=='-') ) {
    return -u64FromString(s+1,end);
  } else if (*s=='+') {
    s++;
  }
  return u64FromString(s,end);
}

bool UInt64::HasInstance(v8::Local<v8::Value> value)
{
  // alternative: store <v8::Value> GetPrototype() and compare
  return Nan::New(tmpl)->HasInstance(value);
}

uint64_t UInt64::Value(v8::Local<v8::Value> value)
{
  return Unwrap(value->ToObject())->value;
}

UInt64 *UInt64::This(Nan::NAN_METHOD_ARGS_TYPE info)
{
  if (!HasInstance(info.Holder())) {
    Nan::ThrowTypeError("Bad UInt64 object");
    return 0;
  }
  return Unwrap(info.Holder());
}

// TODO?  Maybe<uint64_t> / optional
bool UInt64::FromArgument(v8::Local<v8::Value> arg,uint64_t &ret,bool withSign)
{
  if (arg->IsNumber()) {
    ret = (uint64_t)arg->NumberValue(); // TODO? better?
    return true;
  } else if (arg->IsString()) {
    ret = u64FromString(arg->ToString(),withSign);
    return true;
  } else if (HasInstance(arg)) {
    ret = Value(arg);
    return true;
  }
  Nan::ThrowTypeError("Argument must be Number, String or UInt64");
  return false;
}

v8::Local<v8::Object> UInt64::NewInstance(uint64_t value,bool asSigned)
{
  Nan::EscapableHandleScope scope;

  // TODO? could empty v8::Local<> be enough? [or is it converted to undefined?]
  v8::Local<v8::Value> arg = Nan::New<v8::External>(&constructor); // magic token
  v8::Local<v8::Function> cons = Nan::New(asSigned ? constructorSigned : constructor);
  v8::Local<v8::Object> instance = cons->NewInstance(1, &arg);

  UInt64 *obj = new UInt64(value);
  obj->Wrap(instance);

  return scope.Escape(instance);
}

void UInt64::New(Nan::NAN_METHOD_ARGS_TYPE info,bool asSigned)
{
  if ( (info.IsConstructCall())&&(info.Length()==1)&&(info[0]->IsExternal()) ) {
    if (info[0].As<v8::External>()->Value() == &constructor) { // magic token: internal invocation
      // caller will add missing internal UInt64 *
      info.GetReturnValue().Set(info.This());
    } else {
      Nan::ThrowTypeError("Unexpected first argument");
    }
    return;
  }

  // process arguments
  uint64_t value;
  switch (info.Length()) { // TODO? check for undefined instead?
  case 0:
    value = 0;
    break;
  case 1:
    if (!FromArgument(info[0],value,asSigned)) {
      return;
    }
    break;
  case 2:
    if ( (!info[0]->IsNumber())||(!info[1]->IsNumber()) ) {
      Nan::ThrowTypeError("Two argument constructor expects (Number,Number)");
      return;
    }
    if (asSigned) {
      value = ((uint64_t)info[0]->Int32Value() << 32) | info[1]->Uint32Value();
    } else {
      value = ((uint64_t)info[0]->Uint32Value() << 32) | info[1]->Uint32Value();
    }
    break;
  default:
    Nan::ThrowTypeError("Wrong number of arguments");
    return;
  }

  if (info.IsConstructCall()) {
    UInt64 *obj = new UInt64(value);
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    info.GetReturnValue().Set(NewInstance(value,asSigned));
  }
}

NAN_METHOD(UInt64::NewUInt64)
{
  New(info,false);
}

NAN_METHOD(UInt64::NewInt64)
{
  New(info,true);
}

#define RET(val) info.GetReturnValue().Set(val); return;
#define RETSTR(str) RET(Nan::New(str).ToLocalChecked())

NAN_METHOD(UInt64::Compare)
{
  uint64_t a,b;
  if ( (FromArgument(info[0],a))&&(FromArgument(info[1],b)) ) {
    if (a < b) {
      RET(-1);
    } else if (a > b) {
      RET(1);
    } else {
      RET(0);
    }
  }
}

NAN_METHOD(UInt64::SignedCompare)
{
  uint64_t a,b;
  if ( (FromArgument(info[0],a))&&(FromArgument(info[1],b)) ) {
    if ((int64_t)a < (int64_t)b) {
      RET(-1);
    } else if ((int64_t)a > (int64_t)b) {
      RET(1);
    } else {
      RET(0);
    }
  }
}

NAN_GETTER(UInt64::GetSign)
{
  UInt64 *obj = Unwrap(info.Holder()); // or: This(info);
  RET((bool)(obj->value>>63));
}

NAN_SETTER(UInt64::SetSign)
{
  UInt64 *obj = Unwrap(info.Holder());
  if (value->BooleanValue()) {
    obj->value |= 0x8000000000000000;
  } else {
    obj->value &= 0x7fffffffffffffff;
  }
}

NAN_GETTER(UInt64::GetHi32)
{
  UInt64 *obj = Unwrap(info.Holder());
  RET((uint32_t)(obj->value>>32));
}

NAN_SETTER(UInt64::SetHi32)
{
  UInt64 *obj = Unwrap(info.Holder());
  obj->value = (obj->value&0xffffffff) | ((uint64_t)value->Uint32Value()<<32);
}

NAN_GETTER(UInt64::GetLo32)
{
  UInt64 *obj = Unwrap(info.Holder());
  RET((uint32_t)obj->value);
}

NAN_SETTER(UInt64::SetLo32)
{
  UInt64 *obj = Unwrap(info.Holder());
  obj->value = (obj->value&~0xfffffffff) | value->Uint32Value();
}

NAN_METHOD(UInt64::ToString)
{
  UInt64 *obj = This(info);
  if (!obj) {
    return;
  }
  const uint64_t lhs = obj->value;
  const int radix = info[0]->Int32Value();
  if ( (radix<2)||(radix>36) ) {
    Nan::ThrowRangeError("Radix must be between 2 and 36");
    return;
  }
  char scratch65[66], *ret; // 66: one extra char for sign!
  if ( (info[1]->BooleanValue())&&(lhs>>63) ) {
    ret = u64ToString(-lhs, radix, scratch65 + 1);
    *--ret = '-';
  } else {
    ret = u64ToString(lhs, radix, scratch65 + 1);
  }
  RETSTR(ret);
}

#define X(name,code) \
  NAN_METHOD(UInt64::op_ ## name)             \
  {                                           \
    if (UInt64 *obj = This(info)) {           \
      uint64_t &lhs = obj->value;             \
      code;                                   \
      info.GetReturnValue().Set(info.This()); \
    }                                         \
  }
UINT64_UNARY_OPS
#undef X

#define X(name,code) \
  NAN_METHOD(UInt64::op_ ## name)               \
  {                                             \
    if (UInt64 *obj = This(info)) {             \
      uint64_t &lhs = obj->value, rhs;          \
      if (FromArgument(info[0],rhs)) {          \
        code;                                   \
        info.GetReturnValue().Set(info.This()); \
      }                                         \
    }                                           \
  }
UINT64_BINARY_OPS
#undef X

#define X(name,code) \
  NAN_METHOD(UInt64::op_ ## name)               \
  {                                             \
    if (!info[0]->IsNumber()) {                 \
      Nan::ThrowTypeError("Expected Number as argument"); \
      return;                                   \
    }                                           \
    const uint32_t rhs = info[0]->Uint32Value();\
    if (UInt64 *obj = This(info)) {             \
      uint64_t &lhs = obj->value;               \
      code;                                     \
      info.GetReturnValue().Set(info.This());   \
    }                                           \
  }
UINT64_UINT_OPS
#undef X

