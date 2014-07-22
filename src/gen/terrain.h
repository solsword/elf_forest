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
#define   TR_FREQUENCY_CONTINENTS 0.000002 // 15625-chunk ~= 333 km features
#define     TR_FREQUENCY_GEOFORMS 0.00001 // 2083-chunk ~= 22 km features
#define    TR_FREQUENCY_MOUNTAINS 0.00003 // 312-chunk ~= 6.7 km features
#define        TR_FREQUENCY_HILLS 0.0005 // 125-chunk ~= 1.3 km features
#define       TR_FREQUENCY_RIDGES 0.003 // ~20-chunk ~= 215 m features
#define       TR_FREQUENCY_MOUNDS 0.01 // ~6-chunk ~= 64 m features
#define      TR_FREQUENCY_DETAILS 0.05 // ~20-cell ~= 13 m features
#define        TR_FREQUENCY_BUMPS 0.25 // ~4-cell ~= 2.5 m variance

// Distortion frequencies:
#define   TR_DFQ_CONTINENTS 0.0000004 // 1/5 of base
#define     TR_DFQ_GEOFORMS 0.000005 // ??? of base
#define    TR_DFQ_MOUNTAINS 0.0000001 // ??? of base
#define        TR_DFQ_HILLS 0.0002 // 2/5 of base
#define       TR_DFQ_RIDGES 0.0006 // 1/5 of base
#define       TR_DFQ_MOUNDS 0.004 // 2/5 of base
#define      TR_DFQ_DETAILS 0.02 // 2/5 of base
#define        TR_DFQ_BUMPS 0.05 // 1/5 of base

// Distortion strengths:
#define   TR_DS_CONTINENTS 1500000.0 // ~1.5 periods
#define     TR_DS_GEOFORMS 50000.0 // ~3/4 period
#define    TR_DS_MOUNTAINS 5000.0 // ~1/2 period
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
#define        TR_HEIGHT_OCEAN_DEPTHS 8000
#define   TR_HEIGHT_CONTINENTAL_SHELF 14750
#define           TR_HEIGHT_SEA_LEVEL 15000
#define      TR_HEIGHT_COASTAL_PLAINS 15150
#define           TR_HEIGHT_HIGHLANDS 16500
#define      TR_HEIGHT_MOUNTAIN_BASES 18500
#define       TR_HEIGHT_MOUNTAIN_TOPS 27000

// Terrain noise contributions:
#define     TR_SCALE_HILLS 100
#define    TR_SCALE_RIDGES 40
#define    TR_SCALE_MOUNDS 15
#define   TR_SCALE_DETAILS 8
#define     TR_SCALE_BUMPS 2

#define TR_MAX_HEIGHT (\
  (\
    TR_HEIGHT_MOUNTAIN_TOPS +\
    TR_SCALE_HILLS +\
    TR_SCALE_RIDGES +\
    TR_SCALE_MOUNDS +\
    TR_SCALE_DETAILS +\
    TR_SCALE_BUMPS \
  ) * 1.15 \
)

/*********
 * Enums *
 *********/

enum terrain_region_e {
  TR_REGION_DEPTHS = 0,
  TR_REGION_SHELF = 1,
  TR_REGION_PLAINS = 2,
  TR_REGION_HILLS = 3,
  TR_REGION_HIGHLANDS = 4,
  TR_REGION_MOUNTAINS = 5,
};
typedef enum terrain_region_e terrain_region;

/***********
 * Globals *
 ***********/

// A salt that gets added to noise input coordinates:
extern float TR_NOISE_OFFSET;

// Amplification factor for terrain height:
extern float TR_TERRAIN_HEIGHT_AMP;

// An array of names for each terrain region:
extern char const * const TR_REGION_NAMES[];

/********************
 * Inline Functions *
 ********************/

