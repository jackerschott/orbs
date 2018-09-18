#include "render.h"

#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable

float vLen(kVector v);
kVector crossP(kVector v1, kVector v2);
float dotP(kVector v1, kVector v2);
kVector vAdd(kVector v1, kVector v2);
kVector vSub(kVector v1, kVector v2);
kVector vMul(kVector v, float d);
kVector vDiv(kVector v, float d);
kVector vNeg(kVector v);

__kernel void renderScreenPos(uint pWidth, uint pHeight, uint bpp,
  __global int *screenX, __global int *screenY, float rs,
  __global float *pR, __global float *pTheta, __global float *pPhi,
  float cPosX, float cPosY, float cPosZ,
  float cLookDirX, float cLookDirY, float cLookDirZ,
  float cUpDirX, float cUpDirY, float cUpDirZ, float cFov) {
  uint g = get_global_id(0);

  if (pR[g] <= rs) {
    screenX[g] = -1;
    screenY[g] = -1;
      return;
  }

  kPerspectiveCamera c = { { cPosX, cPosY, cPosZ }, { cLookDirX, cLookDirY, cLookDirZ }, { cUpDirX, cUpDirY, cUpDirZ }, cFov };

  kVector pPos;
  pPos.x = pR[g] * sin(pTheta[g]) * cos(pPhi[g]);
  pPos.y = pR[g] * sin(pTheta[g]) * sin(pPhi[g]);
  pPos.z = pR[g] * cos(pTheta[g]);

  float bhSiding = dotP(pPos, vSub(c.pos, pPos));
  float bhAlignm = bhSiding * bhSiding - dotP(vSub(c.pos, pPos), vSub(c.pos, pPos)) * (dotP(pPos, pPos) - rs * rs);
  if (bhSiding <= 0.0f && bhAlignm >= 0.0f) {
    screenX[g] = -1;
    screenY[g] = -1;
    return;
  }

  kVector screenPoint;
  screenPoint = vDiv(vAdd(c.pos, vMul(vSub(pPos, c.pos), dotP(c.lookDir, c.lookDir))),
    dotP(c.lookDir, vSub(pPos, c.pos)));
  kVector xDir = crossP(c.lookDir, c.upDir);
  kVector scrX = vDiv(xDir, pWidth);
  kVector scrY = vDiv(crossP(c.lookDir, xDir), pWidth);
  float camX = dotP(screenPoint, scrX) / dotP(scrX, scrX);
  float camY = dotP(screenPoint, scrY) / dotP(scrY, scrY);

  int pxX = (int)(camX + pWidth / 2.0f);
  int pxY = (int)(camY + pHeight / 2.0f);

  screenX[g] = pxX;
  screenY[g] = pxY;
}

__kernel void renderShapes(uint pWidth, uint pHeight, uint bpp, uint sPixels,
  __global volatile uchar *pixels, uint partPxSize, __global int *screenX, __global int *screenY,
  __global int *screenRelX, __global int *screenRelY, __global float *screenRelFac,
  __global uchar *pCR, __global uchar *pCG, __global uchar *pCB) {
  uint g = get_global_id(0);
  uint pc = g / partPxSize;
  uint px = g % partPxSize;

  int x = screenX[pc] + screenRelX[px];
  int y = screenY[pc] + screenRelY[px];

  int i = (x + pWidth * y) * bpp / 8;
  if (i >= 0 && i < sPixels){
#define BYTE_MAX_CUT(b) b <= 255.0f ? (uchar)b : (uchar)255
    pixels[i] = BYTE_MAX_CUT(pCR[pc] * screenRelFac[px] + pixels[i]);
    pixels[i + 1] = BYTE_MAX_CUT(pCR[pc] * screenRelFac[px] + pixels[i + 1]);
    pixels[i + 2] = BYTE_MAX_CUT(pCR[pc] * screenRelFac[px] + pixels[i + 2]);
  }
}

float vLen(kVector v) {
  return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}
kVector crossP(kVector v1, kVector v2) {
  kVector res = { v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x };
  return res;
}
float dotP(kVector v1, kVector v2) {
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
kVector vAdd(kVector v1, kVector v2) {
  kVector res = { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
  return res;
}
kVector vSub(kVector v1, kVector v2) {
  kVector res = { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
  return res;
}
kVector vMul(kVector v, float d) {
  kVector res = { v.x * d, v.y * d, v.z * d };
  return res;
}
kVector vDiv(kVector v, float d) {
  kVector res = { v.x / d, v.y / d, v.z / d };
  return res;
}
kVector vNeg(kVector v) {
  kVector res = { -v.x, -v.y, -v.z };
  return res;
}