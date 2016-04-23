// rngtable.c
// A table of values with weights for random selection.

#ifdef DEBUG
  #include <stdio.h>
#endif

#include "util.h"

#include "rngtable.h"

/******************************
 * Constructors & Destructors *
 ******************************/

rngtable *create_rngtable(size_t size) {
  rngtable *result = (rngtable*) malloc(sizeof(rngtable));
  result->size = size;
  result->values = (void**) calloc(size, sizeof(void*));
  result->weights = (float*) calloc(size, sizeof(float));
  return result;
}

rngtable* copy_rngtable(rngtable *src) {
  rngtable *result = create_rngtable(src->size);
  size_t i;
  for (i = 0; i < src->size; ++i) {
    result->values[i] = src->values[i];
    result->weights[i] = src->weights[i];
  }
  return result;
}

CLEANUP_IMPL(rngtable) {
  free(doomed->values);
  free(doomed->weights);
  free(doomed);
}

/*************
 * Functions *
 *************/

void* rt_pick_result(rngtable const * const t, ptrdiff_t seed) {
  float total_weight = 0;
  float choice = 0;
  size_t i;
  for (i = 0; i < t->size; ++i) {
    total_weight += t->weights[i];
  }
  if (total_weight == 0) {
    return NULL;
  }
  choice = ptrf(prng(seed + 4680514)) * total_weight;
  total_weight = 0;
  for (i = 0; i < t->size; ++i) {
    choice -= t->weights[i];
    if (choice < 0) {
      return t->values[i];
    }
  }
  // Shouldn't be possible to end up here...
#ifdef DEBUG
  fprintf(
    stderr,
    "WARNING: Ran out of table when choosing a random entry in rngtable.\n"
  );
#endif
  return NULL;
}

void* rt_pick_filtered_result(
  rngtable const * const t,
  void *arg,
  int (*filter)(void*, void*),
  ptrdiff_t seed
) {
  float total_weight = 0;
  float choice = 0;
  size_t i;
  for (i = 0; i < t->size; ++i) {
    if (filter(t->values[i], arg)) {
      total_weight += t->weights[i];
    }
  }
  if (total_weight == 0) {
    return NULL;
  }
  choice = ptrf(prng(seed + 4680514)) * total_weight;
  total_weight = 0;
  for (i = 0; i < t->size; ++i) {
    if (filter(t->values[i], arg)) {
      choice -= t->weights[i];
      if (choice < 0) {
        return t->values[i];
      }
    }
  }
  // Shouldn't be possible to end up here...
#ifdef DEBUG
  fprintf(
    stderr,
    "WARNING: Ran out of table when choosing a random entry in rngtable.\n"
  );
#endif
  return NULL;
}
