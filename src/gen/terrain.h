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
#define    TR_FREQUENCY_PGEOFORMS 0.000003 // ???
#define    TR_FREQUENCY_SGEOFORMS 0.000004 // 7812.5-chunk ~= 170 km features
#define    TR_FREQUENCY_GEODETAIL 0.00002 // 1562.5-chunk ~= 33 km features
#define    TR_FREQUENCY_MOUNTAINS 0.0001 // 312.5-chunk ~= 6.7 km features
#define        TR_FREQUENCY_HILLS 0.0005 // 62.5-chunk ~= 1.3 km features
#define       TR_FREQUENCY_RIDGES 0.003 // ~10-chunk ~= 220 m features
#define       TR_FREQUENCY_MOUNDS 0.01 // ~6-chunk ~= 64 m features
#define      TR_FREQUENCY_DETAILS 0.05 // ~20-cell ~= 13 m features
#define        TR_FREQUENCY_BUMPS 0.25 // ~4-cell ~= 2.5 m variance

// Distortion frequencies:
#define   TR_DFQ_CONTINENTS 0.0000004 // 1/5 of base
#define    TR_DFQ_PGEOFORMS 0.0000006 // 1/5 of base
#define    TR_DFQ_SGEOFORMS 0.0000008 // 1/5 of base
#define    TR_DFQ_GEODETAIL 0.000004 // 1/5 of base
#define    TR_DFQ_MOUNTAINS 0.00002 // 1/5 of base
#define        TR_DFQ_HILLS 0.0002 // 2/5 of base
#define       TR_DFQ_RIDGES 0.0006 // 1/5 of base
#define       TR_DFQ_MOUNDS 0.004 // 2/5 of base
#define      TR_DFQ_DETAILS 0.02 // 2/5 of base
#define        TR_DFQ_BUMPS 0.05 // 1/5 of base

// Distortion strengths:
#define   TR_DS_CONTINENTS 1500000.0 // ~1.5 periods
#define    TR_DS_PGEOFORMS 1200000.0 // ???
#define    TR_DS_SGEOFORMS 400000.0 // 1.6 periods
#define    TR_DS_GEODETAIL 37500.0 // ~3/4 period
#define    TR_DS_MOUNTAINS 5000.0 // ~1/2 period
#define        TR_DS_HILLS 2000.0 // ~1 period
#define       TR_DS_RIDGES 50.0 // ~1/4 period
#define       TR_DS_MOUNDS 30.0 // ~1/2 period
#define      TR_DS_DETAILS 10.0 // ~1/2 period
#define        TR_DS_BUMPS 2.0 // ~1/2 period


// Geoform parameters:

// Noise->geoform mapping (see compute_geoforms):
#define       TR_GEOMAP_SHELF 0.38
#define       TR_GEOMAP_SHORE 0.55
#define      TR_GEOMAP_PLAINS 0.6
#define       TR_GEOMAP_HILLS 0.68
#define   TR_GEOMAP_MOUNTAINS 0.7

// Geoform heights:
#define        TR_HEIGHT_OCEAN_DEPTHS 1500
#define   TR_HEIGHT_CONTINENTAL_SHELF 14750
#define           TR_HEIGHT_SEA_LEVEL 15000
#define      TR_HEIGHT_COASTAL_PLAINS 15300
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

// Max height:
#define TR_MAX_HEIGHT (\
  TR_HEIGHT_MOUNTAIN_TOPS +\
  TR_SCALE_HILLS +\
  TR_SCALE_RIDGES +\
  TR_SCALE_MOUNDS +\
  TR_SCALE_DETAILS +\
  TR_SCALE_BUMPS \
)

// Soil parameters
#define    TR_DIRT_NOISE_SCALE 0.004
#define  TR_DIRT_EROSION_SCALE 0.03

#define TR_DIRT_MOUNTAIN_EROSION 10
#define TR_DIRT_HILL_EROSION 4

#define TR_BASE_SOIL_DEPTH 8

