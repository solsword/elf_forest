// biology.c
// General biology generation.

#ifdef DEBUG
  #include <stdio.h>
#endif

#include "world/world_map.h"
#include "world/species.h"
#include "world/grammar.h"

#include "ecology/grow.h"

#include "data/data.h"

#include "biology.h"


/*************
 * Constants *
 *************/

cell_grammar *BIO_CG_SPROUT_IN_SOIL = NULL;
cell_grammar *BIO_CG_SPROUT_ABOVE_SOIL = NULL;
cell_grammar *BIO_CG_SPROUT_IN_SOIL_UNDERWATER = NULL;
cell_grammar *BIO_CG_SPROUT_ABOVE_SOIL_UNDERWATER = NULL;

/*************
 * Functions *
 *************/

void add_sprout_grammar(
  block seed,
  block root,
  block shoots,
  block substrate,
  int above_soil,
  int underwater
) {
  cg_expansion *parent;
  cg_expansion *intermediate;
  cg_expansion *child;

  // Build the default growth grammars if necessary:
  if (BIO_CG_SPROUT_IN_SOIL == NULL) {
    BIO_CG_SPROUT_IN_SOIL = create_cell_grammar();
  }
  if (BIO_CG_SPROUT_ABOVE_SOIL == NULL) {
    BIO_CG_SPROUT_ABOVE_SOIL = create_cell_grammar();
  }
  if (BIO_CG_SPROUT_IN_SOIL_UNDERWATER == NULL) {
    BIO_CG_SPROUT_IN_SOIL_UNDERWATER = create_cell_grammar();
  }
  if (BIO_CG_SPROUT_ABOVE_SOIL_UNDERWATER == NULL) {
    BIO_CG_SPROUT_ABOVE_SOIL_UNDERWATER = create_cell_grammar();
  }

  // Add an expansion as requested:
  if (above_soil) {
    if (underwater) {
      // Add to SPROUT_ABOVE_SOIL_UNDERWATER:
      parent = create_cell_grammar_expansion();
      parent->type = CGET_LOGICAL_AND;
      parent->offset.combined = 0;
      cg_add_expansion(BIO_CG_SPROUT_ABOVE_SOIL, parent);

      // Check the substrate:
      child = create_cell_grammar_expansion();
      child->type = CGET_BLOCK_EXACT;
      child->offset.xyz.x = 0;
      child->offset.xyz.y = 0;
      child->offset.xyz.z = -1;
      child->offset.xyz.w = 0;
      child->cmp_strategy = CGCS_EXACT;
      child->cmp_mask = BM_ID;
      child->compare = b_make_block(substrate);
      child->rpl_strategy = CGRS_NONE;
      child->rpl_mask = 0;
      child->replace = 0;
      cge_add_child(parent, child);

      // Check the water:
      child = create_cell_grammar_expansion();
      child->type = CGET_BLOCK_EXACT;
      child->offset.xyz.x = 0;
      child->offset.xyz.y = 0;
      child->offset.xyz.z = 0;
      child->offset.xyz.w = 0;
      child->cmp_strategy = CGCS_EXACT;
      child->cmp_mask = BM_ID;
      child->compare = b_make_block(B_WATER); // TODO: Allow water flow?
      child->rpl_strategy = CGRS_NONE;
      child->rpl_mask = 0;
      child->replace = 0;
      cge_add_child(parent, child);

      // Check the seed block and turn it into a sprout block:
      child = create_cell_grammar_expansion();
      child->type = CGET_BLOCK_EXACT;
      child->offset.xyz.x = 0;
      child->offset.xyz.y = 0;
      child->offset.xyz.z = 0;
      child->offset.xyz.w = 1;
      child->cmp_strategy = CGCS_EXACT;
      child->cmp_mask = BM_ID;
      child->compare = b_make_block(seed);
      child->rpl_strategy = CGRS_EXACT;
      child->rpl_mask = BM_ID;
      child->replace = b_make_block(shoots); // we don't change the species
      cge_add_child(parent, child);
    } else {
      // Add to SPROUT_ABOVE_SOIL:
      parent = create_cell_grammar_expansion();
      parent->type = CGET_LOGICAL_AND;
      parent->offset.combined = 0;
      cg_add_expansion(BIO_CG_SPROUT_ABOVE_SOIL, parent);

      // Check the substrate:
      child = create_cell_grammar_expansion();
      child->type = CGET_BLOCK_EXACT;
      child->offset.xyz.x = 0;
      child->offset.xyz.y = 0;
      child->offset.xyz.z = -1;
      child->offset.xyz.w = 0;
      child->cmp_strategy = CGCS_EXACT;
      child->cmp_mask = BM_ID;
      child->compare = b_make_block(substrate);
      child->rpl_strategy = CGRS_NONE;
      child->rpl_mask = 0;
      child->replace = 0;
      cge_add_child(parent, child);

      // Check the seed block and turn it into a sprout block:
      child = create_cell_grammar_expansion();
      child->type = CGET_BLOCK_EITHER;
      child->offset.xyz.x = 0;
      child->offset.xyz.y = 0;
      child->offset.xyz.z = 0;
      child->offset.xyz.w = 1;
      child->cmp_strategy = CGCS_EXACT;
      child->cmp_mask = BM_ID;
      child->compare = b_make_block(seed);
      child->rpl_strategy = CGRS_EXACT;
      child->rpl_mask = BM_ID;
      child->replace = b_make_block(shoots); // we don't change the species
      cge_add_child(parent, child);
    }
  } else {
    if (underwater) {
      // Add to SPROUT_IN_SOIL_UNDERWATER:
      parent = create_cell_grammar_expansion();
      parent->type = CGET_LOGICAL_AND;
      parent->offset.combined = 0;
      cg_add_expansion(BIO_CG_SPROUT_IN_SOIL, parent);

      // Check the substrate:
      child = create_cell_grammar_expansion();
      child->type = CGET_BLOCK_EXACT;
      child->offset.xyz.x = 0;
      child->offset.xyz.y = 0;
      child->offset.xyz.z = 0;
      child->offset.xyz.w = 0;
      child->cmp_strategy = CGCS_EXACT;
      child->cmp_mask = BM_ID;
      child->compare = b_make_block(substrate);
      child->rpl_strategy = CGRS_NONE;
      child->rpl_mask = 0;
      child->replace = 0;
      cge_add_child(parent, child);

      // Check for water above:
      child = create_cell_grammar_expansion();
      child->type = CGET_BLOCK_EXACT;
      child->offset.xyz.x = 0;
      child->offset.xyz.y = 0;
      child->offset.xyz.z = 1;
      child->offset.xyz.w = 0;
      child->cmp_strategy = CGCS_EXACT;
      child->cmp_mask = BM_ID;
      child->compare = b_make_block(B_WATER);
      child->rpl_strategy = CGRS_NONE;
      child->rpl_mask = 0;
      child->replace = 0;
      cge_add_child(parent, child);

      // Check the seed block and turn it into a root block:
      child = create_cell_grammar_expansion();
      child->type = CGET_BLOCK_EXACT;
      child->offset.xyz.x = 0;
      child->offset.xyz.y = 0;
      child->offset.xyz.z = 0;
      child->offset.xyz.w = 1;
      child->cmp_strategy = CGCS_EXACT;
      child->cmp_mask = BM_ID;
      child->compare = b_make_block(seed);
      child->rpl_strategy = CGRS_EXACT;
      child->rpl_mask = BM_ID;
      child->replace = b_make_block(root); // species is unchanged
      cge_add_child(parent, child);

      // Check for space above the seed and turn it into a sprout:
      child = create_cell_grammar_expansion();
      child->type = CGET_BLOCK_EXACT;
      child->offset.xyz.x = 0;
      child->offset.xyz.y = 0;
      child->offset.xyz.z = 1;
      child->offset.xyz.w = 1;
      child->cmp_strategy = CGCS_ROOT_CAN_GROW;
      child->cmp_mask = 0; // ignored due to the strategy
      child->compare = 0; // ignored due to the strategy
      child->rpl_strategy = CGRS_ROOT_SPECIES;
      child->rpl_mask = BM_FULL_BLOCK;
      child->replace = b_make_block(shoots); // species from the root block
      cge_add_child(parent, child);
    } else {
      // Add to SPROUT_IN_SOIL:
      parent = create_cell_grammar_expansion();
      parent->type = CGET_LOGICAL_AND;
      parent->offset.combined = 0;
      cg_add_expansion(BIO_CG_SPROUT_IN_SOIL, parent);

      // Check the substrate:
      child = create_cell_grammar_expansion();
      child->type = CGET_BLOCK_EXACT;
      child->offset.xyz.x = 0;
      child->offset.xyz.y = 0;
      child->offset.xyz.z = 0;
      child->offset.xyz.w = 0;
      child->cmp_strategy = CGCS_EXACT;
      child->cmp_mask = BM_ID;
      child->compare = b_make_block(substrate);
      child->rpl_strategy = CGRS_NONE;
      child->rpl_mask = 0;
      child->replace = 0;
      cge_add_child(parent, child);

      // Check the seed block and turn it into a root block:
      child = create_cell_grammar_expansion();
      child->type = CGET_BLOCK_EXACT;
      child->offset.xyz.x = 0;
      child->offset.xyz.y = 0;
      child->offset.xyz.z = 0;
      child->offset.xyz.w = 1;
      child->cmp_strategy = CGCS_EXACT;
      child->cmp_mask = BM_ID;
      child->compare = b_make_block(seed);
      child->rpl_strategy = CGRS_EXACT;
      child->rpl_mask = BM_ID;
      child->replace = b_make_block(root);
      cge_add_child(parent, child);

      // Check for air above the seed and turn it into a sprout:
      child = create_cell_grammar_expansion();
      child->type = CGET_BLOCK_EITHER;
      child->offset.xyz.x = 0;
      child->offset.xyz.y = 0;
      child->offset.xyz.z = 1;
      child->offset.xyz.w = 0;
      child->cmp_strategy = CGCS_ROOT_CAN_GROW;
      child->cmp_mask = 0; // ignored due to the strategy
      child->compare = 0; // ignored due to the strategy
      child->rpl_strategy = CGRS_ROOT_SPECIES;
      child->rpl_mask = BM_FULL_BLOCK;
      child->replace = b_make_block(shoots); // species from the root block
      cge_add_child(parent, child);

      // Make sure there isn't something blocking our sprout:
      // Create a disjunction over possible blocking stuff...
      intermediate = create_cell_grammar_expansion();
      intermediate->type = CGET_LOGICAL_OR;
      intermediate->offset.combined = 0;

      // Blocks with solid substance:
      child = create_cell_grammar_expansion();
      child->type = CGET_BLOCK_EITHER;
      child->offset.xyz.x = 0;
      child->offset.xyz.y = 0;
      child->offset.xyz.z = 1;
      child->offset.xyz.w = 0;
      child->cmp_strategy = CGCS_BLOCK_INFO;
      child->cmp_mask = BIM_GEOMETRY;
      child->compare = (BI_SBST_SOLID << BIMS_SUBSTANCE);
      child->rpl_strategy = CGRS_NONE;
      child->rpl_mask = 0;
      child->replace = 0;
      cge_add_child(intermediate, child);

      // Blocks with liquid substance:
      child = create_cell_grammar_expansion();
      child->type = CGET_BLOCK_EITHER;
      child->offset.xyz.x = 0;
      child->offset.xyz.y = 0;
      child->offset.xyz.z = 1;
      child->offset.xyz.w = 0;
      child->cmp_strategy = CGCS_BLOCK_INFO;
      child->cmp_mask = BIM_GEOMETRY;
      child->compare = (BI_SBST_LIQUID << BIMS_SUBSTANCE);
      child->rpl_strategy = CGRS_NONE;
      child->rpl_mask = 0;
      child->replace = 0;
      cge_add_child(intermediate, child);

      // Now negate our earlier disjunction...
      child = intermediate;
      intermediate = create_cell_grammar_expansion();
      intermediate->type = CGET_LOGICAL_NOT;
      intermediate->offset.combined = 0;
      cge_add_child(intermediate, child);

      // Finally add the negation to SPROUT_IN_SOIL:
      cg_add_expansion(BIO_CG_SPROUT_IN_SOIL, intermediate);
    }
  }
}

