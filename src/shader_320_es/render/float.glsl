#ifndef FLOAT_GLSL
#define FLOAT_GLSL

#define PI      3.14159265358979323846
#define PI2     6.28318530717958647693
#define PI_2    1.57079632679489661923
#define LN2     0.69314718055994530942
#define SQRT3   1.73205080756887729353

void swap(inout float f1, inout float f2);

void swap(inout float f1, inout float f2) {
  uint i1 = floatBitsToUint(f1);
  uint i2 = floatBitsToUint(f2);
  i2 = i1 ^ i2;
  i1 = i1 ^ i2;
  i2 = i1 ^ i2;
  f1 = uintBitsToFloat(i1);
  f2 = uintBitsToFloat(i2);
}

#endif
