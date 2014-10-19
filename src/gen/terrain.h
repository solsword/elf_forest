#ifndef TERRAIN_H
#define TERRAIN_H

// terrain.h
// Code for determining terrain height.

#include <math.h>

#include "noise/noise.h"
#include "world/world.h"
#include "math/manifold.h"

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
#define        TR_DS_BUMPS 1.5 // ~3/8 period


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
#define     TR_SCALE_DETAILS 6
#define       TR_SCALE_BUMPS 0.7

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
#define    TR_DIRT_NOISE_SCALE 0.0004
#define  TR_DIRT_EROSION_SCALE 0.003

#define TR_DIRT_MOUNTAIN_EROSION 11
#define TR_DIRT_HILL_EROSION 5

#define TR_BASE_SOIL_DEPTH 8

#define TR_ALTITUDE_EROSION_STRENGTH 6

#define TR_SLOPE_EROSION_STRENGTH 12

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

static inline void simplex_component(
  manifold_point *result,
  float x, float y,
  float dx, float dy,
  float frequency,
  ptrdiff_t *salt
) {
  mani_get_sxnoise_2d(
    result,
    x * frequency,
    y * frequency,
    *salt
  ); *salt = expanded_hash_1d(*salt);
  // DEBUG:
  // printf("simp-comp-base: %.3f\n", result->z);
  mani_compose_simple(
    result,
    dx * frequency,
    dy * frequency
  );
}

static inline void worley_component(
  manifold_point *result,
  float x, float y,
  float dx, float dy,
  float frequency,
  ptrdiff_t *salt,
  uint32_t flags
) {
  mani_get_wrnoise_2d(
    result,
    x * frequency,
    y * frequency,
    *salt,
    0, 0,
    flags
  ); *salt = expanded_hash_1d(*salt);
  mani_compose_simple(
    result,
    dx * frequency,
    dy * frequency
  );
}

static inline void get_standard_distortion(
  r_pos_t x, r_pos_t y, ptrdiff_t *salt,
  float frequency,
  float scale,
  manifold_point *result_x,
  manifold_point *result_y
) {
  // x distortion
  simplex_component(
    result_x,
    x, y,
    1, 1,
    frequency,
    salt
  );
  mani_scale_const(result_x, scale);
  // y distortion
  simplex_component(
    result_y,
    x, y,
    1, 1,
    frequency,
    salt
  );
  mani_scale_const(result_y, scale);
}

