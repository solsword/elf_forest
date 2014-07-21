#ifndef TERRAIN_H
#define TERRAIN_H

// terrain.h
// Code for determining terrain height.

#include <math.h>

#include "noise/noise.h"
#include "world/world.h"

/**************
 * Parameters *
 **************/

// Basic frequencies:
#define   TR_FREQUENCY_CONTINENTS 0.0000002 // 312500-chunk ~= 3300 km features
#define     TR_FREQUENCY_GEOFORMS 0.000003 // 20833-chunk ~= 220 km features
#define    TR_FREQUENCY_MOUNTAINS 0.00001 // 6250-chunk ~= 67 km features
#define        TR_FREQUENCY_HILLS 0.0005 // 125-chunk ~= 1.3 km features
#define       TR_FREQUENCY_RIDGES 0.003 // ~20-chunk ~= 215 m features
#define       TR_FREQUENCY_MOUNDS 0.01 // ~6-chunk ~= 64 m features
#define      TR_FREQUENCY_DETAILS 0.05 // ~20-cell ~= 13 m features
#define        TR_FREQUENCY_BUMPS 0.25 // ~4-cell ~= 2.5 m variance

// Distortion frequencies:
#define   TR_DFQ_CONTINENTS 0.00000004 // 1/5 of base
#define     TR_DFQ_GEOFORMS 0.0000018 // 3/5 of base
#define    TR_DFQ_MOUNTAINS 0.000006 // 3/5 of base
#define        TR_DFQ_HILLS 0.0002 // 2/5 of base
#define       TR_DFQ_RIDGES 0.0006 // 1/5 of base
#define       TR_DFQ_MOUNDS 0.004 // 2/5 of base
#define      TR_DFQ_DETAILS 0.02 // 2/5 of base
#define        TR_DFQ_BUMPS 0.05 // 1/5 of base

// Distortion strengths:
#define   TR_DS_CONTINENTS 25000000.0 // ~5 periods
#define     TR_DS_GEOFORMS 6000000.0 // ~2 periods
#define    TR_DS_MOUNTAINS 100000.0 // ~1 period
#define        TR_DS_HILLS 2000.0 // ~1 period
#define       TR_DS_RIDGES 200.0 // ~2/3 period
#define       TR_DS_MOUNDS 50.0 // ~1/2 period
#define      TR_DS_DETAILS 10.0 // ~1/2 period
#define        TR_DS_BUMPS 2.0 // ~1/2 period


// Geoform parameters:

// Noise->geoform mapping (see compute_geoforms):
#define       TR_GEOMAP_SHELF 0.3
#define       TR_GEOMAP_OCEAN 0.45
#define      TR_GEOMAP_PLAINS 0.6
#define       TR_GEOMAP_HILLS 0.7
#define   TR_GEOMAP_MOUNTAINS 0.85

// Geoform heights:
#define        TR_HEIGHT_OCEAN_DEPTHS 1500
#define   TR_HEIGHT_CONTINENTAL_SHELF 14750
#define           TR_HEIGHT_SEA_LEVEL 15000
#define      TR_HEIGHT_COASTAL_PLAINS 15150
#define           TR_HEIGHT_HIGHLANDS 16500
#define      TR_HEIGHT_MOUNTAIN_BASES 18500
#define       TR_HEIGHT_MOUNTAIN_TOPS 27000

// Terrain noise contributions:
#define     TR_SCALE_HILLS 1000
#define    TR_SCALE_RIDGES 200
#define    TR_SCALE_MOUNDS 80
#define   TR_SCALE_DETAILS 8
#define     TR_SCALE_BUMPS 2

/***********
 * Globals *
 ***********/

// A salt that gets added to noise input coordinates:
extern float TR_NOISE_OFFSET;

// Amplification factor for terrain height:
extern float TR_TERRAIN_HEIGHT_AMP;

/********************
 * Inline Functions *
 ********************/

