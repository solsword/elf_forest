// elements.c
// Element generation.

#include "elements.h"

#include "datatypes/rngtable.h"
#include "world/species.h"
#include "world/world_map.h"

#include "util.h"

#ifdef DEBUG
#include <assert.h>
#include <stdio.h>
#endif

/***********
 * Globals *
 ***********/

int N_TOTAL_NUTRIENTS = 0;

/**********
 * Tables *
 **********/

// Element category counts:

static rngtable N_AIR_ELEMENTS = {
  .size = 3,
  .values = (void*[]) { (void*) 1, (void*) 2, (void*) 3 },
  .weights = (float[]) { 1, 2, 2 }
};

static rngtable N_WATER_ELEMENTS = {
  .size = 2,
  .values = (void*[]) { (void*) 2, (void*) 3 },
  .weights = (float[]) { 4, 3 }
};

static rngtable N_LIFE_ELEMENTS = {
  .size = 2,
  .values = (void*[]) { (void*) 3, (void*) 4 },
  .weights = (float[]) { 4, 1 }
};

static rngtable N_STONE_ELEMENTS = {
  .size = 3,
  .values = (void*[]) { (void*) 3, (void*) 4, (void*) 5 },
  .weights = (float[]) { 2, 3, 2 }
};

static rngtable N_METAL_ELEMENTS = {
  .size = 3,
  .values = (void*[]) { (void*) 4, (void*) 5, (void*) 6 },
  .weights = (float[]) { 2, 3, 3 }
};

static rngtable N_RARE_ELEMENTS = {
  .size = 6,
  .values = (void*[]) {
    (void*) 7,
    (void*) 8,
    (void*) 9,
    (void*) 10,
    (void*) 11,
    (void*) 12
  },
  .weights = (float[]) { 9, 10, 11, 10, 9, 9 }
};

// Element category overlaps:

static rngtable O_AIR_WATER = {
  .size = 2,
  .values = (void*[]) { (void*) 0, (void*) 1 },
  .weights = (float[]) { 2, 3 }
};

static rngtable O_AIR_WATER_LIFE = {
  .size = 2,
  .values = (void*[]) { (void*) 1, (void*) 2 },
  .weights = (float[]) { 3, 2 }
};

static rngtable O_LIFE_STONE = {
  .size = 3,
  .values = (void*[]) { (void*) 0, (void*) 1, (void*) 2 },
  .weights = (float[]) { 3, 5, 2 }
};

static rngtable O_LIFE_METAL = {
  .size = 2,
  .values = (void*[]) { (void*) 0, (void*) 1 },
  .weights = (float[]) { 4, 3 }
};

static rngtable O_STONE_METAL = {
  .size = 3,
  .values = (void*[]) { (void*) 0, (void*) 1, (void*) 2 },
  .weights = (float[]) { 2, 3, 2 }
};

// Element frequency distributions by category:

static rngtable STONE_FREQ_DIST = {
  .size = 2,
  .values = (void*[]) { (void*) EL_FREQ_COMMON, (void*) EL_FREQ_UNCOMMON },
  .weights = (float[]) { 3, 1 }
};

static rngtable METAL_FREQ_DIST = {
  .size = 3,
  .values = (void*[]) {
    (void*) EL_FREQ_COMMON,
    (void*) EL_FREQ_UNCOMMON,
    (void*) EL_FREQ_RARE
  },
  .weights = (float[]) { 10, 70, 20 }
};

static rngtable RARE_FREQ_DIST = {
  .size = 2,
  .values = (void*[]) { (void*) EL_FREQ_UNCOMMON, (void*) EL_FREQ_RARE },
  .weights = (float[]) { 1, 4 }
};

// Tables for various element properties:

static rngtable PH_DISTRIBUTION = {
  .size = 3,
  .values = (void*[]) {
    (void*) PHC_ACIDIC,
    (void*) PHC_NEUTRAL,
    (void*) PHC_BASIC
  },
  .weights = (float[]) { 1, 3, 1 },
};

static rngtable LIFE_SOLUBILITY_DISTRIBUTION = {
  .size = 3,
  .values = (void*[]) {
    (void*) SOLUBILITY_SOLUBLE,
    (void*) SOLUBILITY_SLIGHTLY_SOLUBLE,
    (void*) SOLUBILITY_INSOLUBLE
  },
  .weights = (float[]) { 7, 5, 1 }
};