static inline void get_noise(
  r_pos_t x, r_pos_t y, ptrdiff_t *salt,
  manifold_point *continents,
  manifold_point *primary_geoforms, manifold_point *secondary_geoforms,
  manifold_point *geodetails, manifold_point *mountains,
  manifold_point *hills, manifold_point *ridges,
  manifold_point *mounds, manifold_point *details, manifold_point *bumps
) {
  manifold_point temp; // temporary manifold point for intermediates
  manifold_point dst_x, dst_y; //distortion values
  manifold_point cos_part, sin_part; // cos and sin components of continents

  manifold_point scaleinterp;
  manifold_point geodetailed, mountainous, hilly, ridged;
  manifold_point mounded, detailed, bumpy;

  // continents
  // ----------
  get_standard_distortion(
    x, y, salt,
    TR_DFQ_CONTINENTS,
    TR_DS_CONTINENTS,
    &dst_x, &dst_y
  );
    // interpolant between larger and smaller scales:
  simplex_component(
    &scaleinterp,
    x, y,
    1, 1,
    TR_DFQ_CONTINENTS,
    salt
  );
  // DEBUG:
  // printf("cont-scint-simplex: %.3f\n", scaleinterp.z);
  mani_scale_const(&scaleinterp, 0.3);
  mani_offset_const(&scaleinterp, 0.7);
  // DEBUG:
  // printf("cont-scint: %.3f\n", scaleinterp.z);

    // large-scale cos part:
  cos_part.z = cosf((x + dst_x.z) * TR_FREQUENCY_CONTINENTS);
  cos_part.dx = (
    TR_FREQUENCY_CONTINENTS * (1 + dst_x.dx)
  *
    (-sinf((x + dst_x.z) * TR_FREQUENCY_CONTINENTS))
  );
  cos_part.dy = (
    TR_FREQUENCY_CONTINENTS * dst_x.dy
  *
    (-sinf((x + dst_x.z) * TR_FREQUENCY_CONTINENTS))
  );

    // large-scale sin part:
  sin_part.z = sinf((y + dst_y.z) * TR_FREQUENCY_CONTINENTS);
  sin_part.dx = (
    TR_FREQUENCY_CONTINENTS * dst_y.dx
  *
    cosf((y + dst_y.z) * TR_FREQUENCY_CONTINENTS)
  );
  sin_part.dy = (
    TR_FREQUENCY_CONTINENTS * (1 + dst_y.dy)
  *
    cosf((y + dst_y.z) * TR_FREQUENCY_CONTINENTS)
  );

  // DEBUG:
  // printf("cont-lfcos: %.3f\n", cos_part.z);
  // printf("cont-lfsin: %.3f\n", sin_part.z);

    // large-scale part of continents
  mani_copy(continents, &scaleinterp);
  mani_multiply(continents, &cos_part);
  mani_multiply(continents, &sin_part);

  // DEBUG:
  // printf("cont-base: %.3f\n", continents->z);

    // small-scale cos part:
  cos_part.z = cosf((x + 0.8 * dst_y.z) * TR_FREQUENCY_CONTINENTS * 1.6);
  cos_part.dx = (
    TR_FREQUENCY_CONTINENTS * 1.6 * (1 + 0.8 * dst_y.dx)
  *
    (-sinf((x + 0.8 * dst_y.z) * TR_FREQUENCY_CONTINENTS * 1.6))
  );
  cos_part.dy = (
    TR_FREQUENCY_CONTINENTS * 1.6 * 0.8 * dst_y.dy
  *
    (-sinf((x + 0.8 * dst_y.z) * TR_FREQUENCY_CONTINENTS * 1.6))
  );

    // small-scale sin part:
  sin_part.z = sinf((y - 0.8 * dst_x.z) * TR_FREQUENCY_CONTINENTS * 1.6);
  sin_part.dx = (
    -TR_FREQUENCY_CONTINENTS * 1.6 * 0.8 * dst_x.dx
  *
    cosf((y - 0.8 * dst_x.z) * TR_FREQUENCY_CONTINENTS * 1.6)
  );
  sin_part.dy = (
    TR_FREQUENCY_CONTINENTS * 1.6 * (1 - 0.8 * dst_x.dy)
  *
    cosf((y - 0.8 * dst_x.z) * TR_FREQUENCY_CONTINENTS * 1.6)
  );

  // DEBUG:
  // printf("cont-hfcos: %.3f\n", cos_part.z);
  // printf("cont-hfsin: %.3f\n", sin_part.z);

  // we'll use the small cos part to accumulate the other half of the
  // continents manifold and then add it back in:
  mani_scale_const(&scaleinterp, -1);
  mani_offset_const(&scaleinterp, 1);
  mani_multiply(&cos_part, &sin_part);
  mani_multiply(&cos_part, &scaleinterp);
  // DEBUG:
  // printf("cont-hf-base: %.3f\n", cos_part.z);

  mani_add(continents, &cos_part);

  // DEBUG:
  // printf("cont-full: %.3f\n", continents->z);

  // a bit of smoothing at the end:
  mani_offset_const(continents, 1);
  mani_scale_const(continents, 0.5);

  mani_smooth(continents, 3, 0.5);

  mani_scale_const(continents, 2.0);
  mani_offset_const(continents, -1);
    // continents should be in [-1, 1].

  // DEBUG:
  // printf("cont-smooth: %.3f\n", continents->z);

  // geodetails fading
  // -----------------
  simplex_component(
    &geodetailed,
    x - dst_y.z,
    y - dst_x.z,
    1 - dst_y.dx,
    1 - dst_x.dy,
    TR_FREQUENCY_CONTINENTS * 1.7,
    salt
  );
  mani_offset_const(&geodetailed, 1);
  mani_scale_const(&geodetailed, 0.5);
    // geodetailed should be in [0, 1].

  // primary geoforms (mountain bones)
  // ---------------------------------
  get_standard_distortion(
    x, y, salt,
    TR_DFQ_PGEOFORMS,
    TR_DS_PGEOFORMS,
    &dst_x, &dst_y
  );

    // simplex noise component
  simplex_component(
    primary_geoforms,
    x + dst_x.z,
    y + dst_y.z,
    1 + dst_x.dx,
    1 + dst_y.dy,
    TR_FREQUENCY_PGEOFORMS * 0.7,
    salt
  );
  // TODO: is it really okay to ignore the effects of off-axis distortion?
    // worley noise component
  worley_component(
    &temp,
    x - dst_y.z,
    y + dst_x.z,
    1 - dst_y.dx,
    1 + dst_x.dy,
    TR_FREQUENCY_PGEOFORMS * 0.8,
    salt,
    WORLEY_FLAG_INCLUDE_NEXTBEST | WORLEY_FLAG_SMOOTH_SIDES
  );
  mani_add(primary_geoforms, &temp);

  mani_scale_const(primary_geoforms, 0.5);
  mani_offset_const(primary_geoforms, 1);
  mani_scale_const(primary_geoforms, 0.5);
  mani_smooth(primary_geoforms, 2, 0.5);
  mani_multiply(primary_geoforms, primary_geoforms);
  mani_scale_const(primary_geoforms, 2.0);
  mani_offset_const(primary_geoforms, -1);
    // primary_geoforms should be in [-1, 1].

  // mountains fading
  // ----------------
  if (primary_geoforms->z < 0) {
    mountainous.z = 0;
    mountainous.dx = 0;
    mountainous.dy = 0;
  } else {
    mani_copy(&mountainous, primary_geoforms);
  }
  mani_smooth(&mountainous, 2, 0.7);
  simplex_component(
    &temp,
    x + dst_y.z,
    y - dst_x.z,
    1 + dst_y.dx,
    1 - dst_x.dy,
    TR_FREQUENCY_SGEOFORMS * 0.7,
    salt
  );
  mani_scale_const(&temp, 0.2);
  mani_add(&mountainous, &temp);

  mani_scale_const(&mountainous, 1.0/1.2);
  if (mountainous.z < 0) {
    mountainous.z = 0;
    mountainous.dx = 0;
    mountainous.dy = 0;
  }
    // mountainous should be in [0, 1].

  // hills fading
  // ------------
  if (primary_geoforms->z < 0) {
    hilly.z = 0;
    hilly.dx = 0;
    hilly.dy = 0;
  } else {
    mani_copy(&hilly, primary_geoforms);
  }
  mani_pow_const(&hilly, 0.7);
  simplex_component(
    &temp,
    x + dst_x.z * 0.6,
    y - dst_y.z * 0.6,
    1 + dst_x.dx * 0.6,
    1 - dst_y.dy * 0.6,
    TR_FREQUENCY_SGEOFORMS * 0.8,
    salt
  );
  mani_add(&hilly, &temp);
  simplex_component(
    &temp,
    x - dst_y.z * 0.4,
    y + dst_x.z * 0.4,
    1 - dst_y.dx * 0.4,
    1 + dst_x.dy * 0.4,
    TR_FREQUENCY_GEODETAIL * 0.3,
    salt
  );
  mani_scale_const(&temp, 0.5);
  mani_add(&hilly, &temp);
  mani_scale_const(&hilly, 1.0/2.5);
  if (hilly.z < 0) {
    hilly.z = 0;
    hilly.dx = 0;
    hilly.dy = 0;
  }
    // hilly should be in [0, 1].

  // ridges fading
  // -------------
  if (primary_geoforms->z < -0.05) {
    ridged.z = 0;
    ridged.dx = 0;
    ridged.dy = 0;
  } else {
    mani_copy(&ridged, primary_geoforms);
    mani_offset_const(&ridged, 0.05);
  }
  mani_pow_const(&ridged, 0.6);
  simplex_component(
    &temp,
    x + dst_x.z*0.4,
    y - dst_y.z*0.4,
    1 + dst_x.dx*0.4,
    1 - dst_y.dy*0.4,
    TR_FREQUENCY_SGEOFORMS * 0.9,
    salt
  );
  mani_add(&ridged, &temp);

  simplex_component(
    &temp,
    x - dst_y.z,
    y + dst_x.z,
    1 - dst_y.dx,
    1 + dst_x.dy,
    TR_FREQUENCY_GEODETAIL * 0.4,
    salt
  );
  mani_scale_const(&ridged, 0.5);
  mani_add(&ridged, &temp);

  mani_scale_const(&ridged, 0.9/2.5);
  mani_offset_const(&ridged, 0.1);
    // ridged should be in [0.1, 1].

  // secondary geoforms (not used for roughness vaules)
  // --------------------------------------------------
  get_standard_distortion(
    x, y, salt,
    TR_DFQ_SGEOFORMS,
    TR_DS_SGEOFORMS,
    &dst_x, &dst_y
  );

    // simplex component
  simplex_component(
    secondary_geoforms,
    x + dst_x.z,
    y + dst_y.z,
    1 + dst_x.dx,
    1 + dst_y.dy,
    TR_FREQUENCY_SGEOFORMS,
    salt
  );

    // worley component
  worley_component(
    &temp,
    x - dst_y.z,
    y + dst_x.z,
    1 - dst_y.dx,
    1 + dst_x.dy,
    TR_FREQUENCY_SGEOFORMS,
    salt,
    WORLEY_FLAG_INCLUDE_NEXTBEST | WORLEY_FLAG_SMOOTH_SIDES
  );
  mani_add(secondary_geoforms, &temp);
  mani_scale_const(secondary_geoforms, 0.5);
    // secondary_geoforms should be in [-1, 1].

  // geodetails
  // ----------
  get_standard_distortion(
    x, y, salt,
    TR_DFQ_GEODETAIL,
    TR_DS_GEODETAIL,
    &dst_x, &dst_y
  );

    // simplex component
  simplex_component(
    geodetails,
    x + dst_x.z,
    y + dst_y.z,
    1 + dst_x.dx,
    1 + dst_y.dy,
    TR_FREQUENCY_GEODETAIL,
    salt
  );
  mani_scale_const(geodetails, 2.0);

    // worley component
  worley_component(
    &temp,
    x + dst_y.z,
    y - dst_x.z,
    1 + dst_y.dx,
    1 - dst_x.dy,
    TR_FREQUENCY_GEODETAIL,
    salt,
    WORLEY_FLAG_INCLUDE_NEXTBEST | WORLEY_FLAG_SMOOTH_SIDES
  );
  mani_add(geodetails, &temp);

  mani_scale_const(geodetails, 1.0/3.0);
  mani_multiply(geodetails, &geodetailed);
    // geodetails should be in [-1, 1].

  // mountains
  // ---------
  get_standard_distortion(
    x, y, salt,
    TR_DFQ_MOUNTAINS,
    TR_DS_MOUNTAINS,
    &dst_x, &dst_y
  );

    //simplex component
  simplex_component(
    mountains,
    x + dst_x.z,
    y + dst_y.z,
    1 + dst_x.dx,
    1 + dst_y.dy,
    TR_FREQUENCY_MOUNTAINS,
    salt
  );

  mani_offset_const(mountains, 1);
  mani_scale_const(mountains, 0.5);

    // first worley component
  worley_component(
    &temp,
    x + dst_y.z,
    y + dst_x.z,
    1 + dst_y.dx,
    1 + dst_x.dy,
    TR_FREQUENCY_MOUNTAINS,
    salt,
    WORLEY_FLAG_INCLUDE_NEXTBEST | WORLEY_FLAG_SMOOTH_SIDES
  );
  mani_smooth(&temp, 2, 0.5);
  mani_scale_const(&temp, 2.0);
  mani_add(mountains, &temp);

    // second worley component
  worley_component(
    &temp,
    x - dst_x.z,
    y - dst_y.z,
    1 - dst_x.dx,
    1 - dst_y.dy,
    TR_FREQUENCY_MOUNTAINS,
    salt,
    WORLEY_FLAG_INCLUDE_NEXTBEST | WORLEY_FLAG_SMOOTH_SIDES
  );
  mani_add(mountains, &temp);

  mani_scale_const(mountains, 0.25);
  mani_multiply(mountains, &mountainous);
    // mountains should be in [0, 1].

  // mounds fading
  // -------------
  simplex_component(
    &mounded,
    x - dst_y.z,
    y + dst_x.z,
    1 - dst_y.dx,
    1 + dst_x.dy,
    TR_FREQUENCY_MOUNTAINS * 0.6,
    salt
  );
  mani_offset_const(&mounded, 1);
  mani_scale_const(&mounded, 0.25);
  mani_offset_const(&mounded, 0.5);
    // mounded should be in [0.5, 1].

  // hills
  // -----
  get_standard_distortion(
    x, y, salt,
    TR_DFQ_HILLS,
    TR_DS_HILLS,
    &dst_x, &dst_y
  );

  simplex_component(
    hills,
    x + dst_x.z,
    y + dst_y.z,
    1 + dst_x.dx,
    1 + dst_y.dy,
    TR_FREQUENCY_HILLS,
    salt
  );
  mani_multiply(hills, &hilly);
    // hills should be in [-1, 1].

  // detail fading
  // -------------
  simplex_component(
    &detailed,
    x - dst_y.z,
    y + dst_x.z,
    1 - dst_y.dx,
    1 + dst_x.dy,
    TR_FREQUENCY_HILLS * 0.5,
    salt
  );
  mani_offset_const(&detailed, 1);
  mani_scale_const(&detailed, 0.5 * 0.8);
  mani_offset_const(&detailed, 0.2);
    // detailed should be in [0.2, 1].

  // ridges
  // ------
  get_standard_distortion(
    x, y, salt,
    TR_DFQ_RIDGES,
    TR_DS_RIDGES,
    &dst_x, &dst_y
  );

  worley_component(
    ridges,
    x + dst_x.z,
    y + dst_y.z,
    1 + dst_x.dx,
    1 + dst_y.dy,
    TR_FREQUENCY_RIDGES,
    salt,
    WORLEY_FLAG_INCLUDE_NEXTBEST
  );

  worley_component(
    &temp,
    x + dst_y.z,
    y - dst_x.z,
    1 + dst_y.dx,
    1 - dst_x.dy,
    TR_FREQUENCY_RIDGES * 1.8,
    salt,
    WORLEY_FLAG_INCLUDE_NEXTBEST
  );
  mani_scale_const(&temp, 0.5);
  mani_add(ridges, &temp);
  mani_scale_const(ridges, 1.0/1.5);
  mani_multiply(ridges, &ridged);
    // ridges should be in [0, 1].

  // bumps fading
  // ------------
  simplex_component(
    &bumpy,
    x - dst_y.z,
    y + dst_x.z,
    1 - dst_y.dx,
    1 + dst_x.dy,
    TR_FREQUENCY_RIDGES * 0.2,
    salt
  );
  mani_offset_const(&bumpy, 1);
  mani_scale_const(&bumpy, 0.5 * 0.6);
  mani_offset_const(&bumpy, 0.4);
    // bumpy should be in [0.4, 1].

  // mounds
  // ------
  get_standard_distortion(
    x, y, salt,
    TR_DFQ_MOUNDS,
    TR_DS_MOUNDS,
    &dst_x, &dst_y
  );

  simplex_component(
    mounds,
    x + dst_x.z,
    y + dst_y.z,
    1 + dst_x.dx,
    1 + dst_y.dy,
    TR_FREQUENCY_MOUNDS,
    salt
  );
  mani_multiply(mounds, &mounded);
    // mounds should be in [-1, 1].

  // details
  // -------
  get_standard_distortion(
    x, y, salt,
    TR_DFQ_DETAILS,
    TR_DS_DETAILS,
    &dst_x, &dst_y
  );

  simplex_component(
    details,
    x + dst_x.z,
    y + dst_y.z,
    1 + dst_x.dx,
    1 + dst_y.dy,
    TR_FREQUENCY_DETAILS,
    salt
  );
  mani_multiply(details, &detailed);
    // details should be in [-1, 1].

  // bumps
  get_standard_distortion(
    x, y, salt,
    TR_DFQ_BUMPS,
    TR_DS_BUMPS,
    &dst_x, &dst_y
  );

  simplex_component(
    bumps,
    x + dst_x.z,
    y + dst_y.z,
    1 + dst_x.dx,
    1 + dst_y.dy,
    TR_FREQUENCY_BUMPS,
    salt
  );
  mani_multiply(bumps, &bumpy);
    // bumps should be in [-1, 1].
}

