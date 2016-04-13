# UInt64 Native
Native UInt64 object for Nodejs.

This module provides two classes `UInt64` and `Int64`.

### Example

```js
// TODO
var i = new UInt64();
```

## UInt64

#### `UInt64.toString(radix)`
#### `UInt64.toSignedString(radix)`

#### `UInt64.cmp(type, other)`

Compares the instance to `other`,
`type` should be one of `==`, `!=`, `<`, `<=`, `>`, `>=`.

#### `UInt64.grabDouble(roundUp)`

#### `UInt64.eq()` / `.equals()`
#### `UInt64.lt()` / `.lessThan()`
#### `UInt64.lt()` / `.unsignedLessThan()`
#### `UInt64.ilt()` / `.signedLessThan()`
#### `UInt64.gt()` / `.greaterThan()`
#### `UInt64.gt()` / `.unsignedGreaterThan()`
#### `UInt64.igt()` / `.signedGreaterThan()`
#### `UInt64.bitShl()` / `.shiftLeft()`
#### `UInt64.bitShr()` / `.shiftRight()`
#### `UInt64.bitShr()` / `.unsignedShiftRight()`
#### `UInt64.bitSar()` / `.signedShiftRight()`
#### `UInt64.bitRol()` / `.rotateLeft()`
#### `UInt64.bitRor()` / `.rotateRight()`

#### `UInt64.neg()`
#### `UInt64.abs()`
#### `UInt64.add(rhs)`
#### `UInt64.sub(rhs)`
#### `UInt64.not()`
#### `UInt64.and(rhs)`
#### `UInt64.or(rhs)`
#### `UInt64.xor(rhs)`
#### `UInt64.shl(rhs)`
#### `UInt64.shr(rhs)`
#### `UInt64.sar(rhs)`
#### `UInt64.rol(rhs)`
#### `UInt64.ror(rhs)`


### Non Mutating functions

These functions provide the same functionality as above but return a new instance.

#### `UInt64.negate()`
#### `UInt64.magnitude()`
#### `UInt64.plus(rhs)`
#### `UInt64.minus(rhs)`
#### `UInt64.bitNot()`
#### `UInt64.bitAnd(rhs)`
#### `UInt64.bitOr(rhs)`
#### `UInt64.bitXor(rhs)`
#### `UInt64.bitShl(rhs)`
#### `UInt64.bitShr(rhs)`
#### `UInt64.bitSar(rhs)`
#### `UInt64.bitRol(rhs)`
#### `UInt64.bitRor(rhs)`

## Int64

#### `UInt64.toString()` / `.toSignedString()`
#### `UInt64.bitSar()` / `.shiftRight()`
#### `UInt64.ilt()` / `.lessThan()`
#### `UInt64.igt()` / `.greaterThan()`

#### `Int64.grabDouble(roundAwayFromZero)`
