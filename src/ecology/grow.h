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
// Normal growing blocks:
#define         GR_BS_VITALITY 0
#define          GR_BS_RENEWAL 2
#define  GR_BS_TRAJECTORY_BIAS 3
//#define  GR_BS_??? 6

// Growth core blocks:
#define        GR_BS_BODY_PLAN 2
#define              GR_BS_AGE 4
#define     GR_BS_LEAF_BALANCE 6

// Post-shift masks:
#define         GR_BM_VITALITY 0x3
#define          GR_BM_RENEWAL 0x1
#define  GR_BM_TRAJECTORY_BIAS 0x7

#define        GR_BM_BODY_PLAN 0x3
#define              GR_BM_AGE 0x3
#define     GR_BM_LEAF_BALANCE 0x3

// Vitality values:
#define    GR_VITALITY_HEALTHY 0
#define       GR_VITALITY_SICK 1
#define      GR_VITALITY_DYING 2
#define       GR_VITALITY_DEAD 3

/********************
 * Inline Functions *
 ********************/

// TODO: Update these!
// Getters and setters for growth potential, growth direction, and vitality.
// Growth potential goes from 0 to 7, growth direction uses BD_ORI_* constants,
// and vitality uses GR_VITALITY_* constants.

static inline block_data gri_vitality(block b) {
  return (block_data) ((b >> GR_BS_VITALITY) && GR_BM_VITALITY);
}

static inline block_data gri_renewal(block b) {
  return (block_data) ((b >> GR_BS_RENEWAL) && GR_BM_RENEWAL);
}

static inline block_data gri_trajectory_bias(block b) {
  return (block_data) ((b >> GR_BS_TRAJECTORY_BIAS) && GR_BM_TRAJECTORY_BIAS);
}

static inline block_data gri_body_plan(block b) {
  return (block_data) ((b >> GR_BS_BODY_PLAN) && GR_BM_BODY_PLAN);
}

static inline block_data gri_age(block b) {
  return (block_data) ((b >> GR_BS_AGE) && GR_BM_AGE);
}

static inline block_data gri_leaf_balance(block b) {
  return (block_data) ((b >> GR_BS_LEAF_BALANCE) && GR_BM_LEAF_BALANCE);
}


static inline void gri_set_vitality(block *b, block_data vitality) {
  b_set_data(
    b,
    (
      (b_data(*b) & ~(GR_BM_VITALITY << GR_BS_VITALITY))
    |
      ((vitality & GR_BM_VITALITY) << GR_BS_VITALITY)
    )
  );
}

static inline void gri_set_renewal(block *b, block_data renewal) {
  b_set_data(
    b,
    (
      (b_data(*b) & ~(GR_BM_RENEWAL << GR_BS_RENEWAL))
    |
      ((renewal & GR_BM_RENEWAL) << GR_BS_RENEWAL)
    )
  );
}

static inline void gri_set_trajectory_bias(block *b, block_data bias) {
  b_set_data(
    b,
    (
      (b_data(*b) & ~(GR_BM_TRAJECTORY_BIAS << GR_BS_TRAJECTORY_BIAS))
    |
      ((bias & GR_BM_TRAJECTORY_BIAS) << GR_BS_TRAJECTORY_BIAS)
    )
  );
}

static inline void gri_set_body_plan(block *b, block_data plan) {
  b_set_data(
    b,
    (
      (b_data(*b) & ~(GR_BM_BODY_PLAN << GR_BS_BODY_PLAN))
    |
      ((plan & GR_BM_BODY_PLAN) << GR_BS_BODY_PLAN)
    )
  );
}

static inline void gri_set_age(block *b, block_data age) {
  b_set_data(
    b,
    (
      (b_data(*b) & ~(GR_BM_AGE << GR_BS_AGE))
    |
      ((age & GR_BM_AGE) << GR_BS_AGE)
    )
  );
}

static inline void gri_set_leaf_balance(block *b, block_data balance) {
  b_set_data(
    b,
    (
      (b_data(*b) & ~(GR_BM_LEAF_BALANCE << GR_BS_LEAF_BALANCE))
    |
      ((balance & GR_BM_LEAF_BALANCE) << GR_BS_LEAF_BALANCE)
    )
  );
}

// Function for getting the growth rate of a block:
static inline ptrdiff_t get_growth_rate(block b) {
  // TODO: Get this from the species info...
  //species sp = b_species(b);
  return 1;
}

/*************
 * Functions *
 *************/

// Updates growth information for a single growing block. Doesn't deal with
// block expansion (see grow_seed_block and grow_from_core).
void update_growth(block *b);

// Given a cell neighborhood with a seed block in the center, grows the seed
// block.
void grow_seed_block(cell **cell_neighborhood, int is_primary);

// Given a chunk neighborhood and the index of a growth core block within the
// central chunk, runs a cycle of growth particles from that growth core.
// Checks growth cycle info according to the given growth timer t.
void grow_from_core(
  chunk **chunk_neighborhood,
  chunk_index idx,
  ptrdiff_t t
);

// Grows plants in the given chunk for the given number of cycles. It returns 1
// on success, or 0 if the chunk's full neighborhood isn't available.
// Neighboring chunks will be edited directly when plants grow into them, but
// growth will stop at the border (writing an appropriate number of pending
// growth cycles in the new/updated blocks).
int grow_plants(chunk *c, ptrdiff_t cycles);

#endif // ifndef GROW_H
