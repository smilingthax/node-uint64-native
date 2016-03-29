#include "uint64.h"
#include "u64str.h"
#include "ext/bitcount.h"

Nan::Persistent<v8::Function> UInt64::constructor;
Nan::Persistent<v8::FunctionTemplate> UInt64::tmpl;

NAN_MODULE_INIT(UInt64::Init)
{
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(UInt64::New);
  tpl->SetClassName(Nan::New("UInt64").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tmpl.Reset(tpl);

  Nan::SetAccessor(tpl->InstanceTemplate(),Nan::New("sign").ToLocalChecked(), GetSign, SetSign);
  Nan::SetAccessor(tpl->InstanceTemplate(),Nan::New("hi32").ToLocalChecked(), GetHi32, SetHi32);
  Nan::SetAccessor(tpl->InstanceTemplate(),Nan::New("lo32").ToLocalChecked(), GetLo32, SetLo32);

  Nan::SetPrototypeMethod(tpl, "toString", ToString);

#define X(name,code) Nan::SetPrototypeMethod(tpl, #name, op_ ## name);
  UINT64_UNARY_OPS
  UINT64_BINARY_OPS
#undef X

  constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
  Nan::Set(target, Nan::New("UInt64").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}

static uint64_t u64FromString(v8::Local<v8::String> value)
{
  Nan::Utf8String s(value);
  return u64FromString(*s,s.length());
}

bool UInt64::HasInstance(v8::Local<v8::Value> value)
{
  // alternative: store <v8::Value> GetPrototype() and compare
  return Nan::New(tmpl)->HasInstance(value);
}

uint64_t UInt64::Value(v8::Local<v8::Value> value)
{
  return Nan::ObjectWrap::Unwrap<UInt64>(value->ToObject())->value;
}

UInt64 *UInt64::This(Nan::NAN_METHOD_ARGS_TYPE info)
{
  if (!UInt64::HasInstance(info.Holder())) {
    Nan::ThrowTypeError("Bad UInt64 object");
    return 0;
  }
  return Nan::ObjectWrap::Unwrap<UInt64>(info.Holder());
}

// TODO?  Maybe<uint64_t> / optional
bool UInt64::FromArgument(v8::Local<v8::Value> arg,uint64_t &ret)
{
  if (arg->IsNumber()) {
    ret = (uint64_t)arg->NumberValue(); // TODO? better?
    return true;
  } else if (arg->IsString()) {
    ret = u64FromString(arg->ToString());
    return true;
  } else if (HasInstance(arg)) {
    ret = Value(arg);
    return true;
  }
  Nan::ThrowTypeError("Argument must be Number, String or UInt64");
  return false;
}

v8::Local<v8::Object> UInt64::NewInstance(uint64_t value)
{
  Nan::EscapableHandleScope scope;

  // TODO? is empty v8::Local<> enough? [could be converted to undefined?]
  v8::Local<v8::Value> arg = Nan::New<v8::External>(&constructor); // magic token
  v8::Local<v8::Function> cons = Nan::New(constructor);
  v8::Local<v8::Object> instance = cons->NewInstance(1, &arg);

  UInt64 *obj = new UInt64(value);
  obj->Wrap(instance);

  return scope.Escape(instance);
}

NAN_METHOD(UInt64::New)
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
    if (!FromArgument(info[0],value)) {
      return;
    }
    break;
  case 2:
    if ( (!info[0]->IsNumber())||(!info[1]->IsNumber()) ) {
      Nan::ThrowTypeError("Two argument constructor expects (Number,Number)");
      return;
    }
    value = ((uint64_t)info[0]->Uint32Value() << 32) | info[1]->Uint32Value();
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
    info.GetReturnValue().Set(NewInstance(value));
  }
}

#define RET(val) info.GetReturnValue().Set(val); return;
#define RETSTR(str) RET(Nan::New(str).ToLocalChecked())

NAN_GETTER(UInt64::GetSign)
{
  UInt64 *obj = Nan::ObjectWrap::Unwrap<UInt64>(info.Holder()); // or: This(info);
  RET((bool)(obj->value>>63));
}

NAN_SETTER(UInt64::SetSign)
{
  UInt64 *obj = Nan::ObjectWrap::Unwrap<UInt64>(info.Holder());
  if (value->BooleanValue()) {
    obj->value |= 0x8000000000000000;
  } else {
    obj->value &= 0x7fffffffffffffff;
  }
}

NAN_GETTER(UInt64::GetHi32)
{
  UInt64 *obj = Nan::ObjectWrap::Unwrap<UInt64>(info.Holder());
  RET((uint32_t)(obj->value>>32));
}

NAN_SETTER(UInt64::SetHi32)
{
  UInt64 *obj = Nan::ObjectWrap::Unwrap<UInt64>(info.Holder());
  obj->value = (obj->value&0xffffffff) | ((uint64_t)value->Uint32Value()<<32);
}

NAN_GETTER(UInt64::GetLo32)
{
  UInt64 *obj = Nan::ObjectWrap::Unwrap<UInt64>(info.Holder());
  RET((uint32_t)obj->value);
}

NAN_SETTER(UInt64::SetLo32)
{
  UInt64 *obj = Nan::ObjectWrap::Unwrap<UInt64>(info.Holder());
  obj->value = (obj->value&~0xfffffffff) | value->Uint32Value();
}

NAN_METHOD(UInt64::ToString)
{
  UInt64 *obj = This(info);
  if (!obj) {
    return;
  }
  uint64_t lhs = obj->value;
  int radix = 0;
  if (info[0]->IsUndefined()) {
    radix = 10;
  } else if (info[0]->IsNumber()) {
    radix = info[0]->Int32Value();
  }
  if (radix<0) { // internal
    radix=-radix;
    lhs=-lhs;
  }
  if ( (radix<2)||(radix>36) ) {
    Nan::ThrowRangeError("Radix must be between 2 and 36");
    return;
  }
  char scratch65[65];
  RETSTR(u64ToString(lhs, radix, scratch65));
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

