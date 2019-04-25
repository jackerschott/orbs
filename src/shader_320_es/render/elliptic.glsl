#ifndef ELLIPTIC_GLSL
#define ELLIPTIC_GLSL

const float r = 1.0e-18;

vec2 elliptic_k(vec2 m);
vec2 elliptic_e(vec2 m);
vec2 elliptic_pi(vec2 m, vec2 n);
vec2 elliptic_f(vec2 phi, vec2 m);
vec2 elliptic_e(vec2 phi, vec2 m);
vec2 elliptic_pi(vec2 phi, vec2 m, vec2 n);
vec2 RF(vec2 x, vec2 y, vec2 z);
vec2 RJ(vec2 x, vec2 y, vec2 z, vec2 p);
vec2 RD(vec2 x, vec2 y, vec2 z);
vec2 RC(vec2 x, vec2 y);
vec2 RG(vec2 x, vec2 y, vec2 z);
vec2 RF_xy0(vec2 x, vec2 y);
vec2 RG_xy0(vec2 x, vec2 y);
vec2 RD_0yz(vec2 y, vec2 z);

vec2 elliptic_k(vec2 m) {
  return RF_xy0(C_ONE - m, C_ONE);
}
vec2 elliptic_e(vec2 m) {
  return mul_pow2(RG_xy0(C_ONE - m, C_ONE), 1);
}
vec2 elliptic_pi(vec2 m, vec2 n) {
  vec2 y = C_ONE - m;
  vec2 p = C_ONE - n;
  return RF_xy0(y, C_ONE) + cmul(n, RJ(C_ZERO, y, C_ONE, p)) / 3.0;
}
vec2 elliptic_f(vec2 phi, vec2 m) {
  int k = int(floor(mul_pow2(floor(floor(phi.x / PI_2) + 1.0), -1)));
  vec2 A = C_ZERO;
  if (k != 0) {
    phi = vec2(phi.x - float(k) * PI, phi.y);
    A = mul_pow2(float(k) * elliptic_k(m), 1);
  }

  if (phi.x == PI_2) {
    float sh = sinh(phi.y);
    float ch = cosh(phi.y);

    vec2 x = C_FROM_REAL(-sh * sh);
    vec2 y = C_ONE - (ch * ch) * m;
    return ch * RF(x, y, C_ONE) + A;
  }
  else if (phi.x == -PI_2) {
    float sh = sinh(phi.y);
    float ch = sinh(phi.x);

    vec2 x = C_FROM_REAL(-sh * sh);
    vec2 y = C_ONE - (ch * ch) * m;
    vec2 res = ch * RF(x, y, C_ONE);
    return vec2(-res.x, res.y) + A;
  }

  vec2 s, c;
  csincos(phi, s, c);
  vec2 x = cmul(c, c);
  vec2 y = C_ONE - cmul(m, cmul(s, s));
  return cmul(s, RF(x, y, C_ONE)) + A;
}
vec2 elliptic_e(vec2 phi, vec2 m) {
  int k = int(floor(mul_pow2(floor(floor(phi.x / PI_2) + 1.0), -1)));
  vec2 A = C_ZERO;
  if (k != 0) {
    phi = vec2(phi.x - float(k) * PI, phi.y);
    A = mul_pow2(float(k) * elliptic_e(m), 1);
  }

  if (phi.x == PI_2) {
    float sh = sinh(phi.y);
    float ch = cosh(phi.y);

    vec2 x = C_FROM_REAL(-sh * sh);
    vec2 y = C_ONE - (ch * ch) * m;
    return ch * RF(x, y, C_ONE) - cmul((ch * ch * ch) * m, RD(x, y, C_ONE)) / 3.0 + A;
  }
  else if (phi.x == -PI_2) {
    float sh = sinh(phi.y);
    float ch = cosh(phi.y);

    vec2 x = C_FROM_REAL(-sh * sh);
    vec2 y = C_ONE - (ch * ch) * m;
    vec2 res = ch * RF(x, y, C_ONE) - cmul((ch * ch * ch) * m, RD(x, y, C_ONE)) / 3.0;
    return vec2(-res.x, res.y) + A;
  }

  vec2 s, c;
  csincos(phi, s, c);
  vec2 s2 = cmul(s, s);
  vec2 s3 = cmul(s2, s);
  vec2 x = cmul(c, c);
  vec2 y = C_ONE - cmul(m, s2);
  return cmul(s, RF(x, y, C_ONE)) - cmul(cmul(m, s3), RD(x, y, C_ONE)) / 3.0 + A;
}
vec2 elliptic_pi(vec2 phi, vec2 m, vec2 n) {
  int k = int(floor(mul_pow2(floor(floor(phi.x / PI_2) + 1.0), -1)));
  vec2 A = C_ZERO;
  if (k != 0) {
    phi = vec2(phi.x - float(k) * PI, phi.y);
    A = mul_pow2(float(k) * elliptic_pi(m, n), 1);
  }

  if (phi.x == PI_2) {
    float sh = sinh(phi.y);
    float ch = cosh(phi.y);
    float ch2 = ch * ch;

    vec2 x = C_FROM_REAL(-sh * sh);
    vec2 y = C_ONE - ch2 * m;
    vec2 p = C_ONE - ch2 * n;
    return ch * RF(x, y, C_ONE) + cmul((ch2 * ch) * n, RJ(x, y, C_ONE, p)) / 3.0 + A;
  }
  else if (phi.x == -PI_2) {
    float sh = sinh(phi.y);
    float ch = cosh(phi.y);
    float ch2 = ch * ch;

    vec2 x = C_FROM_REAL(-sh * sh);
    vec2 y = C_ONE - ch2 * m;
    vec2 p = C_ONE - ch2 * n;
    vec2 res = ch * RF(x, y, C_ONE) + cmul((-ch2 * ch) * n, RJ(x, y, C_ONE, p)) / 3.0 + A;
    return vec2(-res.x, res.y);
  }

  vec2 s, c;
  csincos(phi, s, c);
  vec2 s2 = cmul(s, s);
  vec2 s3 = cmul(s2, s);
  vec2 x = cmul(c, c);
  vec2 y = C_ONE - cmul(m, s2);
  vec2 p = C_ONE - cmul(n, s2);
  return cmul(s, RF(x, y, C_ONE)) + cmul(cmul(n, s3), RJ(x, y, C_ONE, p)) / 3.0 + A;
}

