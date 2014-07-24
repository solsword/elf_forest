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
#define   TR_FREQUENCY_CONTINENTS 0.000002 // 15625-chunk ~= 330 km features
#define     TR_FREQUENCY_GEOFORMS 0.000004 // 7812.5-chunk ~= 170 km features
#define    TR_FREQUENCY_GEODETAIL 0.00002 // 1562.5-chunk ~= 33 km features
#define    TR_FREQUENCY_MOUNTAINS 0.0001 // 312.5-chunk ~= 6.7 km features
#define        TR_FREQUENCY_HILLS 0.0005 // 62.5-chunk ~= 1.3 km features
#define       TR_FREQUENCY_RIDGES 0.003 // ~10-chunk ~= 220 m features
#define       TR_FREQUENCY_MOUNDS 0.01 // ~6-chunk ~= 64 m features
#define      TR_FREQUENCY_DETAILS 0.05 // ~20-cell ~= 13 m features
#define        TR_FREQUENCY_BUMPS 0.25 // ~4-cell ~= 2.5 m variance

// Distortion frequencies:
#define   TR_DFQ_CONTINENTS 0.0000004 // 1/5 of base
#define     TR_DFQ_GEOFORMS 0.0000008 // 1/5 of base
#define    TR_DFQ_GEODETAIL 0.000004 // 1/5 of base
#define    TR_DFQ_MOUNTAINS 0.00002 // 1/5 of base
#define        TR_DFQ_HILLS 0.0002 // 2/5 of base
#define       TR_DFQ_RIDGES 0.0006 // 1/5 of base
#define       TR_DFQ_MOUNDS 0.004 // 2/5 of base
#define      TR_DFQ_DETAILS 0.02 // 2/5 of base
#define        TR_DFQ_BUMPS 0.05 // 1/5 of base

// Distortion strengths:
#define   TR_DS_CONTINENTS 1500000.0 // ~1.5 periods
#define     TR_DS_GEOFORMS 187500.0 // ~3/4 period
#define    TR_DS_GEODETAIL 37500.0 // ~3/4 period
#define    TR_DS_MOUNTAINS 5000.0 // ~1/2 period
#define        TR_DS_HILLS 2000.0 // ~1 period
#define       TR_DS_RIDGES 50.0 // ~1/4 period
#define       TR_DS_MOUNDS 30.0 // ~1/2 period
#define      TR_DS_DETAILS 10.0 // ~1/2 period
#define        TR_DS_BUMPS 2.0 // ~1/2 period


// Geoform parameters:

// Noise->geoform mapping (see compute_geoforms):
#define       TR_GEOMAP_SHELF 0.32
#define       TR_GEOMAP_OCEAN 0.45
#define      TR_GEOMAP_PLAINS 0.6
#define       TR_GEOMAP_HILLS 0.7
#define   TR_GEOMAP_MOUNTAINS 0.8

// Geoform heights:
#define        TR_HEIGHT_OCEAN_DEPTHS 1500
#define   TR_HEIGHT_CONTINENTAL_SHELF 14750
#define           TR_HEIGHT_SEA_LEVEL 15000
#define      TR_HEIGHT_COASTAL_PLAINS 15150
#define           TR_HEIGHT_HIGHLANDS 16500
#define      TR_HEIGHT_MOUNTAIN_BASES 18500
#define       TR_HEIGHT_MOUNTAIN_TOPS 27000

// Terrain noise contributions:
#define   TR_SCALE_MOUNTAINS 6000
#define       TR_SCALE_HILLS 900
#define      TR_SCALE_RIDGES 60
#define      TR_SCALE_MOUNDS 15
#define     TR_SCALE_DETAILS 8
#define       TR_SCALE_BUMPS 2

#define TR_MAX_HEIGHT (\
  TR_HEIGHT_MOUNTAIN_TOPS +\
  TR_SCALE_HILLS +\
  TR_SCALE_RIDGES +\
  TR_SCALE_MOUNDS +\
  TR_SCALE_DETAILS +\
  TR_SCALE_BUMPS \
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

// A base salt for terrain noise:
extern float TR_NOISE_SALT;

// Amplification factor for terrain height:
extern float TR_TERRAIN_HEIGHT_AMP;

// An array of names for each terrain region:
extern char const * const TR_REGION_NAMES[];

