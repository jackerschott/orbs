#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable

#define PI 3.14159265358979323846

#include "render.h"

float vLen(kVector v);
kVector crossP(kVector v1, kVector v2);
float dotP(kVector v1, kVector v2);
kVector vAdd(kVector v1, kVector v2);
kVector vSub(kVector v1, kVector v2);
kVector vMul(kVector v, float d);
kVector vDiv(kVector v, float d);
kVector vNeg(kVector v);

__kernel void renderBg(uint pWidth, uint pHeight, uint bpp, uint sPixels, __global uchar *pixels,
  uint bgWidth, uint bgHeight, uint bgBpp, uint bgSPixels, __global uchar *bgPixels, float rs, float gRadius,
  float cPosX, float cPosY, float cPosZ,
  float cLookDirX, float cLookDirY, float cLookDirZ,
  float cUpDirX, float cUpDirY, float cUpDirZ, float cFov) {
  uint x = get_global_id(0);
  uint y = get_global_id(1);

  kPerspectiveCamera c = { { cPosX, cPosY, cPosZ }, { cLookDirX, cLookDirY, cLookDirZ }, { cUpDirX, cUpDirY, cUpDirZ }, cFov };

  float theta = PI * (float)y / bgHeight;
  float phi = 2.0f * PI * (float)x / bgWidth;

  kVector gPos;
  gPos.x = gRadius * sin(theta) * cos(phi);
  gPos.y = gRadius * sin(theta) * sin(phi);
  gPos.z = gRadius * cos(theta);

  float bhSiding = dotP(gPos, vSub(c.pos, gPos));
  float bhAlignm = bhSiding * bhSiding - dotP(vSub(c.pos, gPos), vSub(c.pos, gPos)) * (dotP(gPos, gPos) - rs * rs);
  if (bhSiding <= 0.0f && bhAlignm >= 0.0f) {
    return;
  }

  kVector screenPoint;
  screenPoint = vDiv(vAdd(c.pos, vMul(vSub(gPos, c.pos), dotP(c.lookDir, c.lookDir))),
    dotP(c.lookDir, vSub(gPos, c.pos)));
  kVector xDir = crossP(c.lookDir, c.upDir);
  kVector scrX = vDiv(xDir, pWidth);
  kVector scrY = vDiv(crossP(c.lookDir, xDir), pWidth);
  float camX = dotP(screenPoint, scrX) / dotP(scrX, scrX);
  float camY = dotP(screenPoint, scrY) / dotP(scrY, scrY);

  int pxX = (int)(camX + pWidth / 2.0f);
  int pxY = (int)(camY + pHeight / 2.0f);

  int p = (pxX + pWidth * pxY) * bpp / 8;
  int b = (x + bgWidth * y) * bgBpp / 8;
  if (p >= 0 && p < sPixels) {
    pixels[p] = 255; //bgPixels[b];
    pixels[p + 1] = bgPixels[b + 1];
    pixels[p + 2] = bgPixels[b + 2];
    pixels[p + 3] = bgPixels[b + 3];
  }
}