static inline void get_noise(
  r_pos_t x, r_pos_t y,
  float *continents, float *geoforms, float *mountains, float *hills,
  float *ridges, float *mounds, float *details, float *bumps
) {
  float dsx, dsy;
  x += TR_NOISE_OFFSET;
  y += TR_NOISE_OFFSET;
  dsx = TR_DS_CONTINENTS * sxnoise_2d(
    x * TR_DFQ_CONTINENTS,
    y*TR_DFQ_CONTINENTS
  );
  dsy = TR_DS_CONTINENTS * sxnoise_2d(
    y * TR_DFQ_CONTINENTS,
    x*TR_DFQ_CONTINENTS
  );
  *continents = (
    cos((x + dsx) * TR_FREQUENCY_CONTINENTS) *
    sin((y + dsy) * TR_FREQUENCY_CONTINENTS)
  ) + (
    cos((x - dsx) * TR_FREQUENCY_CONTINENTS * 1.8) *
    sin((y - dsy) * TR_FREQUENCY_CONTINENTS * 1.8)
  );
  dsx = TR_DS_GEOFORMS * sxnoise_2d(x * TR_DFQ_GEOFORMS, y*TR_DFQ_GEOFORMS);
  dsy = TR_DS_GEOFORMS * sxnoise_2d(y * TR_DFQ_GEOFORMS, x*TR_DFQ_GEOFORMS);
  *geoforms = sxnoise_2d(
    (x + dsx) * TR_FREQUENCY_GEOFORMS,
    (y + dsy) * TR_FREQUENCY_GEOFORMS
  );
  dsx = TR_DS_MOUNTAINS * sxnoise_2d(x * TR_DFQ_MOUNTAINS, y*TR_DFQ_MOUNTAINS);
  dsy = TR_DS_MOUNTAINS * sxnoise_2d(y * TR_DFQ_MOUNTAINS, x*TR_DFQ_MOUNTAINS);
  *mountains = sxnoise_2d(
    (x + dsx) * TR_FREQUENCY_MOUNTAINS,
    (y + dsy) * TR_FREQUENCY_MOUNTAINS
  );
  *mountains += 2*smooth(
    wrnoise_2d_fancy(
      (x + dsy) * TR_FREQUENCY_MOUNTAINS,
      (y + dsx) * TR_FREQUENCY_MOUNTAINS,
      0, 0,
      WORLEY_FLAG_INCLUDE_NEXTBEST | WORLEY_FLAG_SMOOTH_SIDES
    ),
    2,
    0.5
  );
  *mountains += wrnoise_2d_fancy(
    (x - dsx) * TR_FREQUENCY_MOUNTAINS * 0.7,
    (y - dsy) * TR_FREQUENCY_MOUNTAINS * 0.7,
    0, 0,
    WORLEY_FLAG_INCLUDE_NEXTBEST | WORLEY_FLAG_SMOOTH_SIDES
  );
  *mountains /= 4.0;
  dsx = TR_DS_HILLS * sxnoise_2d(x * TR_DFQ_HILLS, y*TR_DFQ_HILLS);
  dsy = TR_DS_HILLS * sxnoise_2d(y * TR_DFQ_HILLS, x*TR_DFQ_HILLS);
  *hills = sxnoise_2d(
    (x + dsx) * TR_FREQUENCY_HILLS,
    (y + dsy) * TR_FREQUENCY_HILLS
  );
  dsx = TR_DS_RIDGES * sxnoise_2d(x * TR_DFQ_RIDGES, y*TR_DFQ_RIDGES);
  dsy = TR_DS_RIDGES * sxnoise_2d(y * TR_DFQ_RIDGES, x*TR_DFQ_RIDGES);
  *ridges = sxnoise_2d(
    (x + dsx) * TR_FREQUENCY_RIDGES,
    (y + dsy) * TR_FREQUENCY_RIDGES
  );
  *ridges += 0.5 * wrnoise_2d(
    (x + dsy) * TR_FREQUENCY_RIDGES,
    (y - dsx) * TR_FREQUENCY_RIDGES
  );
  *ridges /= 1.5;
  dsx = TR_DS_MOUNDS * sxnoise_2d(x * TR_DFQ_MOUNDS, y*TR_DFQ_MOUNDS);
  dsy = TR_DS_MOUNDS * sxnoise_2d(y * TR_DFQ_MOUNDS, x*TR_DFQ_MOUNDS);
  *mounds = sxnoise_2d(
    (x + dsx) * TR_FREQUENCY_MOUNDS,
    (y + dsy) * TR_FREQUENCY_MOUNDS
  );
  dsx = TR_DS_DETAILS * sxnoise_2d(x * TR_DFQ_DETAILS, y*TR_DFQ_DETAILS);
  dsy = TR_DS_DETAILS * sxnoise_2d(y * TR_DFQ_DETAILS, x*TR_DFQ_DETAILS);
  *details = sxnoise_2d(
    (x + dsx) * TR_FREQUENCY_DETAILS,
    (y + dsy) * TR_FREQUENCY_DETAILS
  );
  dsx = TR_DS_BUMPS * sxnoise_2d(x * TR_DFQ_BUMPS, y*TR_DFQ_BUMPS);
  dsy = TR_DS_BUMPS * sxnoise_2d(y * TR_DFQ_BUMPS, x*TR_DFQ_BUMPS);
  *bumps = sxnoise_2d(
    (x + dsx) * TR_FREQUENCY_BUMPS,
    (y + dsy) * TR_FREQUENCY_BUMPS
  );
}

