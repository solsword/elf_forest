
#include <stdlib.h>
#include <stdio.h>

#include "noise.h"

static float const SCALE = 1/32.0;
static float const NOISE_Z = 0.0;

static int const FALSE_COLOR[99] = {
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
  x += 1600;
  x *= 18;
  y *= 18;
  //x *= 6;
  //y *= 6;
  //x *= 2;
  //y *= 2;
  float ds = 4*sxnoise_2d(x*0.08, y*0.08);
  float dslx = 50*stretch(sxnoise_2d(x*0.02+500, y*0.02), 1.3);
  float dsly = 50*stretch(sxnoise_2d(x*0.02+1000, y*0.02), 1.3);
  float dx = x + ds;
  float dy = y + ds;

  // Worley-based details:
  float result = wrnoise_2d_fancy(
    x*0.19, y*0.19,
    0, 0,
    WORLEY_FLAG_INCLUDE_NEXTBEST | WORLEY_FLAG_SMOOTH_SIDES
    //WORLEY_FLAG_INCLUDE_NEXTBEST
    //WORLEY_FLAG_SMOOTH_SIDES
    //0
  );

  //return result * 2 - 1;

  result += 0.5 * smooth(
    wrnoise_2d_fancy(
      x*0.13+1000, y*0.13,
      0, 0,
      WORLEY_FLAG_INCLUDE_NEXTBEST | WORLEY_FLAG_SMOOTH_SIDES
    ),
    1.7,
    0.5
  );
  result += 0.25 * smooth(
    wrnoise_2d_fancy(
      x*0.41+5000, y*0.41,
      0, 0,
      WORLEY_FLAG_INCLUDE_NEXTBEST | WORLEY_FLAG_SMOOTH_SIDES
    ),
    1.7,
    0.5
  );

  result += sxnoise_2d(x*0.15+800, y*0.15);
  result += sxnoise_2d(x*0.45+300, y*0.45);

  result /= 3.75;
  //result /= 1.75;

  //return result;
  //result = 0;

  // Simplex-based details.
  result += 0.5 * (
    sxnoise_2d(dx*0.12+10, dy*0.12) +
    0.8 * sxnoise_2d(x*0.1+7, y*0.1) +
    0.6 * sxnoise_2d(dx*0.2+9, dy*0.2) +
    0.4 * sxnoise_2d(x*0.41+9, y*0.41)
  ) / 2.8;
  result /= 1.5;
  result = fmax(-1.0, fmin(1.0, result * 2.0));

  //return result;
  //result = 0;

  // Heavily distorted periodic continents.
  result += 1.5 * cos((dx+dslx)*0.1) * sin((dy+dsly)*0.1);
  result += 2.5 * cos((dx-dslx)*0.045) * sin((dy-dsly)*0.045);
  result /= 5.0;

  //return result;
  //result = 0;

  // Stretching the result to establish sea level around 0.28:
  result = smooth((result + 1)/2.0, 2.4, 0.7) * 2 - 1;
  //result = stretch((1 + result)/2.0, 1.6) * 2 - 1;

  // Terrace the oceans, continental shelves, and land:
  result = (1 + result)/2.0;
  float depths = smooth(result/0.15, 2.6, 0.8)*0.19;
  float cshelves = (result - 0.15) / (0.281 - 0.15);
  cshelves = 0.19 + smooth(cshelves, 1.8, 0.6) * (0.281 - 0.19);
  float plains = (result - 0.281)/(0.63-0.281);
  plains = 0.281 + smooth(plains, 2, 0.7) * (0.63 - 0.281);
  float mountains = (result - 0.63)/(1-0.63);
  mountains = 0.63 + smooth(mountains, 1.3, 0.8)*(1 - 0.63);
  if (result < 0.15) {
    result = depths;
    //result = 0;
  } else if (result < 0.281) {
    result = cshelves;
    //result = 0;
  } else if (result < 0.63) {
    result = plains;
    //result = 0;
  } else {
    result = mountains;
    //result = result;
  }
  result = result * 2 - 1.0;
  return result;
}

float wrapped_terrain(float x, float y) {
  return tiled_func(&terrain_noise, x, y, 256*SCALE, 0, 5);
}

float zoomed_noise(float x, float y) {
  float zf = 0.0000004;
  //x *= zf;
  //y *= zf;
  //y += 3050;
  //float top = sxnoise_2d(0, 3050);
  //float bot = sxnoise_2d(256*zf, 256*zf + 3050);
  float top = sxnoise_2d(0.0, 0.0);
  float bot = sxnoise_2d(256.0*zf, 256.0*zf);
  //return 2 * ((sxnoise_2d(x, y) - bot) / (top - bot)) - 1;
  return sxnoise_2d(x, y);
}


