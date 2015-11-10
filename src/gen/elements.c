// elements.c
// Element generation.

#include "elements.h"

#include "datatypes/rngtable.h"
#include "world/species.h"

#include "util.h"

#ifdef DEBUG
#include <assert.h>
#include <stdio.h>
#endif

/**********
 * Tables *
 **********/

// Element category counts:

static rngtable N_AIR_ELEMENTS = {
  .size = 3,
  .values = { 1, 2, 3 },
  .weights = { 1, 2, 2 }
};

static rngtable N_WATER_ELEMENTS = {
  .size = 2,
  .values = { 2, 3 },
  .weights = { 4, 3 }
};

static rngtable N_LIFE_ELEMENTS = {
  .size = 2,
  .values = { 3, 4 },
  .weights = { 4, 1 }
};

static rngtable N_STONE_ELEMENTS = {
  .size = 3,
  .values = { 3, 4, 5 },
  .weights = { 2, 3, 2 }
};

static rngtable N_METAL_ELEMENTS = {
  .size = 3,
  .values = { 4, 5, 6 },
  .weights = { 2, 3, 3 }
};

static rngtable N_RARE_ELEMENTS = {
  .size = 6,
  .values = { 7, 8, 9, 10, 11, 12 },
  .weights = { 9, 10, 11, 10, 9, 9 }
};

// Element category overlaps:

static rngtable O_AIR_WATER = {
  .size = 2,
  .values = { 0, 1 },
  .weights = { 2, 3 }
};

static rngtable O_AIR_WATER_LIFE = {
  .size = 2,
  .values = { 1, 2 },
  .weights = { 3, 2 }
};

static rngtable O_LIFE_STONE = {
  .size = 3,
  .values = { 0, 1, 2 },
  .weights = { 3, 5, 2 }
};

static rngtable O_LIFE_METAL = {
  .size = 2,
  .values = { 0, 1 },
  .weights = { 4, 3 }
};

static rngtable O_STONE_METAL = {
  .size = 3,
  .values = { 0, 1, 2 },
  .weights = { 2, 3, 2 }
};

// Element frequency distributions by category:

static rngtable STONE_FREQ_DIST = {
  .size = 2,
  .values = { EL_FREQ_COMMON, EL_FREQ_UNCOMMON },
  .weights = { 3, 1 }
};

static rngtable METAL_FREQ_DIST = {
  .size = 3,
  .values = { EL_FREQ_COMMON, EL_FREQ_UNCOMMON, EL_FREQ_RARE },
  .weights = { 10, 70, 20 }
};

static rngtable RARE_FREQ_DIST = {
  .size = 2,
  .values = { EL_FREQ_UNCOMMON, EL_FREQ_RARE },
  .weights = { 1, 4 }
};

// Tables for various element properties:

static rngtable PH_DISTRIBUTION = {
  .size = 3,
  .values = { PHC_ACIDIC, PHC_NEUTRAL, PHC_BASIC },
  .weights = { 1, 3, 1 },
};

static rngtable LIFE_SOLUBILITY_DISTRIBUTION = {
  .size = 3,
  .values = {
    SOLUBILITY_SOLUBLE,
    SOLUBILITY_SLIGHTLY_SOLUBLE,
    SOLUBILITY_INSOLUBLE
  },
  .weights = { 7, 5, 1 }
};

static rngtable METAL_SOLUBILITY_DISTRIBUTION = {
  .size = 2,
  .values = { SOLUBILITY_SLIGHTLY_SOLUBLE, SOLUBILITY_INSOLUBLE },
  .weights = { 4, 1 }
};

static rngtable GENERAL_SOLUBILITY_DISTRIBUTION = {
  .size = 3,
  .values = {
    SOLUBILITY_SOLUBLE,
    SOLUBILITY_SLIGHTLY_SOLUBLE,
    SOLUBILITY_INSOLUBLE
  },
  .weights = { 12, 8, 5 }
};

