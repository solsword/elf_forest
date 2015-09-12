#ifndef MATERIAL_SPECIES_H
#define MATERIAL_SPECIES_H

/**************
 * Structures *
 **************/

struct fiber_species_s;
typedef struct fiber_species_s fiber_species;

struct pigment_species_s;
typedef struct pigment_species_s pigment_species;

/*************************
 * Structure Definitions *
 *************************/

struct fiber_species_s {
  material material;
  // fiber_texture_params appearance;
};

struct pigment_species_s {
  material material;
  // pigment_texture_params appearance;
};

#endif // #ifndef MATERIAL_SPECIES_H
