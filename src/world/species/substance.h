#ifndef SUBSTANCE_SPECIES_H
#define SUBSTANCE_SPECIES_H

/**************
 * Structures *
 **************/

struct gas_species_s;
typedef struct gas_species_s gas_species;

/*************************
 * Structure Definitions *
 *************************/

struct gas_species_s {
  material material;
  // gas_texture_params appearance;
};

#endif // #ifndef SUBSTANCE_SPECIES_H
