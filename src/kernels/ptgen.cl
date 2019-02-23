#include "rng.cl"

__kernel void getEllipticPtDistr(uint nPt, float b, float eps, float16 rot,
  float dr, float dz, __global float* uSamples, __global float* gSamples1, __global float* gSamples2,
  __global float4* pos) {
  uint g = get_global_id(0);
  
  float u = uSamples[g];
  float g1 = gSamples1[g];
  float g2 = gSamples2[g];

  if (g >= 1000 && g <= 1010) {
    printf("g1 = %f\n", g1);
    printf("g2 = %f\n", g2);
  }

  float phi0 = (2.0f * M_PI_F) * u;
  float c0 = cos(phi0);
  float s0 = sin(phi0);
  float r0 = b * rsqrt(1.0f - eps * eps * c0 * c0) + g1 * dr;

  float x0 = r0 * c0;
  float y0 = r0 * s0;
  float z0 = g2 * dz;

  pos[g].x = rot.s0 * x0 + rot.s1 * y0 + rot.s2 * z0 + rot.s3;
  pos[g].y = rot.s4 * x0 + rot.s5 * y0 + rot.s6 * z0 + rot.s7;
  pos[g].z = rot.s8 * x0 + rot.s9 * y0 + rot.sA * z0 + rot.sB;
  pos[g].w = 1.0f;
}

__kernel void getPtColors(uint nColors, __global float4* palette, __global float* blurSizes,
  __global float* uSamples, __global float4* colors) {
  uint g = get_global_id(0);

  float sum = blurSizes[0];
  float4 color = palette[nColors - 1];
  for (int i = 0; i < nColors; i++) {
    if (sum > uSamples[g]) {
      color = palette[i];
      break;
    }
    sum += blurSizes[i + 1];
  }

  colors[g].x = color.x;
  colors[g].y = color.y;
  colors[g].z = color.z;
  colors[g].w = color.w;
}
