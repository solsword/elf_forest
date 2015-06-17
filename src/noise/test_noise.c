
#include <stdlib.h>
#include <stdio.h>

#include "noise.h"

static float const SCALE = 1/32.0;
static float const NOISE_Z = 0.0;

static ptrdiff_t const SALT = 1717283;

#define N_FALSE_COLORS 27

#define LIGHT_STRENGTH 0.6

// 26 colors from dark blue to a shoreline and then dark green to bright green
// to yellow, gray, and then white, with an extra magenta to test slight
// out-of-bounds at the top.
static int const FALSE_COLOR[3*(N_FALSE_COLORS + 1)] = {
  0x0,  0x0,  0xf,
  0x1,  0x1,  0xf,
  0x2,  0x2,  0xf,
  0x3,  0x3,  0xf,
  0x4,  0x4,  0xf,
  0x5,  0x5,  0xf,
  0x5,  0x6,  0xf,
  0x6,  0x6,  0xf,
  0x6,  0x7,  0xf, // last blue
  0x0,  0x4,  0x0,
  0x0,  0x5,  0x0,
  0x0,  0x6,  0x0,
  0x0,  0x7,  0x0,
  0x1,  0x8,  0x0,
  0x2,  0x9,  0x1,
  0x3,  0xa,  0x2,
  0x4,  0xb,  0x3,
  0x6,  0xc,  0x4,
  0x8,  0xd,  0x5, // last green
  0xd,  0xd,  0x6,
  0xe,  0xe,  0x7,
  0xf,  0xf,  0x9,
  0xe,  0xe,  0xa, // last yellow
  0xd,  0xd,  0xc,
  0xd,  0xd,  0xd,
  0xe,  0xe,  0xe,
  0xf,  0xf,  0xf, // full white
  0xf,  0xf,  0xf,
};

float unsalt_2d_noise(float x, float y) {
  return sxnoise_2d(x, y, 7);
}

float slice_3d_noise(float x, float y) {
  return sxnoise_3d(x, y, NOISE_Z, SALT);
}

float unsalt_worley_noise(float x, float y) {
  return wrnoise_2d(x, y, 117);
}

float fancy_worley_noise(float x, float y) {
  float dontcare;
  return wrnoise_2d_fancy(
    x, y,
    117,
    0, 0,
    &dontcare, &dontcare,
    0
  );
}

float fancier_worley_noise(float x, float y) {
  float dontcare;
  return wrnoise_2d_fancy(
    x, y,
    117,
    0, 0,
    &dontcare, &dontcare,
    WORLEY_FLAG_INCLUDE_NEXTBEST
  );
}

float fancy_smooth_worley_noise(float x, float y) {
  float dontcare;
  return wrnoise_2d_fancy(
    x, y,
    117,
    0, 0,
    &dontcare, &dontcare,
    WORLEY_FLAG_SMOOTH_SIDES
  );
}

float big_sxnoise_2d(float x, float y, ptrdiff_t salt) {
  return sxnoise_2d(x*0.4, y*0.4, salt);
}

float return_x(float x, float y, ptrdiff_t salt) {
  return x;
}

float return_y(float x, float y, ptrdiff_t salt) {
  return y;
}

float return_y_minus_x(float x, float y, ptrdiff_t salt) {
  return y - x;
}

float dendroid_noise(float x, float y) {
  return -1 + 2*dnnoise_2d(
    x*1.5, y*1.5,
    117,
    &big_sxnoise_2d,
    //&return_x,
    1235
  );
}

float dendroid_and_simplex(float x, float y) {
  float sx = big_sxnoise_2d(x*1.5, y*1.5, 1737);
  float dn = dendroid_noise(x, y);
  return ((0.3 + 0.7*(1-0.5*(1 + sx)))*dn + sx) / 2.0;
}

float dendroid_base_octave(float x, float y, ptrdiff_t seed) {
  return -1 + 2*dnnoise_2d(
    x*0.4, y*0.4,
    187,
    &big_sxnoise_2d,
    312
  );
}

