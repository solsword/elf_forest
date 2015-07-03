// biology.c
// General biology generation.

#include "world/world_map.h"
#include "world/species.h"

#include "data/data.h"

#include "biology.h"


void add_biology(chunk *c) {
  chunk* chunk_neighbors[27];
  cell* cell_neighbors[27];
  cell* cl;
  get_exact_chunk_neighborhood(c->glcpos, chunk_neighbors);
  if (chunk_neighbors[0] == NULL || c->chunk_flags & CF_HAS_BIOLOGY) {
    return; // Return without setting the CF_HAS_BIOLOGY flag.
  }
  ptrdiff_t seed_hash = prng(chunk_hash(c) + 616485);
  // Add seeds:
  for (idx.x = 0; idx.x < CHUNK_SIZE; ++idx.x) {
    for (idx.y = 0; idx.y < CHUNK_SIZE; ++yidx.) {
      for (idx.z = 0; idx.z < CHUNK_SIZE; ++idx.z) {
        get_cell_neighborhood_exact(idx, chunk_neighbors, cell_neighbors);
        cl = cell_neighbors[13]; // shortcut for the central cell
        // Array is in zxy order so up/down is +/- 9, east/west is +/- 3, and
        // north/south is +/- 1.

        // If our secondary is empty we can put a seed here...
        if (b_is_void(cl->secondary)) {
          if (b_is_same_type(cl->primary, B_AIR)) {
            // Grass seeds, mushroom spores, and moss spores settle in the air
            // directly on top of dirt/sand/mud/etc.
            if (b_is_natural_terrain(cell_neighbors[13-9])) {
              // TODO: Species distributions!
              // TODO: Feed in ground block type.
              cl->secondary = b_make_species(
                B_GRASS_SEEDS,
                seed_hash % SPECIES_PER_BLOCK
              );
              seed_hash = prng(
                idx.x + idx.y + glpos.z +
                prng(seed_hash + glpos.x + glpos.y + idx.z)
              );
            } else if (b_is_natural_terrain(cell_neighbors[13+9])) {
              // Some grasses, msuhrooms, and mosses grow below ceilings.
              // TODO: Species distributions!
              cl->secondary = b_make_species(
                B_MOSS_SPORES,
                seed_hash % SPECIES_PER_BLOCK
              );
              seed_hash = prng(
                idx.x + idx.y + glpos.z +
                prng(seed_hash + glpos.x + glpos.y + idx.z)
              );
            }
          } else if (b_is_same_type(cl->primary, B_WATER)) {
            // Aquatic grass seeds and coral start in water above
            // dirt/sand/mud/etc.
            if (b_is_natural_terrain(cell_neighbors[13-9])) {
              // TODO: Species distributions!
              // TODO: Feed in ground block type.
              cl->secondary = b_make_species(
                B_AQUATIC_GRASS_SEEDS,
                seed_hash % SPECIES_PER_BLOCK
              );
              seed_hash = prng(
                idx.x + idx.y + glpos.z +
                prng(seed_hash + glpos.x + glpos.y + idx.z)
              );
            }
          } else if (b_is_natural_terrain(cl->primary)) {
            if (b_is_same_type(cell_neighbors[13+9], B_AIR)) {
              // Vines, herbs, bushes, shrubs, and trees sprout from seeds in
              // the top layer of soil with air above.
              cl->secondary = b_make_species(
                B_HERB_SEEDS,
                seed_hash % SPECIES_PER_BLOCK
              );
              seed_hash = prng(
                idx.x + idx.y + glpos.z +
                prng(seed_hash + glpos.x + glpos.y + idx.z)
              );
            } else if (b_is_same_type(cell_neighbors[13+9], B_WATER)) {
              // Aquatic plants sprout from seeds in terrain below water.
              cl->secondary = b_make_species(
                B_AQUATIC_PLANT_SEEDS,
                seed_hash % SPECIES_PER_BLOCK
              );
              seed_hash = prng(
                idx.x + idx.y + glpos.z +
                prng(seed_hash + glpos.x + glpos.y + idx.z)
              );
            } else if (b_is_same_type(cell_neighbors[13-9], B_AIR)) {
              // Some plants can also grow down into air from ceilings.
              cl->secondary = b_make_species(
                B_VINE_SEEDS,
                seed_hash % SPECIES_PER_BLOCK
              );
              seed_hash = prng(
                idx.x + idx.y + glpos.z +
                prng(seed_hash + glpos.x + glpos.y + idx.z)
              );
            }
            // TODO: Should things grow in underwater ceilings?
          }
        }
      }
    }
  }
  // Grow plants a bit:
  // TODO: HERE
  // Set the CF_HAS_BIOLOGY flag for this chunk:
  c->chunk_flags |= CF_HAS_BIOLOGY;
}
