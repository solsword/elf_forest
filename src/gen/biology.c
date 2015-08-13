// biology.c
// General biology generation.

#include "world/world_map.h"
#include "world/species.h"
#include "world/grammar.h"

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

cell_grammar* add_sprout_grammar(
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
      parent->offset = 0;
      cg_add_expansion(BIO_CG_SPROUT_ABOVE_SOIL, parent);

      // Check the substrate:
      child = create_cell_grammar_expansion();
      child->type = CGCT_BLOCK_EXACT;
      child->offset = { .xyz = {.x = 0, .y = 0, .z = -1, .w = 0 } };
      child->cmp_strategy = CGCS_EXACT;
      child->cmp_mask = BM_ID;
      child->compare = b_make_block(substrate);
      child->rpl_strategy = CGRS_NONE;
      child->rpl_mask = 0;
      child->replace = 0;
      cge_add_child(parent, child);

      // Check the water:
      child = create_cell_grammar_expansion();
      child->type = CGCT_BLOCK_EXACT;
      child->offset = { .xyz = {.x = 0, .y = 0, .z = 0, .w = 0 } };
      child->cmp_strategy = CGCS_EXACT;
      child->cmp_mask = BM_ID;
      child->compare = b_make_block(B_WATER); // TODO: Allow water flow?
      child->rpl_strategy = CGRS_NONE;
      child->rpl_mask = 0;
      child->replace = 0;
      cge_add_child(parent, child);

      // Check the seed block and turn it into a sprout block:
      child = create_cell_grammar_expansion();
      child->type = CGCT_BLOCK_EXACT;
      child->offset = { .xyz = {.x = 0, .y = 0, .z = 0, .w = 1 } };
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
      parent->offset = 0;
      cg_add_expansion(BIO_CG_SPROUT_ABOVE_SOIL, parent);

      // Check the substrate:
      child = create_cell_grammar_expansion();
      child->type = CGCT_BLOCK_EXACT;
      child->offset = { .xyz = {.x = 0, .y = 0, .z = -1, .w = 0 } };
      child->cmp_strategy = CGCS_EXACT;
      child->cmp_mask = BM_ID;
      child->compare = b_make_block(substrate);
      child->rpl_strategy = CGRS_NONE;
      child->rpl_mask = 0;
      child->replace = 0;
      cge_add_child(parent, child);

      // Check the seed block and turn it into a sprout block:
      child = create_cell_grammar_expansion();
      child->type = CGCT_BLOCK_EITHER;
      child->offset = { .xyz = {.x = 0, .y = 0, .z = 0, .w = 1 } };
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
      parent->offset = 0;
      cg_add_expansion(BIO_CG_SPROUT_IN_SOIL, parent);

      // Check the substrate:
      child = create_cell_grammar_expansion();
      child->type = CGCT_BLOCK_EXACT;
      child->offset = { .xyz = {.x = 0, .y = 0, .z = 0, .w = 0 } };
      child->cmp_strategy = CGCS_EXACT;
      child->cmp_mask = BM_ID;
      child->compare = b_make_block(substrate);
      child->rpl_strategy = CGRS_NONE;
      child->rpl_mask = 0;
      child->replace = 0;
      cge_add_child(parent, child);

      // Check for water above:
      child = create_cell_grammar_expansion();
      child->type = CGCT_BLOCK_EXACT;
      child->offset = { .xyz = {.x = 0, .y = 0, .z = 1, .w = 0 } };
      child->cmp_strategy = CGCS_EXACT;
      child->cmp_mask = BM_ID;
      child->compare = b_make_block(B_WATER);
      child->rpl_strategy = CGRS_NONE;
      child->rpl_mask = 0;
      child->replace = 0;
      cge_add_child(parent, child);

      // Check the seed block and turn it into a root block:
      child = create_cell_grammar_expansion();
      child->type = CGCT_BLOCK_EXACT;
      child->offset = { .xyz = {.x = 0, .y = 0, .z = 0, .w = 1 } };
      child->cmp_strategy = CGCS_EXACT;
      child->cmp_mask = BM_ID;
      child->compare = b_make_block(seed);
      child->rpl_strategy = CGRS_EXACT;
      child->rpl_mask = BM_ID;
      child->replace = b_make_block(root); // species is unchanged
      cge_add_child(parent, child);

      // Check for space above the seed and turn it into a sprout:
      child = create_cell_grammar_expansion();
      child->type = CGCT_BLOCK_EXACT;
      child->offset = { .xyz = {.x = 0, .y = 0, .z = 1, .w = 1 } };
      child->cmp_strategy = CGCS_ROOT_GROWTH;
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
      parent->offset = 0;
      cg_add_expansion(BIO_CG_SPROUT_IN_SOIL, parent);

      // Check the substrate:
      child = create_cell_grammar_expansion();
      child->type = CGCT_BLOCK_EXACT;
      child->offset = { .xyz = {.x = 0, .y = 0, .z = 0, .w = 0 } };
      child->cmp_strategy = CGCS_EXACT;
      child->cmp_mask = BM_ID;
      child->compare = b_make_block(substrate);
      child->rpl_strategy = CGRS_NONE;
      child->rpl_mask = 0;
      child->replace = 0;
      cge_add_child(parent, child);

      // Check the seed block and turn it into a root block:
      child = create_cell_grammar_expansion();
      child->type = CGCT_BLOCK_EXACT;
      child->offset = { .xyz = {.x = 0, .y = 0, .z = 0, .w = 1 } };
      child->cmp_strategy = CGCS_EXACT;
      child->cmp_mask = BM_ID;
      child->compare = b_make_block(seed);
      child->rpl_strategy = CGRS_EXACT;
      child->rpl_mask = BM_ID;
      child->replace = b_make_block(root);
      cge_add_child(parent, child);

      // Check for air above the seed and turn it into a sprout:
      child = create_cell_grammar_expansion();
      child->type = CGCT_BLOCK_EITHER; // TODO: Find lower growth resist...
      child->offset = { .xyz = {.x = 0, .y = 0, .z = 1, .w = 0 } };
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
      intermediate->offset = 0;

      // Blocks with solid substance:
      child = create_cell_grammar_expansion();
      child->type = CGET_BLOCK_EITHER;
      child->offset = { .xyz = {.x = 0, .y = 0, .z = 1, .w = 0 } };
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
      child->offset = { .xyz = {.x = 0, .y = 0, .z = 1, .w = 0 } };
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
      intermediate->offset = 0;
      cge_add_child(intermediate, child);

      // Finally add the negation to SPROUT_IN_SOIL:
      cg_add_expansion(BIO_CG_SPROUT_IN_SOIL, intermediate);
    }
  }
}

