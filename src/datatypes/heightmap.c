// heightmap.c
// Two-dimensional arrays of floating point values.

#include "heightmap.h"

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates and sets up a new flat heightmap:
heightmap *create_heightmap(size_t width, size_t height) {
  heightmap *result = (heightmap*) malloc(sizeof(heightmap));
  result->width = width;
  result->height = height;
  result->data = (float*) calloc(width * height, sizeof(float));
  return result;
}

// Frees the memory associated with a heightmap.
void cleanup_heightmap(heightmap *hm) {
  free(hm->data);
  free(hm);
}

/*************
 * Functions *
 *************/

void hm_reset(heightmap *hm) {
  size_t i;
  for (i = 0; i < hm->width * hm->height; ++i) {
    hm->data[i] = 0;
  }
}

void hm_copy(heightmap const * const source, heightmap *dest) {
  size_t i;
  if (dest->width != source->width || dest->height != source->height) {
    free(dest->data);
    dest->width = source->width;
    dest->height = source->height;
    dest->data = (float*) malloc(sizeof(float) * dest->width * dest->height);
  }
  for (i = 0; i < dest->width * dest->height; ++i) {
    dest->data[i] = source->data[i];
  }
}

void hm_scale(heightmap *hm, float factor) {
  size_t i;
  for (i = 0; i < hm->width * hm->height; ++i) {
    hm->data[i] *= factor;
  }
}

void hm_offset(heightmap *hm, float offset) {
  size_t i;
  for (i = 0; i < hm->width * hm->height; ++i) {
    hm->data[i] += offset;
  }
}

void hm_average(
  heightmap *main,
  heightmap const * const add_in,
  float main_weight
) {
  size_t i;
  if (main->width != add_in->width || main->height != add_in->height) {
    return;
  }
  for (i = 0; i < main->width * main->height; ++i) {
    main->data[i] = (
      (main->data[i] * main_weight + add_in->data[i])
    / (main_weight + 1.0)
    );
  }
}

void hm_combine_modulated(
  heightmap *main,
  heightmap const * const add_in,
  heightmap const * const modulate,
  float modulation_strength
) {
  size_t i;
  float mod;
  if (main->width != add_in->width || main->height != add_in->height) {
    return;
  }
  for (i = 0; i < main->width * main->height; ++i) {
    mod = modulate->data[i] * modulation_strength + (1 - modulation_strength);
    main->data[i] = main->data[i] * mod + add_in->data[i] * (1 - mod);
  }
}

void hm_add(heightmap *base, heightmap const * const add_in) {
  size_t i;
  if (base->width != add_in->width || base->height != add_in->height) {
    return;
  }
  for (i = 0; i < base->width * base->height; ++i) {
    base->data[i] += add_in->data[i];
  }
}

void hm_normalize(heightmap *hm) {
  size_t i;
  float min = hm->data[0], max = hm->data[0];
  float z;
  for (i = 0; i < hm->width * hm->height; ++i) {
    z = hm->data[i];
    if (z < min) { min = z; }
    if (z > max) { max = z; }
  }
  for (i = 0; i < hm->width * hm->height; ++i) {
    hm->data[i] = (hm->data[i] - min) / (max - min);
  }
}

void hm_convolve(heightmap *hm, heightmap *filter, heightmap *buffer) {
  size_t idx, cidx;
  ptrdiff_t x, y, xx, yy;
  size_t i, j, fidx;
  size_t xof = filter->width / 2;
  size_t yof = filter->height / 2;
  float total_weight;
  if (
    hm->width != filter->width
 || hm->height != filter->height
 || hm->width != buffer->width
 || hm->height != buffer->height
  ) {
    return;
  }
  for (x = 0; x < hm->width; ++x) {
    for (y = 0; y < hm->height; ++y) {
      idx = x + y * hm->width;
      total_weight = 0;
      i = 0;
      for (xx = x - xof; xx < x + xof + 1; ++xx) {
        j = 0;
        if (xx < 0 || xx > hm->width - 1) {
          i += 1;
          continue;
        }
        for (yy = y - yof; yy < y + yof + 1; ++yy) {
          if (yy < 0 || yy > hm->height - 1) {
            j += 1;
            continue;
          }
          cidx = xx + yy * hm->width;
          fidx = i + j * filter->width;

          buffer->data[idx] += hm->data[cidx] * filter->data[fidx];
          total_weight += filter->data[fidx];

          j += 1;
        }
        i += 1;
      }
      buffer->data[idx] /= total_weight;
    }
  }
  for (i = 0; i < hm->width * hm->height; ++i) {
    hm->data[i] = buffer->data[i];
  }
}

void hm_convolve_q(heightmap *hm, heightmap *filter) {
  heightmap *buffer = create_heightmap(hm->width, hm->height);
  hm_convolve(hm, filter, buffer);
  cleanup_heightmap(buffer);
}

