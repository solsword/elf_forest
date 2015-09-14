// elements.c
// Element generation.

#include "elements.h"

#include "datatypes/rngtable.h"
#include "world/species.h"

#include "util.h"

/**********
 * Tables *
 **********/

rngtable const ELEMENT_PHASE_DISTRIBUTION = {
  .size = 3,
  .values = {
    (void*) PHASE_SOLID,
    (void*) PHASE_LIQUID,
    (void*) PHASE_GAS
  },
  .weights = { 0.75, 0.2, 0.05 }
}

/*********************
 * Private Functions *
 *********************/

static inline phase pick_phase(
  float percent_done,
  ptrdiff_t *seed,
  size_t *liquid_count,
  size_t *gas_count
) {
    phase = (phase) rt_pick_result(ELEMENT_PHASE_DISTRIBUTION, *seed);
    *seed = prng(*seed);
    if (percent_done > (*liquid_count + 1) / MIN_LIQUID_ELEMENTS) {
      phase = PHASE_LIQUID;
    }
    if (percent_done > (*gas_count + 1) / MIN_GASEOUS_ELEMENTS) {
      phase = PHASE_GAS;
    }
    if (phase == PHASE_LIQUID) {
      *liquid_count += 1;
    } else if (phase == PHASE_GAS) {
      *gas_count += 1;
    }
    return phase;
}

/*************
 * Functions *
 *************/

// Generates elements for the world.
void generate_elements(world_map *wm) {
  size_t i;
  size_t total_count;
  size_t n_total;
  float percent_done;
  size_t liquid_count;
  size_t gas_count;
  ptrdiff_t seed = prng(wm->seed + 81811);
  // TODO: Metals...
  phase phase;
  wm->n_main_elements = rint(MIN_MAIN_ELEMENTS, MAX_MAIN_ELEMENTS);
  wm->n_common_elements = rint(MIN_COMMON_ELEMENTS, MAX_COMMON_ELEMENTS);
  wm->n_uncommon_elements = rint(MIN_UNCOMMON_ELEMENTS, MAX_UNCOMMON_ELEMENTS);
  wm->main_elements = (species*) malloc(wm->n_main_elements*sizeof(species));
  wm->common_elements = (species*) malloc(wm->n_main_elements*sizeof(species));
  wm->uncommon_elements =(species*) malloc(wm->n_main_elements*sizeof(species));
  n_total = (
    wm->n_main_elements
  + wm->n_common_elements
  + wm->n_uncommon_elements
  );

  for(i = 0; i < n_main_elements; ++i) {
    percent_done = total_count / (float) n_total;
    phase = pick_phase(percent_done, &seed, &liquid_count, &gas_count);
    wm->main_elements[i] = create_new_element(
      EL_FREQ_MAIN,
      phase,
    );
    total_count += 1;
  }

  for(i = 0; i < n_common_elements; ++i) {
    percent_done = total_count / (float) n_total;
    phase = pick_phase(percent_done, &seed, &liquid_count, &gas_count);
    wm->common_elements[i] = create_new_element(
      EL_FREQ_COMMON,
      phase,
    );
    total_count += 1;
  }

  for(i = 0; i < n_uncommon_elements; ++i) {
    percent_done = total_count / (float) n_total;
    phase = pick_phase(percent_done, &seed, &liquid_count, &gas_count);
    wm->uncommon_elements[i] = create_new_element(
      EL_FREQ_UNCOMMON,
      phase,
    );
    total_count += 1;
  }
}


// Creates a new element.
species create_new_element(
  element_frequency frequency,
  j
) {
}
