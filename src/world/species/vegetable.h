#ifndef VEGETABLE_SPECIES_H
#define VEGETABLE_SPECIES_H
// Vegetable species types.

/**************
 * Structures *
 **************/

// Primary species structures:

struct fungus_species_s;
typedef struct fungus_species_s fungus_species;
struct moss_species_s;
typedef struct moss_species_s moss_species;
struct grass_species_s;
typedef struct grass_species_s grass_species;
struct vine_species_s;
typedef struct vine_species_s vine_species;
struct herb_species_s;
typedef struct herb_species_s herb_species;
struct bush_species_s;
typedef struct bush_species_s bush_species;
struct shrub_species_s;
typedef struct shrub_species_s shrub_species;
struct tree_species_s;
typedef struct tree_species_s tree_species;
struct aquatic_grass_species_s;
typedef struct aquatic_grass_species_s aquatic_grass_species;
struct aquatic_plant_species_s;
typedef struct aquatic_plant_species_s aquatic_plant_species;
struct coral_species_s;
typedef struct coral_species_s coral_species;


// Supporting structures:

// Growth properties are common across all growing things and contain
// information like growth priority and patterns.
struct growth_properties_s;
typedef struct growth_properties_s growth_properties;

// Growth patterns determine how growing things grow.
// Seed growth patterns are used for seed-type blocks.
struct seed_growth_pattern_s;
typedef struct seed_growth_pattern_s seed_growth_pattern;

// Core growth patterns are used for growth-core-type blocks.
struct core_growth_pattern_s;
typedef struct core_growth_pattern_s core_growth_pattern;


// These materials structures each bundle together a few materials that define
// different parts of an organism:

struct fungus_materials_s;
typedef struct fungus_materials_s fungus_materials;

struct plant_materials_s;
typedef struct plant_materials_s plant_materials;

struct coral_materials_s;
typedef struct coral_materials_s coral_materials;

/*************************
 * Structure Definitions *
 *************************/

// Secondary structures:

struct seed_growth_pattern_s {
  cell_grammar *grammar;
  // TODO: More complex growth speeds (by block type; age; for different growth
  // events; etc.)
  int sprout_time;
  // TODO: More here?
};

struct core_growth_pattern_s {
  // TODO: This
};

struct growth_properties_s {
  int seed_growth_resist;
  int growth_resist;
  int growth_strength;
  seed_growth_pattern seed_growth;
  core_growth_pattern core_growth;
};

struct fungus_materials_s {
  material spores;
  material mycelium;
  material stalk;
  material cap;
};

struct plant_materials_s {
  material seed;
  material root;
  material wood;
  material dry_wood;
  material stem;
  material leaf;
  material fruit;
};

struct coral_materials_s {
  material anchor;
  material frond;
};

// Primary structures:

struct fungus_species_s {
  species id;

  fungus_materials materials;
  // fungus_texture_params appearance;
  growth_properties growth;
  adaptation_list adaptations;
};

struct moss_species_s {
  species id;

  plant_materials materials;
  herbaceous_appearance appearance;
  growth_properties growth;
  adaptation_list adaptations;
};

struct grass_species_s {
  species id;

  plant_materials materials;
  herbaceous_appearance appearance;
  growth_properties growth;
  adaptation_list adaptations;
};

struct vine_species_s {
  species id;

  plant_materials materials;
  bush_appearance appearance;
  growth_properties growth;
  adaptation_list adaptations;
};

struct herb_species_s {
  species id;

  plant_materials materials;
  herbaceous_appearance appearance;
  growth_properties growth;
  adaptation_list adaptations;
};

struct bush_species_s {
  species id;

  plant_materials materials;
  bush_appearance appearance;
  growth_properties growth;
  adaptation_list adaptations;
};

struct shrub_species_s {
  species id;

  plant_materials materials;
  bush_appearance appearance;
  growth_properties growth;
  adaptation_list adaptations;
};

struct tree_species_s {
  species id;

  plant_materials materials;
  tree_appearance appearance;
  growth_properties growth;
  adaptation_list adaptations;
};

struct aquatic_grass_species_s {
  species id;

  plant_materials materials;
  herbaceous_appearance appearance;
  growth_properties growth;
  adaptation_list adaptations;
};

struct aquatic_plant_species_s {
  species id;

  plant_materials materials;
  herbaceous_appearance appearance;
  growth_properties growth;
  adaptation_list adaptations;
};

struct coral_species_s {
  species id;

  coral_materials materials;
  coral_appearance appearance;
  growth_properties growth;
  adaptation_list adaptations;
};

/********************
 * Inline Functions *
 ********************/

// Returns the block type corresponding to the seeds of the given species type.
static inline block seed_block_type(species_type spt) {
  switch (spt) {
    default:
      return B_VOID;
    case SPT_FUNGUS:
      return B_MUSHROOM_SPORES;
      // TODO: Giant mushrooms?
    case SPT_MOSS:
      return B_MOSS_SPORES;
    case SPT_GRASS:
      return B_GRASS_SPORES;
    case SPT_VINE:
      return B_VINE_SEEDS;
    case SPT_HERB:
      return B_HERB_SEEDS;
    case SPT_BUSH:
      return B_BUSH_SEEDS;
    case SPT_SHRUB:
      return B_SHRUB_SEEDS;
    case SPT_TREE:
      return B_TREE_SEEDS;
    case SPT_AQUATIC_GRASS:
      return B_AQUATIC_GRASS_SEEDS;
    case SPT_AQUATIC_PLANT:
      return B_AQUATIC_PLANT_SEEDS;
    case SPT_CORAL:
      return B_YOUNG_CORAL;
  }
}

#endif // #ifndef VEGETABLE_SPECIES_H
