#if defined(ELFSCRIPT_REGISTER_DECLARATIONS)
// no declarations
#elif defined(ELFSCRIPT_REGISTER_FORMATS)
{
  .key = "material",
  .unpacker = elfscript__material,
  .packer = material__elfscript,
  .copier = copy_v_material,
  .destructor = cleanup_v_material
},
#else
#ifndef INCLUDE_ELFSCRIPT_CONV_MATERIAL_H
#define INCLUDE_ELFSCRIPT_CONV_MATERIAL_H
// elfscript_material.h
// Conversions elfscript <-> material

#include <stdio.h>

#include "elfscript/elfscript.h"
#include "world/materials.h"

void* elfscript__material(es_scope *sc) {
  material *result;

  SSTR(s_origin, "origin", 6);
  SSTR(s_solid_density, "solid_density", 13);
  SSTR(s_liquid_density, "liquid_density", 14);
  SSTR(s_gas_density, "gas_density", 11);

  SSTR(s_solid_specific_heat, "solid_specific_heat", 19);
  SSTR(s_liquid_specific_heat, "liquid_specific_heat", 20);
  SSTR(s_gas_specific_heat, "gas_specific_heat", 17);

  SSTR(s_cold_damage_temp, "cold_damage_temp", 16);

  SSTR(s_solidus, "solidus", 7);
  SSTR(s_liquidus, "liquidus", 8);
  SSTR(s_boiling_point, "boiling_point", 13);
  SSTR(s_ignition_point, "ignition_point", 14);
  SSTR(s_flash_point, "flash_point", 11);

  SSTR(s_cold_plastic_temp, "cold_plastic_temp", 17);
  SSTR(s_warm_plastic_temp, "warm_plastic_temp", 17);
  SSTR(s_cold_plasticity, "cold_plasticity", 15);
  SSTR(s_warm_plasticity, "warm_plasticity", 15);

  SSTR(s_viscosity, "viscosity", 9);
  SSTR(s_hardness, "hardness", 8);

  result = create_material();

  result->origin = (material_origin) es_as_i(
    es_read_var(sc, s_origin)
  );

  result->solid_density = (density) es_as_i(
    es_read_var(sc, s_solid_density)
  );
  result->liquid_density = (density) es_as_i(
    es_read_var(sc, s_liquid_density)
  );
  result->gas_density = (density) es_as_i(
    es_read_var(sc, s_gas_density)
  );

  result->solid_specific_heat = (specific_heat) es_as_i(
    es_read_var(sc, s_solid_specific_heat)
  );
  result->liquid_specific_heat = (specific_heat) es_as_i(
    es_read_var(sc, s_liquid_specific_heat)
  );
  result->gas_specific_heat = (specific_heat) es_as_i(
    es_read_var(sc, s_gas_specific_heat)
  );

  result->cold_damage_temp = (temperature) es_as_i(
    es_read_var(sc, s_cold_damage_temp)
  );

  result->solidus = (temperature) es_as_i(
    es_read_var(sc, s_solidus)
  );
  result->liquidus = (temperature) es_as_i(
    es_read_var(sc, s_liquidus)
  );
  result->boiling_point = (temperature) es_as_i(
    es_read_var(sc, s_boiling_point)
  );
  result->ignition_point = (temperature) es_as_i(
    es_read_var(sc, s_ignition_point)
  );
  result->flash_point = (temperature) es_as_i(
    es_read_var(sc, s_flash_point)
  );

  result->cold_plastic_temp = (temperature) es_as_i(
    es_read_var(sc, s_cold_plastic_temp)
  );
  result->warm_plastic_temp = (temperature) es_as_i(
    es_read_var(sc, s_warm_plastic_temp)
  );
  result->cold_plasticity = (temperature) es_as_i(
    es_read_var(sc, s_cold_plasticity)
  );
  result->warm_plasticity = (temperature) es_as_i(
    es_read_var(sc, s_warm_plasticity)
  );

  result->viscosity = (viscosity) es_as_n(
    es_read_var(sc, s_viscosity)
  );
  result->hardness = (hardness) es_as_i(
    es_read_var(sc, s_hardness)
  );

  return (void*) result;
}