void hm_slump(heightmap *hm, heightmap *buffer, float max_slope, float rate) {
  size_t i, x, y, idx, lnidx;
  float ln, midpoint;
  if (hm->width != buffer->width || hm->height != buffer->height) {
    return;
  }
  for (x = 0; x < hm->width; ++x) {
    for (y = 0; y < hm->height; ++y) {
      idx = x + hm->width * y;
      ln = hm->data[idx];
      lnidx = idx;
      if (x > 0 && hm->data[idx - 1] < ln) {
        ln = hm->data[idx - 1];
        lnidx = idx - 1;
      }
      if (x < hm->width - 1 && hm->data[idx + 1] < ln) {
        ln = hm->data[idx + 1];
        lnidx = idx + 1;
      }
      if (y > 0 && hm->data[idx - hm->width] < ln) {
        ln = hm->data[idx - hm->width];
        lnidx = idx - hm->width;
      }
      if (y < hm->height - 1 && hm->data[idx + hm->width] < ln) {
        ln = hm->data[idx + hm->width];
        lnidx = idx + hm->width;
      }
      if (lnidx != idx && hm->data[idx] - ln > max_slope) {
        midpoint = ln + (hm->data[idx] - ln) / 2.0;
        buffer->data[idx] -= hm->data[idx] - (midpoint + max_slope/2.0);
        buffer->data[lnidx] += (midpoint - max_slope/2.0) - ln;
      }
    }
  }
  for (i = 0; i < hm->width * hm->height; ++i) {
    hm->data[i] += buffer->data[i] * rate;
  }
}

void hm_slump_q(heightmap *hm, float max_slope, float rate) {
  heightmap *buffer = create_heightmap(hm->width, hm->height);
  hm_slump(hm, buffer, max_slope, rate);
  cleanup_heightmap(buffer);
}

void hm_add_limited(
  heightmap *base,
  heightmap const * const fill,
  heightmap *buffer
) {
  size_t x, y, idx;
  float min_new, max_new;
  float z;
  size_t i;
  if (
    base->width != fill->width
 || base->height != fill->height
 || base->width != buffer->width
 || base->height != buffer->height
  ) {
    return;
  }
  for (x = 0; x < base->width; ++x) {
    for (y = 0; y < base->height; ++y) {
      idx = x + y * base->width;
      min_new = base->data[idx];
      max_new = base->data[idx];
      if (x > 0) {
        z = base->data[idx - 1];
        if (z < min_new) { min_new = z; }
        if (z > max_new) { max_new = z; }
      }
      if (x < base->width - 1) {
        z = base->data[idx + 1];
        if (z < min_new) { min_new = z; }
        if (z > max_new) { max_new = z; }
      }
      if (y > 0) {
        z = base->data[idx - base->width];
        if (z < min_new) { min_new = z; }
        if (z > max_new) { max_new = z; }
      }
      if (y < base->height - 1) {
        z = base->data[idx + base->width];
        if (z < min_new) { min_new = z; }
        if (z > max_new) { max_new = z; }
      }
      z = base->data[idx] + fill->data[idx];
      if (fill->data[idx] < 0 && z < min_new) { z = min_new; }
      if (fill->data[idx] > 0 && z > max_new) { z = max_new; }
      buffer->data[idx] = z;
    }
  }
  for (i = 0; i < base->width * base->height; ++i) {
    base->data[i] = buffer->data[i];
  }
}

void hm_add_limited_q(
  heightmap *base,
  heightmap const * const fill
) {
  heightmap *buffer = create_heightmap(base->width, base->height);
  hm_add_limited(base, fill, buffer);
  cleanup_heightmap(buffer);
}

void hm_process(
  heightmap *hm,
  void *arg,
  float (*process)(heightmap*, size_t, size_t, float, void*)
) {
  size_t x, y, idx;
  for (x = 0; x < hm->width; ++x) {
    for (y = 0; y < hm->height; ++y) {
      idx = x + y * hm->width;
      hm->data[idx] = process(hm, x, y, hm->data[idx], arg);
    }
  }
}

