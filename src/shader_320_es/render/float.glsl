#ifndef FLOAT_GLSL
#define FLOAT_GLSL

#define PI      3.141592741
#define PI2     6.283185482
#define PI_2    1.570796371
#define LN2     0.6931471825
#define SQRT3   1.732050776

void swap(inout float f1, inout float f2);
float mul_pow2(float f, int p);

void swap(inout float f1, inout float f2) {
  uint i1 = floatBitsToUint(f1);
  uint i2 = floatBitsToUint(f2);
  i2 = i1 ^ i2;
  i1 = i1 ^ i2;
  i2 = i1 ^ i2;
  f1 = uintBitsToFloat(i1);
  f2 = uintBitsToFloat(i2);
  // f2 = f1 ^ f2;
  // f1 = f1 ^ f2;
  // f2 = f1 ^ f2;
}
float mul_pow2(float f, int p) {
  return f * pow(2.0, float(p));
  // if (p == 0) {
  //   return f;
  // }
  // int i = floatBitsToInt(f);
  // i += p << 23;
  // return intBitsToFloat(i);
}


#endif
