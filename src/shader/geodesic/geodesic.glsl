#ifndef GEODESIC_GLSL
#define GEODESIC_GLSL

const float bc = 1.5 * SQRT3;
const float uc = 2.0 / 3.0;

float impactParam(float u1, float u2, float dphi);
vec2 b2v(float b, float u1, float phi1);
float v2b(vec2 v, float u1, float phi1);
void phi_phid(float x, float u1, float u2, float dphi, float bc2, out float y, out float dy);
void phi_phid_ext(float x, float u1, float u2, float dphi, float bc2, out float y, out float dy);
vec2 phi_plus(float u, float b, float u0, vec2 R1, vec2 R2, vec2 R3);
void phi_phid_bd_plus(float u, float b, float u0, vec2 R1, vec2 R2, vec2 R3, out vec2 res, out vec2 dres);
vec2 phi_plus_ext(float u, float b, float u0, vec2 R1, vec2 R2, vec2 R3);
void phi_phid_bd_plus_ext(float u, float b, float u0, vec2 R1, vec2 R2, vec2 R3, out vec2 res, out vec2 dres);
vec2 psi(float u, float b, vec2 R1, vec2 R2, vec2 R3);
void psi_psid(float u, float b, vec2 R1, vec2 R2, vec2 R3, out vec2 res, out vec2 dres);
void roots(float b, out vec2 R1, out vec2 R2, out vec2 R3);

float impactParam(float u1, float u2, float dphi) {
  float bc2 = 1.0 / sqrt(u2 * u2 * (1.0 - u2));
  vec2 R1, R2, R3;
  roots(bc2, R1, R2, R3);

  if (u2 < uc) {
    if (dphi < -psi(u1, bc2, R1, R2, R3).x) {
      float x = -1.0;
      float p2x = 0.5;
      float y, dy;
      for (int i = 0; i < 5; ++i) {
        phi_phid(p2x, u1, u2, dphi, bc2, y, dy);
        float newX = x - y / dy;
        if (newX >= 0.0)
          x *= 0.5;
        else x = newX;
        p2x = pow(2.0, x);
      }
      return (1.0 - p2x) * bc2;
    }
    else {
      float x = -1.0;
      float p2x = 0.5;
      float y, dy;
      for (int i = 0; i < 5; ++i) {
        phi_phid_ext(p2x, u1, u2, dphi, bc2, y, dy);
        float newX = x - y / dy;
        if (newX >= 0.0)
          x *= 0.5;
        else x = newX;
        p2x = pow(2.0, x);
      }
      return (bc2 - bc) * p2x + bc;
    }
  }
  else {

  }
}
vec2 b2v(float b, float u1, float phi1) {
  float s_phi1 = sin(phi1);
  float c_phi1 = cos(phi1);
  float ud1 = sqrt(1.0 / (b * b) + u1 * u1 * (u1 - 1.0));
  vec2 v = vec2(-u1 * s_phi1 - ud1 * c_phi1, u1 * c_phi1 - ud1 * s_phi1);
  return normalize(v);
}
float v2b(vec2 v, float u1, float phi1) {
  float s_phi1 = sin(phi1);
  float c_phi1 = cos(phi1);
  float ud1 = u1 * (s_phi1 * v.y + c_phi1 * v.x) / (c_phi1 * v.y - s_phi1 * v.x);
  return 1.0 / sqrt(ud1 * ud1 + u1 * u1 * (1.0 - u1));
}

void phi_phid(float x, float u1, float u2, float dphi, float bc2, out float y, out float dy) {
  float b = (1.0 - x) * bc2;
  vec2 R1, R2, R3;
  roots(b, R1, R2, R3);

  vec2 res, dres;
  phi_phid_bd_plus(u2, b, u1, R1, R2, R3, res, dres);
  y = res.x - dphi;
  dy = -dres.x * LN2 * x * bc2;
}
void phi_phid_ext(float x, float u1, float u2, float dphi, float bc2, out float y, out float dy) {
  float b = (bc2 - bc) * x + bc;
  vec2 R1, R2, R3;
  roots(b, R1, R2, R3);

  vec2 res, dres;
  phi_phid_bd_plus_ext(u2, b, u1, R1, R2, R3, res, dres);
  y = res.x - dphi;
  dy = dres.x * LN2 * x * (bc2 - bc);
}