void  hm_compute_flows(
  heightmap const * const hm,
  heightmap const * const precip,
  heightmap *result,
  heightmap *buffer,
  heightmap *extra_buffer,
  size_t flow_steps
) {
  size_t n, i, idx, oidx;
  ptrdiff_t x, y, xx, yy;
  size_t bidx, sbidx, tbidx;
  float bslope, sbslope, tbslope;
  float slope, stotal;
  if (
    hm->width != precip->width
 || hm->height != precip->height
 || hm->width != result->width
 || hm->height != result->height
 || hm->width != buffer->width
 || hm->height != buffer->height
 || hm->width != extra_buffer->width
 || hm->height != extra_buffer->height
  ) {
    return;
  }

  // Copy precipitation into our buffer:
  hm_copy(precip, buffer);

  for (n = 0; n < flow_steps; ++n) {
    for (x = 0; x < hm->width; ++x) {
      for (y = 0; y < hm->height; ++y) {
        idx = x + hm->width * y;
        
        // First, iterate over neighbors to find our three most-downhill
        // neighbors:
        oidx = 0;
        bidx = 0;
        sbidx = 0;
        tbidx = 0;
        bslope = 0;
        sbslope = 0;
        tbslope = 0;
        slope = 0;
        stotal = 0;
        for (xx = x - 1; xx <= x + 1; ++xx) {
          if (xx < 0 || xx > hm->width - 1) {
            continue;
          }
          for (yy = y - 1; yy <= y + 1; ++yy) {
            if (
              (xx == x && yy == y)
            || yy < 0
            || yy > hm->height - 1
            ) {
              continue;
            }
            oidx = xx + hm->width * yy;
            slope = hm->data[idx] - hm->data[oidx];
            if (slope > bslope) {
              tbidx = sbidx;
              tbslope = sbslope;
              sbidx = bidx;
              sbslope = bslope;
              bidx = oidx;
              bslope = slope;
            } else if (slope > sbslope) {
              tbidx = sbidx;
              tbslope = sbslope;
              sbidx = oidx;
              sbslope = slope;
            } else if (slope > tbslope) {
              tbidx = oidx;
              tbslope = slope;
            }
          }
        }

        // The sum of best slopes:
        stotal = bslope + sbslope + tbslope;

        // Redistribute water downstream:
        if (stotal > 0) {
          // Local water flows onwards to three most-downhill neighbors
          // proportional to their slopes:
          if (bslope != 0) {
            extra_buffer->data[bidx] += buffer->data[idx] * (bslope / stotal);
          }
          if (sbslope != 0) {
            extra_buffer->data[sbidx] += buffer->data[idx] * (sbslope / stotal);
          }
          if (tbslope != 0) {
            extra_buffer->data[tbidx] += buffer->data[idx] * (tbslope / stotal);
          }
        } // if we're at a minima, no water leaves
      }
    }

    // Flip buffers and accumulate:
    for (i = 0; i < hm->width * hm->height; ++i) {
      result->data[i] += buffer->data[i];
      buffer->data[i] = extra_buffer->data[i];
      extra_buffer->data[i] = 0;
    }
  }
}

void hm_compute_flows_q(
  heightmap const * const hm,
  heightmap const * const precip,
  heightmap *result,
  size_t flow_steps
) {
  heightmap *buffer = create_heightmap(hm->width, hm->height);
  heightmap *extra_buffer = create_heightmap(hm->width, hm->height);
  hm_compute_flows(hm, precip, result, buffer, extra_buffer, flow_steps);
  cleanup_heightmap(buffer);
  cleanup_heightmap(extra_buffer);
}

void hm_erode(
  heightmap *hm,
  heightmap const * const modulate, 
  heightmap const * const precip,
  heightmap *flow_buffer,
  heightmap *save_buffer,
  heightmap *extra_buffer,
  size_t flow_steps,
  size_t flow_slump_steps,
  float flow_maxslope,
  float flow_slump_rate,
  float erosion_strength,
  float modulation_strength
) {
  size_t i;
  if (
    hm->width != modulate->width
 || hm->height != modulate->height
 || hm->width != flow_buffer->width
 || hm->height != flow_buffer->height
 || hm->width != save_buffer->width
 || hm->height != save_buffer->height
 || hm->width != extra_buffer->width
 || hm->height != extra_buffer->height
  ) {
    return;
  }
  // Before we set up the save buffer, use it as scratch to compute flows:
  hm_compute_flows(
    hm,
    precip,
    flow_buffer,
    save_buffer,
    extra_buffer,
    flow_steps
  );
  hm_normalize(flow_buffer);
  // Set up the save buffer with our original heightmap data:
  hm_copy(hm, save_buffer);
  // Slump the flow buffer:
  for (i = 0; i < flow_slump_steps; ++i) {
    hm_reset(extra_buffer);
    hm_slump(flow_buffer, extra_buffer, flow_maxslope, flow_slump_rate);
  }
  // Scale the flow buffer and add it to the base heightmap:
  hm_scale(flow_buffer, erosion_strength);
  hm_reset(extra_buffer);
  hm_add_limited(hm, flow_buffer, extra_buffer);

  // Re-normalize the heightmap and combine it with an uneroded version
  // according to the modulation parameters:
  hm_normalize(hm);
  hm_combine_modulated(hm, save_buffer, modulate, modulation_strength);
}

void hm_erode_q(
  heightmap *hm,
  heightmap const * const modulate, 
  heightmap const * const precip,
  size_t flow_steps,
  size_t flow_slump_steps,
  float flow_maxslope,
  float flow_slump_rate,
  float erosion_strength,
  float modulation_strength
) {
  heightmap *flow = create_heightmap(hm->width, hm->height);
  heightmap *save = create_heightmap(hm->width, hm->height);
  heightmap *extra = create_heightmap(hm->width, hm->height);
  hm_erode(
    hm,
    modulate,
    precip,
    flow,
    save,
    extra,
    flow_steps,
    flow_slump_steps,
    flow_maxslope,
    flow_slump_rate,
    erosion_strength,
    modulation_strength
  );
  cleanup_heightmap(flow);
  cleanup_heightmap(save);
  cleanup_heightmap(extra);
}