void setup_biology_gen(void) {
  // Generate sprout grammars for each type of plant:
  add_sprout_grammar(B_MUSHROOM_SPORES, B_VOID, B_MUSHROOM_SHOOTS, B_DIRT, 1,0);
  add_sprout_grammar(B_MUSHROOM_SPORES, B_VOID, B_MUSHROOM_SHOOTS, B_MUD , 1,0);
  add_sprout_grammar(B_MUSHROOM_SPORES, B_VOID, B_MUSHROOM_SHOOTS, B_SAND, 1,0);

  add_sprout_grammar(
    B_GIANT_MUSHROOM_SPORES,
    B_GIANT_MUSHROOM_CORE,
    B_GIANT_MUSHROOM_SPROUT,
    B_DIRT,
    0, 0
  );
  add_sprout_grammar(
    B_GIANT_MUSHROOM_SPORES,
    B_GIANT_MUSHROOM_CORE,
    B_GIANT_MUSHROOM_SPROUT,
    B_MUD,
    0, 0
  );
  add_sprout_grammar(
    B_GIANT_MUSHROOM_SPORES,
    B_GIANT_MUSHROOM_CORE,
    B_GIANT_MUSHROOM_SPROUT,
    B_SAND,
    0, 0
  );

  add_sprout_grammar(B_MOSS_SPORES, B_VOID, B_MOSS_SHOOTS, B_DIRT, 1, 0);
  add_sprout_grammar(B_MOSS_SPORES, B_VOID, B_MOSS_SHOOTS, B_MUD , 1, 0);
  add_sprout_grammar(B_MOSS_SPORES, B_VOID, B_MOSS_SHOOTS, B_SAND, 1, 0);
  // TODO: Let moss grow on dead wood & other things?

  add_sprout_grammar(B_GRASS_SEEDS, B_VOID, B_GRASS_SHOOTS, B_DIRT, 1, 0);
  add_sprout_grammar(B_GRASS_SEEDS, B_VOID, B_GRASS_SHOOTS, B_MUD , 1, 0);
  add_sprout_grammar(B_GRASS_SEEDS, B_VOID, B_GRASS_SHOOTS, B_SAND, 1, 0);

  add_sprout_grammar(B_VINE_SEEDS, B_VINE_CORE, B_VINE_SHOOTS, B_DIRT, 0, 0);
  add_sprout_grammar(B_VINE_SEEDS, B_VINE_CORE, B_VINE_SHOOTS, B_MUD , 0, 0);
  add_sprout_grammar(B_VINE_SEEDS, B_VINE_CORE, B_VINE_SHOOTS, B_SAND, 0, 0);
  // TODO: Vines should be able to sprout from ceilings...

  add_sprout_grammar(B_HERB_SEEDS, B_HERB_CORE, B_HERB_SHOOTS, B_DIRT, 0, 0);
  add_sprout_grammar(B_HERB_SEEDS, B_HERB_CORE, B_HERB_SHOOTS, B_MUD , 0, 0);
  add_sprout_grammar(B_HERB_SEEDS, B_HERB_CORE, B_HERB_SHOOTS, B_SAND, 0, 0);

  add_sprout_grammar(B_BUSH_SEEDS, B_BUSH_CORE, B_BUSH_SHOOTS, B_DIRT, 0, 0);
  add_sprout_grammar(B_BUSH_SEEDS, B_BUSH_CORE, B_BUSH_SHOOTS, B_MUD , 0, 0);
  add_sprout_grammar(B_BUSH_SEEDS, B_BUSH_CORE, B_BUSH_SHOOTS, B_SAND, 0, 0);

  add_sprout_grammar(B_SHRUB_SEEDS, B_SHRUB_CORE, B_SHRUB_SHOOTS, B_DIRT, 0, 0);
  add_sprout_grammar(B_SHRUB_SEEDS, B_SHRUB_CORE, B_SHRUB_SHOOTS, B_MUD , 0, 0);
  add_sprout_grammar(B_SHRUB_SEEDS, B_SHRUB_CORE, B_SHRUB_SHOOTS, B_SAND, 0, 0);

  add_sprout_grammar(B_TREE_SEEDS, B_TREE_CORE, B_TREE_SHOOTS, B_DIRT, 0, 0);
  add_sprout_grammar(B_TREE_SEEDS, B_TREE_CORE, B_TREE_SHOOTS, B_MUD , 0, 0);
  add_sprout_grammar(B_TREE_SEEDS, B_TREE_CORE, B_TREE_SHOOTS, B_SAND, 0, 0);

  add_sprout_grammar(
    B_AQUATIC_GRASS_SEEDS,
    B_AQUATIC_GRASS_ROOTS,
    B_AQUATIC_GRASS_SHOOTS,
    B_DIRT,
    0, 1
  );
  add_sprout_grammar(
    B_AQUATIC_GRASS_SEEDS,
    B_AQUATIC_GRASS_ROOTS,
    B_AQUATIC_GRASS_SHOOTS,
    B_MUD,
    0, 1
  );
  add_sprout_grammar(
    B_AQUATIC_GRASS_SEEDS,
    B_AQUATIC_GRASS_ROOTS,
    B_AQUATIC_GRASS_SHOOTS,
    B_SAND,
    0, 1
  );

  add_sprout_grammar(
    B_AQUATIC_PLANT_SEEDS,
    B_AQUATIC_PLANT_CORE,
    B_AQUATIC_PLANT_SHOOTS,
    B_DIRT,
    0, 1
  );
  add_sprout_grammar(
    B_AQUATIC_PLANT_SEEDS,
    B_AQUATIC_PLANT_CORE,
    B_AQUATIC_PLANT_SHOOTS,
    B_MUD,
    0, 1
  );
  add_sprout_grammar(
    B_AQUATIC_PLANT_SEEDS,
    B_AQUATIC_PLANT_CORE,
    B_AQUATIC_PLANT_SHOOTS,
    B_SAND,
    0, 1
  );

  add_sprout_grammar(B_YOUNG_CORAL, B_VOID, B_CORAL_CORE, B_DIRT, 1, 1);
  add_sprout_grammar(B_YOUNG_CORAL, B_VOID, B_CORAL_CORE, B_MUD , 1, 1);
  add_sprout_grammar(B_YOUNG_CORAL, B_VOID, B_CORAL_CORE, B_SAND, 1, 1);
}