static inline void get_noise(
  r_pos_t x, r_pos_t y,
  float *continents, float *geoforms, float *mountains, float *hills,
  float *ridges, float *mounds, float *details, float *bumps
) {
  float dsx, dsy;
  float mountainous, hilly, ridged, mounded, detailed, bumpy;
  x += TR_NOISE_OFFSET;
  y += TR_NOISE_OFFSET;

  // continents
  dsx = TR_DS_CONTINENTS * sxnoise_2d(
    x * TR_DFQ_CONTINENTS,
    y * TR_DFQ_CONTINENTS
  );
  dsy = TR_DS_CONTINENTS * sxnoise_2d(
    x * TR_DFQ_CONTINENTS + 7000,
    y * TR_DFQ_CONTINENTS
  );
  *continents = (
    cosf((x + dsx) * TR_FREQUENCY_CONTINENTS) *
    sinf((y + dsy) * TR_FREQUENCY_CONTINENTS)
  ) + (
    cosf((x - dsx) * TR_FREQUENCY_CONTINENTS * 1.8) *
    sinf((y - dsy) * TR_FREQUENCY_CONTINENTS * 1.8)
  );

  // geoforms
  dsx = TR_DS_GEOFORMS * sxnoise_2d(x*TR_DFQ_GEOFORMS, y*TR_DFQ_GEOFORMS+500);
  dsy = TR_DS_GEOFORMS * sxnoise_2d(y*TR_DFQ_GEOFORMS, x*TR_DFQ_GEOFORMS);
  *geoforms = sxnoise_2d(
    (x + dsx) * TR_FREQUENCY_GEOFORMS,
    (y + dsy) * TR_FREQUENCY_GEOFORMS
  );
  mountainous = sxnoise_2d(
    (x + dsy + 5000) * TR_FREQUENCY_GEOFORMS * 1.5,
    (y - dsx) * TR_FREQUENCY_GEOFORMS * 1.5
  );
  mountainous = (1 + mountainous) / 2.0;
  mountainous = 0.2 + 0.8 * mountainous;

  // mountains
  dsx = TR_DS_MOUNTAINS * sxnoise_2d(x*TR_DFQ_MOUNTAINS, y*TR_DFQ_MOUNTAINS+20);
  dsy = TR_DS_MOUNTAINS * sxnoise_2d(y*TR_DFQ_MOUNTAINS, x*TR_DFQ_MOUNTAINS+50);
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
  *mountains *= mountainous;

  hilly = sxnoise_2d(
    (x + dsy + 1000) * TR_FREQUENCY_MOUNTAINS * 0.3,
    (y - dsx) * TR_FREQUENCY_MOUNTAINS * 0.3
  );
  hilly = (1 + hilly) / 2.0;
  hilly = 0.3 + 0.7 * hilly;
  ridged = sxnoise_2d(
    (x - dsy + 2000) * TR_FREQUENCY_MOUNTAINS * 0.4,
    (y + dsx) * TR_FREQUENCY_MOUNTAINS * 0.4
  );
  ridged = (1 + ridged) / 2.0;
  ridged = 0.3 + 0.7 * ridged;

  // hills
  dsx = TR_DS_HILLS * sxnoise_2d(x * TR_DFQ_HILLS, y*TR_DFQ_HILLS+2000);
  dsy = TR_DS_HILLS * sxnoise_2d(y * TR_DFQ_HILLS, x*TR_DFQ_HILLS);
  *hills = sxnoise_2d(
    (x + dsx) * TR_FREQUENCY_HILLS,
    (y + dsy) * TR_FREQUENCY_HILLS
  );
  *hills *= hilly;

  mounded = sxnoise_2d(
    (x - dsy + 600) * TR_FREQUENCY_HILLS * 0.6,
    (y + dsx) * TR_FREQUENCY_HILLS * 0.6
  );
  mounded = (1 + mounded) / 2.0;
  mounded = 0.5 + 0.5 * mounded;

  // ridges
  dsx = TR_DS_RIDGES * sxnoise_2d(x * TR_DFQ_RIDGES, y*TR_DFQ_RIDGES);
  dsy = TR_DS_RIDGES * sxnoise_2d(y * TR_DFQ_RIDGES, x*TR_DFQ_RIDGES+2000);
  *ridges = sxnoise_2d(
    (x + dsx) * TR_FREQUENCY_RIDGES,
    (y + dsy) * TR_FREQUENCY_RIDGES
  );
  *ridges += 0.5 * wrnoise_2d(
    (x + dsy) * TR_FREQUENCY_RIDGES,
    (y - dsx) * TR_FREQUENCY_RIDGES
  );
  *ridges /= 1.5;
  *ridges *= ridged;

  detailed = sxnoise_2d(
    (x - dsy + 30000) * TR_FREQUENCY_RIDGES * 0.5,
    (y + dsx) * TR_FREQUENCY_RIDGES * 0.5
  );
  detailed = (1 + detailed) / 2.0;
  detailed = 0.2 + 0.8 * detailed;

  // mounds
  dsx = TR_DS_MOUNDS * sxnoise_2d(x * TR_DFQ_MOUNDS, y*TR_DFQ_MOUNDS+9000);
  dsy = TR_DS_MOUNDS * sxnoise_2d(y * TR_DFQ_MOUNDS, x*TR_DFQ_MOUNDS);
  *mounds = sxnoise_2d(
    (x + dsx) * TR_FREQUENCY_MOUNDS,
    (y + dsy) * TR_FREQUENCY_MOUNDS
  );
  *mounds *= mounded;

  bumpy = sxnoise_2d(
    (x - dsy + 10000) * TR_FREQUENCY_MOUNDS * 0.2,
    (y + dsx) * TR_FREQUENCY_MOUNDS * 0.2
  );
  bumpy = (1 + bumpy) / 2.0;
  bumpy = 0.4 + 0.6 * bumpy;

  // details
  dsx = TR_DS_DETAILS * sxnoise_2d(x * TR_DFQ_DETAILS, y*TR_DFQ_DETAILS);
  dsy = TR_DS_DETAILS * sxnoise_2d(y * TR_DFQ_DETAILS, x*TR_DFQ_DETAILS+4500);
  *details = sxnoise_2d(
    (x + dsx) * TR_FREQUENCY_DETAILS,
    (y + dsy) * TR_FREQUENCY_DETAILS
  );
  *details *= detailed;

  // bumps
  dsx = TR_DS_BUMPS * sxnoise_2d(x * TR_DFQ_BUMPS, y*TR_DFQ_BUMPS+8400);
  dsy = TR_DS_BUMPS * sxnoise_2d(y * TR_DFQ_BUMPS, x*TR_DFQ_BUMPS);
  *bumps = sxnoise_2d(
    (x + dsx) * TR_FREQUENCY_BUMPS,
    (y + dsy) * TR_FREQUENCY_BUMPS
  );
  *bumps *= bumpy;
}

