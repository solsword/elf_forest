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


// Various parameters that determine the appearance of an herb.
struct herb_appearance_s;
typedef struct herb_appearance_s herb_appearance;

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

struct herb_appearance_s {
  // TODO: More appearance diversity?
  leaves_filter_args seeds;
  branch_filter_args roots;
  herb_leaves_filter_args shoots;
  herb_leaves_filter_args leaves;
  leaves_filter_args buds;
  leaves_filter_args flowers;
  leaves_filter_args fruit;
};

// Primary structures:

struct fungus_species_s {
  species id;

  fungus_materials materials;
  // fungus_texture_params appearance;
  growth_properties growth;
};

struct moss_species_s {
  species id;

  plant_materials materials;
  // moss_texture_params appearance;
  growth_properties growth;
};

struct grass_species_s {
  species id;

  plant_materials materials;
  // grass_texture_params appearance;
  growth_properties growth;
};

struct vine_species_s {
  species id;

  plant_materials materials;
  // vine_texture_params appearance;
  growth_properties growth;
};

struct herb_species_s {
  species id;

  plant_materials materials;
  herb_appearance appearance;
  growth_properties growth;
};

struct bush_species_s {
  species id;

  plant_materials materials;
  // bush_texture_params appearance;
  growth_properties growth;
};

struct shrub_species_s {
  species id;

  plant_materials materials;
  // shrub_texture_params appearance;
  growth_properties growth;
};

struct tree_species_s {
  species id;

  plant_materials materials;
  // tree_texture_params appearance;
  growth_properties growth;
};

struct aquatic_grass_species_s {
  species id;

  plant_materials materials;
  // aquatic_grass_texture_params appearance;
  growth_properties growth;
};

struct aquatic_plant_species_s {
  species id;

  plant_materials materials;
  // aquatic_plant_texture_params appearance;
  growth_properties growth;
};

struct coral_species_s {
  species id;

  coral_materials materials;
  // coral_texture_params appearance;
  growth_properties growth;
};

#endif // #ifndef VEGETABLE_SPECIES_H
