#define PI 3.14159265358979323846

#include "rng.cl"

__kernel void getEllipticPtDistr(uint nPt, float b, float eps, float3 n,
  float dr, float dz, __global float* uSamples, __global float* gSamples1, __global float* gSamples2,
  __global float4* pos) {
  uint g = get_global_id(0);

  float u = uSamples[g];
  float g1 = gSamples1[g];
  float g2 = gSamples2[g];

  float phi0 = 2.0 * PI * u;
  float r0 = b / sqrt(1.0 - eps * eps * cos(phi0) * cos(phi0)) + g1 * dr;

  float x0 = r0 * cos(phi0);
  float y0 = r0 * sin(phi0);
  float z0 = g2 * dz;

  float3 ez = { 0.0, 0.0, 1.0 };
  float alpha = n.z / length(n);
  pos[g].x = sin(alpha) * n.x * z0 - (1 - cos(alpha)) * n.x * n.y * y0 + (1 - (1 - cos(alpha)) * n.x * n.x) * x0;
  pos[g].y = sin(alpha) * n.y * z0 + (1 - (1 - cos(alpha)) * n.y * n.y) * y0 - (1 - cos(alpha)) * n.x * n.y * x0;
  pos[g].z = ((1 - cos(alpha)) * (-n.y * n.y - n.x * n.x) + 1) * z0 - sin(alpha) * n.y * y0 - sin(alpha) * n.x * x0;
  pos[g].w = 1.0;
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