es_scope *material__elfscript(void *v_mat) {
  material *mat = (material*) v_mat;
  es_scope *result;

  SSTR(s_origin, "origin", 6);
  SSTR(s_solid_density, "solid_density", 12);
  SSTR(s_liquid_density, "liquid_density", 14);
  SSTR(s_gas_density, "gas_density", 11);

  SSTR(s_solid_specific_heat, "solid_specific_heat", 19);
  SSTR(s_liquid_specific_heat, "liquid_specific_heat", 20);
  SSTR(s_gas_specific_heat, "gas_specific_heat", 17);

  SSTR(s_cold_damage_temp, "cold_damage_temp", 16);

  SSTR(s_solidus, "solidus", 7);
  SSTR(s_liquidus, "liquidus", 8);
  SSTR(s_boiling_point, "boiling_point", 13);
  SSTR(s_ignition_point, "ignition_point", 14);
  SSTR(s_flash_point, "flash_point", 11);

  SSTR(s_cold_plastic_temp, "cold_plastic_temp", 17);
  SSTR(s_warm_plastic_temp, "warm_plastic_temp", 17);
  SSTR(s_cold_plasticity, "cold_plasticity", 15);
  SSTR(s_warm_plasticity, "warm_plasticity", 15);

  SSTR(s_viscosity, "viscosity", 9);
  SSTR(s_hardness, "hardness", 8);

  result = create_es_scope();

  es_write_var(
    sc,
    s_origin,
    create_es_int_var((es_int_t) mat->origin)
  );

  es_write_var(
    sc,
    s_solid_density,
    create_es_int_var((es_int_t) mat->solid_density)
  );
  es_write_var(
    sc,
    s_liquid_density,
    create_es_int_var((es_int_t) mat->liquid_density)
  );
  es_write_var(
    sc,
    s_gas_density,
    create_es_int_var((es_int_t) mat->gas_density)
  );

  es_write_var(
    sc,
    s_solid_specific_heat,
    create_es_int_var((es_int_t) mat->solid_specific_heat)
  );
  es_write_var(
    sc,
    s_liquid_specific_heat,
    create_es_int_var((es_int_t) mat->liquid_specific_heat)
  );
  es_write_var(
    sc,
    s_gas_specific_heat,
    create_es_int_var((es_int_t) mat->gas_specific_heat)
  );

  es_write_var(
    sc,
    s_cold_damage_temp,
    create_es_int_var((es_int_t) mat->cold_damage_temp)
  );

  es_write_var(
    sc,
    s_solidus,
    create_es_int_var((es_int_t) mat->solidus)
  );
  es_write_var(
    sc,
    s_liquidus,
    create_es_int_var((es_int_t) mat->liquidus)
  );
  es_write_var(
    sc,
    s_boiling_point,
    create_es_int_var((es_int_t) mat->boiling_point)
  );
  es_write_var(
    sc,
    s_ignition_point,
    create_es_int_var((es_int_t) mat->ignition_point)
  );
  es_write_var(
    sc,
    s_flash_point,
    create_es_int_var((es_int_t) mat->flash_point)
  );

  es_write_var(
    sc,
    s_cold_plastic_temp,
    create_es_int_var((es_int_t) mat->cold_plastic_temp)
  );
  es_write_var(
    sc,
    s_warm_plastic_temp,
    create_es_int_var((es_int_t) mat->warm_plastic_temp)
  );
  es_write_var(
    sc,
    s_cold_plasticity,
    create_es_int_var((es_int_t) mat->cold_plasticity)
  );
  es_write_var(
    sc,
    s_warm_plasticity,
    create_es_int_var((es_int_t) mat->warm_plasticity)
  );

  es_write_var(
    sc,
    s_viscosity,
    create_es_int_var((es_int_t) mat->viscosity)
  );
  es_write_var(
    sc,
    s_hardness,
    create_es_int_var((es_int_t) mat->hardness)
  );

  return result;
}

#endif // INCLUDE_ELFSCRIPT_CONV_MATERIAL_H
#endif // ELFSCRIPT_REGISTRATION