vec2 phi_plus(float u, float b, float u0, float phi0, vec2 R1, vec2 R2, vec2 R3) {
  return psi(u, b, R1, R2, R3) - psi(u0, b, R1, R2, R3);
}
void phi_phid_bd_plus(float u, float b, float u0, vec2 R1, vec2 R2, vec2 R3, out vec2 res, out vec2 dres) {
  if (b == 0.0) {
    res = C_ZERO;
    dres = C_ZERO;
    return;
  }
  vec2 psi1, dpsi1, psi2, dpsi2;
  psi_psid(u0, b, R1, R2, R3, psi1, dpsi1);
  psi_psid(u, b, R1, R2, R3, psi2, dpsi2);
  
  res = psi2 - psi1;
  dres = dpsi2 - dpsi1;
}
vec2 phi_plus_ext(float u, float b, float u0, vec2 R1, vec2 R2, vec2 R3) {
  return -psi(u0, b, R1, R2, R3) - psi(u, b, R1, R2, R3);
}
void phi_phid_bd_plus_ext(float u, float b, float u0,
  vec2 R1, vec2 R2, vec2 R3, out vec2 res, out vec2 dres) {
  vec2 psi1, dpsi1, psi2, dpsi2;
  psi_psid(u0, b, R1, R2, R3, psi1, dpsi1);
  psi_psid(u, b, R1, R2, R3, psi2, dpsi2);
  
  res = -psi1 - psi2;
  dres = -dpsi1 - dpsi2;
}

vec2 psi(float u, float b, vec2 R1, vec2 R2, vec2 R3) {
  if (b == 0.0) {
    return C_ZERO;
  }
  
  vec2 u_ = C_FROM_REAL(u);
  vec2 R3_R1 = R3 - R1;
  vec2 R3_R2 = R3 - R2;
  vec2 R3_uc = R3 - u_;

  vec2 phi = casin(csqrt(cdiv(R3_uc, R3_R2)));
  vec2 m = cdiv(R3_R2, R3_R1);
  vec2 F = elliptic_f(phi, m);

  vec2 res = -2.0 * cmul_i(cdiv(F, csqrt(R3_R1)));
  if (b > bc && u > R2.x && u < R3.x) {
    res = -res;
  }
  return res;
}
void psi_psid(float u, float b, vec2 R1, vec2 R2, vec2 R3, out vec2 res, out vec2 dres) {
  if (b == 0.0) {
    res = C_ZERO;
    dres = vec2(-1.0 / 0.0, -1.0 / 0.0);
    return;
  }

  vec2 u_ = C_FROM_REAL(u);
  float b2 = b * b;

  vec2 R3_R1 = R3 - R1;
  vec2 R3_R2 = R3 - R2;
  vec2 R3_u = R3 - u_;

  vec2 phi = casin(csqrt(cdiv(R3_u, R3_R2)));
  vec2 m = cdiv(R3_R2, R3_R1);
  vec2 F, E;
  elliptic_fe(phi, m, F, E);

  vec2 S1 = cmul_i(cdiv(F, csqrt(R3_R1)));
  vec2 S2 = cmul_i(cdiv(E, csqrt(R3_R1)));

  vec2 T1 = 2.0 * cdiv(C_FROM_REAL(3.0 - 9.0 * u + 2.0 * b2 * (u - u * u)), csqrt(C_FROM_REAL(1.0 + b2 * (u - 1.0) * u * u)));
  vec2 T2 = 18.0 * S1 / b;
  vec2 T3 = -4.0 * b * (cmul(R1, S1) + cmul(R3_R1, S2));

  res = -2.0 * S1;
  if (b > bc && u > R2.x && u < R3.x) {
    res = -res;
    T2 = -T2;
    T3 = -T3;
  }
  dres = (T1 + T2 + T3) / (4.0 * b2 - 27.0);
}
void roots(float b, out vec2 R1, out vec2 R2, out vec2 R3) {
  if (b == 0.0) {
    R1 = C_NAN;
    R2 = C_NAN;
    R3 = C_NAN;
    return;
  }
  vec2 B = sqrt(12.0 * (bc + b)) * csqrt(C_FROM_REAL(bc - b)) + C_FROM_REAL(9.0);
  B = C_ONE - 18.0 * cdiv(C_ONE, B);
  B = croot(B, 3u);
  R1 = (C_ONE - cdiv(vec2(0.5, 0.5 * SQRT3), B) - cmul(vec2(0.5, -0.5 * SQRT3), B)) / 3.0;
  R2 = (C_ONE + cdiv(C_ONE, B) + B) / 3.0;
  R3 = (C_ONE - cdiv(vec2(0.5, -0.5 * SQRT3), B) - cmul(vec2(0.5, 0.5 * SQRT3), B)) / 3.0;

  if (b > bc) {
    swap(R2, R3);
    R1.y = 0.0;
    R2.y = 0.0;
    R3.y = 0.0;
  }
  else {
    R1.y = 0.0;
  }
}

#endif
