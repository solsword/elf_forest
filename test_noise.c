
#include <stdlib.h>
#include <stdio.h>

#include "noise.h"

static const float SCALE = 1/32.0;
static const float NOISE_Z = 0.0;

static const int FALSE_COLOR[96] = {
  0x0,  0x0,  0xf,
  0x1,  0x1,  0xf,
  0x2,  0x2,  0xf,
  0x3,  0x3,  0xf,
  0x4,  0x4,  0xf,
  0x5,  0x5,  0xf,
  0x5,  0x6,  0xf,
  0x6,  0x6,  0xf,
  0x6,  0x7,  0xf,
  0x0,  0x7,  0x0,
  0x0,  0x8,  0x0,
  0x1,  0x9,  0x1,
  0x2,  0xa,  0x2,
  0x3,  0xb,  0x3,
  0x4,  0xc,  0x4,
  0x5,  0xc,  0x5,
  0x6,  0xc,  0x5,
  0x7,  0xd,  0x5,
  0x9,  0xe,  0x6,
  0xa,  0xe,  0x6,
  0xc,  0xd,  0x6,
  0xd,  0xc,  0x6,
  0xe,  0xb,  0x7,
  0xe,  0xb,  0x7,
  0xf,  0xb,  0x8,
  0xf,  0xb,  0x9,
  0xf,  0xc,  0xa,
  0xe,  0xd,  0xb,
  0xe,  0xd,  0xc,
  0xe,  0xd,  0xd,
  0xf,  0xe,  0xe,
  0xf,  0xf,  0xf,
};

float slice_3d_noise(float x, float y) {
  return sxnoise_3d(x, y, NOISE_Z);
}

float fractal_2d_noise(float x, float y) {
  return fractal_sxnoise_2d(
    x, y,
    4,
    2.0,
    0.5,
    0.0, 0.0
  );
}

float slice_fractal_3d_noise(float x, float y) {
  return fractal_sxnoise_3d(
    x, y, NOISE_Z,
    4,
    2.0,
    0.5,
    0.0, 0.0, 0.0
  );
}

float example_noise(float x, float y) {
  return fractal_sxnoise_3d_table(
    x, y, NOISE_Z,
    //3, BASE__DIST, BASE__DIST_F
    //4, BASE__NORM__NORM__NORM, NULL
    8, EX_TERRAIN, EX_TERRAIN_F
  );
}

void write_ppm(float (*noisefunc)(float, float), const char *filename) {
  FILE *fp;
  fp = fopen(filename, "w");
  fprintf(fp, "P3\n");
  fprintf(fp, "# noise test ppm\n");
  fprintf(fp, "# Auto-generated test of simplex noise function in noise.c.\n");
  fprintf(fp, "256 256\n");
  fprintf(fp, "15\n");
  int i, j, col;
  col = 0;
  for (i = 0; i < 256; ++i) {
    for (j = 0; j < 256; ++j) {
      float n = noisefunc((float)i*SCALE, (float)j*SCALE);  // [-1, 1)
      int v = (int) ((n + 1.0) * 16.5);                     // [0, 32]
      if (v < 0 || v > 32) {
        printf("n, v: %.3f, %d\n", n, v);
      }
      fprintf(
        fp,
        "%02d %02d %02d ",
        FALSE_COLOR[v*3],
        FALSE_COLOR[v*3+1],
        FALSE_COLOR[v*3+2]
      );
      col += 9;
      if (col >= 70) {
        fprintf(fp, "\n");
        col = 0;
      }
    }
  }
  fprintf(fp, "\n");
  fclose(fp);
  return;
}

int main(int argc, char** argv) {
  write_ppm(&sxnoise_2d, "noise_test_2D.ppm");
  write_ppm(&slice_3d_noise, "noise_test_3D.ppm");
  write_ppm(&fractal_2d_noise, "noise_test_2D_F.ppm");
  write_ppm(&slice_fractal_3d_noise, "noise_test_3D_F.ppm");
  write_ppm(&example_noise, "noise_test_ex.ppm");
  return 0;
}
