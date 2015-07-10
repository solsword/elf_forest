// biology.c
// General biology generation.

#include "world/world_map.h"
#include "world/species.h"

#include "data/data.h"

#include "biology.h"


void add_biology(chunk *c) {
  chunk_neighborhood ch_nbh;
  cell_neighborhood cl_nbh;
  cell* cl;
  block_index idx;
  global_pos glpos;
  if (c->chunk_flags & CF_HAS_BIOLOGY) {
    return; // Already has biology
  }
  fill_chunk_neighborhood(&(c->glcpos), &ch_nbh);
  if (ch_nbh.members[0] == NULL || c->chunk_flags & CF_HAS_BIOLOGY) {
    return; // Return without setting the CF_HAS_BIOLOGY flag.
  }
  ptrdiff_t seed_hash = prng(chunk_hash(c) + 616485);
  // Add seeds:
  idx.xyz.w = 0;
  for (idx.xyz.x = 0; idx.xyz.x < CHUNK_SIZE; ++idx.xyz.x) {
    for (idx.xyz.y = 0; idx.xyz.y < CHUNK_SIZE; ++idx.xyz.y) {
      for (idx.xyz.z = 0; idx.xyz.z < CHUNK_SIZE; ++idx.xyz.z) {
        // Get global cell position for hashing:
        cidx__glpos(c, &idx, &glpos);
        fill_cell_neighborhood_exact(idx, &ch_nbh, &cl_nbh);
        cl = cl_nbh.members[NBH_CENTER]; // shortcut for the central cell
        // Array is in xyz order so up/down is +/- 1, north/south is +/- 3, and
        // east/west is +/- 9.

        // If our secondary is empty we can put a seed here...
        if (b_is_void(cl->blocks[1])) {
          if (b_is_same_type(cl->blocks[0], B_AIR)) {
            // Grass seeds, mushroom spores, and moss spores settle in the air
            // directly on top of dirt/sand/mud/etc.
            if (
              b_is_natural_terrain(cl_nbh.members[NBH_CENTER-1]->blocks[0])
            ) {
              // TODO: Species distributions!
              // TODO: Feed in ground block type.
              cl->blocks[1] = b_make_species(
                B_GRASS_SEEDS,
                seed_hash % SPECIES_PER_BLOCK
              );
              seed_hash = prng(
                idx.xyz.x + idx.xyz.y + glpos.z +
                prng(seed_hash + glpos.x + glpos.y + idx.xyz.z)
              );
            } else if (
              b_is_natural_terrain(cl_nbh.members[NBH_CENTER+1]->blocks[0])
            ) {
              // Some grasses, msuhrooms, and mosses grow below ceilings.
              // TODO: Species distributions!
              cl->blocks[1] = b_make_species(
                B_MOSS_SPORES,
                seed_hash % SPECIES_PER_BLOCK
              );
              seed_hash = prng(
                idx.xyz.x + idx.xyz.y + glpos.z +
                prng(seed_hash + glpos.x + glpos.y + idx.xyz.z)
              );
            }
          } else if (b_is_same_type(cl->blocks[0], B_WATER)) {
            // Aquatic grass seeds and coral start in water above
            // dirt/sand/mud/etc.
            if (
              b_is_natural_terrain(cl_nbh.members[NBH_CENTER-1]->blocks[0])
            ) {
              // TODO: Species distributions!
              // TODO: Feed in ground block type.
              cl->blocks[1] = b_make_species(
                B_AQUATIC_GRASS_SEEDS,
                seed_hash % SPECIES_PER_BLOCK
              );
              seed_hash = prng(
                idx.xyz.x + idx.xyz.y + glpos.z +
                prng(seed_hash + glpos.x + glpos.y + idx.xyz.z)
              );
            }
          } else if (b_is_natural_terrain(cl->blocks[0])) {
            if (
              b_is_same_type(cl_nbh.members[NBH_CENTER+1]->blocks[0], B_AIR)
            ) {
              // Vines, herbs, bushes, shrubs, and trees sprout from seeds in
              // the top layer of soil with air above.
              cl->blocks[1] = b_make_species(
                B_HERB_SEEDS,
                seed_hash % SPECIES_PER_BLOCK
              );
              seed_hash = prng(
                idx.xyz.x + idx.xyz.y + glpos.z +
                prng(seed_hash + glpos.x + glpos.y + idx.xyz.z)
              );
            } else if (
              b_is_same_type(cl_nbh.members[NBH_CENTER+1]->blocks[0], B_WATER)
            ) {
              // Aquatic plants sprout from seeds in terrain below water.
              cl->blocks[1] = b_make_species(
                B_AQUATIC_PLANT_SEEDS,
                seed_hash % SPECIES_PER_BLOCK
              );
              seed_hash = prng(
                idx.xyz.x + idx.xyz.y + glpos.z +
                prng(seed_hash + glpos.x + glpos.y + idx.xyz.z)
              );
            } else if (
              b_is_same_type(cl_nbh.members[NBH_CENTER-1]->blocks[0], B_AIR)
            ) {
              // Some plants can also grow down into air from ceilings.
              cl->blocks[1] = b_make_species(
                B_VINE_SEEDS,
                seed_hash % SPECIES_PER_BLOCK
              );
              seed_hash = prng(
                idx.xyz.x + idx.xyz.y + glpos.z +
                prng(seed_hash + glpos.x + glpos.y + idx.xyz.z)
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
