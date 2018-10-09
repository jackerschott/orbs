#ifndef OBJECTS3D_HPP
#define OBJECTS3D_HPP

typedef unsigned char byte;

struct color {
  byte r;
  byte g;
  byte b;
};
struct probColor {
  float r;
  float g;
  float b;
  float p;
};
struct particle {
  float r;
  float theta;
  float phi;
  float vr;
  float vtheta;
  float vphi;
  color pcolor;
};
struct vector {
  float x;
  float y;
  float z;

  float getLength();
  static vector crossProduct(vector v1, vector v2);
  static float dotProduct(vector v1, vector v2);

  vector operator +(vector v);
  vector operator -(vector v);
  vector operator *(float d);
  vector operator /(float d);
  vector operator -();
};
struct perspectiveCamera {
  vector pos;
  vector lookDir;
  vector upDir;
  float fov;
};

#endif