/*********************
 * Private Functions *
 *********************/

/*************
 * Functions *
 *************/

// Generates elements for the world.
void generate_elements(world_map *wm) {
  size_t c_air, c_water, c_life, c_stone, c_metal, c_rare;
  size_t o_air_water, o_air_water_life, o_life_stone, o_life_metal,
         o_stone_metal;
  species sp;
  element_species *esp;
  size_t i, j, sofar;
  ptrdiff_t seed = prng(wm->seed + 81811);
  // Figure out how many of each element category we have:
  c_air = (size_t) rt_pick_result(N_AIR_ELEMENTS, seed);
  seed = prng(seed);
  c_water = (size_t) rt_pick_result(N_WATER_ELEMENTS, seed);
  seed = prng(seed);
  c_life = (size_t) rt_pick_result(N_LIFE_ELEMENTS, seed);
  seed = prng(seed);
  c_stone = (size_t) rt_pick_result(N_STONE_ELEMENTS, seed);
  seed = prng(seed);
  c_metal = (size_t) rt_pick_result(N_METAL_ELEMENTS, seed);
  seed = prng(seed);
  c_rare = (size_t) rt_pick_result(N_RARE_ELEMENTS, seed);
  seed = prng(seed);

  // And decide on the overlap between categories:
  o_air_water = (size_t) rt_pick_result(O_AIR_WATER, seed);
  seed = prng(seed);
  if (c_air < o_air_water) { o_air_water = c_air; }
  if (c_water < o_air_water) { o_air_water = c_water; }

  o_air_water_life = (size_t) rt_pick_result(O_AIR_WATER_LIFE, seed);
  seed = prng(seed);
  if (o_air_water_life > (c_air + c_water - o_air_water)) {
    o_air_water_life = (c_air + c_water - o_air_water);
  }
  if (c_life < o_air_water_life) { o_air_water_life = c_life; }

  o_life_stone = (size_t) rt_pick_result(O_LIFE_STONE, seed);
  seed = prng(seed);
  if (c_life < o_life_stone) { o_life_stone = c_life; }
  if (c_stone < o_life_stone) { o_life_stone = c_stone; }
  o_life_metal = (size_t) rt_pick_result(O_LIFE_METAL, seed);
  seed = prng(seed);
  if (c_life < o_life_metal) { o_life_metal = c_life; }
  if (c_metal < o_life_metal) { o_life_metal = c_metal; }

  o_stone_metal = (size_t) rt_pick_result(O_STONE_METAL, seed);
  seed = prng(seed);
  if (c_stone < o_stone_metal) { o_stone_metal = c_stone; }
  if (c_metal < o_stone_metal) { o_stone_metal = c_metal; }
  if (c_metal < o_life_metal + o_stone_metal) {
    // This shouldn't be possible, but we'll include it just in case the
    // constants change. We're fine with inflating the metals count
    c_metal = o_life_metal + o_stone_metal;
  }

  // Now start generating elements:

  // The air elements:
  for (i = 0; i < c_air; ++i) {
    sp = create_element_species();
    esp = get_element_species(sp);
    esp->categories = EL_CATEGORY_AIR;
    esp->frequency = EL_FREQ_UBIQUITOUS;
    l_append_element(wm->air_elements, (void*) esp);
    l_append_element(wm->all_elements, (void*) esp);
  }
  // The water-air overlap elements:
  for (i = 0; i < o_air_water; ++i) {
#ifdef DEBUG
    assert(i < l_get_length(wm->air_elements));
#endif
    esp = (element_species*) l_get_item(wm->air_elements, i);
    esp->categories &= EL_CATEGORY_WATER;
    l_append_element(wm->water_elements, (void*) esp);
  }
  // Water elements beyond the overlap with air:
  for (; i < c_water; ++i) {
    sp = create_element_species();
    esp = get_element_species(sp);
    esp->categories = EL_CATEGORY_WATER;
    esp->frequency = EL_FREQ_UBIQUITOUS;
    l_append_element(wm->water_elements, (void*) esp);
    l_append_element(wm->all_elements, (void*) esp);
  }

  // Shuffle our all elements list:
  l_shuffle(wm->all_elements, seed);
  seed = prng(seed);

  // Life elements which overlap with air and/or water (no specific control):
  for (i = 0; i < o_air_water_life; ++i) {
#ifdef DEBUG
    assert(i < l_get_length(wm->all_elements));
#endif
    esp = (element_species*) l_get_item(wm->all_elements, i);
    esp->categories &= EL_CATEGORY_LIFE;
    l_append_element(wm->life_elements, (void*) esp);
  }
  // Extra life elements:
  for (; i < c_life; ++i) {
    sp = create_element_species();
    esp = get_element_species(sp);
    esp->categories = EL_CATEGORY_LIFE;
    esp->frequency = EL_FREQ_COMMON;
    l_append_element(wm->life_elements, (void*) esp);
    l_append_element(wm->all_elements, (void*) esp);
  }

  // Shuffle our life elements list:
  l_shuffle(wm->life_elements, seed);
  seed = prng(seed);

  // Stone elements that overlap with life elements:
  for (i = 0; i < o_life_stone; ++i) {
#ifdef DEBUG
    assert(i < l_get_length(wm->life_elements));
#endif
    esp = (element_species*) l_get_item(wm->life_elements, i);
    esp->categories &= EL_CATEGORY_STONE;
    l_append_element(wm->stone_elements, (void*) esp);
  }
  // Extra stone elements:
  for (; i < c_stone; ++i) {
    sp = create_element_species();
    esp = get_element_species(sp);
    esp->categories = EL_CATEGORY_STONE;
    esp->frequency = (element_frequency) rt_pick_result(STONE_FREQ_DIST, seed);
    seed = prng(seed);
    l_append_element(wm->stone_elements, (void*) esp);
    l_append_element(wm->all_elements, (void*) esp);
  }

  // Note: Metal overlap is a bit noisy as the joint life/stone/metal overlap
  // isn't fully regulated. In other words, actual metal/life overlap could be
  // as high as o_life_stone + min(o_life_stone, o_life_metal) if the metal
  // elements taken from the stone pool for stone/metal overlap happen to be
  // stone/life overlap elements.

  // Shuffle our life elements list again and our stone elements list:
  l_shuffle(wm->life_elements, seed);
  seed = prng(seed);
  l_shuffle(wm->stone_elements, seed);
  seed = prng(seed);

  // Metal elements that overlap with life elements:
  for (i = 0; i < o_life_metal; ++i) {
#ifdef DEBUG
    assert(i < l_get_length(wm->life_elements));
#endif
    esp = (element_species*) l_get_item(wm->life_elements, i);
    esp->categories &= EL_CATEGORY_METAL;
    l_append_element(wm->metal_elements, (void*) esp);
  }
  sofar = i;
  // Metal elements that overlap with stone elements (might also overlap with
  // life elements creating extra life/metal overlap although we try to avoid
  // this).
  j = 0;
  for (i = 0; i < o_stone_metal; ++i) {
#ifdef DEBUG
    assert(j < l_get_length(wm->stone_elements));
#endif
    esp = (element_species*) l_get_item(wm->stone_elements, j);
    if (
      el_is_member(esp, EL_CATEGORY_LIFE)
   && ( (c_stone - j) > (o_stone_metal - i) )
    ) {
#ifdef DEBUG
      if (c_stone < j || o_stone_metal < i) {
        // These are underflow conditions that theoretically shouldn't be
        // possible...
        fprintf(stderr, "Warning: Underflow during element generation!\n");
      }
#endif
      // In this case we skip this stone element to avoid the extra life/metal
      // overlap. j is advanced (see below), but i is not. An infinite loop
      // shouldn't be possible, because eventually c_stone - j would reach 0.
      i -= 1;
    } else {
      esp->categories &= EL_CATEGORY_METAL;
      l_append_element(wm->metal_elements, (void*) esp);
    }
    j += 1;
  }
  i += sofar;
  // Extra metal elements:
  for (; i < c_metal; ++i) {
    sp = create_element_species();
    esp = get_element_species(sp);
    esp->categories = EL_CATEGORY_METAL;
    esp->frequency = (element_frequency) rt_pick_result(METAL_FREQ_DIST, seed);
    seed = prng(seed);
    l_append_element(wm->metal_elements, (void*) esp);
    l_append_element(wm->all_elements, (void*) esp);
  }

  // Rare elements:
  for (i = 0; i < c_rare; ++i) {
    sp = create_element_species();
    esp = get_element_species(sp);
    esp->categories = EL_CATEGORY_RARE;
    esp->frequency = (element_frequency) rt_pick_result(RARE_FREQ_DIST, seed);
    seed = prng(seed);
    l_append_element(wm->rare_elements, (void*) esp);
    l_append_element(wm->all_elements, (void*) esp);
  }

  // Fill out element information now that category and frequency information
  // has been decided:
  for (i = 0; i < l_get_length(wm->all_elements); ++i) {
    fill_out_element((element_species*) l_get_item(wm->all_elements, i), seed);
    seed = prng(seed + i);
  }
}


