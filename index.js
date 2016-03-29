
// Native: Getter/Setter: .hi32:Uint32, .lo32:Uint32, .sign:Boolean
//         Mutators: neg, not,
//                   add, sub, rsub, and, or, xor  // same for signed
//         Tests: isZero, eq, lt, gt
//         More: toString, clz, ctz

// TODO? .toString default radix==16 ?
// and/or:  .toHexString(padding?,signed?)  with leading '0x' ?

var u64=require('./build/Release/u64.node');

u64.UInt64.prototype.clone = function() {
  return new u64.UInt64(this);
};

function hex32(v) { // assumes v>=0
  var ret=v.toString(16);
  return '00000000'.slice(ret.length) + ret;
}

u64.UInt64.prototype.inspect = function() {
  return '<UInt64 '+hex32(this.hi32)+' '+hex32(this.lo32)+'>';
};

u64.UInt64.prototype.toSignedString = function(radix) {
  if (this.sign) {
    return '-' + this.toString((radix===undefined) ? -10 : -radix);
  } else {
    return this.toString(radix);
  }
};

// Non-Mutating
u64.UInt64.negate = function() {
  return this.clone().neg();
};

u64.UInt64.plus = function(rhs) {
  return this.clone().add(rhs);
};

u64.UInt64.minus = function(rhs) {
  return this.clone().sub(rhs);
};

u64.UInt64.bitNot = function() {
  return this.clone().not();
};

u64.UInt64.bitAnd = function(rhs) {
  return this.clone().and(rhs);
};

u64.UInt64.bitOr = function(rhs) {
  return this.clone().or(rhs);
};

u64.UInt64.bitXor = function(rhs) {
  return this.clone().xor(rhs);
};

// long forms
u64.UInt64.equals = u64.UInt64.eq;
u64.UInt64.lessThan = u64.UInt64.lt;
u64.UInt64.greaterThan = u64.UInt64.gt;

module.exports=u64.UInt64;