void add_biology(chunk *c) {
  world_map_pos wmpos;
  world_region *wr, *wr_second;
  float strbest, strsecond;
  world_region *wr_neighborhood[9];
  biome *local_biome;
  list *sp_list;
  frequent_species fqsp;
  chunk_neighborhood ch_nbh;
  cell_neighborhood cl_nbh;
  cell* cl;
  block_index idx;
  block substrate;
  global_pos glpos;
  ptrdiff_t seed_hash = prng(chunk_hash(c) + 616485);
  ptrdiff_t spacing_hash = prng(THE_WORLD->seed + 44544);

  if (c->chunk_flags & CF_HAS_BIOLOGY) {
    return; // Already has biology
  }
  fill_chunk_neighborhood(&(c->glcpos), &ch_nbh);
  if (ch_nbh.members[0] == NULL) {
    return; // Return without setting the CF_HAS_BIOLOGY flag.
  }
  // Look up the local biomes:
  glcpos__wmpos(&(c->glcpos), &wmpos);
  // TODO: Avoid using THE_WORLD here?
  wr = get_world_neighborhood_small(THE_WORLD, &wmpos, wr_neighborhood);
  // Use the center of our chunk to compute region contenders:
  idx.xyz.x = CHUNK_SIZE / 2;
  idx.xyz.y = CHUNK_SIZE / 2;
  idx.xyz.z = CHUNK_SIZE / 2;
  idx.xyz.w = 0;
  cidx__glpos(c, &idx, &glpos);
  compute_region_contenders(
    THE_WORLD,
    neighborhood,
    glpos,
    1232331,
    &wr,
    &wr_second,
    &strbest,
    &strsecond
  );
  // Mix biomes for this chunk:
  local_biome = create_merged_biome(
    wr,
    wr_second,
    strbest,
    strsecond
  );
  // Add seeds:
  idx.xyz.w = 0;
  for (idx.xyz.x = 0; idx.xyz.x < CHUNK_SIZE; ++idx.xyz.x) {
    for (idx.xyz.y = 0; idx.xyz.y < CHUNK_SIZE; ++idx.xyz.y) {
      for (idx.xyz.z = 0; idx.xyz.z < CHUNK_SIZE; ++idx.xyz.z) {
        // Get global cell position for hashing:
        cidx__glpos(c, &idx, &glpos);
        // TODO: optimize this within a loop...
        fill_cell_neighborhood_exact(idx, &ch_nbh, &cl_nbh);

        cl = cl_nbh.members[NBH_CENTER]; // shortcut for the central cell
        // Array is in xyz order so up/down is +/- 1, north/south is +/- 3, and
        // east/west is +/- 9.

        // If our secondary is empty we can put a seed here: figure out what
        // distribution to draw from.
        sp_list = NULL;
        if (b_is_void(cl->blocks[1])) {
          if (b_id(cl->blocks[0]) == B_AIR) {
            // Ephemeral species seeds settle in the air directly on top of
            // dirt/sand/mud/etc.
            substrate = cl_nbh.members[NBH_CENTER - 1]->blocks[0];
            if (b_is_natural_terrain(substrate)) {
              sp_list = local_biome->ephemeral_terrestrial_flora;
            }
          } else if (b_id(cl->blocks[0]) == B_WATER) {
            // Aquatic ephemeral species start in water above dirt/sand/mud/etc.
            substrate = cl_nbh.members[NBH_CENTER-1]->blocks[0];
            if (b_is_natural_terrain(substrate)) {
              sp_list = local_biome->ephemeral_aquatic_flora;
            }
          } else if (b_is_natural_terrain(cl->blocks[0])) {
            substrate = cl->blocks[0];
            if (b_id(cl_nbh.members[NBH_CENTER+1]->blocks[0]) == B_AIR) {
              // Most terrestrial species sprout in the ground with air above.
              // TODO: Subterranean species!
              // Select between spacings:
              if (wide_spacing_hit(glpos, spacing_hash)) {
                sp_list = local_biome->wide_spaced_terrestrial_flora;
              } else if (medium_spacing_hit(glpos, spacing_hash)) {
                sp_list = local_biome->medium_spaced_terrestrial_flora;
              } else if (close_spacing_hit(glpos, spacing_hash)) {
                sp_list = local_biome->close_spaced_terrestrial_flora;
              } else {
                sp_list = local_biome->ubiquitous_terrestrial_flora;
              }
            } else if (b_id(cl_nbh.members[NBH_CENTER+1]->blocks[0])==B_WATER) {
              // Aquatic plants sprout from seeds in terrain below water.
              // Select between spacings:
              if (wide_spacing_hit(glpos, spacing_hash)) {
                sp_list = local_biome->wide_spaced_aquatic_flora;
              } else if (medium_spacing_hit(glpos, spacing_hash)) {
                sp_list = local_biome->medium_spaced_aquatic_flora;
              } else if (close_spacing_hit(glpos, spacing_hash)) {
                sp_list = local_biome->close_spaced_aquatic_flora;
              } else {
                sp_list = local_biome->ubiquitous_aquatic_flora;
              }
            } else if (b_id(cl_nbh.members[NBH_CENTER-1]->blocks[0]) == B_AIR) {
              // Some plants can also grow down into air from ceilings.
              sp_list = local_biome->hanging_terrestrial_flora;
            }
            // TODO: Should things grow in underwater ceilings?
          }
          // Now we generate a seed from the species list we decided on:
          if (sp_list != NULL) {
            fqsp = pick_appropriate_frequent_species(
              sp_list,
              substrate,
              seed_hash
            )
            if (frequent_species_species_type(fqsp) != SPT_NO_SPECIES) {
              cl->blocks[1] = b_make_species(
                seed_block_type(frequent_species_species_type(fqsp)),
                frequent_species_species(fqsp)
              );
              // TODO: Real sprout timers...
              gri_set_sprout_timer(&(cl->blocks[1]), 1);
              seed_hash = prng(
                idx.xyz.x + idx.xyz.y + glpos.z +
                prng(seed_hash + glpos.x + glpos.y + idx.xyz.z)
              );
            }
          }
        }
      }
    }
  }
  // Clean up the local biome info now that we're done with it:
  cleanup_biome(local_biome);
  // Grow plants a bit:
  // TODO: How many cycles to use?
#ifdef DEBUG
  if (!grow_plants(c, 2)) {
    // At this point if growth fails (it really shouldn't) the best we can do
    // is emit a warning...
    fprintf(
      stderr,
      "WARNING: Growth failed after adding seeds during biology generation.\n"
    );
  }
#else
  grow_plants(c, 2);
#endif
  // Set the CF_HAS_BIOLOGY flag for this chunk:
  c->chunk_flags |= CF_HAS_BIOLOGY;
}