static rngtable METAL_SOLUBILITY_DISTRIBUTION = {
  .size = 2,
  .values = (void*[]) {
    (void*) SOLUBILITY_SLIGHTLY_SOLUBLE,
    (void*) SOLUBILITY_INSOLUBLE
  },
  .weights = (float[]) { 4, 1 }
};

static rngtable GENERAL_SOLUBILITY_DISTRIBUTION = {
  .size = 3,
  .values = (void*[]) {
    (void*) SOLUBILITY_SOLUBLE,
    (void*) SOLUBILITY_SLIGHTLY_SOLUBLE,
    (void*) SOLUBILITY_INSOLUBLE
  },
  .weights = (float[]) { 12, 8, 5 }
};

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
  list *possibly_nutritious;
  ptrdiff_t seed = prng(wm->seed + 81811);
  // Figure out how many of each element category we have:
  c_air = (size_t) rt_pick_result(&N_AIR_ELEMENTS, seed);
  seed = prng(seed);
  c_water = (size_t) rt_pick_result(&N_WATER_ELEMENTS, seed);
  seed = prng(seed);
  c_life = (size_t) rt_pick_result(&N_LIFE_ELEMENTS, seed);
  seed = prng(seed);
  c_stone = (size_t) rt_pick_result(&N_STONE_ELEMENTS, seed);
  seed = prng(seed);
  c_metal = (size_t) rt_pick_result(&N_METAL_ELEMENTS, seed);
  seed = prng(seed);
  c_rare = (size_t) rt_pick_result(&N_RARE_ELEMENTS, seed);
  seed = prng(seed);

  // And decide on the overlap between categories:
  o_air_water = (size_t) rt_pick_result(&O_AIR_WATER, seed);
  seed = prng(seed);
  if (c_air < o_air_water) { o_air_water = c_air; }
  if (c_water < o_air_water) { o_air_water = c_water; }

  o_air_water_life = (size_t) rt_pick_result(&O_AIR_WATER_LIFE, seed);
  seed = prng(seed);
  if (o_air_water_life > (c_air + c_water - o_air_water)) {
    o_air_water_life = (c_air + c_water - o_air_water);
  }
  if (c_life < o_air_water_life) { o_air_water_life = c_life; }

  o_life_stone = (size_t) rt_pick_result(&O_LIFE_STONE, seed);
  seed = prng(seed);
  if (c_life < o_life_stone) { o_life_stone = c_life; }
  if (c_stone < o_life_stone) { o_life_stone = c_stone; }
  o_life_metal = (size_t) rt_pick_result(&O_LIFE_METAL, seed);
  seed = prng(seed);
  if (c_life < o_life_metal) { o_life_metal = c_life; }
  if (c_metal < o_life_metal) { o_life_metal = c_metal; }

  o_stone_metal = (size_t) rt_pick_result(&O_STONE_METAL, seed);
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
    esp->frequency = (element_frequency) rt_pick_result(&STONE_FREQ_DIST, seed);
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
    esp->frequency = (element_frequency) rt_pick_result(&METAL_FREQ_DIST, seed);
    seed = prng(seed);
    l_append_element(wm->metal_elements, (void*) esp);
    l_append_element(wm->all_elements, (void*) esp);
  }

  // Rare elements:
  for (i = 0; i < c_rare; ++i) {
    sp = create_element_species();
    esp = get_element_species(sp);
    esp->categories = EL_CATEGORY_RARE;
    esp->frequency = (element_frequency) rt_pick_result(&RARE_FREQ_DIST, seed);
    seed = prng(seed);
    l_append_element(wm->rare_elements, (void*) esp);
    l_append_element(wm->all_elements, (void*) esp);
  }

  // Fill out element information now that category and frequency information
  // has been decided:
  possibly_nutritious = create_list();
  for (i = 0; i < l_get_length(wm->all_elements); ++i) {
    esp = (element_species*) l_get_item(wm->all_elements, i);
    if (
      !el_is_member_of_any(
        esp,
        EL_CATEGORY_AIR | EL_CATEGORY_WATER | EL_CATEGORY_LIFE
      )
    ) {
      l_append_element(possibly_nutritious, esp);
    }
    fill_out_element(esp, seed);
    seed = prng(seed + i);
  }

  // Add nutrition properties to hit our quotas:
  size_t n_ex_plant_critical, n_ex_animal_critical;
  size_t n_plant_beneficial, n_animal_beneficial;
  size_t n_plant_detrimental, n_animal_detrimental;
  size_t n_plant_poisons, n_animal_poisons;
  size_t total_plant_nutrients, total_animal_nutrients;

  n_ex_plant_critical = randi(
    seed,
    MIN_EXTRA_PLANT_CRITICAL_NUTRIENTS,
    MAX_EXTRA_PLANT_CRITICAL_NUTRIENTS
  );
  seed = prng(seed);
  n_ex_animal_critical = randi(
    seed,
    MIN_EXTRA_ANIMAL_CRITICAL_NUTRIENTS,
    MAX_EXTRA_ANIMAL_CRITICAL_NUTRIENTS
  );
  seed = prng(seed);

  n_plant_beneficial = randi(
    seed,
    MIN_PLANT_BENEFICIAL_NUTRIENTS,
    MAX_PLANT_BENEFICIAL_NUTRIENTS
  );

  seed = prng(seed);
  n_animal_beneficial = randi(
    seed,
    MIN_ANIMAL_BENEFICIAL_NUTRIENTS,
    MAX_ANIMAL_BENEFICIAL_NUTRIENTS
  );
  seed = prng(seed);

  n_plant_detrimental = randi(
    seed,
    MIN_PLANT_DETRIMENTAL_NUTRIENTS,
    MAX_PLANT_DETRIMENTAL_NUTRIENTS
  );
  seed = prng(seed);
  n_animal_detrimental = randi(
    seed,
    MIN_ANIMAL_DETRIMENTAL_NUTRIENTS,
    MAX_ANIMAL_DETRIMENTAL_NUTRIENTS
  );
  seed = prng(seed);

  n_plant_poisons = randi(
    seed,
    MIN_PLANT_POISONS,
    MAX_PLANT_POISONS
  );
  seed = prng(seed);
  n_animal_poisons = randi(
    seed,
    MIN_ANIMAL_POISONS,
    MAX_ANIMAL_POISONS
  );
  seed = prng(seed);

  total_plant_nutrients = (
    n_ex_plant_critical
  + n_plant_beneficial
  + n_plant_detrimental
  + n_plant_poisons
  );

  total_animal_nutrients = (
    n_ex_animal_critical
  + n_animal_beneficial
  + n_animal_detrimental
  + n_animal_poisons
  );

  // Limit plant numbers to what's available:

  int try_next = 0;
  size_t last_count = total_plant_nutrients + 1;
  size_t available = l_get_length(possibly_nutritious);
  // If we randomly decided to generate more nutrients than there are elements
  // that could possibly be nutrients (hopefully a rare case) we need to reduce
  // the number of nutrients we're asking for.
  while (
     total_plant_nutrients > available
  && last_count > total_plant_nutrients
  ) {
    last_count = total_plant_nutrients;
    // This code tries to be fair about which counts we reduce (although it
    // doesn't take into account which numbers rolled high/low, which maybe it
    // should). To do so, it loops through all possible things to decrement
    // twice, starting at a different place in the first half every time, so
    // that each time it has different priorities but will always try
    // everything it can.
    switch (try_next) {
      case 0:
      default:
        if (n_plant_detrimental > MIN_PLANT_DETRIMENTAL_NUTRIENTS) {
          n_plant_detrimental -= 1;
          total_plant_nutrients -= 1;
          break;
        } // else flow through
      case 1:
        if (n_plant_beneficial > MIN_PLANT_BENEFICIAL_NUTRIENTS) {
          n_plant_beneficial -= 1;
          total_plant_nutrients -= 1;
          break;
        } // else flow through
      case 2:
        if (n_plant_poisons > MIN_PLANT_POISONS) {
          n_plant_poisons -= 1;
          total_plant_nutrients -= 1;
          break;
        } // else flow through
      case 3:
        if (n_ex_plant_critical > MIN_EXTRA_PLANT_CRITICAL_NUTRIENTS) {
          n_ex_plant_critical -= 1;
          total_plant_nutrients -= 1;
          break;
        } // else flow through
        // Here we try each again so that no matter which cases we skipped,
        // every possibility will be tried at least once.
        if (n_plant_detrimental > MIN_PLANT_DETRIMENTAL_NUTRIENTS) {
          n_plant_detrimental -= 1;
          total_plant_nutrients -= 1;
          break;
        } // else flow through
        if (n_plant_beneficial > MIN_PLANT_BENEFICIAL_NUTRIENTS) {
          n_plant_beneficial -= 1;
          total_plant_nutrients -= 1;
          break;
        } // else flow through
        if (n_plant_poisons > MIN_PLANT_POISONS) {
          n_plant_poisons -= 1;
          total_plant_nutrients -= 1;
          break;
        } // else flow through
        if (n_ex_plant_critical > MIN_EXTRA_PLANT_CRITICAL_NUTRIENTS) {
          n_ex_plant_critical -= 1;
          total_plant_nutrients -= 1;
          break;
        } // else flow through
        // At this point we've exhausted our options, so the last_count check
        // will presumably terminate the while loop.
    }
    try_next += 1;
    if (try_next > 3) { try_next = 0; }
  }

  if (total_plant_nutrients > available) {
    // This can only happen if we've pushed all target nutrient counts to their
    // minimums and we're still above the total number of pure-stone +
    // pure-metal + pure-rare elements that were generated. In this case, we'll
    // have to live without hitting our minimum nutrient targets.
    n_ex_plant_critical = 0;
    n_plant_beneficial = 0;
    n_plant_detrimental = 0;
    n_plant_poisons = 0;

    if (available >= 1) { n_ex_plant_critical += 1; }
    if (available >= 2) { n_plant_beneficial += 1; }
    if (available >= 3) { n_plant_poisons += 1; }
    if (available >= 4) { n_plant_detrimental += 1; }

    if (available >= 5) { n_plant_beneficial += 1; }
    if (available >= 6) { n_plant_detrimental += 1; }
    if (available >= 7) { n_plant_poisons += 1; }
  }

  // Limit animal numbers to what's available:
  try_next = 0;
  last_count = total_animal_nutrients + 1;
  // See above...
  while (
     total_animal_nutrients > available
  && last_count > total_animal_nutrients
  ) {
    last_count = total_animal_nutrients;
    // See above...
    switch (try_next) {
      case 0:
      default:
        if (n_animal_detrimental > MIN_ANIMAL_DETRIMENTAL_NUTRIENTS) {
          n_animal_detrimental -= 1;
          total_animal_nutrients -= 1;
          break;
        } // else flow through
      case 1:
        if (n_animal_beneficial > MIN_ANIMAL_BENEFICIAL_NUTRIENTS) {
          n_animal_beneficial -= 1;
          total_animal_nutrients -= 1;
          break;
        } // else flow through
      case 2:
        if (n_animal_poisons > MIN_ANIMAL_POISONS) {
          n_animal_poisons -= 1;
          total_animal_nutrients -= 1;
          break;
        } // else flow through
      case 3:
        if (n_ex_animal_critical > MIN_EXTRA_ANIMAL_CRITICAL_NUTRIENTS) {
          n_ex_animal_critical -= 1;
          total_animal_nutrients -= 1;
          break;
        } // else flow through
        // Here we try each again so that no matter which cases we skipped,
        // every possibility will be tried at least once.
        if (n_animal_detrimental > MIN_ANIMAL_DETRIMENTAL_NUTRIENTS) {
          n_animal_detrimental -= 1;
          total_animal_nutrients -= 1;
          break;
        } // else flow through
        if (n_animal_beneficial > MIN_ANIMAL_BENEFICIAL_NUTRIENTS) {
          n_animal_beneficial -= 1;
          total_animal_nutrients -= 1;
          break;
        } // else flow through
        if (n_animal_poisons > MIN_ANIMAL_POISONS) {
          n_animal_poisons -= 1;
          total_animal_nutrients -= 1;
          break;
        } // else flow through
        if (n_ex_animal_critical > MIN_EXTRA_ANIMAL_CRITICAL_NUTRIENTS) {
          n_ex_animal_critical -= 1;
          total_animal_nutrients -= 1;
          break;
        } // else flow through
        // At this point we've exhausted our options, so the last_count check
        // will presumably terminate the while loop.
    }
    try_next += 1;
    if (try_next > 3) { try_next = 0; }
  }

  if (total_animal_nutrients > available) {
    // This can only happen if we've pushed all target nutrient counts to their
    // minimums and we're still above the total number of pure-stone +
    // pure-metal + pure-rare elements that were generated. In this case, we'll
    // have to live without hitting our minimum nutrient targets.
    n_ex_animal_critical = 0;
    n_animal_beneficial = 0;
    n_animal_detrimental = 0;
    n_animal_poisons = 0;

    if (available >= 1) { n_ex_animal_critical += 1; }
    if (available >= 2) { n_animal_beneficial += 1; }
    if (available >= 3) { n_animal_poisons += 1; }
    if (available >= 4) { n_animal_detrimental += 1; }

    if (available >= 5) { n_animal_beneficial += 1; }
    if (available >= 6) { n_animal_detrimental += 1; }
    if (available >= 7) { n_animal_poisons += 1; }
  }

  // Shuffle our list and then assign plant nutrients:
  l_shuffle(possibly_nutritious, seed);
  seed = prng(seed);

  total_plant_nutrients = n_ex_plant_critical;
  sofar = 0;
  for (i = 0; i < total_plant_nutrients; ++i) {
    esp = (element_species*) l_get_item(possibly_nutritious, i);
    if (ptrf(seed) < 0.7) {
      esp->plant_nutrition = NT_CAT_CRITICAL;
    } else {
      esp->plant_nutrition = NT_CAT_CRITICAL_CAN_OVERDOSE;
    }
    seed = prng(seed);
    if (!l_contains(wm->all_nutrients, (void*) esp)) {
      l_append_element(wm->all_nutrients, (void*) esp);
    }
  }
  total_plant_nutrients += n_plant_beneficial;
  for (; i < total_plant_nutrients; ++i) {
    esp = (element_species*) l_get_item(possibly_nutritious, i);
    if (ptrf(seed) < 0.5) {
      esp->plant_nutrition = NT_CAT_BENEFICIAL;
    } else {
      esp->plant_nutrition = NT_CAT_BENEFICIAL_CAN_OVERDOSE;
    }
    seed = prng(seed);
    if (!l_contains(wm->all_nutrients, (void*) esp)) {
      l_append_element(wm->all_nutrients, (void*) esp);
    }
  }
  total_plant_nutrients += n_plant_detrimental;
  for (; i < total_plant_nutrients; ++i) {
    esp = (element_species*) l_get_item(possibly_nutritious, i);
    esp->plant_nutrition = NT_CAT_DETRIMENTAL;
    if (!l_contains(wm->all_nutrients, (void*) esp)) {
      l_append_element(wm->all_nutrients, (void*) esp);
    }
  }
  total_plant_nutrients += n_plant_poisons;
  for (; i < total_plant_nutrients; ++i) {
    esp = (element_species*) l_get_item(possibly_nutritious, i);
    esp->plant_nutrition = NT_CAT_POISONOUS;
    if (!l_contains(wm->all_nutrients, (void*) esp)) {
      l_append_element(wm->all_nutrients, (void*) esp);
    }
  }

  // Re-shuffle the same list and assign animal nutrients (animal/plant
  // nutrient overlap is thus random).
  // TODO: Avoid critical plant nutrients which are poisonous to animals!
  l_shuffle(possibly_nutritious, seed);
  seed = prng(seed);

  total_animal_nutrients = n_ex_animal_critical;
  for (i = 0; i < total_animal_nutrients; ++i) {
    esp = (element_species*) l_get_item(possibly_nutritious, i);
    if (ptrf(seed) < 0.7) {
      esp->animal_nutrition = NT_CAT_CRITICAL;
    } else {
      esp->animal_nutrition = NT_CAT_CRITICAL_CAN_OVERDOSE;
    }
    seed = prng(seed);
    if (!l_contains(wm->all_nutrients, (void*) esp)) {
      l_append_element(wm->all_nutrients, (void*) esp);
    }
  }
  total_animal_nutrients += n_animal_beneficial;
  for (; i < total_animal_nutrients; ++i) {
    esp = (element_species*) l_get_item(possibly_nutritious, i);
    if (ptrf(seed) < 0.5) {
      esp->animal_nutrition = NT_CAT_BENEFICIAL;
    } else {
      esp->animal_nutrition = NT_CAT_BENEFICIAL_CAN_OVERDOSE;
    }
    seed = prng(seed);
    if (!l_contains(wm->all_nutrients, (void*) esp)) {
      l_append_element(wm->all_nutrients, (void*) esp);
    }
  }
  total_animal_nutrients += n_animal_detrimental;
  for (; i < total_animal_nutrients; ++i) {
    esp = (element_species*) l_get_item(possibly_nutritious, i);
    esp->animal_nutrition = NT_CAT_DETRIMENTAL;
    if (!l_contains(wm->all_nutrients, (void*) esp)) {
      l_append_element(wm->all_nutrients, (void*) esp);
    }
  }
  total_animal_nutrients += n_animal_poisons;
  for (; i < total_animal_nutrients; ++i) {
    esp = (element_species*) l_get_item(possibly_nutritious, i);
    esp->animal_nutrition = NT_CAT_POISONOUS;
    if (!l_contains(wm->all_nutrients, (void*) esp)) {
      l_append_element(wm->all_nutrients, (void*) esp);
    }
  }

  // Set our global total nutrient count:
  N_TOTAL_NUTRIENTS = l_get_length(wm->all_nutrients);

  cleanup_list(possibly_nutritious);
}


