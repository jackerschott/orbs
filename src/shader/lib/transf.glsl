#ifndef TRANSF_GLSL
#define TRANSF_GLSL

mat4 rot(vec3 axis, float angle) {
  axis = normalize(axis);
  float s = sin(angle);
  float c = cos(angle);
  float oc = 1.0 - c;
  
  return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y + axis.z * s,  oc * axis.z * axis.x - axis.y * s,  0.0,
              oc * axis.x * axis.y - axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z + axis.x * s,  0.0,
              oc * axis.z * axis.x + axis.y * s,  oc * axis.y * axis.z - axis.x * s,  oc * axis.z * axis.z + c,           0.0,
              0.0,                                0.0,                                0.0,                                1.0);
}

mat4 red_xy(vec3 p1, vec3 p2, bool invert) {
  vec3 n;
  vec3 rotAxis;
  float rotAngle;

  // Get rotation axis and rotation angle
  n = cross(p1, p2);
  if (n == vec3(0.0)) {
    if (p1.x == 0.0 && p1.y == 0.0) {
      rotAxis = vec3(1.0, 0.0, 0.0);
      rotAngle = PI_2;
    }
    else {
      rotAxis = vec3(-p1.y, p1.x, 0.0);
      rotAngle = asin(p1.z);
    }
  }
  else {
    if (n.x == 0.0 && n.y == 0.0) {
      rotAxis = vec3(1.0, 0.0, 0.0);
      rotAngle = PI;
    }
    else {
      n = normalize(n);
      rotAxis = vec3(n.y, -n.x, 0.0);
      rotAngle = acos(n.z);
    }
  }
  if (invert)
    rotAngle += PI;

  return rot(rotAxis, rotAngle);
}

#endif