vec2 RF(vec2 x, vec2 y, vec2 z) {
  vec2 xn = x;
  vec2 yn = y;
  vec2 zn = z;
  vec2 A0 = (x + y + z) / 3.0;
  float Q = pow(3.0 * r, -1.0 / 6.0) * sqrt(max(max(cabs2(A0 - x), cabs2(A0 - y)), cabs2(A0 - z)));

  float f = 1.0;
  vec2 A = A0;
  while (true) {
    vec2 sx = csqrt(xn);
    vec2 sy = csqrt(yn);
    vec2 sz = csqrt(zn);
    vec2 lda = cmul(sx, sy + sz) + cmul(sy, sz);
    // A = mul_pow2(A + lda, -2);
    // if (f * f * Q * Q < cabs2(A))
    //   break;
    // xn = mul_pow2(xn + lda, -2);
    // yn = mul_pow2(yn + lda, -2);
    // zn = mul_pow2(zn + lda, -2);
    // f = mul_pow2(f, -2);
    A = 0.25 * (A + lda);
    if (f * f * Q * Q < cabs2(A))
      break;
    xn = 0.25 * (xn + lda);
    yn = 0.25 * (yn + lda);
    zn = 0.25 * (zn + lda);
    f /= 4.0;
  }

  vec2 X = f * cdiv(A0 - x, A);
  vec2 Y = f * cdiv(A0 - y, A);
  vec2 Z = -X - Y;

  vec2 E_ = cmul(X, Y);
  vec2 E2 = E_ - cmul(Z, Z);
  vec2 E3 = cmul(E_, Z);
  vec2 res = (C_FROM_REAL(9240.0) - 924.0 * E2 + 660.0 * E3 + 385.0 * cmul(E2, E2) - 630.0 * cmul(E2, E3)) / 9240.0;
  return cdiv(res, csqrt(A));
}
vec2 RJ(vec2 x, vec2 y, vec2 z, vec2 p) {
  vec2 p_y;
  vec2 y_q;
  vec2 G;
  bool princVal = x.x >= 0.0 && y.x >= 0.0 && z.x >= 0.0 && x.y == 0.0 && y.y == 0.0 && z.y == 0.0 && p.y == 0.0 && p.x < 0.0;
  if (princVal) {
    vec2 z_y_y_x = cmul(z - y, y - x);
    if (z_y_y_x.x < 0.0) {
      swap(x, y);
      z_y_y_x = -z_y_y_x;
    }
    vec2 q = -p;
    y_q = y + q;
    p_y = cdiv(z_y_y_x, y_q);
    p = p_y + y;
    vec2 xz = cmul(x, z);
    vec2 pq = cmul(p, q);
    vec2 xz_pq = xz + pq;
    G = 3.0 * (csqrt(cdiv(cmul(xz, y), xz_pq)) * RC(xz_pq, pq) - RF(x, y, z));
  }

  vec2 xn = x;
  vec2 yn = y;
  vec2 zn = z;
  vec2 pn = p;
  vec2 A0 = (x + y + z + mul_pow2(p, 1)) / 5.0;
  vec2 delta = cmul(cmul(p - x, p - y), p - z);
  float Q = pow(mul_pow2(r, -2), -1.0 / 6.0) * sqrt(max(max(cabs2(A0 - x), cabs2(A0 - y)), max(cabs2(A0 - z), cabs2(A0 - p))));

  float f = 1.0;
  vec2 A = A0;
  vec2 rc_sum = C_ZERO;
  vec2 d;
  vec2 e;
  bool first = true;
  while (true) {
    if (!first)
      rc_sum += mul_pow2(f * cdiv(RC(C_ONE, C_ONE + e), d), 2);
    else first = false;
    vec2 sx = csqrt(xn);
    vec2 sy = csqrt(yn);
    vec2 sz = csqrt(zn);
    vec2 lda = cmul(sx, sy + sz) + cmul(sy, sz);
    A = mul_pow2(A + lda, -2);
    vec2 sp = csqrt(pn);
    d = cmul(cmul(sp + sx, sp + sy), sp + sz);
    e = f * f * f * cdiv(delta, cmul(d, d));
    if (f * f * Q * Q < cabs2(A))
      break;
    xn = mul_pow2(xn + lda, -2);
    yn = mul_pow2(yn + lda, -2);
    zn = mul_pow2(zn + lda, -2);
    pn = mul_pow2(pn + lda, -2);
    f = mul_pow2(f, -2);
  }

  vec2 X = f * cdiv(A0 - x, A);
  vec2 Y = f * cdiv(A0 - y, A);
  vec2 Z = f * cdiv(A0 - z, A);
  vec2 P = mul_pow2(-X - Y - Z, -1);

  vec2 XY = cmul(X, Y);
  vec2 XYZ = cmul(XY, Z);
  vec2 PP = cmul(P, P);
  vec2 PPP = cmul(PP, P);
  vec2 E2 = XY + cmul(Z, X + Y) - 3.0 * PP;
  vec2 E2P = cmul(E2, P);
  vec2 E3 = XYZ + mul_pow2(E2P, 1) + mul_pow2(PPP, 2);
  vec2 E4 = cmul(mul_pow2(XYZ, 1) + E2P + 3.0 * PPP, P);
  vec2 E5 = cmul(XYZ, PP);

  vec2 res = (C_FROM_REAL(24024.0) - 5148.0 * E2 + 4004.0 * E3 + 2457.0 * cmul(E2, E2) - 3276.0 * E4 - 4158.0 * cmul(E2, E3) + 2772.0 * E5) / 24024.0;
  res = f * cdiv(res, cmul(A, csqrt(A))) + 6.0 * rc_sum;
  if (princVal)
    return cdiv(cmul(p_y, res) + G, y_q);
  else return res;
}
vec2 RD(vec2 x, vec2 y, vec2 z) {
  vec2 xn = x;
  vec2 yn = y;
  vec2 zn = z;
  vec2 A0 = (x + y + 3.0 * z) / 5.0;
  float Q = pow(mul_pow2(r, -2), -1.0 / 6.0) * sqrt(max(max(cabs2(A0 - x), cabs2(A0 - y)), cabs2(A0 - z)));

  float f = 1.0;
  vec2 A = A0;
  vec2 rem_sum = C_ZERO;
  vec2 sz;
  vec2 z_lda;
  bool first = true;
  while (true) {
    if (!first)
      rem_sum += cdiv(C_FROM_REAL(mul_pow2(f, 2)), cmul(sz, z_lda));
    else first = false;
    vec2 sx = csqrt(xn);
    vec2 sy = csqrt(yn);
    sz = csqrt(zn);
    vec2 lda = cmul(sx, sy + sz) + cmul(sy, sz);
    A = mul_pow2(A + lda, -2);
    if (f * f * Q * Q < cabs2(A))
      break;
    z_lda = zn + lda;
    xn = mul_pow2(xn + lda, -2);
    yn = mul_pow2(yn + lda, -2);
    zn = mul_pow2(z_lda, -2);
    f = mul_pow2(f, -2);
  }

  vec2 X = f * cdiv(A0 - x, A);
  vec2 Y = f * cdiv(A0 - y, A);
  vec2 Z = -(X + Y) / 3.0;

  vec2 XY = cmul(X, Y);
  vec2 ZZ = cmul(Z, Z);
  vec2 E2 = XY - 6.0 * ZZ;
  vec2 E3 = cmul(3.0 * XY - mul_pow2(ZZ, 3), Z);
  vec2 E4 = 3.0 * cmul(XY - ZZ, ZZ);
  vec2 E5 = cmul(XY, cmul(ZZ, Z));

  vec2 res = C_FROM_REAL(24024.0) - 5148.0 * E2 + 4004.0 * E3 + 2457.0 * cmul(E2, E2) - 3276.0 * E4 - 4158.0 * cmul(E2, E3) + 2772.0 * E5;
  return f * cdiv(res, cmul(csqrt(A), A)) + 3.0 * rem_sum;
}
vec2 RC(vec2 x, vec2 y) {
  bool princVal = y.y == 0.0 && y.x < 0.0;
  vec2 F;
  if (princVal) {
    if (x == C_ZERO)
      return C_ZERO;
    F = x;
    y = -y;
    x = x + y;
    F = csqrt(cdiv(F, x));
  }

  vec2 xn = x;
  vec2 yn = y;
  vec2 A0 = (x + mul_pow2(y, 1)) / 3.0;
  float Q = pow(3.0 * r, -0.125) * sqrt(cabs2(A0 - x));

  float f = 1.0;
  vec2 A = A0;
  while (true) {
    vec2 lda = mul_pow2(cmul(csqrt(xn), csqrt(yn)), 1) + yn;
    A = mul_pow2(A + lda, -2);
    f = mul_pow2(f, -2);
    if (f * f * Q * Q < cabs2(A))
      break;
    xn = mul_pow2(xn + lda, -2);
    yn = mul_pow2(yn + lda, -2);
  }
  vec2 s = f * cdiv(y - A0, A);
  vec2 s2 = cmul(s, s);
  vec2 s3 = cmul(s2, s);
  vec2 s4 = cmul(s3, s);
  vec2 s5 = cmul(s4, s);
  vec2 s6 = cmul(s5, s);
  vec2 s7 = cmul(s6, s);
  vec2 res = (C_FROM_REAL(80080.0) + 24024.0 * s2 + 11440.0 * s3 + 30030.0 * s4 + 32760.0 * s5 + 61215.0 * s6 + 90090.0 * s7) / 80080.0;
  if (princVal)
    return cmul(F, cdiv(res, csqrt(A)));
  return cdiv(res, csqrt(A));
}
vec2 RG(vec2 x, vec2 y, vec2 z) {
  vec2 t1 = cmul(z, RF(x, y, z));
  vec2 t2 = cmul(cmul(x - z, y - z), RD(x, y, z)) / 3.0;
  vec2 t3 = csqrt(cdiv(cmul(x, y), z));
  return mul_pow2(t1 - t2 + t3, -1);
}