static inline void geomap_segment(
  manifold_point *result, manifold_point *base, manifold_point *interp,
  float sm_str, float sm_ctr,
  float lower, float upper,
  float lower_height, float upper_height
) {
  mani_copy(interp, base);
  // DEBUG:
  // printf("Geomap segment :: base %.2f\n", base->z);
  // printf("Geomap segment :: interp %.2f\n", interp->z);
  mani_offset_const(interp, -lower);
  // printf("Geomap segment :: offset :: interp %.2f\n", interp->z);
  mani_scale_const(interp, 1.0/(upper - lower));
  // printf("Geomap segment :: scale :: interp %.2f\n", interp->z);
  mani_smooth(interp, sm_str, sm_ctr);
  // printf("Geomap segment :: smooth :: interp %.2f\n", interp->z);
  mani_copy(result, interp);
  // printf("Geomap segment :: copy :: result %.2f\n", result->z);
  mani_scale_const(
    result,
    (upper_height - lower_height)
  );
  // printf("Geomap segment :: scale :: result %.2f\n", result->z);
  mani_offset_const(result, lower_height);
  // printf("Geomap segment :: offset :: result %.2f\n", result->z);
  // printf("\n\n\n");
}

// Remaps the given value (on [0, 1]) to account for the shape and height of
// the overall height profile.
static inline void geomap(
  manifold_point *result,
  manifold_point *base,
  terrain_region* region,
  manifold_point* interp
) {
  if (base->z < TR_GEOMAP_SHELF) { // deep ocean
    *region = TR_REGION_DEPTHS;
    geomap_segment(
      result, base, interp,
      0.5, 0.5,
      0, TR_GEOMAP_SHELF,
      TR_HEIGHT_OCEAN_DEPTHS, TR_HEIGHT_CONTINENTAL_SHELF
    );
  } else if (base->z < TR_GEOMAP_SHORE) { // continental shelf
    *region = TR_REGION_SHELF;
    geomap_segment(
      result, base, interp,
      1.6, 0.2,
      TR_GEOMAP_SHELF, TR_GEOMAP_SHORE,
      TR_HEIGHT_CONTINENTAL_SHELF, TR_HEIGHT_SEA_LEVEL
    );
    // TODO: Sea cliffs?
  } else if (base->z < TR_GEOMAP_PLAINS) { // coastal plain
    *region = TR_REGION_PLAINS;
    geomap_segment(
      result, base, interp,
      0.8, 0.5,
      TR_GEOMAP_SHORE, TR_GEOMAP_PLAINS,
      TR_HEIGHT_SEA_LEVEL, TR_HEIGHT_COASTAL_PLAINS
    );
  } else if (base->z < TR_GEOMAP_HILLS) { // hills
    *region = TR_REGION_HILLS;
    geomap_segment(
      result, base, interp,
      1.4, 0.35,
      TR_GEOMAP_PLAINS, TR_GEOMAP_HILLS,
      TR_HEIGHT_COASTAL_PLAINS, TR_HEIGHT_HIGHLANDS
    );
  } else if (base->z < TR_GEOMAP_MOUNTAINS) { // highlands
    *region = TR_REGION_HIGHLANDS;
    geomap_segment(
      result, base, interp,
      1.3, 0.5,
      TR_GEOMAP_HILLS, TR_GEOMAP_MOUNTAINS,
      TR_HEIGHT_HIGHLANDS, TR_HEIGHT_MOUNTAIN_BASES
    );
  } else { // mountains
    *region = TR_REGION_MOUNTAINS;
    geomap_segment(
      result, base, interp,
      1.6, 1.0,
      TR_GEOMAP_MOUNTAINS, 1.0,
      TR_HEIGHT_MOUNTAIN_BASES, TR_HEIGHT_MOUNTAIN_TOPS
    );
  }
}

