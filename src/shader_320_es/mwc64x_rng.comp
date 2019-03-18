#define MWC64X_A 4294883355U
// long representation: 18446383549859758079
#define MWC64X_M uvec2(4294967295U, 4294883354U)

void MWC64X_Step(inout uvec2 s) {
  uint x = s.x;
  uint c = s.y;

  uint xn = MWC64X_A * x + c;
  uint carry = uint(xn < c);
  uint cn = uint64_mul(uint64_fromUint(MWC64X_A), uint64_fromUint(x)).y + carry;

  s.x = xn;
  s.y = cn;
}

void MWC64X_Skip(inout uvec2 s, uvec2 distance) {
  uvec2 tmp = MWC_SkipImpl_Mod64(s, uint64_fromUint(MWC64X_A), MWC64X_M, distance);
  s.x = tmp.x;
  s.y = tmp.y;
}

void MWC64X_SeedStreams(out uvec2 s, uvec2 baseOffset, uvec2 perStreamOffset, uint stream_id) {
  uvec2 tmp = MWC_SeedImpl_Mod64(uint64_fromUint(MWC64X_A), MWC64X_M, 1U, 0U, baseOffset, perStreamOffset, stream_id);
  s.x = tmp.x;
  s.y = tmp.y;
}

uint MWC64X_NextUint(inout uvec2 s) {
  uint res = s.x ^ s.y;
  MWC64X_Step(s);
  return res;
}
