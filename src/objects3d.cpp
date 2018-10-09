#include <cmath>

#include "objects3d.hpp"

float vector::getLength() {
  return sqrt(x * x + y * y + z * z);
}
vector vector::crossProduct(vector v1, vector v2) {
  return { v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x };
}
float vector::dotProduct(vector v1, vector v2) {
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

vector vector::operator+(vector v) {
  return { x + v.x, y + v.y, z + v.z };
}
vector vector::operator-(vector v) {
  return { x - v.x, y - v.y, z - v.z };
}
vector vector::operator*(float d) {
  return { x * d, y * d, z * d };
}
vector vector::operator/(float d) {
  return { x / d, y / d, z / d };
}
vector vector::operator-() {
  return { -x, -y, -z };
}