frequent_species pick_appropriate_frequent_species(
  list *sp_list,
  block substrate,
  ptrdiff_t seed
) {
  float total_weight = 0;
  float choice;
  float rare_smoothing;
  size_t i;
  frequent_species fqsp;

  if (l_is_empty(sp_list)) {
    // Return an invalid species...
    frequent_species_set_species_type(&fqsp, SPT_NO_SPECIES);
    frequent_species_set_species(&fqsp, 0);
    frequent_species_set_frequency(&fqsp, 0.0);
    return fqsp;
  }

  rare_smoothing = expdist(ptrf(seed + 31376), BIO_RARE_SPECIES_SMOOTHING_EXP);
  rare_smoothing *= BIO_RARE_SPECIES_SMOOTHING_ADJUST;

  for (i = 0; i < l_get_length(sp_list); ++i) {
    fqsp = (frequent_species) l_get_item(sp_list, i);
    total_weight += (
      species_compatability(fqsp, substrate)
    * frequent_species_frequency(fqsp)
    ) + rare_smoothing;
  }

  choice = ptrf(prng(seed + 866859)) * total_weight;

  for (i = 0; i < l_get_length(sp_list); ++i) {
    fqsp = (frequent_species) l_get_item(sp_list, i);
    choice -= (
      species_compatability(fqsp, substrate)
    * frequent_species_frequency(fqsp)
    ) + rare_smoothing;
    if (choice < 0) {
      return fqsp;
    }
  }
#ifdef DEBUG
  pritnf("Error: Ran out of appropriate species to pick from!\n");
  exit(1);
#endif
  // Just arbitrarily return the first item (this shouldn't be reachable):
  return (frequent_species) l_get_item(sp_list, 0);
}