/********************
 * Inline Functions *
 ********************/

static inline void get_noise(
  r_pos_t x, r_pos_t y, ptrdiff_t salt,
  float *continents, float *geoforms, float *geodetails, float *mountains,
  float *hills, float *ridges, float *mounds, float *details, float *bumps
) {
  float dsx, dsy, frequency;
  float scaleinterp;
  float geodetailed, mountainous, hilly, ridged, mounded, detailed, bumpy;

  // continents
  dsx = TR_DS_CONTINENTS * sxnoise_2d(
    x * TR_DFQ_CONTINENTS,
    y * TR_DFQ_CONTINENTS,
    salt
  );
  salt = expanded_hash_1d(salt);
  dsy = TR_DS_CONTINENTS * sxnoise_2d(
    x * TR_DFQ_CONTINENTS,
    y * TR_DFQ_CONTINENTS,
    salt
  );
  salt = expanded_hash_1d(salt);
  scaleinterp = (
    0.7 +
    0.4 * sxnoise_2d(
      x * TR_DFQ_CONTINENTS,
      y * TR_DFQ_CONTINENTS,
      salt
    )
  );
  salt = expanded_hash_1d(salt);
  frequency = TR_FREQUENCY_CONTINENTS;
  *continents = scaleinterp * (
    cosf((x + dsx) * frequency) *
    sinf((y + dsy) * frequency)
  );
  *continents += (1 - scaleinterp) * (
    cosf((x + 0.8 * dsy) * frequency * 1.6) *
    sinf((y - 0.8 * dsx) * frequency * 1.6)
  );
  *continents = (1 + *continents) / 2.0;
  *continents = smooth(*continents, 3, 0.5);
  *continents = (2 * (*continents)) - 1;

  geodetailed = sxnoise_2d(
    (x - dsy) * frequency * 1.7,
    (y - dsx) * frequency * 1.7,
    salt
  );
  salt = expanded_hash_1d(salt);
  geodetailed = (1 + geodetailed) / 2.0;
  //geodetailed = 0.2 + 0.8 * geodetailed;

  // geoforms
  dsx = TR_DS_GEOFORMS*sxnoise_2d(x*TR_DFQ_GEOFORMS, y*TR_DFQ_GEOFORMS, salt);
  salt = expanded_hash_1d(salt);
  dsy = TR_DS_GEOFORMS*sxnoise_2d(y*TR_DFQ_GEOFORMS, x*TR_DFQ_GEOFORMS, salt);
  salt = expanded_hash_1d(salt);
  frequency = TR_FREQUENCY_GEOFORMS * (
    1 +
    0.3 * sxnoise_2d(
      x * TR_DFQ_GEOFORMS,
      y * TR_DFQ_GEOFORMS,
      salt
    )
  );
  salt = expanded_hash_1d(salt);
  *geoforms = sxnoise_2d(
    (x + dsx) * frequency,
    (y + dsy) * frequency,
    salt
  );
  salt = expanded_hash_1d(salt);
  *geoforms += wrnoise_2d_fancy(
    (x - dsy) * frequency,
    (y + dsx) * frequency,
    salt,
    0, 0,
    WORLEY_FLAG_INCLUDE_NEXTBEST | WORLEY_FLAG_SMOOTH_SIDES
  );
  salt = expanded_hash_1d(salt);
  *geoforms /= 2.0;

  mountainous = sxnoise_2d(
    (x + dsy) * frequency * 0.7,
    (y - dsx) * frequency * 0.7,
    salt
  );
  salt = expanded_hash_1d(salt);
  mountainous = (1 + mountainous) / 2.0;
  mountainous *= mountainous; 
  mountainous = smooth(mountainous, 1.8, 0.5);

  // geodetails
  dsx = TR_DS_GEODETAIL * sxnoise_2d(
    x * TR_DFQ_GEODETAIL,
    y * TR_DFQ_GEODETAIL,
    salt
  );
  salt = expanded_hash_1d(salt);
  dsy = TR_DS_GEODETAIL * sxnoise_2d(
    y * TR_DFQ_GEODETAIL,
    x * TR_DFQ_GEODETAIL,
    salt
  );
  salt = expanded_hash_1d(salt);
  frequency = TR_FREQUENCY_GEODETAIL;

  *geodetails = sxnoise_2d(
    (x + dsx) * frequency,
    (y + dsy) * frequency,
    salt
  );
  salt = expanded_hash_1d(salt);
  *geodetails += wrnoise_2d_fancy(
    (x + dsy) * frequency,
    (y - dsx) * frequency,
    salt,
    0, 0,
    WORLEY_FLAG_INCLUDE_NEXTBEST | WORLEY_FLAG_SMOOTH_SIDES
  );
  salt = expanded_hash_1d(salt);
  *geodetails /= 2.0;
  *geodetails *= geodetailed;

  hilly = sxnoise_2d(
    (x + dsy) * frequency * 0.3,
    (y - dsx) * frequency * 0.3,
    salt
  );
  salt = expanded_hash_1d(salt);
  hilly = (1 + hilly) / 2.0;
  hilly = pow(hilly, 1.5);
  hilly = 0.1 + 0.9 * hilly;

  ridged = sxnoise_2d(
    (x - dsy) * frequency * 0.4,
    (y + dsx) * frequency * 0.4,
    salt
  );
  salt = expanded_hash_1d(salt);
  ridged = (1 + ridged) / 2.0;
  ridged = 0.3 + 0.7 * ridged;

  // mountains
  dsx = TR_DS_MOUNTAINS * sxnoise_2d(
    x * TR_DFQ_MOUNTAINS,
    y * TR_DFQ_MOUNTAINS,
    salt
  );
  salt = expanded_hash_1d(salt);
  dsy = TR_DS_MOUNTAINS * sxnoise_2d(
    y * TR_DFQ_MOUNTAINS,
    x * TR_DFQ_MOUNTAINS,
    salt
  );
  salt = expanded_hash_1d(salt);
  frequency = TR_FREQUENCY_MOUNTAINS;
  *mountains = sxnoise_2d(
    x * frequency,
    y * frequency,
    salt
  );
  salt = expanded_hash_1d(salt);
  *mountains += 2*smooth(
    wrnoise_2d_fancy(
      (x + dsy) * frequency,
      (y + dsx) * frequency,
      salt,
      0, 0,
      WORLEY_FLAG_INCLUDE_NEXTBEST | WORLEY_FLAG_SMOOTH_SIDES
    ),
    2,
    0.5
  );
  salt = expanded_hash_1d(salt);
  *mountains += wrnoise_2d_fancy(
    (x - dsx) * frequency * 0.7,
    (y - dsy) * frequency * 0.7,
    salt,
    0, 0,
    WORLEY_FLAG_INCLUDE_NEXTBEST | WORLEY_FLAG_SMOOTH_SIDES
  );
  salt = expanded_hash_1d(salt);
  *mountains /= 4.0;
  *mountains *= mountainous;

  mounded = sxnoise_2d(
    (x - dsy) * frequency * 0.6,
    (y + dsx) * frequency * 0.6,
    salt
  );
  salt = expanded_hash_1d(salt);
  mounded = (1 + mounded) / 2.0;
  mounded = 0.5 + 0.5 * mounded;

  // hills
  dsx = TR_DS_HILLS * sxnoise_2d(x * TR_DFQ_HILLS, y*TR_DFQ_HILLS, salt);
  salt = expanded_hash_1d(salt);
  dsy = TR_DS_HILLS * sxnoise_2d(y * TR_DFQ_HILLS, x*TR_DFQ_HILLS, salt);
  salt = expanded_hash_1d(salt);
  frequency = TR_FREQUENCY_HILLS;
  *hills = sxnoise_2d(
    (x + dsx) * frequency,
    (y + dsy) * frequency,
    salt
  );
  salt = expanded_hash_1d(salt);
  *hills *= hilly;

  detailed = sxnoise_2d(
    (x - dsy) * frequency * 0.5,
    (y + dsx) * frequency * 0.5,
    salt
  );
  salt = expanded_hash_1d(salt);
  detailed = (1 + detailed) / 2.0;
  detailed = 0.2 + 0.8 * detailed;

  // ridges
  dsx = TR_DS_RIDGES * sxnoise_2d(x * TR_DFQ_RIDGES, y*TR_DFQ_RIDGES, salt);
  salt = expanded_hash_1d(salt);
  dsy = TR_DS_RIDGES * sxnoise_2d(y * TR_DFQ_RIDGES, x*TR_DFQ_RIDGES, salt);
  salt = expanded_hash_1d(salt);
  frequency = TR_FREQUENCY_RIDGES;
  *ridges = wrnoise_2d(
    (x + dsx) * frequency,
    (y + dsy) * frequency,
    salt
  );
  salt = expanded_hash_1d(salt);
  *ridges += 0.5 * wrnoise_2d(
    (x + dsy) * frequency * 1.8,
    (y - dsx) * frequency * 1.8,
    salt
  );
  salt = expanded_hash_1d(salt);
  *ridges /= 1.5;
  *ridges *= ridged;

  bumpy = sxnoise_2d(
    (x - dsy) * frequency * 0.2,
    (y + dsx) * frequency * 0.2,
    salt
  );
  salt = expanded_hash_1d(salt);
  bumpy = (1 + bumpy) / 2.0;
  bumpy = 0.4 + 0.6 * bumpy;

  // mounds
  dsx = TR_DS_MOUNDS * sxnoise_2d(x * TR_DFQ_MOUNDS, y*TR_DFQ_MOUNDS, salt);
  salt = expanded_hash_1d(salt);
  dsy = TR_DS_MOUNDS * sxnoise_2d(y * TR_DFQ_MOUNDS, x*TR_DFQ_MOUNDS, salt);
  salt = expanded_hash_1d(salt);
  frequency = TR_FREQUENCY_MOUNDS;
  *mounds = sxnoise_2d(
    (x + dsx) * frequency,
    (y + dsy) * frequency,
    salt
  );
  salt = expanded_hash_1d(salt);
  *mounds *= mounded;

  // details
  dsx = TR_DS_DETAILS * sxnoise_2d(x*TR_DFQ_DETAILS, y*TR_DFQ_DETAILS, salt);
  salt = expanded_hash_1d(salt);
  dsy = TR_DS_DETAILS * sxnoise_2d(y*TR_DFQ_DETAILS, x*TR_DFQ_DETAILS, salt);
  salt = expanded_hash_1d(salt);
  frequency = TR_FREQUENCY_DETAILS;
  *details = sxnoise_2d(
    (x + dsx) * frequency,
    (y + dsy) * frequency,
    salt
  );
  *details *= detailed;

  // bumps
  dsx = TR_DS_BUMPS * sxnoise_2d(x*TR_DFQ_BUMPS, y*TR_DFQ_BUMPS, salt);
  salt = expanded_hash_1d(salt);
  dsy = TR_DS_BUMPS * sxnoise_2d(y*TR_DFQ_BUMPS, x*TR_DFQ_BUMPS, salt);
  salt = expanded_hash_1d(salt);
  frequency = TR_FREQUENCY_BUMPS;
  *bumps = sxnoise_2d(
    (x + dsx) * frequency,
    (y + dsy) * frequency,
    salt
  );
  salt = expanded_hash_1d(salt);
  *bumps *= bumpy;
}

