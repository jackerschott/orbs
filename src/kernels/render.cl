#define PI 3.14159265358979323846

__kernel void getPtPositions(__global float4* pos, __global float* r, __global float* theta, __global float* phi) {
  uint i = get_global_id(0);

  pos[i].x = r[i] * sin(theta[i]) * cos(phi[i]);
  pos[i].y = r[i] * sin(theta[i]) * sin(phi[i]);
  pos[i].z = r[i] * cos(theta[i]);
  pos[i].w = 1.0f;
}