float species_compatability(frequent_species fqsp, block substrate) {
  // TODO: This function!
  if (b_id(substrate) == B_STONE) {
    return 0.0;
  }
  return 1.0;
}

growth_properties* get_growth_properties(block b) {
  any_species a_sp;
  block__any_species(b, &a_sp);
  if (a_sp.type == SPT_FUNGUS) {
    return &(get_fungus_species(a_sp.id)->growth);
  } else if (a_sp.type == SPT_MOSS) {
    return &(get_moss_species(a_sp.id)->growth);
  } else if (a_sp.type == SPT_GRASS) {
    return &(get_grass_species(a_sp.id)->growth);
  } else if (a_sp.type == SPT_VINE) {
    return &(get_vine_species(a_sp.id)->growth);
  } else if (a_sp.type == SPT_HERB) {
    return &(get_herb_species(a_sp.id)->growth);
  } else if (a_sp.type == SPT_BUSH) {
    return &(get_bush_species(a_sp.id)->growth);
  } else if (a_sp.type == SPT_SHRUB) {
    return &(get_shrub_species(a_sp.id)->growth);
  } else if (a_sp.type == SPT_TREE) {
    return &(get_tree_species(a_sp.id)->growth);
  } else if (a_sp.type == SPT_AQUATIC_GRASS) {
    return &(get_aquatic_grass_species(a_sp.id)->growth);
  } else if (a_sp.type == SPT_AQUATIC_PLANT) {
    return &(get_aquatic_plant_species(a_sp.id)->growth);
  } else if (a_sp.type == SPT_CORAL) {
    return &(get_coral_species(a_sp.id)->growth);
  } else { // Not a growing species.
    return NULL;
  }
}


