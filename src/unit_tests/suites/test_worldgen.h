#undef TEST_SUITE_NAME
#undef TEST_SUITE_TESTS
#define TEST_SUITE_NAME worldgen
#define TEST_SUITE_TESTS { \
    &test_create_world, \
    &test_load_chunk, \
    &test_load_stacked_chunks, \
    NULL, \
  }

#ifndef TEST_WORLDGEN_H
#define TEST_WORLDGEN_H

#include <stdio.h>

#include "gen/worldgen.h"
#include "jobs/jobs.h"
#include "data/data.h"
#include "world/species.h"
#include "world/entities.h"

#include "unit_tests/test_suite.h"

/********************
 * Shared Variables *
*********************/

world_map *TEST_WORLD = NULL;

/******************
 * Test Functions *
 ******************/

size_t test_create_world(void) {
  setup_data();
  setup_entities();
  setup_species();
  setup_terrain_gen(1821271);
  TEST_WORLD = create_world_map(178352, 64, 64);
  printf("Generating test world geology...\n");
  generate_geology(TEST_WORLD);
  printf("  ...done.\n");
  // HACK:
  THE_WORLD = TEST_WORLD;
  return 0;
}

size_t test_load_chunk(void) {
  global_chunk_pos glcpos = { .x = 5, .y = 5, .z = 5 };
  mark_for_loading(&glcpos, LOD_BASE);
  tick_load_chunks(&glcpos);
  return 0;
}

size_t test_load_stacked_chunks(void) {
  global_chunk_pos glcpos = { .x = 0, .y = 0, .z = 0 };
  mark_for_loading(&glcpos, LOD_BASE);
  glcpos.z += 1;
  mark_for_loading(&glcpos, LOD_BASE);
  tick_load_chunks(&glcpos);
  return 0;
}

#endif //ifndef TEST_WORLDGEN_H