// Remaps the given value (on [0, 1]) to account for the shape and height of
// the overall height profile.
static inline float geomap(float base, terrain_region* region, float* interp) {
  if (base < TR_GEOMAP_SHELF) { // deep ocean
    *region = TR_REGION_DEPTHS;
    *interp = base / TR_GEOMAP_SHELF;
    *interp = smooth(*interp, 2, 0.1);
    return (
      TR_HEIGHT_OCEAN_DEPTHS +
      *interp * (TR_HEIGHT_CONTINENTAL_SHELF - TR_HEIGHT_OCEAN_DEPTHS)
    );

  } else if (base < TR_GEOMAP_OCEAN) { // continental shelf
    *region = TR_REGION_SHELF;
    *interp = (base - TR_GEOMAP_SHELF) / (TR_GEOMAP_OCEAN - TR_GEOMAP_SHELF);
    *interp = smooth(*interp, 1.6, 0.2);
    // TODO: Sea cliffs?
    return (
      TR_HEIGHT_CONTINENTAL_SHELF +
      *interp * (TR_HEIGHT_SEA_LEVEL - TR_HEIGHT_CONTINENTAL_SHELF)
    );

  } else if (base < TR_GEOMAP_PLAINS) { // coastal plain
    *region = TR_REGION_PLAINS;
    *interp = (base - TR_GEOMAP_OCEAN) / (TR_GEOMAP_PLAINS - TR_GEOMAP_OCEAN);
    *interp = smooth(*interp, 1.2, 0.5);
    return (
      TR_HEIGHT_SEA_LEVEL +
      *interp * (TR_HEIGHT_COASTAL_PLAINS - TR_HEIGHT_SEA_LEVEL)
    );

  } else if (base < TR_GEOMAP_HILLS) { // hills
    *region = TR_REGION_HILLS;
    *interp = (base - TR_GEOMAP_PLAINS) / (TR_GEOMAP_HILLS - TR_GEOMAP_PLAINS);
    *interp = smooth(*interp, 1.4, 0.35);
    return (
      TR_HEIGHT_COASTAL_PLAINS +
      *interp * (TR_HEIGHT_HIGHLANDS - TR_HEIGHT_COASTAL_PLAINS)
    );

  } else if (base < TR_GEOMAP_MOUNTAINS) { // highlands
    *region = TR_REGION_HIGHLANDS;
    *interp = (base - TR_GEOMAP_HILLS)/(TR_GEOMAP_MOUNTAINS - TR_GEOMAP_HILLS);
    *interp = 0.2 * sin(*interp*4*M_PI) + 0.8 * smooth(*interp, 1.3, 0.5);
    return (
      TR_HEIGHT_HIGHLANDS +
      *interp * (TR_HEIGHT_MOUNTAIN_BASES - TR_HEIGHT_HIGHLANDS)
    );

  } else { // mountains
    *region = TR_REGION_MOUNTAINS;
    *interp = (base - TR_GEOMAP_MOUNTAINS) / (1 - TR_GEOMAP_MOUNTAINS);
    *interp = pow(*interp, 2.4);
    return (
      TR_HEIGHT_MOUNTAIN_BASES +
      *interp * (TR_HEIGHT_MOUNTAIN_TOPS - TR_HEIGHT_MOUNTAIN_BASES)
    );

  }
}

// Computes basic geoform information to be used by terrain_height.
static inline void comptue_base_geoforms(
  region_pos* pos,
  float* continents, float* geoforms, float* mountains, float* hills,
  float* ridges, float* mounds, float* details, float* bumps,
  float* base,
  terrain_region* region,
  float* tr_interp,
  float* height
) {
  // make some noise:
  get_noise(
    pos->x, pos->y,
    continents, geoforms, mountains, hills,
    ridges, mounds, details, bumps
  );

  *base = (3* (*continents) + 1.5*(*geoforms) + (*mountains)) / 5.5;
  *base = (1 + (*base))/2.0; // squash into [0, 1]
  *base = smooth(*base, 0.5, 0.5); // pull things in
  // remap everything:
  *height = geomap(*base, region, tr_interp);
  //*height = geomap(*base, region, tr_interp);
  //*height = *base * TR_HEIGHT_MOUNTAIN_TOPS;
}

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

// Computes the terrain region and interpolation values at the given position.
void geoform_info(region_pos *pos, terrain_region* region, float* tr_interp);
#endif // ifndef TERRAIN_H
