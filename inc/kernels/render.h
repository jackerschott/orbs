#ifndef RENDER_H
#define RENDER_H

#include "objects3d.h"

typedef union PxWriteHelper PxWriteHelper;

union PxWriteHelper {
  struct {
    unsigned char bytes[8];
  };
  unsigned int intHelpers[2];
};

#endif