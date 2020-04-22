#define UINT64_ZERO uvec2(0U, 0U)
#define UINT64_ONE uvec2(0U, 1U)

#define FULL_SHIFT 32U

uvec2 uint64_not(uvec2 a);
uvec2 uint64_and(uvec2 a, uvec2 b);
uvec2 uint64_or(uvec2 a, uvec2 b);
uvec2 uint64_xor(uvec2 a, uvec2 b);
uvec2 uint64_rshift(uvec2 a, uint shift);
uvec2 uint64_lshift(uvec2 a, uint shift);
uvec2 uint64_addOne(uvec2 a);
uvec2 uint64_add(uvec2 a, uvec2 b);
uvec2 uint64_sub(uvec2 a, uvec2 b);
uvec2 uint64_mul(uvec2 a, uvec2 b);
uvec2 uint64_div(uvec2 a, uvec2 b);
uvec2 uint64_mod(uvec2 a, uvec2 b);
void uint64_div_mod(uvec2 a, uvec2 b, out uvec2 quot, out uvec2 rem);
bool uint64_isEqual(uvec2 a, uvec2 b);
bool uint64_isNotEqual(uvec2 a, uvec2 b);
bool uint64_isGreater(uvec2 a, uvec2 b);
bool uint64_isSmaller(uvec2 a, uvec2 b);
bool uint64_isGreaterOrEqual(uvec2 a, uvec2 b);
bool uint64_isSmallerOrEqual(uvec2 a, uvec2 b);
uvec2 uint64_fromUint(uint a);
bool uint64_toBool(uvec2 a);
uint uint64_toUint(uvec2 a);

// Bitwise operations
uvec2 uint64_not(uvec2 a) {
  return uvec2(~a.x, ~a.y);
}
uvec2 uint64_and(uvec2 a, uvec2 b) {
  return uvec2(a.x & b.x, a.y & b.y);
}
uvec2 uint64_or(uvec2 a, uvec2 b) {
  return uvec2( a.x | b.x, a.y | b.y );
}
uvec2 uint64_xor(uvec2 a, uvec2 b) {
  return uvec2( a.x ^ b.x, a.y ^ b.y );
}
uvec2 uint64_rshift(uvec2 a, uint shift) {
  if (shift == 0U)
    return a;
  if (shift >= FULL_SHIFT)
    return uvec2( 0U, a.x >> (shift - FULL_SHIFT) );
  else return uvec2( a.x >> shift, (a.x << (FULL_SHIFT - shift)) + (a.y >> shift));
}
uvec2 uint64_lshift(uvec2 a, uint shift) {
  if (shift == 0U)
    return a;
  if (shift >= FULL_SHIFT)
    return uvec2( a.y << (shift - FULL_SHIFT), 0U );
  else return uvec2( (a.x << shift) + (a.y >> (FULL_SHIFT - shift)), a.y << shift );
}

// Arithmetic operations
uvec2 uint64_addOne(uvec2 a) {
  uint l = a.y + 1U;
  if (l > 0U)
    return uvec2( a.x, l );
  else return uvec2( a.x + 1U, l );
}
uvec2 uint64_add(uvec2 a, uvec2 b) {
  uint l = a.y + b.y;
  uint carry = uint(l < a.y);
  return uvec2( a.x + b.x + carry, l );
}
uvec2 uint64_sub(uvec2 a, uvec2 b) {
  b = uint64_not(b);
  b = uint64_addOne(b);
  return uint64_add(a, b);
}
uvec2 uint64_mul(uvec2 a, uvec2 b) {
  uvec2 res = UINT64_ZERO;
  for (uint i = 0U; i < FULL_SHIFT; ++i) {
    uint pos = 1U << i;
    if ((b.y & pos) > 0U)
      res = uint64_add(res, uint64_lshift(a, i));
    if ((b.x & pos) > 0U)
      res = uint64_add(res, uint64_lshift(a, i + FULL_SHIFT));
  }
  return res;
}
uvec2 uint64_div(uvec2 a, uvec2 b) {
  uvec2 rem = a;
  uvec2 quot = UINT64_ZERO;
  for (int i = 2 * int(FULL_SHIFT) - 1; i >= 0; --i) {
    uvec2 shifted = uint64_rshift(rem, uint(i));
    if (shifted.x == 0U && shifted.y == 0U)
      continue;
    quot = uint64_lshift(quot, 1U);
    if (uint64_isGreaterOrEqual(shifted, b)) {
      quot = uint64_addOne(quot);
      rem = uint64_sub(rem, uint64_lshift(b, uint(i)));
    }
  }
  return quot;
}
uvec2 uint64_mod(uvec2 a, uvec2 b) {
  uvec2 rem = a;
  uvec2 quot = UINT64_ZERO;
  for (int i = 2 * int(FULL_SHIFT) - 1; i >= 0; --i) {
    uvec2 shifted = uint64_rshift(rem, uint(i));
    quot = uint64_lshift(quot, 1U);
    if (uint64_isGreaterOrEqual(shifted, b)) {
      quot = uint64_addOne(quot);
      rem = uint64_sub(rem, uint64_lshift(b, uint(i)));
    }
  }
  return rem;
}
void uint64_div_mod(uvec2 a, uvec2 b, out uvec2 quot, out uvec2 rem) {
  rem = a;
  quot = UINT64_ZERO;
  for (int i = 2 * int(FULL_SHIFT) - 1; i >= 0; --i) {
    uvec2 shifted = uint64_rshift(rem, uint(i));
    quot = uint64_lshift(quot, 1U);
    if (uint64_isGreaterOrEqual(shifted, b)) {
      quot = uint64_addOne(quot);
      rem = uint64_sub(rem, uint64_lshift(b, uint(i)));
    }
  }
}

// Comparisons
bool uint64_isEqual(uvec2 a, uvec2 b) {
  return a.x == b.x && a.y == b.y;
}
bool uint64_isNotEqual(uvec2 a, uvec2 b) {
  return a.x != b.x || a.y != b.y;
}
bool uint64_isGreater(uvec2 a, uvec2 b) {
  if (a.x == b.x)
    return a.y > b.y;
  else return a.x > b.x;
}
bool uint64_isSmaller(uvec2 a, uvec2 b) {
  if (a.x == b.x)
    return a.y < b.y;
  else return a.x < b.x;
}
bool uint64_isGreaterOrEqual(uvec2 a, uvec2 b) {
  if (a.x == b.x)
    return a.y >= b.y;
  else return a.x >= b.x;
}
bool uint64_isSmallerOrEqual(uvec2 a, uvec2 b) {
  if (a.x == b.x)
    return a.y <= b.y;
  else return a.x <= b.x;
}

// Conversions
uvec2 uint64_fromUint(uint a) {
  return uvec2(0U, a);
}
bool uint64_toBool(uvec2 a) {
  return a.x != 0U || a.y != 0U;
}
uint uint64_toUint(uvec2 a) {
  return a.y;
}