float fractal_dendroid_noise(float x, float y) {
  float base = dendroid_base_octave(x, y, 187);
  float add = -1 + 2*dnnoise_2d(
    x, y,
    171,
    &dendroid_base_octave,
    187
  );
  return (base + 0.5*add)/1.5;
}

float dendroid_distorted_simplex_noise(float x, float y) {
  float d = 0.3 * dendroid_noise(x*0.8, y*0.8);
  return unsalt_2d_noise(x + d, y + d);
}

float dendroid_mountains(float x, float y) {
  // Note that we aren't even taking full advantage of dendroid noise's
  // capability to "flow" across underlying manifolds here.
  x *= 1.5;
  y *= 1.5;
  float dx = 0.23 * sxnoise_2d(x*1.1, y*1.1, 81721);
  float dy = 0.23 * sxnoise_2d(x*1.1, y*1.1, 45467);
  dx += 0.12 * sxnoise_2d(x*2.4, y*2.4, 5464);
  dy += 0.12 * sxnoise_2d(x*2.4, y*2.4, 8744);
  float str = sxnoise_2d(x*0.4, y*0.4, 5515);
  dx *= str;
  dy *= str;
  float cont = sinf(x*0.55 + sxnoise_2d(x*0.4, y*0.4, 4157) - 0.3);
  cont *= cosf(y*0.52 + sxnoise_2d(x*0.4, y*0.4, 6745) + 0.3);
  float interp = sxnoise_2d(x*0.3, y*0.3, 79465);
  interp = (1 + interp)/2.0;
  interp *= interp;
  interp *= interp;
  interp = 0.1 + 0.9*interp;
  interp *= (1 + cont)/2.0;
  float base = dendroid_noise(x + dx, y + dy);
  base = (1 + base) / 2.0;
  base *= 1.2;
  float alt = sxnoise_2d(x*0.8, y*0.8, 88467);
  alt += 0.5*sxnoise_2d(x*1.5, y*1.5, 6714);
  alt /= 1.5;
  alt *= 0.14;
  float full = interp*(0.8*base+0.2*alt) + (1 - interp)*(0.95*alt+0.05*base);
  return 1.3*(cont+full)/2.0 - 0.2;
}

float fractal_2d_noise(float x, float y) {
  return fractal_sxnoise_2d(
    x*0.5, y*0.5,
    4,
    2.0,
    0.5,
    0.0, 0.0
  );
}

float slice_fractal_3d_noise(float x, float y) {
  return fractal_sxnoise_3d(
    x*0.5, y*0.5, NOISE_Z,
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
    1829401
  );
}