ptrdiff_t get_species_growth_strength(block b, int resist) {
  block id = b_id(b);
  growth_properties *grp = get_growth_properties(b);
  if (id == B_VOID) {
    return 0;
  } else if (id == B_AIR || id == B_ETHER) {
    return -1;
  } else if (grp == NULL) {
    // A non-air non-water non-plant block: can't grow here.
    return 1000;
  }
  // Unpack the desired property:
  if (resist) {
    if (bi_seed(b)) {
      return grp->seed_growth_resist;
    } else {
      return grp->growth_resist;
    }
  } else {
    return grp->growth_strength;
  }
}



species create_new_fungus_species(ptrdiff_t seed) {
  species result = create_fungus_species();
  return result;
}

species create_new_moss_species(ptrdiff_t seed) {
  species result = create_moss_species();
  return result;
}

species create_new_grass_species(ptrdiff_t seed) {
  species result = create_grass_species();
  return result;
}

species create_new_vine_species(ptrdiff_t seed) {
  species result = create_vine_species();
  return result;
}

species create_new_herb_species(ptrdiff_t seed) {
  species result = create_herb_species();
  herb_species* hsp = get_herb_species(result);

  determine_new_plant_materials(&(hsp->materials), seed);
  seed = prng(seed);

  determine_new_herb_appearance(&(hsp->appearance), seed);
  seed = prng(seed);

  hsp->growth.seed_growth.grammar = BIO_CG_SPROUT_IN_SOIL;
  hsp->growth.seed_growth.sprout_time = (
    posmod(prng(seed + 1771), 5)
  + posmod(prng(seed + 311), 8)
  );
  seed = prng(seed);

  // TODO: Real values here...
  hsp->growth.seed_growth_resist = 5;
  hsp->growth.growth_resist = 10;
  hsp->growth.growth_strength = 8;

  determine_new_herb_core_growth(&(hsp->growth.core_growth), seed);

  return result;
}

