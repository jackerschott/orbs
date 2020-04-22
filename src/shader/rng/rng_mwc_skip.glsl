uvec2 MWC_AddMod64(uvec2 a, uvec2 b, uvec2 M) {
  uvec2 v = uint64_add(a, b);
  if (uint64_isGreaterOrEqual(v, M) || uint64_isSmaller(v, a))
    v = uint64_sub(v, M);
  return v;
}

uvec2 MWC_MulMod64(uvec2 a, uvec2 b, uvec2 M) {
  uvec2 r = UINT64_ZERO;
  while (uint64_isNotEqual(a, UINT64_ZERO)) {
    if (uint64_toBool(uint64_and(a, UINT64_ONE)))
      r = MWC_AddMod64(r, b, M);
    b = MWC_AddMod64(b, b, M);
    a = uint64_rshift(a, 1U);
  }
  return r;
}

uvec2 MWC_PowMod64(uvec2 a, uvec2 e, uvec2 M) {
  uvec2 sqr = a;
  uvec2 acc = UINT64_ONE;
  while (uint64_isNotEqual(e, UINT64_ZERO)) {
    if (uint64_toBool(uint64_and(e, UINT64_ONE)))
      acc = MWC_MulMod64(acc, sqr, M);
    sqr = MWC_MulMod64(sqr, sqr, M);
    e = uint64_rshift(e, 1U);
  }
  return acc;
}

uvec2 MWC_SkipImpl_Mod64(uvec2 curr, uvec2 A, uvec2 M, uvec2 distance) {
  uvec2 m = MWC_PowMod64(A, distance, M);
  uvec2 x = uint64_add(uint64_mul(uint64_fromUint(curr.x), A), uint64_fromUint(curr.y));
  x = MWC_MulMod64(x, m, M);
  
  uvec2 quot, rem;
  uint64_div_mod(x, A, quot, rem);
  return uvec2(uint64_toUint(quot), uint64_toUint(rem));
}

uvec2 MWC_SeedImpl_Mod64(uvec2 A, uvec2 M, uint vecSize, uint vecOffset, uvec2 streamBase, uvec2 streamGap, uint stream_id) {
  // long representation: 4077358422479273989
  #define MWC_BASEID uvec2(3922919429U, 949333985U)

  uvec2 dist = uint64_add(streamBase, uint64_mul(uint64_fromUint(stream_id * vecSize + vecOffset), streamGap));
  uvec2 m = MWC_PowMod64(A, dist, M);

  uvec2 x = MWC_MulMod64(MWC_BASEID, m, M);

  uvec2 quot, rem;
  uint64_div_mod(x, A, quot, rem);
  return uvec2(uint64_toUint(quot), uint64_toUint(rem));
}
