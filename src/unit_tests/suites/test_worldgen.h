#undef TEST_SUITE_NAME
#undef TEST_SUITE_TESTS
#define TEST_SUITE_NAME worldgen
#define TEST_SUITE_TESTS { \
    &test_create_world, \
    &test_load_chunk, \
    &test_load_stacked_chunks, \
    &test_job_gencolumn, \
    NULL, \
  }

#ifndef TEST_WORLDGEN_H
#define TEST_WORLDGEN_H

#include <stdio.h>

#include "gen/worldgen.h"
#include "jobs/jobs.h"
#include "data/data.h"

#include "unit_tests/test_suite.h"

/********************
 * Shared Variables *
*********************/

world_map *TEST_WORLD = NULL;

/******************
 * Test Functions *
 ******************/

size_t test_create_world(void) {
  TEST_WORLD = create_world_map(178352, 64, 64);
  printf("Generating test world geology...\n");
  generate_geology(TEST_WORLD);
  printf("  ...done.\n");
  // HACK:
  THE_WORLD = TEST_WORLD;
  setup_data();
  return 0;
}

size_t test_load_chunk(void) {
  region_chunk_pos rcpos = { .x = 5, .y = 5, .z = 5 };
  mark_for_loading(&rcpos, LOD_BASE);
  tick_load_chunks();
}

size_t test_load_stacked_chunks(void) {
  region_chunk_pos rcpos = { .x = 0, .y = 0, .z = 0 };
  mark_for_loading(&rcpos, LOD_BASE);
  rcpos.z += 1;
  mark_for_loading(&rcpos, LOD_BASE);
  tick_load_chunks();
}

size_t test_job_gencolumn(void) {
  // Note that test_create_world must be run before this test!
  setup_jobs();
  region_chunk_pos tc = { .x = 0, .y = 0, .z = 0 };
  launch_job_gencolumn(TEST_WORLD, &tc);
  printf("Generating test chunks...\n");
  while (do_step()) {} // run to exhaustion
  cleanup_jobs();
  printf("  ...done.\n");
  return 0;
}

#endif //ifndef TEST_WORLDGEN_H