species create_new_bush_species(ptrdiff_t seed) {
  species result = create_bush_species();
  return result;
}

species create_new_shrub_species(ptrdiff_t seed) {
  species result = create_shrub_species();
  return result;
}

species create_new_tree_species(ptrdiff_t seed) {
  species result = create_tree_species();
  return result;
}

species create_new_aquatic_grass_species(ptrdiff_t seed) {
  species result = create_aquatic_grass_species();
  return result;
}

species create_new_aquatic_plant_species(ptrdiff_t seed) {
  species result = create_aquatic_plant_species();
  return result;
}

species create_new_coral_species(ptrdiff_t seed) {
  species result = create_coral_species();
  return result;
}


// Individual attribute generation:

void determine_new_plant_materials(plant_materials *target, ptrdiff_t seed) {
  target->seed.origin = MO_ORGANIC;
  target->root.origin = MO_ORGANIC;
  target->wood.origin = MO_ORGANIC;
  target->dry_wood.origin = MO_ORGANIC;
  target->stem.origin = MO_ORGANIC;
  target->leaf.origin = MO_ORGANIC;
  target->fruit.origin = MO_ORGANIC;

  // TODO: Look up plant densities
  // Solid densities are relative to water.
  target->seed.solid_density = mat_density(0.4);
  target->root.solid_density = mat_density(0.25);
  target->wood.solid_density = mat_density(0.25);
  target->dry_wood.solid_density = mat_density(0.25);
  target->stem.solid_density = mat_density(0.25);
  target->leaf.solid_density = mat_density(0.25);
  target->fruit.solid_density = mat_density(0.75);

  // Liquid densities are relative to water.
  target->seed.liquid_density = mat_density(0.6);
  target->root.liquid_density = mat_density(0.5);
  target->wood.liquid_density = mat_density(0.5);
  target->dry_wood.liquid_density = mat_density(0.5);
  target->stem.liquid_density = mat_density(0.5);
  target->leaf.liquid_density = mat_density(0.5);
  target->fruit.liquid_density = mat_density(0.9);

  // Gas densities are relative to air.
  target->seed.gas_density = mat_density(1.2);
  target->root.gas_density = mat_density(1.2);
  target->wood.gas_density = mat_density(1.2);
  target->dry_wood.gas_density = mat_density(1.2);
  target->stem.gas_density = mat_density(1.2);
  target->leaf.gas_density = mat_density(1.2);
  target->fruit.gas_density = mat_density(1.4);

  // TODO: Something sensible here...
  // Specific heats are relative to air.
  target->seed.solid_specific_heat = mat_specific_heat(1.0);
  target->root.solid_specific_heat = mat_specific_heat(1.0);
  target->wood.solid_specific_heat = mat_specific_heat(1.0);
  target->dry_wood.solid_specific_heat = mat_specific_heat(1.0);
  target->stem.solid_specific_heat = mat_specific_heat(1.0);
  target->leaf.solid_specific_heat = mat_specific_heat(1.0);
  target->fruit.solid_specific_heat = mat_specific_heat(1.0);

  // TODO: RNG here!
  // Freezing is bad for plants.
  target->seed.cold_damage_temp = 0;
  target->root.cold_damage_temp = 0;
  target->wood.cold_damage_temp = -10;
  target->dry_wood.cold_damage_temp = -40;
  target->stem.cold_damage_temp = 0;
  target->leaf.cold_damage_temp = 0;
  target->fruit.cold_damage_temp = 0;

  // TODO: Real numbers here!
  target->seed.solidus = 250;
  target->root.solidus = 250;
  target->wood.solidus = 250;
  target->dry_wood.solidus = 250;
  target->stem.solidus = 250;
  target->leaf.solidus = 250;
  target->fruit.solidus = 250;

  target->seed.liquidus = 300;
  target->root.liquidus = 300;
  target->wood.liquidus = 300;
  target->dry_wood.liquidus = 300;
  target->stem.liquidus = 300;
  target->leaf.liquidus = 300;
  target->fruit.liquidus = 300;

  target->seed.boiling_point = 900;
  target->root.boiling_point = 900;
  target->wood.boiling_point = 900;
  target->dry_wood.boiling_point = 900;
  target->stem.boiling_point = 900;
  target->leaf.boiling_point = 900;
  target->fruit.boiling_point = 900;

  target->seed.ignition_point = 101; // just post-boiling
  target->root.ignition_point = 101; // just post-boiling
  target->wood.ignition_point = 101; // just post-boiling
  target->dry_wood.ignition_point = 101; // just post-boiling
  target->stem.ignition_point = 101; // just post-boiling
  target->leaf.ignition_point = 101; // just post-boiling
  target->fruit.ignition_point = 101; // just post-boiling

  target->seed.flash_point = 200;
  target->root.flash_point = 200;
  target->wood.flash_point = 250;
  target->dry_wood.flash_point = 180;
  target->stem.flash_point = 200;
  target->leaf.flash_point = 200;
  target->fruit.flash_point = 200;

  // TODO: Real numbers here!
  target->seed.cold_plastic_temp = 0;
  target->root.cold_plastic_temp = 0;
  target->wood.cold_plastic_temp = 0;
  target->dry_wood.cold_plastic_temp = 0;
  target->stem.cold_plastic_temp = 0;
  target->leaf.cold_plastic_temp = 0;
  target->fruit.cold_plastic_temp = 0;

  target->seed.warm_plastic_temp = 0;
  target->root.warm_plastic_temp = 0;
  target->wood.warm_plastic_temp = 0;
  target->dry_wood.warm_plastic_temp = 0;
  target->stem.warm_plastic_temp = 0;
  target->leaf.warm_plastic_temp = 0;
  target->fruit.warm_plastic_temp = 0;

  target->seed.cold_plasticity = 0;
  target->root.cold_plasticity = 0;
  target->wood.cold_plasticity = 0;
  target->dry_wood.cold_plasticity = 0;
  target->stem.cold_plasticity = 0;
  target->leaf.cold_plasticity = 0;
  target->fruit.cold_plasticity = 0;

  target->seed.warm_plasticity = 0;
  target->root.warm_plasticity = 0;
  target->wood.warm_plasticity = 0;
  target->dry_wood.warm_plasticity = 0;
  target->stem.warm_plasticity = 0;
  target->leaf.warm_plasticity = 0;
  target->fruit.warm_plasticity = 0;

  // Relative to water.
  target->seed.viscosity = 0;
  target->root.viscosity = 0;
  target->wood.viscosity = 0;
  target->dry_wood.viscosity = 0;
  target->stem.viscosity = 0;
  target->leaf.viscosity = 0;
  target->fruit.viscosity = 0;

  // Wood & fingernails ~= 60; stone is mostly 100-220
  target->seed.hardness = 45;
  target->root.hardness = 15;
  target->wood.hardness = 60;
  target->dry_wood.hardness = 65;
  target->stem.hardness = 20;
  target->leaf.hardness = 15;
  target->fruit.hardness = 15;
}

void determine_new_herb_appearance(
  herbaceous_appearance *target,
  ptrdiff_t seed
) {
  target->seeds
  target->roots
  target->shoots
  target->stems
  target->leaves
  target->buds
  target->flowers
  target->fruit
  // TODO: HERE
}

void determine_new_herb_core_growth(core_growth_pattern *target,ptrdiff_t seed){
  // TODO: HERE
}

