#include "render.h"

float vLen(kVector v) {
  return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}
kVector crossP(kVector v1, kVector v2) {
  kVector res = { v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x};
  return res;
}
float dotP(kVector v1, kVector v2) {
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
kVector vAdd(kVector v1, kVector v2) {
  kVector res = {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
  return res;
}
kVector vSub(kVector v1, kVector v2) {
  kVector res = {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
  return res;
}
kVector vMul(kVector v, float d) {
  kVector res = {v.x * d, v.y * d, v.z * d};
  return res;
}
kVector vDiv(kVector v, float d) {
  kVector res = {v.x / d, v.y / d, v.z / d};
  return res;
}
kVector vNeg(kVector v) {
  kVector res = {-v.x, -v.y, -v.z};
  return res;
}

__kernel void render(uint pWidth, uint pHeight, uint bpp,
  uint sPixels, __global uchar *pixels, float rs,
  __global float *pR, __global float *pTheta, __global float *pPhi,
  __global uchar *pCR, __global uchar *pCG, __global uchar *pCB,
  float cPosX, float cPosY, float cPosZ,
  float cLookDirX, float cLookDirY, float cLookDirZ,
  float cUpDirX, float cUpDirY, float cUpDirZ, float cFov) {

  uint i = get_global_id(0);

  if (pR[i] <= rs)
    return;

  kParticle p = { pR[i], pTheta[i], pPhi[i], { pCR[i], pCG[i], pCB[i] } };
  kPerspectiveCamera c = { { cPosX, cPosY, cPosZ }, { cLookDirX, cLookDirY, cLookDirZ }, { cUpDirX, cUpDirY, cUpDirZ }, cFov };

  kVector pPos;
  pPos.x = p.r * sin(p.theta) * cos(p.phi);
  pPos.y = p.r * sin(p.theta) * sin(p.phi);
  pPos.z = p.r * cos(p.theta);

  float bhSiding = dotP(pPos, vSub(c.pos, pPos));
  float bhAlignm = bhSiding * bhSiding - dotP(vSub(c.pos, pPos), vSub(c.pos, pPos)) * (dotP(pPos, pPos) - rs * rs);
  if (bhSiding <= 0.0 && bhAlignm >= 0.0)
    return;

  kVector screenPoint;
  screenPoint = vDiv(vAdd(c.pos, vMul(vSub(pPos, c.pos), dotP(c.lookDir, c.lookDir))),
    dotP(c.lookDir, vSub(pPos, c.pos)));
  kVector xDir = crossP(c.lookDir, c.upDir);
  kVector screenX = vDiv(xDir, pWidth);
  kVector screenY = vDiv(crossP(c.lookDir, xDir), pWidth);
  float camX = dotP(screenPoint, screenX) / dotP(screenX, screenX);
  float camY = dotP(screenPoint, screenY) / dotP(screenY, screenY);

  int pxX = (int)(camX + pWidth / 2.0);
  int pxY = (int)(camY + pHeight / 2.0);
  int k = (pxX + pWidth * pxY) * bpp / 8;
  if (k >= 0 && k < sPixels) {
    pixels[k] = p.pcolor.r;
    pixels[k + 1] = p.pcolor.g;
    pixels[k + 2] = p.pcolor.b;
  }
}