void setup_biology_gen(void) {
  setup_sprout_grammar(B_HERB_SEEDS);
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



ptrdiff_t get_species_growth_strength(block b, int resist) {
  block id = b_id(b);
  species sp = b_species(b);
  growth_properties *grp = NULL;
  int is_seed = 0;
  if (id == B_VOID) {
    return 0;
  } else if (id == B_AIR || id == B_ETHER) {
    return -1;
  } else if (
     id == B_MUSHROOM_SPORES
  || id == B_MUSHROOM_SHOOTS
  || id == B_MUSHROOM_GROWN
  || id == B_GIANT_MUSHROOM_CORE
  || id == B_GIANT_MUSHROOM_MYCELIUM
  || id == B_GIANT_MUSHROOM_SPROUT
  || id == B_GIANT_MUSHROOM_STALK
  || id == B_GIANT_MUSHROOM_CAP
  ) {
    grp = &(get_fungus_species(sp)->growth);
    is_seed = (id == B_MUSHROOM_SPORES);
  } else if (
       id == B_MOSS_SPORES
    || id == B_MOSS_SHOOTS
    || id == B_MOSS_GROWN
    || id == B_MOSS_FLOWERING
    || id == B_MOSS_FRUITING
  ) {
    grp = &(get_moss_species(sp)->growth);
    is_seed = (id == B_MOSS_SPORES);
  } else if (
       id == B_GRASS_SEEDS
    || id == B_GRASS_ROOTS
    || id == B_GRASS_SHOOTS
    || id == B_GRASS_GROWN
    || id == B_GRASS_BUDDING
    || id == B_GRASS_FLOWERING
    || id == B_GRASS_FRUITING
  ) {
    grp = &(get_grass_species(sp)->growth);
    is_seed = (id == B_GRASS_SEEDS);
  } else if (
       id == B_VINE_SEEDS
    || id == B_VINE_CORE
    || id == B_VINE_ROOTS
    || id == B_VINE_SHOOTS
    || id == B_VINE_SPROUTING
    || id == B_VINE_GROWN
    || id == B_VINE_BUDDING
    || id == B_VINE_FLOWERING
    || id == B_VINE_FRUITING
    || id == B_VINE_SHEDDING
    || id == B_VINE_DORMANT
  ) {
    grp = &(get_vine_species(sp)->growth);
    is_seed = (id == B_VINE_SEEDS);
  } else if (
       id == B_HERB_SEEDS
    || id == B_HERB_CORE
    || id == B_HERB_ROOTS
    || id == B_HERB_SHOOTS
    || id == B_HERB_GROWN
    || id == B_HERB_BUDDING
    || id == B_HERB_FLOWERING
    || id == B_HERB_FRUITING
  ) {
    grp = &(get_herb_species(sp)->growth);
    is_seed = (id == B_HERB_SEEDS);
  } else if (
       id == B_BUSH_SEEDS
    || id == B_BUSH_CORE
    || id == B_BUSH_ROOTS
    || id == B_BUSH_SHOOTS
    || id == B_BUSH_BRANCHES_SPROUTING
    || id == B_BUSH_BRANCHES_GROWN
    || id == B_BUSH_BRANCHES_BUDDING
    || id == B_BUSH_BRANCHES_FLOWERING
    || id == B_BUSH_BRANCHES_FRUITING
    || id == B_BUSH_BRANCHES_SHEDDING
    || id == B_BUSH_BRANCHES_DORMANT
    || id == B_BUSH_LEAVES_SPROUTING
    || id == B_BUSH_LEAVES_GROWN
    || id == B_BUSH_LEAVES_BUDDING
    || id == B_BUSH_LEAVES_FLOWERING
    || id == B_BUSH_LEAVES_FRUITING
    || id == B_BUSH_LEAVES_SHEDDING
    || id == B_BUSH_LEAVES_DORMANT
  ) {
    grp = &(get_bush_species(sp)->growth);
    is_seed = (id == B_BUSH_SEEDS);
  } else if (
       id == B_SHRUB_SEEDS
    || id == B_SHRUB_CORE
    || id == B_SHRUB_ROOTS
    || id == B_SHRUB_THICK_ROOTS
    || id == B_SHRUB_SHOOTS
    || id == B_SHRUB_BRANCHES_SPROUTING
    || id == B_SHRUB_BRANCHES_GROWN
    || id == B_SHRUB_BRANCHES_BUDDING
    || id == B_SHRUB_BRANCHES_FLOWERING
    || id == B_SHRUB_BRANCHES_FRUITING
    || id == B_SHRUB_BRANCHES_SHEDDING
    || id == B_SHRUB_BRANCHES_DORMANT
    || id == B_SHRUB_LEAVES_SPROUTING
    || id == B_SHRUB_LEAVES_GROWN
    || id == B_SHRUB_LEAVES_BUDDING
    || id == B_SHRUB_LEAVES_FLOWERING
    || id == B_SHRUB_LEAVES_FRUITING
    || id == B_SHRUB_LEAVES_SHEDDING
    || id == B_SHRUB_LEAVES_DORMANT
  ) {
    grp = &(get_shrub_species(sp)->growth);
    is_seed = (id == B_SHRUB_SEEDS);
  } else if (
       id == B_TREE_SEEDS
    || id == B_TREE_CORE
    || id == B_TREE_THICK_CORE
    || id == B_TREE_ROOTS
    || id == B_TREE_THICK_ROOTS
    || id == B_TREE_HEART_ROOTS
    || id == B_TREE_SHOOTS
    || id == B_TREE_TRUNK
    || id == B_TREE_BARE_BRANCHES
    || id == B_TREE_BRANCHES_SPROUTING
    || id == B_TREE_BRANCHES_GROWN
    || id == B_TREE_BRANCHES_BUDDING
    || id == B_TREE_BRANCHES_FLOWERING
    || id == B_TREE_BRANCHES_FRUITING
    || id == B_TREE_BRANCHES_SHEDDING
    || id == B_TREE_BRANCHES_DORMANT
    || id == B_TREE_LEAVES_SPROUTING
    || id == B_TREE_LEAVES_GROWN
    || id == B_TREE_LEAVES_BUDDING
    || id == B_TREE_LEAVES_FLOWERING
    || id == B_TREE_LEAVES_FRUITING
    || id == B_TREE_LEAVES_SHEDDING
    || id == B_TREE_LEAVES_DORMANT
  ) {
    grp = &(get_tree_species(sp)->growth);
    is_seed = (id == B_TREE_SEEDS);
  } else if (
       id == B_AQUATIC_GRASS_SEEDS
    || id == B_AQUATIC_GRASS_ROOTS
    || id == B_AQUATIC_GRASS_SHOOTS
    || id == B_AQUATIC_GRASS_GROWN
    || id == B_AQUATIC_GRASS_FLOWERING
    || id == B_AQUATIC_GRASS_FRUITING
  ) {
    grp = &(get_aquatic_grass_species(sp)->growth);
    is_seed = (id == B_AQUATIC_GRASS_SEEDS);
  } else if (
       id == B_AQUATIC_PLANT_SEEDS
    || id == B_AQUATIC_PLANT_CORE
    || id == B_AQUATIC_PLANT_ANCHORS
    || id == B_AQUATIC_PLANT_SHOOTS
    || id == B_AQUATIC_PLANT_STEMS
    || id == B_AQUATIC_PLANT_LEAVES_GROWN
    || id == B_AQUATIC_PLANT_LEAVES_FLOWERING
    || id == B_AQUATIC_PLANT_LEAVES_FRUITING
  ) {
    grp = &(get_aquatic_plant_species(sp)->growth);
    is_seed = (id == B_AQUATIC_PLANT_SEEDS);
  } else if (
       id == B_YOUNG_CORAL
    || id == B_CORAL_CORE
    || id == B_CORAL_BODY
    || id == B_CORAL_FROND
  ) {
    grp = &(get_coral_species(sp)->growth);
    is_seed = (id == B_YOUNG_CORAL);
  } else {
    // A non-air non-water non-plant block: can't grow here.
    return 1000;
  }
  // Unpack the desired property:
  if (resist) {
    if (is_seed) {
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
  target->seeds.origin = MO_ORGANIC;
  target->roots.origin = MO_ORGANIC;
  target->wood.origin = MO_ORGANIC;
  target->dry_wood.origin = MO_ORGANIC;
  target->stem.origin = MO_ORGANIC;
  target->leaf.origin = MO_ORGANIC;
  target->fruit.origin = MO_ORGANIC;

  // TODO: Look up plant densities
  // Solid densities are relative to water.
  target->seeds.solid_density = mat_density(0.4);
  target->root.solid_density = mat_density(0.25);
  target->wood.solid_density = mat_density(0.25);
  target->dry_wood.solid_density = mat_density(0.25);
  target->stem.solid_density = mat_density(0.25);
  target->leaf.solid_density = mat_density(0.25);
  target->fruit.solid_density = mat_density(0.75);

  // Liquid densities are relative to water.
  target->seeds.liquid_density = mat_density(0.6);
  target->root.liquid_density = mat_density(0.5);
  target->wood.liquid_density = mat_density(0.5);
  target->dry_wood.liquid_density = mat_density(0.5);
  target->stem.liquid_density = mat_density(0.5);
  target->leaf.liquid_density = mat_density(0.5);
  target->fruit.liquid_density = mat_density(0.9);

  // Gas densities are relative to air.
  target->seeds.gas_density = mat_density(1.2);
  target->root.gas_density = mat_density(1.2);
  target->wood.gas_density = mat_density(1.2);
  target->dry_wood.gas_density = mat_density(1.2);
  target->stem.gas_density = mat_density(1.2);
  target->leaf.gas_density = mat_density(1.2);
  target->fruit.gas_density = mat_density(1.4);

  // TODO: Something sensible here...
  // Specific heats are relative to air.
  target->seeds.solid_specific_heat = mat_specific_heat(1.0);
  target->root.solid_specific_heat = mat_specific_heat(1.0);
  target->wood.solid_specific_heat = mat_specific_heat(1.0);
  target->dry_wood.solid_specific_heat = mat_specific_heat(1.0);
  target->stem.solid_specific_heat = mat_specific_heat(1.0);
  target->leaf.solid_specific_heat = mat_specific_heat(1.0);
  target->fruit.solid_specific_heat = mat_specific_heat(1.0);

  // TODO: RNG here!
  // Freezing is bad for plants.
  target->seeds.cold_damage_temp = 0;
  target->root.cold_damage_temp = 0;
  target->wood.cold_damage_temp = -10;
  target->dry_wood.cold_damage_temp = -40;
  target->stem.cold_damage_temp = 0;
  target->leaf.cold_damage_temp = 0;
  target->fruit.cold_damage_temp = 0;

  // TODO: Real numbers here!
  target->seeds.solidus = 250;
  target->root.solidus = 250;
  target->wood.solidus = 250;
  target->dry_wood.solidus = 250;
  target->stem.solidus = 250;
  target->leaf.solidus = 250;
  target->fruit.solidus = 250;

  target->seeds.liquidus = 300;
  target->root.liquidus = 300;
  target->wood.liquidus = 300;
  target->dry_wood.liquidus = 300;
  target->stem.liquidus = 300;
  target->leaf.liquidus = 300;
  target->fruit.liquidus = 300;

  target->seeds.boiling_point = 900;
  target->root.boiling_point = 900;
  target->wood.boiling_point = 900;
  target->dry_wood.boiling_point = 900;
  target->stem.boiling_point = 900;
  target->leaf.boiling_point = 900;
  target->fruit.boiling_point = 900;

  target->seeds.ignition_point = 101; // just post-boiling
  target->root.ignition_point = 101; // just post-boiling
  target->wood.ignition_point = 101; // just post-boiling
  target->dry_wood.ignition_point = 101; // just post-boiling
  target->stem.ignition_point = 101; // just post-boiling
  target->leaf.ignition_point = 101; // just post-boiling
  target->fruit.ignition_point = 101; // just post-boiling

  target->seeds.flash_point = 200;
  target->root.flash_point = 200;
  target->wood.flash_point = 250;
  target->dry_wood.flash_point = 180;
  target->stem.flash_point = 200;
  target->leaf.flash_point = 200;
  target->fruit.flash_point = 200;

  // TODO: Real numbers here!
  target->seeds.malleability = 0;
  target->root.malleability = 0;
  target->wood.malleability = 0;
  target->dry_wood.malleability = 0;
  target->stem.malleability = 0;
  target->leaf.malleability = 0;
  target->fruit.malleability = 0;

  // Relative to water.
  target->seeds.viscosity = 0;
  target->root.viscosity = 0;
  target->wood.viscosity = 0;
  target->dry_wood.viscosity = 0;
  target->stem.viscosity = 0;
  target->leaf.viscosity = 0;
  target->fruit.viscosity = 0;

  // Wood & fingernails ~= 60; stone is mostly 100-220
  target->seeds.hardness = 45;
  target->root.hardness = 15;
  target->wood.hardness = 60;
  target->dry_wood.hardness = 65;
  target->stem.hardness = 20;
  target->leaf.hardness = 15;
  target->fruit.hardness = 15;
}

void determine_new_herb_appearance(herb_appearance *target, seed) {
}

void determine_new_herb_core_growth(core_growth *target, seed) {
}

