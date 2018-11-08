#include "mwc64x.cl"
#include "rngTables.cl"

float normPdf(float x);

__kernel void genSamples(uint nSamples, ulong off, __global uint* samples) {
  uint g = get_global_id(0);
  uint perKernel = nSamples / get_global_size(0);
  
#define STREAM_GAP 1099511627776UL
  mwc64x_state_t rng;
  MWC64X_SeedStreams(&rng, off, STREAM_GAP);

  for (int i = 0; i < perKernel; i++) {
    samples[g * perKernel + i] = MWC64X_NextUint(&rng);
  }
}

__kernel void genFloatSamples(uint nSamples, ulong off, __global float* samples) {
  uint g = get_global_id(0);
  uint perKernel = nSamples / get_global_size(0);

#define STREAM_GAP 1099511627776UL
  mwc64x_state_t rng;
  MWC64X_SeedStreams(&rng, off, STREAM_GAP);

  for (int i = 0; i < perKernel; i++) {
    samples[g * perKernel + i] = MWC64X_NextUint(&rng) / ((float)UINT_MAX);
  }
}

// If nSamples are not divisible by get_global_size(0), only nSamples samples become filled
__kernel void genGaussianSamples(uint nSamples, ulong off, __global float* samples) {
  uint g = get_global_id(0);
  uint perKernel = nSamples / get_global_size(0);

#define STREAM_GAP 1099511627776UL
  mwc64x_state_t rng;
  MWC64X_SeedStreams(&rng, off, STREAM_GAP);
  for (uint i = 0; i < perKernel; i++) {
    while (true) {
      int u0 = as_int(MWC64X_NextUint(&rng));
      uint u1 = MWC64X_NextUint(&rng);
      uint r = u1 & 0x0000007F;

      if (r > 0) {
        if (abs(u0) < ixRatio[r]) {
          samples[g * perKernel + i] = fac * u0 * xr[r];
          break;
        }
        else {
          float x = fac * u0 * xr[r];
          float y = yt[r - 1] + ufac * u1 * (yt[r] - yt[r - 1]);

          if (y < normPdf(x)) {
            samples[g * perKernel + i] = x;
            break;
          }
          else continue;
        }
      }
      else {
        if (abs(u0) < iar0) {
          samples[g * perKernel + i] = fac * u0 * a / yt[r];
          break;
        }
        else {
          float x;
          float y;
          while (true) {
            uint u2 = MWC64X_NextUint(&rng);
            uint u3 = MWC64X_NextUint(&rng);
            x = -log(u2 * ufac) / xr[r];
            y = -log(u3 * ufac);
            if (2 * y > x * x) {
              samples[g * perKernel + i] = (u2 & 0x00000001) ? (x + xr[r]) : -(x + xr[r]);
              break;
            }
          }
          break;
        }
      }

    }
  }
}

float normPdf(float x) {
  return exp(-0.5 * x * x);
}