vec2 RF_xy0(vec2 x, vec2 y) {
  const float r = 1.0e-10;

  x = csqrt(x);
  y = csqrt(y);

  while (true) {
    vec2 x_ = x;
    x = mul_pow2(x_ + y, -1);
    y = csqrt(cmul(x_, y));
    if (cabs2(x - y) < 7.29 * r * cabs2(x))
      break;
  }

  return cdiv(C_FROM_REAL(PI), x + y);
}
vec2 RG_xy0(vec2 x, vec2 y) {
  const float r = 1.0e-10;

  vec2 x0 = csqrt(x);
  vec2 y0 = csqrt(y);
  x = x0;
  y = y0;
  vec2 x_y = x - y;
  vec2 sum = C_ZERO;
  float f = 0.25;
  bool first = true;
  while (true) {
    if (!first)
      sum += f * cmul(x_y, x_y);
    else first = false;
    vec2 x_ = x;
    x = mul_pow2(x_ + y, -1);
    y = csqrt(cmul(x_, y));
    x_y = x - y;
    if (cabs2(x_y) < 7.29 * r * cabs2(x))
      break;
    f = mul_pow2(f, 1);
  }

  vec2 m = mul_pow2(x0 + y0, -1);
  vec2 res = cmul(m, m) - sum;
  return mul_pow2(PI * cdiv(res, x + y), -1);
}
vec2 RD_0yz(vec2 y, vec2 z) {
  const float r = 1.0e-10;

  vec2 yo = y;
  vec2 zo = z;
  vec2 y0 = csqrt(y);
  vec2 z0 = csqrt(z);
  y = y0;
  z = z0;
  vec2 y_z = y - z;
  vec2 sum = C_ZERO;
  float f = 0.25;
  bool first = true;
  while (true) {
    if (!first)
      sum += f * cmul(y_z, y_z);
    else first = false;
    vec2 y_ = y;
    y = mul_pow2(y_ + z, -1);
    z = csqrt(cmul(y_, z));
    y_z = y - z;
    if (cabs2(y_z) < 7.29 * r * cabs2(y))
      break;
    f = mul_pow2(f, 1);
  }

  vec2 m = mul_pow2(y0 + z0, -1);
  vec2 RF = cdiv(C_FROM_REAL(PI), y + z);
  vec2 RG = cmul(m, m) - sum;
  RG = mul_pow2(cmul(RG, RF), -1);
  return 3.0 * cdiv(mul_pow2(RG, 1) - cmul(zo, RF), cmul(zo, yo - zo));
}

#endif