float terrain_noise(float x, float y, ptrdiff_t salt) {
  x += 1600;
  x *= 18;
  y *= 18;
  //x *= 6;
  //y *= 6;
  //x *= 2;
  //y *= 2;
  float ds = 4*sxnoise_2d(x*0.08, y*0.08, salt);
  float dslx = 50*stretch(sxnoise_2d(x*0.02+500, y*0.02, salt), 1.3);
  float dsly = 50*stretch(sxnoise_2d(x*0.02+1000, y*0.02, salt), 1.3);
  float dx = x + ds;
  float dy = y + ds;
  float dontcare;

  // Worley-based details:
  float result = wrnoise_2d_fancy(
    x*0.19, y*0.19,
    salt,
    0, 0,
    &dontcare, &dontcare,
    WORLEY_FLAG_INCLUDE_NEXTBEST | WORLEY_FLAG_SMOOTH_SIDES
    //WORLEY_FLAG_INCLUDE_NEXTBEST
    //WORLEY_FLAG_SMOOTH_SIDES
    //0
  );

  //return result * 2 - 1;

  result += 0.5 * smooth(
    wrnoise_2d_fancy(
      x*0.13+1000, y*0.13,
      salt,
      0, 0,
      &dontcare, &dontcare,
      WORLEY_FLAG_INCLUDE_NEXTBEST | WORLEY_FLAG_SMOOTH_SIDES
    ),
    2.1,
    0.5
  );
  result += 0.25 * smooth(
    wrnoise_2d_fancy(
      x*0.41+5000, y*0.41,
      salt,
      0, 0,
      &dontcare, &dontcare,
      WORLEY_FLAG_INCLUDE_NEXTBEST | WORLEY_FLAG_SMOOTH_SIDES
    ),
    2.1,
    0.5
  );

  result += sxnoise_2d(x*0.15+800, y*0.15, salt);
  result += sxnoise_2d(x*0.45+300, y*0.45, salt);

  result /= 3.75;
  //result /= 1.75;

  //return result;
  //result = 0;

  // Simplex-based details.
  result += 0.5 * (
    sxnoise_2d(dx*0.12+10, dy*0.12, salt) +
    0.8 * sxnoise_2d(x*0.1+7, y*0.1, salt) +
    0.6 * sxnoise_2d(dx*0.2+9, dy*0.2, salt) +
    0.4 * sxnoise_2d(x*0.41+9, y*0.41, salt)
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
  result = smooth((result + 1)/2.0, 3.2, 0.7) * 2 - 1;
  //result = stretch((1 + result)/2.0, 1.6) * 2 - 1;

  // Terrace the oceans, continental shelves, and land:
  result = (1 + result)/2.0;
  float depths = smooth(result/0.15, 3.3, 0.8)*0.19;
  float cshelves = (result - 0.15) / (0.281 - 0.15);
  cshelves = 0.19 + smooth(cshelves, 2.4, 0.6) * (0.281 - 0.19);
  float plains = (result - 0.281)/(0.63-0.281);
  plains = 0.281 + smooth(plains, 2.5, 0.7) * (0.63 - 0.281);
  float mountains = (result - 0.63)/(1-0.63);
  mountains = 0.63 + smooth(mountains, 1.8, 0.8)*(1 - 0.63);
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

float unsalted_terrain(float x, float y) {
  return terrain_noise(x, y, SALT);
}

float wrapped_terrain(float x, float y) {
  return tiled_func(&terrain_noise, x, y, 256*SCALE, 0, 5);
}

float grad_noise(float x, float y, float *dx, float *dy) {
  return sxnoise_grad_2d(x * 0.8, y * 0.8, 17, dx, dy);
}

float zoomed_noise(float x, float y) {
  //float zf = 0.0000004;
  float zf =   0.0004;
  float yo = 10000;
  x *= zf;
  y *= zf;
  y += yo;
  float top = -2, bot = 2;
  float test = sxnoise_2d(0.0, yo, SALT);
  if (test > top) { top = test; }
  if (test < bot) { bot = test; }
  test = sxnoise_2d(0.0, 256*SCALE*zf + yo, SALT);
  if (test > top) { top = test; }
  if (test < bot) { bot = test; }
  test = sxnoise_2d(256*SCALE*zf, 256*SCALE*zf + yo, SALT);
  if (test > top) { top = test; }
  if (test < bot) { bot = test; }
  test = sxnoise_2d(256*SCALE*zf, yo, SALT);
  if (test > top) { top = test; }
  if (test < bot) { bot = test; }
  float result = sxnoise_2d(x, y, SALT);
  //*
  result -= bot;
  result /= (top - bot);
  return 2*result - 1;
  // */ return result;
}


static inline void colormap(float n, int* color, float* fraction) {
  float scaled;
  if (n == 1.0) {
    scaled = N_FALSE_COLORS - 1;
  } else {
    scaled = ((n+1)/2.0)*N_FALSE_COLORS;
  }
  *color = (int) ffloor(scaled);
  *fraction = scaled - *color;
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
  fprintf(fp, "255\n");
  col = 0;
  for (j = 0; j < 256; ++j) {
    for (i = 0; i < 256; ++i) {
      n = noisefunc(i*SCALE, j*SCALE); // [-1, 1)

      colormap(n, &color, &nf);

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
      lighting = lighting*LIGHT_STRENGTH + (1 - LIGHT_STRENGTH);

      // Interpolate colors from the table:
      if (color < 0 || color >= N_FALSE_COLORS) {
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
      if (light) {
        r *= lighting;
        g *= lighting;
        b *= lighting;
      }
      r *= 255;
      g *= 255;
      b *= 255;
      fprintf(fp, "%03d %03d %03d ", (int) r, (int) g, (int) b);
      col += 12;
      if (col >= 68) {
        fprintf(fp, "\n");
        col = 0;
      }
    }
  }
  fprintf(fp, "\n");
  fclose(fp);
  return;
}

void write_grad_ppm(
  float (*ngfunc)(float, float, float*, float*),
  char const * const filename
) {
  int i, j, ij, col;
  float r, g, b;
  FILE *fp;
  float n[256*256];
  float dx[256*256];
  float dy[256*256];
  float max = 0, maxdx = 0, maxdy = 0;
  float min = 0, mindx = 0, mindy = 0;
  float lightx = 0.2;
  float lighty = -0.5;
  float lightz = 1.0;
  float grx, gry, grz, grl;
  float lvx, lvy, lvz, lvl;
  float lighting;
  float nn, ndx, ndy, cdx, cdy;
  float ndxdx, ndxdy, ndydx, ndydy;
  fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Error: couldn't open destination file '%s'.\n", filename);
    exit(1);
  }
  fprintf(fp, "P3\n");
  fprintf(fp, "# noise test ppm\n");
  fprintf(fp, "# Auto-generated test of noise gradient function in noise.c.\n");
  fprintf(fp, "256 256\n");
  fprintf(fp, "255\n");
  col = 0;

  // First, compute noise and gradient values and record mins and maxes:
  for (j = 0; j < 256; ++j) {
    for (i = 0; i < 256; ++i) {
      ij = 256 * j + i;
      n[ij] = ngfunc(i*SCALE, j*SCALE, &(dx[ij]), &(dy[ij])); // [-1, 1)
      if (n[ij] > max) { max = n[ij]; }
      if (dx[ij] > maxdx) { maxdx = dx[ij]; }
      if (dy[ij] > maxdy) { maxdy = dy[ij]; }
      if (n[ij] < min) { min = n[ij]; }
      if (dx[ij] < mindx) { mindx = dx[ij]; }
      if (dy[ij] < mindy) { mindy = dy[ij]; }
    }
  }

  // Now loop over our values again to write out colors:
  for (j = 0; j < 256; ++j) {
    for (i = 0; i < 256; ++i) {
      ij = 256 * j + i;

      nn = (n[ij] - min) / (max - min);
      ndx = (dx[ij] - mindx) / (maxdx - mindx);
      ndy = (dy[ij] - mindy) / (maxdy - mindy);

      cdx = 0;
      if (i < 255) {
        cdx += nn - (n[ij+1] - min) / (max - min);
      }
      if (i > 0) {
        cdx += (n[ij-1] - min) / (max - min) - nn;
      }
      if (i > 0 && i < 255) {
        cdx /= 2.0;
      }

      cdy = 0;
      if (j < 255) {
        cdy = nn - (n[ij+256] - min) / (max - min);
      }
      if (j > 0) {
        cdy += (n[ij-256] - min) / (max - min) - nn;
      }
      if (j < 255 && j > 0) {
        cdy /= 2.0;
      }

      ndxdx = 0;
      if (i < 255) {
        ndxdx = ndx - (dx[ij+1] - mindx) / (maxdx - mindx);
      }
      if (i > 0) {
        ndxdx += (dx[ij-1] - mindx) / (maxdx - mindx) - ndx;
      }
      if (i < 255 && i > 0) {
        ndxdx /= 2.0;
      }

      ndxdy = 0;
      if (j < 255) {
        ndxdy = ndx - (dx[ij+256] - mindx) / (maxdx - mindx);
      }
      if (j > 0) {
        ndxdy += (dx[ij-256] - mindx) / (maxdx - mindx) - ndx;
      }
      if (j < 255 && j > 0) {
        ndxdy /= 2.0;
      }

      ndydx = 0;
      if (i < 255) {
        ndydx = ndy - (dy[ij+1] - mindy) / (maxdy - mindy);
      }
      if (i > 0) {
        ndydx += (dy[ij-1] - mindy) / (maxdy - mindy) - ndy;
      }
      if (i < 255 && i > 0) {
        ndydx /= 2.0;
      }

      ndydy = 0;
      if (j < 255) {
        ndydy = ndy - (dy[ij+256] - mindy) / (maxdy - mindy);
      }
      if (j > 0) {
        ndydy += (dy[ij-256] - mindy) / (maxdy - mindy) - ndy;
      }
      if (j < 255 && j > 0) {
        ndydy /= 2.0;
      }

      // Base color:
      r = nn;
      g = r;
      b = r;

      /*
      r = ndx;
      g = r;
      b = r;
      // */

      /*
      r *= ndx;
      g *= ndx;
      // */
      /*
      r *= ndy;
      g *= ndy;
      // */

      // Analytical gradient:
      //*
      grx = -dx[ij];
      gry = -dy[ij];
      grz = 1;
      // */
      // Computed gradient:
      /*
      grx = cdx*32;
      gry = cdy*32;
      grz = 0.7;
      // */
      // Normalize:
      grl = sqrtf(grx*grx + gry*gry + grz*grz);
      grx /= grl;
      gry /= grl;
      grz /= grl;
      // Compute light vector:
      lvx = lightx*256 - i;
      lvy = lighty*256 - j;
      lvz = (lightz - n[ij]);
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
      lighting = lighting*LIGHT_STRENGTH + (1 - LIGHT_STRENGTH);

      //*
      r *= lighting;
      g *= lighting;
      b *= lighting;
      // */

      r *= 255;
      g *= 255;
      b *= 255;
      fprintf(fp, "%03d %03d %03d ", (int) r, (int) g, (int) b);
      col += 12;
      if (col >= 68) {
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
  write_noise_ppm(&unsalt_2d_noise, "nt_2D.ppm", 0, 0);
  write_noise_ppm(&unsalt_2d_noise, "nt_2D_shaded.ppm", 0, 1);
  write_noise_ppm(&unsalt_worley_noise, "nt_worley.ppm", 0, 0);
  write_noise_ppm(&fancy_worley_noise, "nt_worley_fancy.ppm", 0, 0);
  write_noise_ppm(&fancier_worley_noise, "nt_worley_fancier.ppm", 0, 0);
  write_noise_ppm(&fancy_smooth_worley_noise, "nt_worley_fancy_sm.ppm", 0, 0);
  write_noise_ppm(&fancier_worley_noise, "nt_worley_fshade.ppm", 0, 1);
  write_noise_ppm(&fancy_smooth_worley_noise, "nt_worley_fsmshade.ppm", 0, 1);
  write_noise_ppm(&dendroid_noise, "nt_dendroid.ppm", 0, 0);
  write_noise_ppm(&dendroid_noise, "nt_dendroid_lit.ppm", 0, 1);
  write_noise_ppm(&dendroid_and_simplex, "nt_dend_simp_lit.ppm", 0, 1);
  write_noise_ppm(&fractal_dendroid_noise, "nt_dend_fract_lit.ppm", 0, 1);
  write_noise_ppm(&dendroid_distorted_simplex_noise, "nt_sxn_d_dnn.ppm", 0, 1);
  write_noise_ppm(&dendroid_mountains, "nt_dendroid_mountains.ppm", 1, 1);
  write_noise_ppm(&slice_3d_noise, "nt_3D.ppm", 1, 1);
  write_noise_ppm(&fractal_2d_noise, "nt_2D_F.ppm", 1, 1);
  write_noise_ppm(&slice_fractal_3d_noise, "nt_3D_F.ppm", 1, 1);
  write_noise_ppm(&example_noise, "nt_ex.ppm", 1, 1);
  write_noise_ppm(&example_wrapped_noise, "nt_wrapped.ppm", 1, 1);
  write_noise_ppm(&unsalted_terrain, "nt_terrain.ppm", 1, 1);
  write_noise_ppm(&wrapped_terrain, "nt_wrapped_terrain.ppm", 1, 1);
  write_noise_ppm(&zoomed_noise, "nt_zoomed.ppm", 0, 0);
  write_grad_ppm(&grad_noise, "nt_grad.ppm");
  return 0;
}