#define TR_ALTITUDE_EROSION_STRENGTH 9

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
  r_pos_t x, r_pos_t y, ptrdiff_t *salt,
  float *continents, float *primary_geoforms, float *secondary_geoforms,
  float *geodetails, float *mountains,
  float *hills, float *ridges, float *mounds, float *details, float *bumps
) {
  float dsx, dsy;
  float scaleinterp;
  float geodetailed, mountainous, hilly, ridged, mounded, detailed, bumpy;

  // continents
  dsx = TR_DS_CONTINENTS * sxnoise_2d(
    x * TR_DFQ_CONTINENTS,
    y * TR_DFQ_CONTINENTS,
    *salt
  );
  *salt = expanded_hash_1d(*salt);
  dsy = TR_DS_CONTINENTS * sxnoise_2d(
    x * TR_DFQ_CONTINENTS,
    y * TR_DFQ_CONTINENTS,
    *salt
  );
  *salt = expanded_hash_1d(*salt);
  scaleinterp = (
    0.7 +
    0.4 * sxnoise_2d(
      x * TR_DFQ_CONTINENTS,
      y * TR_DFQ_CONTINENTS,
      *salt
    )
  );
  *salt = expanded_hash_1d(*salt);
  *continents = scaleinterp * (
    cosf((x + dsx) * TR_FREQUENCY_CONTINENTS) *
    sinf((y + dsy) * TR_FREQUENCY_CONTINENTS)
  );
  *continents += (1 - scaleinterp) * (
    cosf((x + 0.8 * dsy) * TR_FREQUENCY_CONTINENTS * 1.6) *
    sinf((y - 0.8 * dsx) * TR_FREQUENCY_CONTINENTS * 1.6)
  );
  *continents = (1 + *continents) / 2.0;
  *continents = smooth(*continents, 3, 0.5);
  *continents = (2 * (*continents)) - 1;

  geodetailed = sxnoise_2d(
    (x - dsy) * TR_FREQUENCY_CONTINENTS * 1.7,
    (y - dsx) * TR_FREQUENCY_CONTINENTS * 1.7,
    *salt
  );
  *salt = expanded_hash_1d(*salt);
  geodetailed = (1 + geodetailed) / 2.0;

  // primary geoforms (mountain bones)
  dsx = TR_DS_PGEOFORMS*sxnoise_2d(
    x*TR_DFQ_PGEOFORMS,
    y*TR_DFQ_PGEOFORMS,
    *salt
  );
  *salt = expanded_hash_1d(*salt);
  dsy = TR_DS_PGEOFORMS*sxnoise_2d(
    y*TR_DFQ_PGEOFORMS,
    x*TR_DFQ_PGEOFORMS,
    *salt
  );
  *salt = expanded_hash_1d(*salt);
  *primary_geoforms = sxnoise_2d(
    (x + dsx) * TR_FREQUENCY_PGEOFORMS * 0.7,
    (y + dsy) * TR_FREQUENCY_PGEOFORMS * 0.7,
    *salt
  );
  *salt = expanded_hash_1d(*salt);
  *primary_geoforms += wrnoise_2d_fancy(
    (x - dsy) * TR_FREQUENCY_PGEOFORMS * 0.8,
    (y + dsx) * TR_FREQUENCY_PGEOFORMS * 0.8,
    *salt,
    0, 0,
    WORLEY_FLAG_INCLUDE_NEXTBEST | WORLEY_FLAG_SMOOTH_SIDES
  );
  *salt = expanded_hash_1d(*salt);
  *primary_geoforms /= 2.0;
  *primary_geoforms = (1 + *primary_geoforms) / 2.0;
  *primary_geoforms = smooth(*primary_geoforms, 2, 0.5);
  *primary_geoforms *= *primary_geoforms;
  *primary_geoforms = (2.0 * *primary_geoforms) - 1;

  mountainous = fmax(0, *primary_geoforms);
  mountainous = smooth(mountainous, 2, 0.7);
  mountainous += 0.2 * sxnoise_2d(
    (x + dsy) * TR_FREQUENCY_SGEOFORMS * 0.7,
    (y - dsx) * TR_FREQUENCY_SGEOFORMS * 0.7,
    *salt
  );
  *salt = expanded_hash_1d(*salt);
  mountainous /= 1.2;
  mountainous = fmax(0, mountainous);
  //mountainous = (1 + mountainous) / 2.0;
  //mountainous = smooth(mountainous, 1.5, 0.6);
  //mountainous = 0;

  hilly = fmax(0, *primary_geoforms);
  hilly = pow(hilly, 0.7);
  hilly += sxnoise_2d(
    (x + dsx*0.6) * TR_FREQUENCY_SGEOFORMS * 0.8,
    (y - dsy*0.6) * TR_FREQUENCY_SGEOFORMS * 0.8,
    *salt
  );
  hilly += 0.5 * sxnoise_2d(
    (x - dsy*0.4) * TR_FREQUENCY_GEODETAIL * 0.3,
    (y + dsx*0.4) * TR_FREQUENCY_GEODETAIL * 0.3,
    *salt
  );
  hilly /= 2.5;
  hilly = fmax(0, hilly);
  *salt = expanded_hash_1d(*salt);
  //hilly = (1 + hilly) / 2.0;
  //hilly = pow(hilly, 1.5);

  ridged = fmax(0, *primary_geoforms + 0.05);
  ridged = pow(ridged, 0.6);
  ridged += sxnoise_2d(
    (x + dsx*0.4) * TR_FREQUENCY_SGEOFORMS * 0.9,
    (y - dsy*0.4) * TR_FREQUENCY_SGEOFORMS * 0.9,
    *salt
  );
  ridged += 0.5 * sxnoise_2d(
    (x - dsy) * TR_FREQUENCY_GEODETAIL * 0.4,
    (y + dsx) * TR_FREQUENCY_GEODETAIL * 0.4,
    *salt
  );
  ridged /= 2.5;
  *salt = expanded_hash_1d(*salt);
  ridged = 0.1 + 0.9 * ridged;

  // secondary geoforms (not used for roughness vaules)
  dsx = TR_DS_SGEOFORMS*sxnoise_2d(
    x*TR_DFQ_SGEOFORMS,
    y*TR_DFQ_SGEOFORMS,
    *salt
  );
  *salt = expanded_hash_1d(*salt);
  dsy = TR_DS_SGEOFORMS*sxnoise_2d(
    y*TR_DFQ_SGEOFORMS,
    x*TR_DFQ_SGEOFORMS,
    *salt
  );
  *salt = expanded_hash_1d(*salt);
  *secondary_geoforms = sxnoise_2d(
    (x + dsx) * TR_FREQUENCY_SGEOFORMS,
    (y + dsy) * TR_FREQUENCY_SGEOFORMS,
    *salt
  );
  *salt = expanded_hash_1d(*salt);
  *secondary_geoforms += wrnoise_2d_fancy(
    (x - dsy) * TR_FREQUENCY_SGEOFORMS,
    (y + dsx) * TR_FREQUENCY_SGEOFORMS,
    *salt,
    0, 0,
    WORLEY_FLAG_INCLUDE_NEXTBEST | WORLEY_FLAG_SMOOTH_SIDES
  );
  *salt = expanded_hash_1d(*salt);
  *secondary_geoforms /= 2.0;

  // geodetails
  dsx = TR_DS_GEODETAIL * sxnoise_2d(
    x * TR_DFQ_GEODETAIL,
    y * TR_DFQ_GEODETAIL,
    *salt
  );
  *salt = expanded_hash_1d(*salt);
  dsy = TR_DS_GEODETAIL * sxnoise_2d(
    y * TR_DFQ_GEODETAIL,
    x * TR_DFQ_GEODETAIL,
    *salt
  );
  *salt = expanded_hash_1d(*salt);

  *geodetails = 2.0 * sxnoise_2d(
    (x + dsx) * TR_FREQUENCY_GEODETAIL,
    (y + dsy) * TR_FREQUENCY_GEODETAIL,
    *salt
  );
  *salt = expanded_hash_1d(*salt);
  *geodetails += wrnoise_2d_fancy(
    (x + dsy) * TR_FREQUENCY_GEODETAIL,
    (y - dsx) * TR_FREQUENCY_GEODETAIL,
    *salt,
    0, 0,
    WORLEY_FLAG_INCLUDE_NEXTBEST | WORLEY_FLAG_SMOOTH_SIDES
  );
  *salt = expanded_hash_1d(*salt);
  *geodetails /= 3.0;
  *geodetails *= geodetailed;

  // mountains
  dsx = TR_DS_MOUNTAINS * sxnoise_2d(
    x * TR_DFQ_MOUNTAINS,
    y * TR_DFQ_MOUNTAINS,
    *salt
  );
  *salt = expanded_hash_1d(*salt);
  dsy = TR_DS_MOUNTAINS * sxnoise_2d(
    y * TR_DFQ_MOUNTAINS,
    x * TR_DFQ_MOUNTAINS,
    *salt
  );
  *salt = expanded_hash_1d(*salt);
  *mountains = sxnoise_2d(
    x * TR_FREQUENCY_MOUNTAINS,
    y * TR_FREQUENCY_MOUNTAINS,
    *salt
  );
  *salt = expanded_hash_1d(*salt);
  *mountains = (1 + *mountains) / 2.0;
  *mountains += 2*smooth(
    wrnoise_2d_fancy(
      (x + dsy) * TR_FREQUENCY_MOUNTAINS,
      (y + dsx) * TR_FREQUENCY_MOUNTAINS,
      *salt,
      0, 0,
      WORLEY_FLAG_INCLUDE_NEXTBEST | WORLEY_FLAG_SMOOTH_SIDES
    ),
    2,
    0.5
  );
  *salt = expanded_hash_1d(*salt);
  *mountains += wrnoise_2d_fancy(
    (x - dsx) * TR_FREQUENCY_MOUNTAINS * 0.7,
    (y - dsy) * TR_FREQUENCY_MOUNTAINS * 0.7,
    *salt,
    0, 0,
    WORLEY_FLAG_INCLUDE_NEXTBEST | WORLEY_FLAG_SMOOTH_SIDES
  );
  *salt = expanded_hash_1d(*salt);
  *mountains /= 4.0;
  *mountains *= mountainous;

  mounded = sxnoise_2d(
    (x - dsy) * TR_FREQUENCY_MOUNTAINS * 0.6,
    (y + dsx) * TR_FREQUENCY_MOUNTAINS * 0.6,
    *salt
  );
  *salt = expanded_hash_1d(*salt);
  mounded = (1 + mounded) / 2.0;
  mounded = 0.5 + 0.5 * mounded;

  // hills
  dsx = TR_DS_HILLS * sxnoise_2d(x * TR_DFQ_HILLS, y*TR_DFQ_HILLS, *salt);
  *salt = expanded_hash_1d(*salt);
  dsy = TR_DS_HILLS * sxnoise_2d(y * TR_DFQ_HILLS, x*TR_DFQ_HILLS, *salt);
  *salt = expanded_hash_1d(*salt);
  *hills = sxnoise_2d(
    (x + dsx) * TR_FREQUENCY_HILLS,
    (y + dsy) * TR_FREQUENCY_HILLS,
    *salt
  );
  *salt = expanded_hash_1d(*salt);
  *hills *= hilly;

  detailed = sxnoise_2d(
    (x - dsy) * TR_FREQUENCY_HILLS * 0.5,
    (y + dsx) * TR_FREQUENCY_HILLS * 0.5,
    *salt
  );
  *salt = expanded_hash_1d(*salt);
  detailed = (1 + detailed) / 2.0;
  detailed = 0.2 + 0.8 * detailed;

  // ridges
  dsx = TR_DS_RIDGES * sxnoise_2d(x * TR_DFQ_RIDGES, y*TR_DFQ_RIDGES, *salt);
  *salt = expanded_hash_1d(*salt);
  dsy = TR_DS_RIDGES * sxnoise_2d(y * TR_DFQ_RIDGES, x*TR_DFQ_RIDGES, *salt);
  *salt = expanded_hash_1d(*salt);
  *ridges = wrnoise_2d(
    (x + dsx) * TR_FREQUENCY_RIDGES,
    (y + dsy) * TR_FREQUENCY_RIDGES,
    *salt
  );
  *salt = expanded_hash_1d(*salt);
  *ridges += 0.5 * wrnoise_2d(
    (x + dsy) * TR_FREQUENCY_RIDGES * 1.8,
    (y - dsx) * TR_FREQUENCY_RIDGES * 1.8,
    *salt
  );
  *salt = expanded_hash_1d(*salt);
  *ridges /= 1.5;
  *ridges *= ridged;

  bumpy = sxnoise_2d(
    (x - dsy) * TR_FREQUENCY_RIDGES * 0.2,
    (y + dsx) * TR_FREQUENCY_RIDGES * 0.2,
    *salt
  );
  *salt = expanded_hash_1d(*salt);
  bumpy = (1 + bumpy) / 2.0;
  bumpy = 0.4 + 0.6 * bumpy;

  // mounds
  dsx = TR_DS_MOUNDS * sxnoise_2d(x * TR_DFQ_MOUNDS, y*TR_DFQ_MOUNDS, *salt);
  *salt = expanded_hash_1d(*salt);
  dsy = TR_DS_MOUNDS * sxnoise_2d(y * TR_DFQ_MOUNDS, x*TR_DFQ_MOUNDS, *salt);
  *salt = expanded_hash_1d(*salt);
  *mounds = sxnoise_2d(
    (x + dsx) * TR_FREQUENCY_MOUNDS,
    (y + dsy) * TR_FREQUENCY_MOUNDS,
    *salt
  );
  *salt = expanded_hash_1d(*salt);
  *mounds *= mounded;

  // details
  dsx = TR_DS_DETAILS * sxnoise_2d(x*TR_DFQ_DETAILS, y*TR_DFQ_DETAILS, *salt);
  *salt = expanded_hash_1d(*salt);
  dsy = TR_DS_DETAILS * sxnoise_2d(y*TR_DFQ_DETAILS, x*TR_DFQ_DETAILS, *salt);
  *salt = expanded_hash_1d(*salt);
  *details = sxnoise_2d(
    (x + dsx) * TR_FREQUENCY_DETAILS,
    (y + dsy) * TR_FREQUENCY_DETAILS,
    *salt
  );
  *details *= detailed;

  // bumps
  dsx = TR_DS_BUMPS * sxnoise_2d(x*TR_DFQ_BUMPS, y*TR_DFQ_BUMPS, *salt);
  *salt = expanded_hash_1d(*salt);
  dsy = TR_DS_BUMPS * sxnoise_2d(y*TR_DFQ_BUMPS, x*TR_DFQ_BUMPS, *salt);
  *salt = expanded_hash_1d(*salt);
  *bumps = sxnoise_2d(
    (x + dsx) * TR_FREQUENCY_BUMPS,
    (y + dsy) * TR_FREQUENCY_BUMPS,
    *salt
  );
  *salt = expanded_hash_1d(*salt);
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

  } else if (base < TR_GEOMAP_SHORE) { // continental shelf
    *region = TR_REGION_SHELF;
    *interp = (base - TR_GEOMAP_SHELF) / (TR_GEOMAP_SHORE - TR_GEOMAP_SHELF);
    *interp = smooth(*interp, 1.6, 0.2);
    // TODO: Sea cliffs?
    return (
      TR_HEIGHT_CONTINENTAL_SHELF +
      *interp * (TR_HEIGHT_SEA_LEVEL - TR_HEIGHT_CONTINENTAL_SHELF)
    );

  } else if (base < TR_GEOMAP_PLAINS) { // coastal plain
    *region = TR_REGION_PLAINS;
    *interp = (base - TR_GEOMAP_SHORE) / (TR_GEOMAP_PLAINS - TR_GEOMAP_SHORE);
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
    *interp = pow(*interp, 1.6);
    return (
      TR_HEIGHT_MOUNTAIN_BASES +
      *interp * (TR_HEIGHT_MOUNTAIN_TOPS - TR_HEIGHT_MOUNTAIN_BASES)
    );

  }
}

