
#include <stdlib.h>
#include <stdio.h>

#include "noise.h"

static float const SCALE = 1/32.0;
static float const NOISE_Z = 0.0;

static int const FALSE_COLOR[96] = {
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
  0xd,  0xd,  0x6,
  0xe,  0xe,  0x7,
  0xe,  0xe,  0x7,
  0xf,  0xf,  0x8,
  0xf,  0xf,  0x9,
  0xe,  0xe,  0xa,
  0xd,  0xd,  0xb,
  0xd,  0xd,  0xc,
  0xe,  0xe,  0xd,
  0xf,  0xf,  0xe,
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

float example_wrapped_noise(float x, float y) {
  return tiled_func(
    &sxnoise_2d,
    x, y,
    128*SCALE, 128*SCALE,
    22
  );
}

float terrain_noise(float x, float y) {
  x += 1800;
  x *= 18;
  y *= 18;
  float ds = 4*sxnoise_2d(x*0.08, y*0.08);
  float dslx = 50*stretch(sxnoise_2d(x*0.02+500, y*0.02), 1.3);
  float dsly = 50*stretch(sxnoise_2d(x*0.02+1000, y*0.02), 1.3);
  float dsmx = 60 * sxnoise_2d(x*0.035+700, y*0.01);
  float dsmy = 80 * sxnoise_2d(x*0.035+700, y*0.01);
  float dx = x + ds;
  float dy = y + ds;

  // Worley-based details:
  float result = wrnoise_2d_fancy(
    x*0.19, y*0.19,
    0, 0,
    WORLEY_FLAG_IGNORE_NEAREST | WORLEY_FLAG_INCLUDE_NEXTBEST
  );
  result += wrnoise_2d_fancy(
    x*0.13+1000, y*0.13,
    0, 0,
    WORLEY_FLAG_INCLUDE_NEXTBEST
  );
  result /= 2.0;
  result = 2*result - 1;

  //return result;
  //result = 0;

  // Simplex-based details.
  result += (
    sxnoise_2d(dx*0.12+10, dy*0.12) +
    0.8 * sxnoise_2d(x*0.1+7, y*0.1) +
    0.6 * sxnoise_2d(dx*0.2+9, dy*0.2) +
    0.4 * sxnoise_2d(x*0.41+9, y*0.41)
  ) / 2.8;
  result /= 2.0;
  result = stretch(result, 0.7);

  //return result;
  //result = 0;

  // Heavily distorted periodic continents.
  result += 1.5 * cos((dx+dslx)*0.1) * sin((dy+dsly)*0.1);
  result += 2.5 * cos((dx-dslx)*0.045) * sin((dy-dsly)*0.045);
  result /= 5.0;

  //return result;
  result = 0;

  // Simplex-based "mountain ranges."
  result += sxnoise_2d((x+dsmx)*0.032, (y+dsmy)*0.032);
  result /= 1.3;

  return result;
  //result = 0;

  // Stretching the result to establish sea level around 0.28:
  result = stretch(result, 0.4);
  result = stretch((1 + result)/2.0, 1.6) * 2 - 1;

  // Terrace the oceans, continental shelves, and land:
  result = (1 + result)/2.0;
  if (result < 0.15) {
    result /= 0.15;
    result = pow(result, 4) * 0.23;
  } else if (result < 0.28) {
    result = (result - 0.15) / (0.28 - 0.15);
    result = 0.23 + pow(result, 1.7) * (0.28 - 0.15);
  } else {
    result = (result - 0.28) / (1 - 0.28);
    result = 0.28 + pow(result, 4) * (1 - 0.28);
  }
  result = result * 2 - 1.000001;
  return result;
}

float wrapped_terrain(float x, float y) {
  return tiled_func(&terrain_noise, x, y, 256*SCALE, 0, 5);
}

void write_noise_ppm(
  float (*noisefunc)(float, float),
  char const * const filename
) {
  FILE *fp;
  fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Error: couldn't open destination file '%s'.\n", filename);
    exit(1);
  }
  fprintf(fp, "P3\n");
  fprintf(fp, "# noise test ppm\n");
  fprintf(fp, "# Auto-generated test of noise function in noise.c.\n");
  fprintf(fp, "256 256\n");
  fprintf(fp, "15\n");
  int i, j, col;
  col = 0;
  for (j = 0; j < 256; ++j) {
    for (i = 0; i < 256; ++i) {
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
  write_noise_ppm(&sxnoise_2d, "noise_test_2D.ppm");
  write_noise_ppm(&slice_3d_noise, "noise_test_3D.ppm");
  write_noise_ppm(&fractal_2d_noise, "noise_test_2D_F.ppm");
  write_noise_ppm(&slice_fractal_3d_noise, "noise_test_3D_F.ppm");
  write_noise_ppm(&example_noise, "noise_test_ex.ppm");
  write_noise_ppm(&example_wrapped_noise, "noise_test_wrapped.ppm");
  write_noise_ppm(&terrain_noise, "noise_test_terrain.ppm");
  write_noise_ppm(&wrapped_terrain, "noise_test_wrapped_terrain.ppm");
  return 0;
}