// Creates a new element.
void fill_out_element(element_species *esp, ptrdiff_t seed) {
  float x;
  // for building colors
  float hue, sat, val;
  // pH tendency
  pH_category phc = (pH_category) rt_pick_result(PH_DISTRIBUTION, seed);
  seed = prng(seed);
  switch (phc) {
    default:
    case PHC_NEUTRAL:
      esp->pH_tendency = randf_pnorm(seed, 6.0, 8.0) * 0.9;
      seed = prng(seed);
      esp->pH_tendency += randf(seed, 4.0, 10.0) * 0.1;
      seed = prng(seed);
      break;

    case PHC_ACIDIC:
      esp->pH_tendency = randf_pnorm(seed, 3.0, 7.0) * 0.5;
      seed = prng(seed);
      esp->pH_tendency += randf(seed, 0.0, 7.0) * 0.5;
      seed = prng(seed);
      break;

    case PHC_BASIC:
      esp->pH_tendency = randf_pnorm(seed, 7.0, 11.0) * 0.5;
      seed = prng(seed);
      esp->pH_tendency += randf(seed, 7.0, 14.0) * 0.5;
      seed = prng(seed);
      break;
  }

  // solubility
  if (el_is_member(esp, EL_CATEGORY_WATER)) {
    esp->solubility = SOLUBILITY_SOLUBLE;
  } else if (el_is_member(esp, EL_CATEGORY_LIFE)) {
    esp->solubility = (solubility) rt_pick_result(
      LIFE_SOLUBILITY_DISTRIBUTION,
      seed
    );
    seed = prng(seed);
  } else if (el_is_member(esp, EL_CATEGORY_METAL)) {
    esp->solubility = (solubility) rt_pick_result(
      METAL_SOLUBILITY_DISTRIBUTION,
      seed
    );
    seed = prng(seed);
  } else {
    esp->solubility = (solubility) rt_pick_result(
      GENERAL_SOLUBILITY_DISTRIBUTION,
      seed
    );
    seed = prng(seed);
  }

  // corrosion resistance
  switch (esp->solubility) {
    case SOLUBILITY_SOLUBLE:
      esp->corrosion_resistance = randi(
        seed,
        MIN_SOLUBLE_CORROSION_RESISTANCE,
        MAX_SOLUBLE_CORROSION_RESISTANCE
      );
      seed = prng(seed);
      break;
    default:
    case SOLUBILITY_SLIGHTLY_SOLUBLE:
      esp->corrosion_resistance = randi(
        seed,
        MIN_SLIGHTLY_CORROSION_RESISTANCE,
        MAX_SLIGHTLY_CORROSION_RESISTANCE
      );
      seed = prng(seed);
      break;
    case SOLUBILITY_INSOLUBLE:
      esp->corrosion_resistance = randi(
        seed,
        MIN_INSOLUBLE_CORROSION_RESISTANCE,
        MAX_INSOLUBLE_CORROSION_RESISTANCE
      );
      seed = prng(seed);
      break;
  }

  // TODO: HERE and below
  // TODO: distributions based on categories...
  esp->stone_density_tendency = ptrf(seed);
  seed = prng(seed);
  esp->stone_specific_heat_tendency = ptrf(seed);
  seed = prng(seed);
  esp->stone_transition_temp_tendency = ptrf(seed);
  seed = prng(seed);
  esp->stone_plasticity_tendency = ptrf(seed);
  seed = prng(seed);
  esp->stone_hardness_tendency = ptrf(seed);
  seed = prng(seed);
  esp->stone_cohesion_tendency = ptrf(seed);
  seed = prng(seed);
  esp->stone_light_dark_tendency = ptrf(seed);
  seed = prng(seed);

  // TODO: Constrain these a bit.
  hue = randf(seed, 0.0, 1.0);
  seed = prng(seed);
  sat = randf(seed, 0.0, 1.0);
  seed = prng(seed);
  val = randf(seed, 0.0, 1.0);
  seed = prng(seed);
  esp->stone_chroma = float_color(hue, sat, val, 1.0);

  // TODO: Constrain these a bit.
  hue = randf(seed, 0.0, 1.0);
  seed = prng(seed);
  sat = randf(seed, 0.0, 1.0);
  seed = prng(seed);
  val = randf(seed, 0.0, 1.0);
  seed = prng(seed);
  esp->stone_oxide_chroma = float_color(hue, sat, val, 1.0);

  // TODO: Constrain these a bit.
  hue = randf(seed, 0.0, 1.0);
  seed = prng(seed);
  sat = randf(seed, 0.0, 1.0);
  seed = prng(seed);
  val = randf(seed, 0.0, 1.0);
  seed = prng(seed);
  esp->stone_tint_chroma = float_color(hue, sat, val, 1.0);

  // Tendencies when forming metals and alloys:
  // TODO: distributions based on categories...
  esp->metal_luster_tendency = ptrf(seed);
  seed = prng(seed);
  esp->metal_hardness_tendency = ptrf(seed);
  seed = prng(seed);
  esp->metal_plasticity_tendency = ptrf(seed);
  seed = prng(seed);
  esp->metal_light_dark_tendency = ptrf(seed);
  seed = prng(seed);

  // TODO: Constrain these a bit.
  hue = randf(seed, 0.0, 1.0);
  seed = prng(seed);
  sat = randf(seed, 0.0, 1.0);
  seed = prng(seed);
  val = randf(seed, 0.0, 1.0);
  seed = prng(seed);
  esp->metal_chroma = float_color(hue, sat, val, 1.0);

  // TODO: Constrain these a bit.
  hue = randf(seed, 0.0, 1.0);
  seed = prng(seed);
  sat = randf(seed, 0.0, 1.0);
  seed = prng(seed);
  val = randf(seed, 0.0, 1.0);
  seed = prng(seed);
  esp->metal_tint_chroma = float_color(hue, sat, val, 1.0);

  // [-1,1) tendency to synergize in alloys:
  // TODO: distributions based on categories...
  esp->alloy_performance = -1  +  2 * ptrf(seed);
  seed = prng(seed);

  // Biological properties:
  // TODO: distributions based on categories...
  esp->nutrient = randi(seed, 0, 1);
  seed = prng(seed);
  if (!esp->nutrient) {
    esp->poison = randi(seed, 0, 1);
    seed = prng(seed);
  }
}