// Remaps the given value (on [0, 1]) to account for the shape and height of
// the overall height profile.
static inline float geomap(float base, terrain_region* region, float* interp) {
  if (base < TR_GEOMAP_SHELF) { // deep ocean
    *region = TR_REGION_DEPTHS;
    *interp = base / TR_GEOMAP_SHELF;
    *interp = smooth(*interp, 0.5, 0.5);
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
    *interp = smooth(*interp, 0.8, 0.5);
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
    *interp = smooth(*interp, 1.3, 0.5);
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
static inline void compute_base_geoforms(
  region_pos* pos, ptrdiff_t salt,
  float* continents, float* geoforms, float* geodetails, float* mountains,
  float* hills, float* ridges, float* mounds, float* details, float* bumps,
  float* base,
  terrain_region* region,
  float* tr_interp,
  float* height
) {
  // make some noise:
  get_noise(
    pos->x, pos->y, salt,
    continents, geoforms, geodetails, mountains,
    hills, ridges, mounds, details, bumps
  );

  //*
  *base = (
    2*(*continents) +
    2*(*geoforms) +
    (*geodetails)
  ) / 5;
  // */
  //*base = *continents;
  *base = (1 + (*base))/2.0; // squash into [0, 1]
  *base = smooth(*base, 2, 0.5); // spread things out
  // remap everything:
  *height = geomap(*base, region, tr_interp);
  // DEBUG:
  //*height = *base * TR_MAX_HEIGHT;
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
