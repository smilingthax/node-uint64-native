
// Native: Getter/Setter: .hi32:Uint32, .lo32:Uint32, .sign:Boolean
//         Mutators: neg, not, abs,
//                   add, sub, rsub, and, or, xor, // same for signed
//                   shl, shr, sar, rol, ror,
//                   add2, sub2                    // take+return carry
//         Tests: eq, lt, gt, ilt, igt, isZero
//                UInt64.Compare, Int64.Compare
//         More: toString, clz, ctz

// TODO? .toString default radix==16 ?
// and/or:  .toHexString(padding?,signed?)  with leading '0x' ?

var u64=require('./build/Release/u64.node');
var UInt64=u64.UInt64,
    Int64=u64.Int64;

UInt64.prototype.clone = function() {
  return new this.constructor(this);
};

function hex32(v) { // assumes v>=0
  var ret=v.toString(16);
  return '00000000'.slice(ret.length) + ret;
}

UInt64.prototype.inspect = function() {
  return '<'+this.constructor.name+' '+hex32(this.hi32)+' '+hex32(this.lo32)+'>';
};

var rawToString = UInt64.prototype.toString;

UInt64.prototype.toString = function(radix) {
  if (radix===undefined) {
    radix=10;
  }
  return rawToString.call(this,radix,false);
};

UInt64.prototype.toSignedString = function(radix) {
  if (radix===undefined) {
    radix=10;
  }
  return rawToString.call(this,radix,true);
};


// TODO?
UInt64.prototype.cmp = function(type,other) {
  switch (type) {
  case '<':  return this.lessThan(other); // (un)signed for (U)Int64 !
  case '<=': return !this.greaterThan(other);
  case '>':  return this.greaterThan(other);
  case '>=': return !this.lessThan(other);
  case '==': return this.equals(other);
  case '!=': return !this.equals(other);
  default:
    throw new Error('Expected type to be one of ==, !=, <, <=, >, >=');
  }
};

var Bit54=new UInt64(0x00200000,0);

// modifies val to contain the remainder after subtracting the double...
UInt64.prototype.grabDouble = function(roundUp) {
/* basically:
  var ret=this.hi32*Math.pow(2,32)+this.lo32; // u64.doubleShift(this.hi32,32) ?
  this.sub(ret); // BUT: could now be "< 0", if ret was rounded upwards!
*/
  if (this.isZero()) {
    return 0;
  }
  var clz=this.clz();
  if (clz>=11) { // <=53bits: precise
    var ret=u64.buildDouble(+1,this.shl(clz-11),63-clz);
    this.and(0);
    return ret;
  }
  var mask=0x7ff>>clz; // bits that fall away, clz \in [1,10]
  var mantissa=this.bitShr(11-clz); // clones
  this.and(mask); // remainder
  if ( (roundUp>0)&&(!this.isZero()) ) {
    this.sub(mask+1);
    mantissa.add(1);
    if (mantissa.eq(Bit54)) { // overflow
      mantissa.shr(1);
      clz--; // possibly -1 now
    }
  }
  return u64.buildDouble(+1,mantissa,63-clz);
};


// Non-Mutating Api
UInt64.prototype.negate = function() {
  return this.clone().neg();
};

UInt64.prototype.magnitude = function() {
  return this.clone().abs();
};

UInt64.prototype.plus = function(rhs) {
  return this.clone().add(rhs);
};

UInt64.prototype.minus = function(rhs) {
  return this.clone().sub(rhs);
};

UInt64.prototype.bitNot = function() {
  return this.clone().not();
};

UInt64.prototype.bitAnd = function(rhs) {
  return this.clone().and(rhs);
};

UInt64.prototype.bitOr = function(rhs) {
  return this.clone().or(rhs);
};

UInt64.prototype.bitXor = function(rhs) {
  return this.clone().xor(rhs);
};

UInt64.prototype.bitShl = function(rhs) {
  return this.clone().shl(rhs);
};

UInt64.prototype.bitShr = function(rhs) {
  return this.clone().shr(rhs);
};

UInt64.prototype.bitSar = function(rhs) {
  return this.clone().sar(rhs);
};

UInt64.prototype.bitRol = function(rhs) {
  return this.clone().rol(rhs);
};

UInt64.prototype.bitRor = function(rhs) {
  return this.clone().ror(rhs);
};


// long forms
UInt64.prototype.toUnsignedString = UInt64.prototype.toString;

UInt64.prototype.equals = UInt64.prototype.eq;
UInt64.prototype.lessThan = UInt64.prototype.lt;
UInt64.prototype.unsignedLessThan = UInt64.prototype.lt;
UInt64.prototype.signedLessThan = UInt64.prototype.ilt; // "below"
UInt64.prototype.greaterThan = UInt64.prototype.gt;
UInt64.prototype.unsignedGreaterThan = UInt64.prototype.gt;
UInt64.prototype.signedGreaterThan = UInt64.prototype.igt; // "above"

UInt64.prototype.shiftLeft = UInt64.prototype.bitShl;
UInt64.prototype.shiftRight = UInt64.prototype.bitShr;
UInt64.prototype.unsignedShiftRight = UInt64.prototype.bitShr;
UInt64.prototype.signedShiftRight = UInt64.prototype.bitSar;
UInt64.prototype.rotateLeft = UInt64.prototype.bitRol;
UInt64.prototype.rotateRight = UInt64.prototype.bitRor;


// ... Int64 ...
Int64.prototype.toString = UInt64.prototype.toSignedString;
Int64.prototype.shiftRight = UInt64.prototype.bitSar;
Int64.prototype.lessThan = UInt64.prototype.ilt;
Int64.prototype.greaterThan = UInt64.prototype.igt;

// special case: roundAwayFromZero===+/-Infinity: round toward +/- Inf
Int64.prototype.grabDouble = function(roundAwayFromZero/*=false*/) {
  if (!this.sign) {
    return UInt64.prototype.grabDouble.call(this,roundAwayFromZero);
  }
  if (!isFinite(roundAwayFromZero || 0)) {
    roundAwayFromZero=-roundAwayFromZero; // *=-1
  }
  var ret=-UInt64.prototype.grabDouble.call(this.neg(),roundAwayFromZero);
  this.neg(); // adjust sign of remainder
  return ret;
};


module.exports=u64;