void write_noise_ppm(
  float (*noisefunc)(float, float),
  char const * const filename,
  uint8_t map_colors,
  uint8_t light
) {
  float n, nf;
  int color;
  int i, j, col;
  float lightx = 0.2;
  float lighty = -0.5;
  float lightz = 1.0;
  float grx, gry, grz, grl;
  float lvx, lvy, lvz, lvl;
  float lighting;
  float r, g, b;
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
  fprintf(fp, "256\n");
  col = 0;
  for (j = 0; j < 256; ++j) {
    for (i = 0; i < 256; ++i) {
      n = noisefunc(i*SCALE, j*SCALE); // [-1, 1)

      nf = ((n+1.0) * 15.9999) - floor((n+1.0) * 15.9999);

      color = (int) floor((n + 1.0) * 15.9999); // [0, 31]

      // Compute gradient:
      grx = (n - noisefunc((i+1)*SCALE, j*SCALE))*32;
      grx += (noisefunc((i-1)*SCALE, j*SCALE) - n)*32;
      grx /= 2.0;
      gry = (n - noisefunc(i*SCALE, (j+1)*SCALE))*32;
      gry += (noisefunc(i*SCALE, (j-1)*SCALE) - n)*32;
      gry /= 2.0;
      grz = 1;
      //grx *= -1;
      //gry *= -1;
      //grz *= -1;
      grl = sqrtf(grx*grx + gry*gry + grz*grz);
      // Normalize:
      grx /= grl;
      gry /= grl;
      grz /= grl;
      // Compute light vector:
      lvx = lightx*256 - i;
      lvy = lighty*256 - j;
      lvz = (lightz - n);
      lvz *= 32;
      lvl = sqrtf(lvx*lvx + lvy*lvy + lvz*lvz);
      // Normalize:
      lvx /= lvl;
      lvy /= lvl;
      lvz /= lvl;
      // Dot product:
      lighting = (
        grx * lvx +
        gry * lvy +
        grz * lvz
      ); // [-1, 1]
      lighting = (lighting + 1)/2.0; // [0, 1]

      // Interpolate colors from the table:
      if (color < 0 || color > 31) {
        printf("Out of range! n, color = %.3f, %d\n", n, color);
        r = 1.0;
        g = 0.5;
        b = 0;
      } else {
        if (map_colors) {
          r = (1 - nf)*FALSE_COLOR[color*3] + nf*FALSE_COLOR[(color+1)*3];
          g = (1 - nf)*FALSE_COLOR[color*3+1] + nf*FALSE_COLOR[(color+1)*3+1];
          b = (1 - nf)*FALSE_COLOR[color*3+2] + nf*FALSE_COLOR[(color+1)*3+2];
          r /= 15.0;
          g /= 15.0;
          b /= 15.0;
        } else {
          r = (n+1)/2.0;
          g = (n+1)/2.0;
          b = (n+1)/2.0;
        }
      }
      /*
      r = 0.5;
      g = 0.5;
      b = 0.5;
      // */
      if (light) {
        r *= lighting;
        g *= lighting;
        b *= lighting;
      }
      r *= 255;
      g *= 255;
      b *= 255;
      fprintf(fp, "%02d %02d %02d ", (int) r, (int) g, (int) b);
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
  write_noise_ppm(&sxnoise_2d, "noise_test_2D.ppm", 1, 1);
  write_noise_ppm(&slice_3d_noise, "noise_test_3D.ppm", 1, 1);
  write_noise_ppm(&fractal_2d_noise, "noise_test_2D_F.ppm", 1, 1);
  write_noise_ppm(&slice_fractal_3d_noise, "noise_test_3D_F.ppm", 1, 1);
  write_noise_ppm(&example_noise, "noise_test_ex.ppm", 1, 1);
  write_noise_ppm(&example_wrapped_noise, "noise_test_wrapped.ppm", 1, 1);
  write_noise_ppm(&terrain_noise, "noise_test_terrain.ppm", 1, 1);
  write_noise_ppm(&wrapped_terrain, "noise_test_wrapped_terrain.ppm", 1, 1);
  write_noise_ppm(&zoomed_noise, "noise_test_zoomed.ppm", 0, 1);
  return 0;
}
