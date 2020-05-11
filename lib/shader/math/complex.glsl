#define C_ONE   vec2(1.0, 0.0)
#define C_MONE  vec2(-1.0, 0.0)
#define C_I     vec2(0.0, 1.0)
#define C_MI    vec2(0.0, -1.0)
#define C_ZERO  vec2(0.0, 0.0)
#define C_RINF  vec2(1.0 / 0.0, 0.0)
#define C_MRINF vec2(-1.0 / 0.0, 0.0)
#define C_IINF  vec2(0.0, 1.0 / 0.0)
#define C_MIINF vec2(0.0, -1.0 / 0.0)
#define C_NAN   vec2(0.0 / 0.0, 0.0 / 0.0)

#define C_FROM_REAL(R) vec2(R, 0.0)
#define C_FROM_IMAG(I) vec2(0.0, I)

void swap(inout vec2 z1, inout vec2 z2);
vec2 cmul_i(vec2 z);
vec2 cmul(vec2 z, vec2 w);
vec2 cdiv(vec2 z, vec2 w);
float cabs2(vec2 z);
vec2 csqrt(vec2 z);
vec2 croot(vec2 z, uint n);

vec2 cexp(vec2 z);
vec2 clog(vec2 z);
vec2 csincos(vec2 z);
vec2 casin(vec2 z);
vec2 cacos(vec2 z);

void swap(inout vec2 z1, inout vec2 z2) {
  swap(z1.x, z2.x);
  swap(z1.y, z2.y);
}
vec2 cmul_i(vec2 z) {
  return vec2(-z.y, z.x);
}
vec2 cmul(vec2 z, vec2 w) {
  return vec2(z.x * w.x - z.y * w.y, z.x * w.y + z.y * w.x);
}
vec2 cdiv(vec2 z, vec2 w) {
  return vec2(z.x * w.x + z.y * w.y, z.y * w.x - z.x * w.y) / (w.x * w.x + w.y * w.y);
}
float cabs2(vec2 z) {
  return z.x * z.x + z.y * z.y;
}
vec2 csqrt(vec2 z) {
  #define a z.x
  #define b z.y

  if (b == 0.0) {
    if (a < 0.0) {
      return C_FROM_IMAG(sqrt(-a));
    }
    else {
      return C_FROM_REAL(sqrt(a));
    }
  }
  else if (a == 0.0) {
    if (b < 0.0) {
      float c = sqrt(-0.5 * b);
      return vec2(c, -c);
    }
    else {
      float c = sqrt(0.5 * b);
      return vec2(c, c);
    }
  }
  else {
    float r = sqrt(a * a + b * b);
    if (a >= 0.0) {
      float u = sqrt(2.0 * (r + a));
      return vec2(0.5 * u, b / u);
    }
    else {
      float u = sqrt(2.0 * (r - a));
      if (b < 0.0) {
        return vec2(-b / u, -0.5 * u);
      }
      else {
        return vec2(b / u, 0.5 * u);
      }
    }
  }

  #undef a
  #undef b
}
vec2 croot(vec2 z, uint n) {
  if (n == 1U) {
    return z;
  }
  else if (n == 2U) {
    return csqrt(z);
  }
  else if (n == 4U) {
    z = csqrt(z);
    return csqrt(z);
  }
  else if (z.y == 0.0 && z.x >= 0.0) {
    return vec2(pow(z.x, 1.0 / float(n)), 0.0);
  }
  else {
    return cexp(clog(z) / float(n));
  }
}

vec2 cexp(vec2 z) {
  #define a z.x
  #define b z.y

  if (b == 0.0) {
    return C_FROM_REAL(exp(a));
  }
  else if (a == 0.0) {
    return vec2(cos(b), sin(b));
  }
  else {
    return exp(a) * vec2(cos(b), sin(b));
  }

  #undef a
  #undef b
}
vec2 clog(vec2 z) {
  #define a z.x
  #define b z.y

  if (b == 0.0) {
    if (a > 0.0) {
      return vec2(log(a), PI);
    }
    else if (a < 0.0) {
      return vec2(log(-a), PI);
    }
    else return C_NAN;
  }
  else if (a == 0.0) {
    if (b > 0.0) {
      return vec2(log(b), PI_2);
    }
    else {
      return vec2(log(-b), -PI_2);
    }
  }
  else {
    return vec2(0.5 * log(a * a + b * b), atan(b, a));
  }

  #undef a
  #undef b
}
void csincos(vec2 z, out vec2 s, out vec2 c) {
  #define a z.x
  #define b z.y

  if (b == 0.0) {
    s = C_FROM_REAL(sin(a));
    c = C_FROM_REAL(cos(a));
  }
  else if (a == 0.0) {
    s = C_FROM_IMAG(sinh(b));
    c = C_FROM_REAL(cosh(b));
  }
  else {
    float s_ = sin(z.x);
    float c_ = cos(z.x);
    float sh_ = sinh(z.y);
    float ch_ = cosh(z.y);
    s = vec2(s_ * ch_, c_ * sh_);
    c = vec2(c_ * ch_, -s_ * sh_);
  }

  #undef a
  #undef b
}
vec2 casin(vec2 z) {
  #define a z.x
  #define b z.y

  if (a == 0.0) {
    return C_FROM_IMAG(asinh(b));
  }
  else {
    vec2 t = csqrt(C_ONE - cmul(z, z));
    if (b == 0.0 && t.y == 0.0) {
      return C_FROM_REAL(atan(a, t.x));
    }
    else {
      vec2 res = clog(cmul_i(z) + t);
      return -cmul_i(res);
    }
  }

  #undef a
  #undef b
}
vec2 cacos(vec2 z) {
  #define a z.x
  #define b z.y

  if (z == C_ONE) {
    return C_ZERO;
  }
  else {
    vec2 t = C_ONE;
    if (b == 0.0 && a > t.x) {
      return C_FROM_IMAG(-casin(z).y);
    }
    else {
      return C_FROM_REAL(PI_2) - casin(z);
    }
  }

  #undef a
  #undef b
}
