#ifndef ANIMAL_SPECIES_H
#define ANIMAL_SPECIES_H

/**************
 * Structures *
 **************/

// Primary species structures:

struct animal_species_s; // non-sentient animals
typedef struct animal_species_s animal_species;
struct mythical_species_s; // mythical creatures
typedef struct mythical_species_s mythical_species;
struct sentient_species_s; // sentient species
typedef struct sentient_species_s sentient_species;

/*************************
 * Structure Definitions *
 *************************/

struct animal_species_s {
  species id;

  material material;
  // entity_texture_params appearance;
};

struct mythical_species_s {
  species id;

  material material;
  // entity_texture_params appearance;
};

struct sentient_species_s {
  species id;

  material material;
  // entity_texture_params appearance;
};

#endif // #ifndef ANIMAL_SPECIES_H
