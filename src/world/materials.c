// materials.c
// Material management and properties.

#include "util.h"

#include "blocks.h"
#include "materials.h"

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates and returns a new material.
material *create_material(void) {
  material *result = (material*) malloc(sizeof(material));
  result->origin = MO_UNKNOWN;
  result->solid_density = 0;
  result->liquid_density = 0;
  result->gas_density = 0;
  result->solid_specific_heat = 0;
  result->liquid_specific_heat = 0;
  result->gas_specific_heat = 0;
  result->cold_damage_temp = 0;
  result->solidus = 0;
  result->liquidus = 0;
  result->boiling_point = 0;
  result->ignition_point = 0;
  result->flash_point = 0;
  result->cold_plastic_temp = 0;
  result->warm_plastic_temp = 0;
  result->cold_plasticity = 0;
  result->warm_plasticity = 0;
  result->viscosity = 0;
  result->hardness = 0;
  return result;
}

material *copy_material(material *source) {
  material *result = (material*) malloc(sizeof(material));
  result->origin = source->origin;
  result->solid_density = source->solid_density;
  result->liquid_density = source->liquid_density;
  result->gas_density = source->gas_density;
  result->solid_specific_heat = source->solid_specific_heat;
  result->liquid_specific_heat = source->liquid_specific_heat;
  result->gas_specific_heat = source->gas_specific_heat;
  result->cold_damage_temp = source->cold_damage_temp;
  result->solidus = source->solidus;
  result->liquidus = source->liquidus;
  result->boiling_point = source->boiling_point;
  result->ignition_point = source->ignition_point;
  result->flash_point = source->flash_point;
  result->cold_plastic_temp = source->cold_plastic_temp;
  result->warm_plastic_temp = source->warm_plastic_temp;
  result->cold_plasticity = source->cold_plasticity;
  result->warm_plasticity = source->warm_plasticity;
  result->viscosity = source->viscosity;
  result->hardness = source->hardness;
  return result;
}

void *copy_v_material(void *source) {
  return (void*) copy_material((material*) source);
}

CLEANUP_IMPL(material) {
  free(doomed);
}

void set_material(material *dest, material const * const source) {
  dest->origin = source->origin;
  dest->solid_density = source->solid_density;
  dest->liquid_density = source->liquid_density;
  dest->gas_density = source->gas_density;
  dest->solid_specific_heat = source->solid_specific_heat;
  dest->liquid_specific_heat = source->liquid_specific_heat;
  dest->gas_specific_heat = source->gas_specific_heat;
  dest->cold_damage_temp = source->cold_damage_temp;
  dest->solidus = source->solidus;
  dest->liquidus = source->liquidus;
  dest->boiling_point = source->boiling_point;
  dest->ignition_point = source->ignition_point;
  dest->flash_point = source->flash_point;
  dest->cold_plastic_temp = source->cold_plastic_temp;
  dest->warm_plastic_temp = source->warm_plastic_temp;
  dest->cold_plasticity = source->cold_plasticity;
  dest->warm_plasticity = source->warm_plasticity;
  dest->viscosity = source->viscosity;
  dest->hardness = source->hardness;
}

/*************
 * Functions *
 *************/

void mutate_material(
  material const * const src,
  material *dst,
  ptrdiff_t seed
) {
  int tmpi;
  float tmpf;
  seed = prng(seed + 912881);

  dst->origin = src->origin;

  dst->solid_density = src->solid_density * randf(seed, 0.95, 1.05);
  seed = prng(seed);

  tmpf = randf(seed, 0.95, 1.05);
  seed = prng(seed);
  dst->solid_density = src->solid_density * tmpf;
  dst->liquid_density = src->liquid_density * tmpf;
  dst->gas_density = src->gas_density * tmpf;

  tmpf = randf(seed, 0.95, 1.05);
  seed = prng(seed);
  dst->solid_specific_heat = src->solid_specific_heat * tmpf;
  dst->liquid_specific_heat = src->liquid_specific_heat * tmpf;
  dst->gas_specific_heat = src->gas_specific_heat * tmpf;

  dst->cold_damage_temp = src->cold_damage_temp * randf(seed, 0.9, 1.1);
  seed = prng(seed);

  tmpf = randf(seed, 0.9, 1.1);
  seed = prng(seed);
  dst->solidus = src->solidus * tmpf;
  dst->liquidus = src->liquidus * tmpf;
  dst->boiling_point = src->boiling_point * tmpf;
  dst->ignition_point = src->ignition_point * tmpf;
  dst->flash_point = src->flash_point * tmpf;

  dst->cold_plastic_temp = src->cold_plastic_temp * tmpf;
  dst->warm_plastic_temp = src->warm_plastic_temp * tmpf;

  tmpi = randi(seed, -8, 8);
  dst->cold_plasticity = src->cold_plasticity;
  if (
     dst->cold_plasticity >= 8
  && dst->cold_plasticity <= umaxof(plasticity) - 16
  ) {
    if (dst->cold_plasticity < 16) {
      dst->cold_plasticity += tmpi / 4;
    } else if (dst->cold_plasticity < 32) {
      dst->cold_plasticity += tmpi / 2;
    } else {
      dst->cold_plasticity += tmpi;
    }
  }

  dst->warm_plasticity = src->warm_plasticity;
  if (
     dst->warm_plasticity >= 8
  && dst->warm_plasticity <= umaxof(plasticity) - 16
  ) {
    if (dst->warm_plasticity < 16) {
      dst->warm_plasticity += tmpi / 4;
    } else if (dst->warm_plasticity < 32) {
      dst->warm_plasticity += tmpi / 2;
    } else {
      dst->warm_plasticity += tmpi;
    }
  }

  dst->viscosity = src->viscosity * randf(seed, 0.9, 1.1);
  seed = prng(seed);

  if (src->hardness > 8 && src->hardness < umaxof(hardness) - 8) {
    dst->hardness = src->hardness + randi(seed, -6, 6);
    seed = prng(seed);
  } else {
    dst->hardness = src->hardness;
  }
}

