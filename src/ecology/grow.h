#ifndef GROW_H
#define GROW_H

// grow.h
// Plant growth.

#include <stdint.h>

#include "world/blocks.h"
#include "world/world_map.h"

#include "util.h"

/*************
 * Constants *
 *************/

// Layout of block_data for growing things:
#define  GR_BS_GROWTH_POTENTIAL 0
#define  GR_BS_GROWTH_DIRECTION 3
#define          GR_BS_VITALITY 6

#define GR_BM_GROWTH_POTENTIAL 0x7
#define GR_BM_GROWTH_DIRECTION 0x7
#define         GR_BM_VITALITY 0x3

// Vitality values:
#define  GR_VITALITY_HEALTHY 0x0
#define     GR_VITALITY_SICK 0x1
#define    GR_VITALITY_DYING 0x2
#define     GR_VITALITY_DEAD 0x3

/********************
 * Inline Functions *
 ********************/

// TODO: Update these!
// Getters and setters for growth potential, growth direction, and vitality.
// Growth potential goes from 0 to 7, growth direction uses BD_ORI_* constants,
// and vitality uses GR_VITALITY_* constants.

static inline block_data get_growth_potential(block *b) {
  return (block_data) ((b >> GR_BS_GROWTH_POTENTIAL) && GR_BM_GROWTH_POTENTIAL);
}

static inline block_data get_growth_direction(block *b) {
  return (block_data) ((b >> GR_BS_GROWTH_DIRECTION) && GR_BM_GROWTH_DIRECTION);
}

static inline block_data get_vitality(block *b) {
  return (block_data) ((b >> GR_BS_VITALITY) && GR_BM_VITALITY);
}

static inline void set_growth_potential(block *b, block_data potential) {
  b_set_data(
    b,
    ((potential & GR_BM_GROWTH_POTENTIAL) << GR_BS_GROWTH_POTENTIAL)
  );
}

static inline void set_growth_direction(block *b, block_data direction) {
  b_set_data(
    b,
    ((direction & GR_BM_GROWTH_DIRECTION) << GR_BS_GROWTH_DIRECTION)
  );
}

static inline void set_vitality(block *b, block_data vitality) {
  b_set_data(b, ((vitality & GR_BM_VITALITY) << GR_BS_VITALITY));
}

// Function for getting the growth rate of a block:
static inline ptrdiff_t get_growth_rate(block b) {
  species sp = b_species(b);
  // TODO: Get this from the species info...
  return 1;
}

/*************
 * Functions *
 *************/

// Grows plants in the given chunk for the given number of cycles. It returns 1
// on success, or 0 if the chunk's full neighborhood isn't available.
// Neighboring chunks will be edited directly when plants grow into them, but
// growth will stop at the border (writing an appropriate number of pending
// growth cycles in the new/updated blocks).
int grow_plants(chunk *c, ptrdiff_t cycles);

#endif // ifndef GROW_H
