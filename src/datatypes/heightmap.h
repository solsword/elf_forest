#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

// heightmap.h
// Two-dimensional arrays of floating point values.

#include <stdlib.h>
#include <stddef.h>

/**************
 * Structures *
 **************/

// A 2-d array of height values.
struct heightmap_s;
typedef struct heightmap_s heightmap;

/*************************
 * Structure Definitions *
 *************************/

struct heightmap_s {
  size_t width, height;
  float *data;
};

/********************
 * Inline Functions *
 ********************/

// Returns the height of the given heightmap at the given x/y position, without
// any error checking. Can easily be made to read memory outside the heightmap.
static inline float hm_height(heightmap *hm, size_t x, size_t y) {
  return hm->data[x + hm->width * y];
}

// Returns the index of the point at (x, y) within the given heightmap's data
// array. Doesn't check whether x and y are in-bounds or not.
static inline size_t hm_idx(heightmap *hm, size_t x, size_t y) {
  return x + hm->width * y;
}

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates and sets up a new flat heightmap:
heightmap *create_heightmap(size_t width, size_t height);

// Frees the memory associated with a heightmap.
void cleanup_heightmap(heightmap *hm);

/*************
 * Functions *
 *************/

// Sets the given heightmap to all zeroes.
void hm_reset(heightmap *hm);

// Copies the source heightmap, overwriting the destination heightmap. If the
// two heightmaps have different dimensions, the destination heightmap's data
// will be freed and reallocated.
void hm_copy(heightmap const * const source, heightmap *dest);

// Multiplies each height in the heightmap by the given scaling factor.
void hm_scale(heightmap *hm, float factor);

// Adds the given offset to each entry in the heightmap.
void hm_offset(heightmap *hm, float offset);

// Computes the weighted average of two heightmaps, storing the result in the
// first. The first heightmap is given a weight of main_weight, while the other
// has a weight of 1.0. If the heightmaps have different sizes, does nothing.
void hm_average(
  heightmap *main,
  heightmap const * const add_in,
  float main_weight
);

// Combines two heightmaps using a third heightmap to decide point-by-point how
// to combine them. The values in the modulation map should be between 0 and 1,
// with a 0 indicating that the main heightmap should dominate, and a 1
// indicating that the add_in heightmap should dominate. 'modulation_strength'
// should also be between 0 and 1, with a 0 indicating that the add_in
// heightmap will be ignored regardless of the modulation map, and a 1
// indicating that the modulation map will be followed completely. The result
// of the combination is stored in the main heightmap; if the heightmaps have
// different sizes, it does nothing.
void hm_combine_modulated(
  heightmap *main,
  heightmap const * const add_in,
  heightmap const * const modulate,
  float modulation_strength
);

// Adds the given heightmap to the base heightmap, storing the result in the
// base heightmap. Does nothing if the heightmaps are different sizes.
void hm_add(heightmap *base, heightmap const * const add_in);

// Modifies the given heightmap so that all of its values are in [0-1], with
// its previous minimum value at 0 and its previous maximum value at 1. If the
// heightmap has width and/or height 0, this will cause a segmentation fault.
void hm_normalize(heightmap *hm);

// Applies a convolution filter to the heightmap. The given convolution filter
// should have odd dimensions. After summing nearby entries, results are
// normalized according to the total weight from the filter, so filter entries
// don't need to add up to any specific number. Filter entries can of course be
// negative, but if the filter has a negative sum it will behave like its
// inverse due to the normalization. This can be tricky when parts of a filter
// are ignored near edges where they're out-of-bounds. Note that the _q version
// of this function creates and destroys its own buffer heightmap rather than
// requiring one as input.
void hm_convolve(heightmap *hm, heightmap *filter, heightmap *buffer);
void hm_convolve_q(heightmap *hm, heightmap *filter);

// For each entry in the heightmap with an orthogonal neighbor that's lower
// than it by more than 'max_slope', average it with that neighbor (or at
// least, move both towards the average by an amount proportional to 'rate').
// Note that the _q version of this function allocates and destroys a buffer
// array rather than requiring one as input.
void hm_slump(heightmap *hm, heightmap *buffer, float max_slope, float rate);
void hm_slump_q(heightmap *hm, float max_slope, float rate);

// Add the fill heightmap to the base heightmap, storing the result in base,
// but don't increase/decrease any point in base so that it becomes
// higher/lower than the tallest/lowest orthogonal neighbor it had before the
// operation. Does nothing if the heightmaps given are different sizes. The _q
// version allocates and destroys a temporary processing buffer instead of
// requiring one as input.
void hm_add_limited(
  heightmap *base,
  heightmap const * const fill,
  heightmap *buffer
);
void hm_add_limited_q(
  heightmap *base,
  heightmap const * const fill
);

// Loop over each cell in the heightmap, calling the given function on each
// with the heightmap, the cell's x-coordinate, y-coordinate, and height, and
// the extra constant argument as arguments, and updating the cell's height
// with the result.
void hm_process(
  heightmap *hm,
  void *arg,
  float (*process)(heightmap*, size_t, size_t, float, void*)
);

// Simulates water flow using the given height and precipitation maps, storing
// results in the 'result' parameter. The required buffer should be zeroed when
// given, or the _q version can be used to automatically allocate and destroy a
// temporary buffer. The flow_steps parameter controls how many simulation
// steps are performed, which in turn limits how far water can travel from
// where it falls originally.
void hm_compute_flows(
  heightmap const * const hm,
  heightmap const * const precip,
  heightmap *result,
  heightmap *buffer,
  heightmap *extra_buffer,
  size_t flow_steps
);

void hm_compute_flows_q(
  heightmap const * const hm,
  heightmap const * const precip,
  heightmap *result,
  size_t flow_steps
);

// Simulates erosion of the given heightmap (not very accurately) by computing
// flows for it and using hm_add_limited to build up low-lying areas of the
// heightmap (doesn't wear down high areas like real erosion). Three buffers
// are required (and are assumed to be zeroed when given); the _q version
// constructs and destroys the buffers automatically. The modulation heightmap
// acts as an erosion mask, as after using hm_add_limited, hm_erode uses
// hm_combine_modulated with the given modulation strength against a copy of
// the original heightmap. The precipitation heighmap influences flow
// strengths (see hm_compute_flows). The flow_steps, flow_maxslope, and
// flow_slump_rate control postprocessing of the flow information using
// hm_slump, while the erosion_strength is multiplied by the flow information
// before being passed to hm_add_limited.
void hm_erode(
  heightmap *hm,
  heightmap const * const precip,
  heightmap const * const modulate, 
  heightmap *flow_buffer,
  heightmap *save_buffer,
  heightmap *extra_buffer,
  size_t flow_steps,
  size_t flow_slump_steps,
  float flow_maxslope,
  float flow_slump_rate,
  float erosion_strength,
  float modulation_strength
);

void hm_erode_q(
  heightmap *hm,
  heightmap const * const precip,
  heightmap const * const modulate, 
  size_t flow_steps,
  size_t flow_slump_steps,
  float flow_maxslope,
  float flow_slump_rate,
  float erosion_strength,
  float modulation_strength
);

#endif //ifndef HEIGHTMAP_H
