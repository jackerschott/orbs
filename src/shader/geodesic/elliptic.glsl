#ifndef ELLIPTIC_GLSL
#define ELLIPTIC_GLSL

#define N_LOOPS_MAX 100

const float r = 1.0e-7; // 1.0e-18;
const float r0 = 1.0e-7; // 1.0e-10;

vec2 elliptic_k(vec2 m);
vec2 elliptic_e(vec2 m);
vec2 elliptic_pi(vec2 m, vec2 n);
vec2 elliptic_f(vec2 phi, vec2 m);
vec2 elliptic_e(vec2 phi, vec2 m);
vec2 elliptic_pi(vec2 phi, vec2 m, vec2 n);
void elliptic_fe(vec2 phi, vec2 m, out vec2 resF, out vec2 resE);
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
  return 2.0 * RG_xy0(C_ONE - m, C_ONE);
}
vec2 elliptic_pi(vec2 m, vec2 n) {
  vec2 y = C_ONE - m;
  vec2 p = C_ONE - n;
  return RF_xy0(y, C_ONE) + cmul(n, RJ(C_ZERO, y, C_ONE, p)) / 3.0;
}
vec2 elliptic_f(vec2 phi, vec2 m) {
  int k = int(floor(0.5 * floor(floor(phi.x / PI_2) + 1.0)));
  vec2 A = C_ZERO;
  if (k != 0) {
    phi = vec2(phi.x - float(k) * PI, phi.y);
    A = 2.0 * float(k) * elliptic_k(m);
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
    float ch = cosh(phi.y);

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
  int k = int(floor(0.5 * floor(floor(phi.x / PI_2) + 1.0)));
  vec2 A = C_ZERO;
  if (k != 0) {
    phi = vec2(phi.x - float(k) * PI, phi.y);
    A = 2.0 * float(k) * elliptic_e(m);
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
  int k = int(floor(0.5 * floor(floor(phi.x / PI_2) + 1.0)));
  vec2 A = C_ZERO;
  if (k != 0) {
    phi = vec2(phi.x - float(k) * PI, phi.y);
    A = 2.0 * float(k) * elliptic_pi(m, n);
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
void elliptic_fe(vec2 phi, vec2 m, out vec2 resF, out vec2 resE) {
  int k = int(floor(0.5 * floor(floor(phi.x / PI_2) + 1.0)));
  vec2 A_k = C_ZERO;
  vec2 A_e = C_ZERO;
  if (k != 0) {
    float k_ = float(k);
    float k2_ = 2.0 * k_;
    phi = vec2(phi.x - k_ * PI, phi.y);
    A_k = k2_ * elliptic_k(m);
    A_e = k2_ * elliptic_e(m);
  }

  if (phi.x == PI_2) {
    float sh = sinh(phi.y);
    float ch = cosh(phi.y);
    float ch2 = ch * ch;

    vec2 x = C_FROM_REAL(-sh * sh);
    vec2 y = C_ONE - ch2 * m;
    vec2 ch_RF = ch * RF(x, y, C_ONE);
    resF = ch_RF + A_k;
    resE = ch_RF - cmul((ch2 * ch) * m, RD(x, y, C_ONE)) / 3.0 + A_e;
    return;
  }
  else if (phi.x == -PI_2) {
    float sh = sinh(phi.y);
    float ch = cosh(phi.y);
    float ch2 = ch * ch;

    vec2 x = C_FROM_REAL(-sh * sh);
    vec2 y = C_ONE - ch2 * m;
    vec2 ch_RF = ch * RF(x, y, C_ONE);
    vec2 F = ch_RF;
    vec2 E = ch_RF - cmul((ch2 * ch) * m, RD(x, y, C_ONE)) / 3.0;
    resF = vec2(-F.x, F.y) + A_k;
    resE = vec2(-E.x, E.y) + A_e;
    return;
  }

  vec2 s, c;
  csincos(phi, s, c);
  vec2 s2 = cmul(s, s);
  vec2 s3 = cmul(s2, s);
  vec2 x = cmul(c, c);
  vec2 y = C_ONE - cmul(m, cmul(s, s));
  vec2 s_RF = cmul(s, RF(x, y, C_ONE));
  resF = s_RF + A_k;
  resE = s_RF - cmul(cmul(m, s3), RD(x, y, C_ONE)) / 3.0 + A_e;
}
vec2 RF(vec2 x, vec2 y, vec2 z) {
  vec2 xn = x;
  vec2 yn = y;
  vec2 zn = z;
  vec2 A0 = (x + y + z) / 3.0;
  float Q2 = pow(3.0 * r, -1.0 / 3.0) * max(max(cabs2(A0 - x), cabs2(A0 - y)), cabs2(A0 - z));

  float f = 1.0;
  vec2 A = A0;
  for (int i = 0; i < N_LOOPS_MAX; ++i) {
    vec2 sx = csqrt(xn);
    vec2 sy = csqrt(yn);
    vec2 sz = csqrt(zn);
    vec2 lda = cmul(sx, sy + sz) + cmul(sy, sz);
    A = 0.25 * (A + lda);
    if (f * f * Q2 < cabs2(A))
      break;
    xn = 0.25 * (xn + lda);
    yn = 0.25 * (yn + lda);
    zn = 0.25 * (zn + lda);
    f *= 0.25;
  }

  vec2 X = f * cdiv(A0 - x, A);
  vec2 Y = f * cdiv(A0 - y, A);
  vec2 Z = -(X + Y);

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
  vec2 A0 = (x + y + z + 2.0 * p) / 5.0;
  vec2 delta = cmul(cmul(p - x, p - y), p - z);
  float Q = pow(0.25 * r, -1.0 / 6.0) * sqrt(max(max(cabs2(A0 - x), cabs2(A0 - y)), max(cabs2(A0 - z), cabs2(A0 - p))));

  float f = 1.0;
  vec2 A = A0;
  vec2 rc_sum = C_ZERO;
  vec2 d = C_ZERO;
  vec2 e = C_ZERO;
  bool first = true;
  for (int i = 0; i < N_LOOPS_MAX; ++i) {
    if (!first)
      rc_sum += 4.0 * f * cdiv(RC(C_ONE, C_ONE + e), d);
    else first = false;
    vec2 sx = csqrt(xn);
    vec2 sy = csqrt(yn);
    vec2 sz = csqrt(zn);
    vec2 lda = cmul(sx, sy + sz) + cmul(sy, sz);
    A = 0.25 * (A + lda);
    vec2 sp = csqrt(pn);
    d = cmul(cmul(sp + sx, sp + sy), sp + sz);
    e = f * f * f * cdiv(delta, cmul(d, d));
    if (f * f * Q * Q < cabs2(A))
      break;
    xn = 0.25 * (xn + lda);
    yn = 0.25 * (yn + lda);
    zn = 0.25 * (zn + lda);
    pn = 0.25 * (pn + lda);
    f *= 0.25;
  }

  vec2 X = f * cdiv(A0 - x, A);
  vec2 Y = f * cdiv(A0 - y, A);
  vec2 Z = f * cdiv(A0 - z, A);
  vec2 P = 0.5 * (-X - Y - Z);

  vec2 XY = cmul(X, Y);
  vec2 XYZ = cmul(XY, Z);
  vec2 PP = cmul(P, P);
  vec2 PPP = cmul(PP, P);
  vec2 E2 = XY + cmul(Z, X + Y) - 3.0 * PP;
  vec2 E2P = cmul(E2, P);
  vec2 E3 = XYZ + 2.0 * E2P + 4.0 * PPP;
  vec2 E4 = cmul(2.0 * XYZ + E2P + 3.0 * PPP, P);
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
  float Q2 = pow(0.25 * r, -1.0 / 3.0) * max(max(cabs2(A0 - x), cabs2(A0 - y)), cabs2(A0 - z));

  float f = 1.0;
  vec2 A = A0;
  vec2 rem_sum = C_ZERO;
  vec2 sz = C_ZERO;
  vec2 z_lda = C_ZERO;
  bool first = true;
  for (int i = 0; i < N_LOOPS_MAX; ++i) {
    if (!first)
      rem_sum += cdiv(C_FROM_REAL(4.0 * f), cmul(sz, z_lda));
    else first = false;
    vec2 sx = csqrt(xn);
    vec2 sy = csqrt(yn);
    sz = csqrt(zn);
    vec2 lda = cmul(sx, sy + sz) + cmul(sy, sz);
    A = 0.25 * (A + lda);
    if (f * f * Q2 < cabs2(A))
      break;
    z_lda = zn + lda;
    xn = 0.25 * (xn + lda);
    yn = 0.25 * (yn + lda);
    zn = 0.25 * z_lda;
    f *= 0.25;
  }

  vec2 X = f * cdiv(A0 - x, A);
  vec2 Y = f * cdiv(A0 - y, A);
  vec2 Z = -(X + Y) / 3.0;

  vec2 XY = cmul(X, Y);
  vec2 ZZ = cmul(Z, Z);
  vec2 E2 = XY - 6.0 * ZZ;
  vec2 E3 = cmul(3.0 * XY - 8.0 * ZZ, Z);
  vec2 E4 = 3.0 * cmul(XY - ZZ, ZZ);
  vec2 E5 = cmul(XY, cmul(ZZ, Z));

  vec2 res = (C_FROM_REAL(24024.0) - 5148.0 * E2 + 4004.0 * E3 + 2457.0 * cmul(E2, E2) - 3276.0 * E4 - 4158.0 * cmul(E2, E3) + 2772.0 * E5) / 24024.0;
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
  vec2 A0 = (x + 2.0 * y) / 3.0;
  float Q = pow(3.0 * r, -0.125) * sqrt(cabs2(A0 - x));

  float f = 1.0;
  vec2 A = A0;
  for (int i = 0; i < N_LOOPS_MAX; ++i) {
    vec2 lda = 2.0 * cmul(csqrt(xn), csqrt(yn)) + yn;
    A = 0.25 * (A + lda);
    f *= 0.25;
    if (f * f * Q * Q < cabs2(A))
      break;
    xn = 0.25 * (xn + lda);
    yn = 0.25 * (yn + lda);
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
  return 0.5 * (t1 - t2 + t3);
}

vec2 RF_xy0(vec2 x, vec2 y) {
  x = csqrt(x);
  y = csqrt(y);

  for (int i = 0; i < N_LOOPS_MAX; ++i) {
    vec2 x_ = x;
    x = 0.5 * (x_ + y);
    y = csqrt(cmul(x_, y));
    if (cabs2(x - y) < 7.29 * r0 * cabs2(x))
      break;
  }

  return cdiv(C_FROM_REAL(PI), x + y);
}
vec2 RG_xy0(vec2 x, vec2 y) {
  vec2 x0 = csqrt(x);
  vec2 y0 = csqrt(y);
  x = x0;
  y = y0;
  vec2 x_y = x - y;
  vec2 sum = C_ZERO;
  float f = 0.25;
  bool first = true;
  for (int i = 0; i < N_LOOPS_MAX; ++i) {
    if (!first)
      sum += f * cmul(x_y, x_y);
    else first = false;
    vec2 x_ = x;
    x = 0.5 * (x_ + y);
    y = csqrt(cmul(x_, y));
    x_y = x - y;
    if (cabs2(x_y) < 7.29 * r0 * cabs2(x))
      break;
    f *= 2.0;
  }

  vec2 m = 0.5 * (x0 + y0);
  vec2 res = cmul(m, m) - sum;
  return 0.5 * PI * cdiv(res, x + y);
}
vec2 RD_0yz(vec2 y, vec2 z) {
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
  for (int i = 0; i < N_LOOPS_MAX; ++i) {
    if (!first)
      sum += f * cmul(y_z, y_z);
    else first = false;
    vec2 y_ = y;
    y = 0.5 * (y_ + z);
    z = csqrt(cmul(y_, z));
    y_z = y - z;
    if (cabs2(y_z) < 7.29 * r0 * cabs2(y))
      break;
    f = 2.0;
  }

  vec2 m = 0.5 * (y0 + z0);
  vec2 RF = cdiv(C_FROM_REAL(PI), y + z);
  vec2 RG = cmul(m, m) - sum;
  RG = 0.5 * cmul(RG, RF);
  return 3.0 * cdiv(2.0 * RG - cmul(zo, RF), cmul(zo, yo - zo));
}

#endif
