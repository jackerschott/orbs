#ifndef OBJECTS3D_H
#define OBJECTS3D_H

typedef struct kColor kColor;
typedef struct kProbColor kProbColor;
typedef struct kParticle kParticle;
typedef struct kVector kVector;
typedef struct kPerspectiveCamera kPerspectiveCamera;

struct kColor {
  unsigned char r;
  unsigned char g;
  unsigned char b;
};
struct kProbColor {
  double r;
  double g;
  double b;
  double p;
};
struct kParticle {
  double r;
  double theta;
  double phi;
  kColor pcolor;
};
struct kVector {
  double x;
  double y;
  double z;
};
struct kPerspectiveCamera {
  kVector pos;
  kVector lookDir;
  kVector upDir;
  double fov;
};

#endif