// Computes basic geoform information to be used by compute_terrain_height.
static inline void compute_base_geoforms(
  region_pos* pos, ptrdiff_t *salt,
  float* continents, float* primary_geoforms, float* secondary_geoforms,
  float* geodetails, float* mountains,
  float* hills, float* ridges, float* mounds, float* details, float* bumps,
  float* base,
  terrain_region* region,
  float* tr_interp,
  float* height
) {
  // make some noise:
  get_noise(
    pos->x, pos->y, salt,
    continents, primary_geoforms, secondary_geoforms,
    geodetails, mountains,
    hills, ridges, mounds, details, bumps
  );
  *salt = expanded_hash_1d(*salt);

  //*
  *base = (
    2.5*(*continents) +
    2*(*primary_geoforms) +
    1.5*(*secondary_geoforms) +
    1*(*geodetails)
  ) / 7;
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

// Computes the terrain height at the given region position in blocks, and
// writes out the rock and dirt heights at that location.
void compute_terrain_height(
  region_pos *pos,
  float *r_rocks,
  float *r_dirt
);

// Alters the various detail values according to the terrain region
// classification.
void alter_terrain_values(
  region_pos *pos, ptrdiff_t *salt,
  terrain_region region, float tr_interp,
  float *mountains, float *hills, float *ridges,
  float *mounds, float *details, float *bumps
);

// Figures out the dirt height at the given location and writes it into the
// result parameter.
void compute_dirt_height(
  region_pos *pos, ptrdiff_t *salt,
  float rocks_height,
  float mountains, float hills, float bumps,
  float *result
);

// Computes the terrain region and interpolation values at the given position.
void geoform_info(region_pos *pos, terrain_region* region, float* tr_interp);
#endif // ifndef TERRAIN_H