// Remaps the given value (on [0, 1]) to account for the shape and height of
// the overall height profile.
static inline float geomap(float base) {
  float interp;
  if (base < TR_GEOMAP_SHELF) { // deep ocean
    interp = base / TR_GEOMAP_SHELF;
    interp = smooth(interp, 2, 0.1);
    return (
      TR_HEIGHT_OCEAN_DEPTHS +
      interp * (TR_HEIGHT_CONTINENTAL_SHELF - TR_HEIGHT_OCEAN_DEPTHS)
    );

  } else if (base < TR_GEOMAP_OCEAN) { // continental shelf
    interp = (base - TR_GEOMAP_SHELF) / (TR_GEOMAP_OCEAN - TR_GEOMAP_SHELF);
    interp = smooth(interp, 1.6, 0.2);
    // TODO: Sea cliffs?
    return (
      TR_HEIGHT_CONTINENTAL_SHELF +
      interp * (TR_HEIGHT_SEA_LEVEL - TR_HEIGHT_CONTINENTAL_SHELF)
    );

  } else if (base < TR_GEOMAP_PLAINS) { // coastal plain
    interp = (base - TR_GEOMAP_OCEAN) / (TR_GEOMAP_PLAINS - TR_GEOMAP_OCEAN);
    interp = smooth(interp, 1.2, 0.5);
    return (
      TR_HEIGHT_SEA_LEVEL +
      interp * (TR_HEIGHT_COASTAL_PLAINS - TR_HEIGHT_SEA_LEVEL)
    );

  } else if (base < TR_GEOMAP_HILLS) { // hills
    interp = (base - TR_GEOMAP_PLAINS) / (TR_GEOMAP_HILLS - TR_GEOMAP_PLAINS);
    interp = smooth(interp, 1.4, 0.35);
    return (
      TR_HEIGHT_COASTAL_PLAINS +
      interp * (TR_HEIGHT_HIGHLANDS - TR_HEIGHT_COASTAL_PLAINS)
    );

  } else if (base < TR_GEOMAP_MOUNTAINS) { // highlands
    interp = (base - TR_GEOMAP_HILLS) / (TR_GEOMAP_MOUNTAINS - TR_GEOMAP_HILLS);
    interp = 0.2 * sin(interp*4*M_PI) + 0.8 * smooth(interp, 1.3, 0.5);
    return (
      TR_HEIGHT_HIGHLANDS +
      interp * (TR_HEIGHT_MOUNTAIN_BASES - TR_HEIGHT_HIGHLANDS)
    );

  } else { // mountains
    interp = (base - TR_GEOMAP_MOUNTAINS) / (1 - TR_GEOMAP_MOUNTAINS);
    interp = pow(interp, 2.4);
    return (
      TR_HEIGHT_MOUNTAIN_BASES +
      interp * (TR_HEIGHT_MOUNTAIN_TOPS - TR_HEIGHT_MOUNTAIN_BASES)
    );

  }
}

/*
static inline void compute_geoforms(
  float index,
  float *depths, float *oceans, float *plains, float *hills, float *mountains
) {
  if (index < TR_GEOMAP_DEPTHS) {
    *depths = 1.0;
  } else if (index < TR_GEOMAP_OCEAN) {
    *depths = fmax(
      0.0,
      (TR_GEOMAP_OCEAN - index) / (TR_GEOMAP_OCEAN - TR_GEOMAP_DEPTHS)
    );
    *depths = smooth(*depths, 2, 0.5);
    *oceans = 1.0 - *depths;
  } else if (index < TR_GEOMAP_PLAINS) {
    *oceans = fmax(
      0.0,
        (TR_GEOMAP_PLAINS - index) / (TR_GEOMAP_PLAINS - TR_GEOMAP_OCEAN)
    );
    *oceans = smooth(*oceans, 2, 0.5);
    *plains = 1.0 - *oceans;
  } else if (index < TR_GEOMAP_HILLS) {
    *plains = fmax(
      0.0,
        (TR_GEOMAP_HILLS - index) / (TR_GEOMAP_HILLS - TR_GEOMAP_PLAINS)
    );
    *plains = smooth(*plains, 2, 0.5);
    *hills = 1.0 - *plains;
  } else if (index < TR_GEOMAP_MOUNTAINS) {
    *hills = fmax(
      0.0,
        (TR_GEOMAP_MOUNTAINS - index) / (TR_GEOMAP_MOUNTAINS - TR_GEOMAP_HILLS)
    );
    *mountains = smooth(*mountains, 2, 0.5);
    *mountains = 1.0 - *hills;
  } else {
    *mountains = 1.0;
  }
}
*/

static inline float oabs(float noise) {
  return (0.999 - fabs(noise));
}

static inline float oabssq(float noise) {
  noise = oabs(noise);
  return noise * noise;
}

static inline float oabscb(float noise) {
  noise = oabs(noise);
  return noise * noise * noise;
}

/*************
 * Functions *
 *************/

// Returns the terrain height at the given region position in blocks.
float terrain_height(region_pos *pos);

#endif // ifndef TERRAIN_H
