#ifndef OBJECTS3D_HPP
#define OBJECTS3D_HPP

typedef unsigned char byte;

struct color {
  byte r;
  byte g;
  byte b;
};
struct probColor {
  double r;
  double g;
  double b;
  double p;
};
struct particle {
  double r;
  double theta;
  double phi;
  double vr;
  double vtheta;
  double vphi;
  color pcolor;
};
struct vector {
  double x;
  double y;
  double z;

  double getLength();
  static vector crossProduct(vector v1, vector v2);
  static double dotProduct(vector v1, vector v2);

  vector operator +(vector v);
  vector operator -(vector v);
  vector operator *(double d);
  vector operator /(double d);
  vector operator -();
};
struct perspectiveCamera {
  vector pos;
  vector lookDir;
  vector upDir;
  double fov;
};

#endif
