// materials.c
// Material management and properties.

#include "util.h"

#include "blocks.h"
#include "materials.h"

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