// Creates a new element.
void fill_out_element(element_species *esp, ptrdiff_t seed) {
  // pH tendency
  pH_category phc = (pH_category) rt_pick_result(&PH_DISTRIBUTION, seed);
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
      &LIFE_SOLUBILITY_DISTRIBUTION,
      seed
    );
    seed = prng(seed);
  } else if (el_is_member(esp, EL_CATEGORY_METAL)) {
    esp->solubility = (solubility) rt_pick_result(
      &METAL_SOLUBILITY_DISTRIBUTION,
      seed
    );
    seed = prng(seed);
  } else {
    esp->solubility = (solubility) rt_pick_result(
      &GENERAL_SOLUBILITY_DISTRIBUTION,
      seed
    );
    seed = prng(seed);
  }

  // corrosion resistance
  switch (esp->solubility) {
    case SOLUBILITY_SOLUBLE:
      esp->corrosion_resistance = randf(
        seed,
        MIN_SOLUBLE_CORROSION_RESISTANCE,
        MAX_SOLUBLE_CORROSION_RESISTANCE
      );
      seed = prng(seed);
      break;
    default:
    case SOLUBILITY_SLIGHTLY_SOLUBLE:
      esp->corrosion_resistance = randf(
        seed,
        MIN_SLIGHTLY_CORROSION_RESISTANCE,
        MAX_SLIGHTLY_CORROSION_RESISTANCE
      );
      seed = prng(seed);
      break;
    case SOLUBILITY_INSOLUBLE:
      esp->corrosion_resistance = randf(
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

  // TODO: Constrain these a bit.
  esp->stone_brightness_tendency = ptrf(seed);
  seed = prng(seed);
  esp->stone_chroma = randf(seed, 0, 2*M_PI);
  seed = prng(seed);
  esp->stone_oxide_chroma = randf(seed, 0, 2*M_PI);
  seed = prng(seed);
  esp->stone_tint_chroma = randf(seed, 0, 2*M_PI);
  seed = prng(seed);

  // Tendencies when forming metals and alloys:
  // TODO: distributions based on categories...
  esp->metal_luster_tendency = ptrf(seed);
  seed = prng(seed);
  esp->metal_hardness_tendency = ptrf(seed);
  seed = prng(seed);
  esp->metal_plasticity_tendency = ptrf(seed);
  seed = prng(seed);

  // TODO: Constrain these a bit.
  esp->metal_brightness_tendency = ptrf(seed);
  seed = prng(seed);
  esp->metal_chroma = randf(seed, 0, 2*M_PI);
  seed = prng(seed);
  // TODO: Metal oxide chroma!
  esp->metal_tint_chroma = randf(seed, 0, 2*M_PI);
  seed = prng(seed);

  // [-1,1) tendency to synergize in alloys:
  // TODO: distributions based on categories...
  esp->alloy_performance = -1  +  2 * ptrf(seed);
  seed = prng(seed);

  // Biological properties:
  // TODO: distributions based on categories...
  esp->plant_nutrition = NT_CAT_NONE;
  esp->animal_nutrition = NT_CAT_NONE;

  // Life elements are critical nutrients:
  if (el_is_member(esp, EL_CATEGORY_LIFE)) {
    if (ptrf(seed) < 0.85) {
      esp->plant_nutrition = NT_CAT_CRITICAL;
    } else {
      esp->plant_nutrition = NT_CAT_CRITICAL_CAN_OVERDOSE;
    }
    seed = prng(seed);
    if (ptrf(seed) < 0.85) {
      esp->animal_nutrition = NT_CAT_CRITICAL;
    } else {
      esp->animal_nutrition = NT_CAT_CRITICAL_CAN_OVERDOSE;
    }
    seed = prng(seed);
  }
}