// Computes basic geoform information to be used by compute_terrain_height.
static inline void compute_base_geoforms(
  region_pos* pos, ptrdiff_t *salt,
  manifold_point *continents,
  manifold_point *primary_geoforms, manifold_point *secondary_geoforms,
  manifold_point *geodetails, manifold_point *mountains,
  manifold_point *hills, manifold_point *ridges,
  manifold_point *mounds, manifold_point *details, manifold_point *bumps,
  manifold_point* base,
  terrain_region* region,
  manifold_point* tr_interp,
  manifold_point* height
) {
  manifold_point temp;

  // make some noise:
  get_noise(
    pos->x, pos->y, salt,
    continents, primary_geoforms, secondary_geoforms,
    geodetails, mountains,
    hills, ridges, mounds, details, bumps
  );
  *salt = expanded_hash_1d(*salt);

  // DEBUG: print noise
  //*
  // printf("Base geoforms noise!\n");
  // printf("continents: %.8f\n", continents->z);
  // printf("primary_geoforms: %.8f\n", primary_geoforms->z);
  // printf("secondary_geoforms: %.8f\n", secondary_geoforms->z);
  // printf("geodetails: %.8f\n", geodetails->z);
  // printf("mountains: %.8f\n", mountains->z);
  // printf("hills: %.8f\n", hills->z);
  // printf("ridges: %.8f\n", ridges->z);
  // printf("mounds: %.8f\n", mounds->z);
  // printf("details: %.8f\n", details->z);
  // printf("bumps: %.8f\n", bumps->z);
  // printf("\n\n\n");
  // */

  base->z = 0;
  base->dx = 0;
  base->dy = 0;

  mani_copy(&temp, continents);
  mani_scale_const(&temp, 2.5);
  mani_add(base, &temp);

  mani_copy(&temp, primary_geoforms);
  mani_scale_const(&temp, 2);
  mani_add(base, &temp);

  mani_copy(&temp, secondary_geoforms);
  mani_scale_const(&temp, 1.5);
  mani_add(base, &temp);

  mani_add(base, geodetails);

  mani_scale_const(base, 1.0/7.0);

  mani_offset_const(base, 1);
  mani_scale_const(base, 0.5); // squash into [0, 1]

  mani_smooth(base, 2, 0.5); // spread things out

  // remap everything:
  geomap(height, base, region, tr_interp);
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

// Performs initial setup for terrain generation.
void setup_terrain_gen(ptrdiff_t seed);

// Computes the terrain height at the given region position in blocks, and
// writes out the gross (large-scale), rock and dirt heights at that location.
void compute_terrain_height(
  region_pos *pos,
  manifold_point *r_gross,
  manifold_point *r_rocks,
  manifold_point *r_dirt
);

// Alters the various detail values according to the terrain region
// classification.
void alter_terrain_values(
  region_pos *pos, ptrdiff_t *salt,
  terrain_region region, manifold_point *tr_interp,
  manifold_point *mountains, manifold_point *hills, manifold_point *ridges,
  manifold_point *mounds, manifold_point *details, manifold_point *bumps
);

// Figures out the dirt height at the given location and writes it into the
// result parameter.
void compute_dirt_height(
  region_pos *pos, ptrdiff_t *salt,
  manifold_point *rocks_height,
  manifold_point *mountains, manifold_point *hills,
  manifold_point *details, manifold_point *bumps,
  manifold_point *result
);

// Computes the terrain region and interpolation values at the given position.
void geoform_info(region_pos *pos, terrain_region* region, float* tr_interp);
#endif // ifndef TERRAIN_H