__kernel void computePtProj(uint pWidth, uint pHeight, uint bpp,
  __global int *ptProjX, __global int *ptProjY, uint tileSize, uint ptSize,
	float rs, __global float *ptR, __global float *ptTheta, __global float *ptPhi,
  float cPosX, float cPosY, float cPosZ, float cLookDirX, float cLookDirY, float cLookDirZ,
  float cUpDirX, float cUpDirY, float cUpDirZ, float cFov) {
	uint pt = get_global_id(0);
	int nPt = get_global_size(1);

  if (ptR[pt] <= rs) {
		ptProjX[pt] = -1;
		ptProjY[pt] = -1;
      return;
  }

  kPerspectiveCamera c = { { cPosX, cPosY, cPosZ }, { cLookDirX, cLookDirY, cLookDirZ }, { cUpDirX, cUpDirY, cUpDirZ }, cFov };

  kVector ptPos;
  ptPos.x = ptR[pt] * sin(ptTheta[pt]) * cos(ptPhi[pt]);
  ptPos.y = ptR[pt] * sin(ptTheta[pt]) * sin(ptPhi[pt]);
  ptPos.z = ptR[pt] * cos(ptTheta[pt]);

  float bhSiding = dotP(ptPos, vSub(c.pos, ptPos));
  float bhAlignm = bhSiding * bhSiding - dotP(vSub(c.pos, ptPos), vSub(c.pos, ptPos)) * (dotP(ptPos, ptPos) - rs * rs);
  if (bhSiding <= 0.0f && bhAlignm >= 0.0f) {
		ptProjX[pt] = -1;
		ptProjY[pt] = -1;
    return;
  }

  kVector screenPoint;
  screenPoint = vDiv(vAdd(c.pos, vMul(vSub(ptPos, c.pos), dotP(c.lookDir, c.lookDir))),
    dotP(c.lookDir, vSub(ptPos, c.pos)));
  kVector xDir = crossP(c.lookDir, c.upDir);
  kVector scrX = vDiv(xDir, pWidth);
  kVector scrY = vDiv(crossP(c.lookDir, xDir), pWidth);
  float camX = dotP(screenPoint, scrX) / dotP(scrX, scrX);
  float camY = dotP(screenPoint, scrY) / dotP(scrY, scrY);

  int pxX = (int)(camX + pWidth / 2.0f);
  int pxY = (int)(camY + pHeight / 2.0f);


	uint nTilesX = pWidth / tileSize + 1;
	uint nTilesY = pHeight / tileSize + 1;
	uint nTiles = nTilesX * nTilesY;
	uint txtl = (pxX - ptSize) / tileSize;
	uint tytl = (pxY - ptSize) / tileSize;
	uint txtr = (pxX + ptSize) / tileSize;
	uint tytr = tytl;
	uint txbl = txtl;
	uint tybl = (pxY + ptSize) / tileSize;
	uint txbr = txtr;
	uint tybr = tybl;

#define IS_IN_IMAGE(tx, ty) ((tx) >= 0 && (tx) < nTilesX && (ty) >= 0 && (ty) < nTilesY)
	bool onVerInters = txtr > txtl;
	bool onHorInters = tybl > tytl;
	if (IS_IN_IMAGE(txtl, tytl)) {
		ptProjX[(txtl + nTilesX * tytl) * nPt + pt] = pxX;
		ptProjY[(txtl + nTilesX * tytl) * nPt + pt] = pxY;
	}
	if (onVerInters && IS_IN_IMAGE(txtr, tytr)) {
		ptProjX[(txtr + nTilesX * tytr) * nPt + pt] = pxX;
		ptProjY[(txtr + nTilesX * tytr) * nPt + pt] = pxY;
	}
	if (onHorInters && IS_IN_IMAGE(txbl, tybl)) {
		ptProjX[(txbl + nTilesX * tybl) * nPt + pt] = pxX;
		ptProjY[(txbl + nTilesX * tybl) * nPt + pt] = pxY;
	}
	if (onVerInters && onHorInters && IS_IN_IMAGE(txbr, tybr)) {
		ptProjX[(txbr + nTilesX * tybr) * nPt + pt] = pxX;
		ptProjY[(txbr + nTilesX * tybr) * nPt + pt] = pxY;
	}
}

__kernel void drawPtShapes(uint pWidth, uint pHeight, uint bpp, ulong sPixels,
  __global uchar *pixelData, uint ptNPixels, __global int *ptProjX, __global int *ptProjY,
  __global int *ptProjRelX, __global int *ptProjRelY, __global float *ptProjRelCFac,
  __global uchar *ptCR, __global uchar *ptCG, __global uchar *ptCB) {
  uint g = get_global_id(0);
	uint pt = g / ptNPixels;
	uint ptr = g % ptNPixels;

	long p = ((ptProjX[pt] + ptProjRelX[ptr]) + pWidth * (ptProjY[pt] + ptProjRelY[ptr])) * bpp / 8;
  if (p >= 0 && p < sPixels) {
#define BYTE_MAX_CUT(i) (i) > 255 ? (uchar)255 : (uchar)(i)
		pixelData[p] = BYTE_MAX_CUT(ptProjRelCFac[ptr] * ptCR[pt] + pixelData[p]);
		pixelData[p + 1] = BYTE_MAX_CUT(ptProjRelCFac[ptr] * ptCG[pt] + pixelData[p + 1]);
		pixelData[p + 2] = BYTE_MAX_CUT(ptProjRelCFac[ptr] * ptCB[pt] + pixelData[p + 2]);
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