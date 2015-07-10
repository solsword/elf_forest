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


species create_new_fungus_species(ptrdiff_t seed) {
  return create_fungus_species();
}

species create_new_moss_species(ptrdiff_t seed) {
  return create_moss_species();
}

species create_new_grass_species(ptrdiff_t seed) {
  species result = create_grass_species();
}

species create_new_vine_species(ptrdiff_t seed) {
  return create_vine_species();
}

species create_new_herb_species(ptrdiff_t seed) {
  species result = create_herb_species();
  herb_species* hsp = get_herb_species(result);

  determine_new_plant_leaf_material(&(hsp->material), seed);
  seed = prng(seed);

  determine_new_herb_appearance(&(hsp->appearance), seed);
  seed = prng(seed);

  hsp->seed_growth = BIO_CG_SPROUT_IN_SOIL;

  determine_new_herb_core_growth(&(hsp->core_growth), seed);

  return result;
}

species create_new_bush_species(ptrdiff_t seed) {
  return create_bush_species();
}

species create_new_shrub_species(ptrdiff_t seed) {
  return create_shrub_species();
}

species create_new_tree_species(ptrdiff_t seed) {
  return create_tree_species();
}

species create_new_aquatic_grass_species(ptrdiff_t seed) {
  return create_aquatic_grass_species();
}

species create_new_aquatic_plant_species(ptrdiff_t seed) {
  return create_aquatic_plant_species();
}

species create_new_coral_species(ptrdiff_t seed) {
  return create_coral_species();
}


// Individual attribute generation:

void determine_new_plant_leaf_material(material *target, ptrdiff_t seed) {
  target->seeds.origin = MO_ORGANIC; 
  target->roots.origin = MO_ORGANIC; 
  target->leaves.origin = MO_ORGANIC; 
  target->fruit.origin = MO_ORGANIC; 

  // TODO: Look up plant densities
  target->seeds.solid_density = mat_density(0.4); // relative to water
  target->roots.solid_density = mat_density(0.25); // relative to water
  target->leaves.solid_density = mat_density(0.25); // relative to water
  target->fruit.solid_density = mat_density(0.75); // relative to water

  target->seeds.liquid_density = mat_density(0.6); // relative to water
  target->roots.liquid_density = mat_density(0.5); // relative to water
  target->leaves.liquid_density = mat_density(0.5); // relative to water
  target->fruit.liquid_density = mat_density(0.9); // relative to water

  target->seeds.gas_density = mat_density(1.2); // relative to air
  target->roots.gas_density = mat_density(1.2); // relative to air
  target->leaves.gas_density = mat_density(1.2); // relative to air
  target->fruit.gas_density = mat_density(1.4); // relative to air

  // TODO: Something sensible here...
  target->seeds.solid_specific_heat = mat_specific_heat(1.0); // relative to air
  target->roots.solid_specific_heat = mat_specific_heat(1.0); // relative to air
  target->leaves.solid_specific_heat = mat_specific_heat(1.0);// relative to air
  target->fruit.solid_specific_heat = mat_specific_heat(1.0); // relative to air

  // TODO: RNG here!
  target->seeds.cold_damage_temp = 0; // freezing is bad!
  target->roots.cold_damage_temp = 0; // freezing is bad!
  target->leaves.cold_damage_temp = 0; // freezing is bad!
  target->fruit.cold_damage_temp = 0; // freezing is bad!

  // TODO: Real numbers here!
  target->seeds.solidus = 250;
  target->roots.solidus = 250;
  target->leaves.solidus = 250;
  target->fruit.solidus = 250;

  target->seeds.liquidus = 300;
  target->roots.liquidus = 300;
  target->leaves.liquidus = 300;
  target->fruit.liquidus = 300;

  target->seeds.boiling_point = 900;
  target->roots.boiling_point = 900;
  target->leaves.boiling_point = 900;
  target->fruit.boiling_point = 900;

  target->seeds.ignition_point = 101; // just post-boiling
  target->roots.ignition_point = 101; // just post-boiling
  target->leaves.ignition_point = 101; // just post-boiling
  target->fruit.ignition_point = 101; // just post-boiling

  target->seeds.flash_point = 200;
  target->roots.flash_point = 200;
  target->leaves.flash_point = 200;
  target->fruit.flash_point = 200;

  // TODO: Real numbers here!
  target->seeds.malleability = 0;
  target->roots.malleability = 0;
  target->leaves.malleability = 0;
  target->fruit.malleability = 0;

  target->seeds.viscosity = 0;
  target->roots.viscosity = 0;
  target->leaves.viscosity = 0;
  target->fruit.viscosity = 0;

  target->seeds.hardness = 15;
  target->roots.hardness = 15;
  target->leaves.hardness = 15;
  target->fruit.hardness = 15;
}

void determine_new_herb_appearance(herb_appearance *target, seed) {
}

void determine_new_herb_core_growth(core_growth *target, seed) {
}

