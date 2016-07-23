#if defined(EFD_REGISTER_DECLARATIONS)
// no declarations
#elif defined(EFD_REGISTER_FORMATS)
{
  .key = "material",
  .unpacker = efd__material,
  .packer = material__efd,
  .copier = copy_v_material,
  .destructor = cleanup_v_material
},
#else
#ifndef INCLUDE_EFD_CONV_MATERIAL_H
#define INCLUDE_EFD_CONV_MATERIAL_H
// efd_material.h
// Conversions efd <-> material

#include <stdio.h>

#include "efd/efd.h"
#include "world/materials.h"

void* efd__material(efd_node *n) {
  material *result;
  efd_node *val;

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

  efd_assert_type(n, EFD_NT_CONTAINER);

  if (efd_normal_child_count(n) != 1) {
    efd_report_error(
      s_("ERROR: 'material' proto must have exactly 1 child."),
      n
    );
    exit(EXIT_FAILURE);
  }

  val = efd_fresh_value(efd_nth(n, 0));

  efd_assert_type(val, EFD_NT_CONTAINER);

  result = create_material();

  result->origin = (material_origin) efd_as_i(efd_lookup(val, s_origin));

  result->solid_density = (density) efd_as_i(efd_lookup(val, s_solid_density));
  result->liquid_density = (density)efd_as_i(efd_lookup(val,s_liquid_density));
  result->gas_density = (density) efd_as_i(efd_lookup(val, s_gas_density));

  result->solid_specific_heat = (specific_heat) efd_as_i(
    efd_lookup(val, s_solid_specific_heat)
  );
  result->liquid_specific_heat = (specific_heat) efd_as_i(
    efd_lookup(val, s_liquid_specific_heat)
  );
  result->gas_specific_heat = (specific_heat) efd_as_i(
    efd_lookup(val, s_gas_specific_heat)
  );

  result->cold_damage_temp = (temperature) efd_as_i(
    efd_lookup(val, s_cold_damage_temp)
  );

  result->solidus = (temperature) efd_as_i(efd_lookup(val, s_solidus));
  result->liquidus = (temperature) efd_as_i(efd_lookup(val, s_liquidus));
  result->boiling_point = (temperature) efd_as_i(
    efd_lookup(val, s_boiling_point)
  );
  result->ignition_point = (temperature) efd_as_i(
    efd_lookup(val, s_ignition_point)
  );
  result->flash_point = (temperature) efd_as_i(efd_lookup(val, s_flash_point));

  result->cold_plastic_temp = (temperature) efd_as_i(
    efd_lookup(val, s_cold_plastic_temp)
  );
  result->warm_plastic_temp = (temperature) efd_as_i(
    efd_lookup(val, s_warm_plastic_temp)
  );
  result->cold_plasticity = (temperature) efd_as_i(
    efd_lookup(val, s_cold_plasticity)
  );
  result->warm_plasticity = (temperature) efd_as_i(
    efd_lookup(val, s_warm_plasticity)
  );

  result->viscosity = (viscosity) efd_as_n(efd_lookup(val, s_viscosity));
  result->hardness = (hardness) efd_as_i(efd_lookup(val, s_hardness));

  return (void*) result;
}

efd_node *material__efd(void *v_mat) {
  material *mat = (material*) v_mat;
  efd_node *result, *child;

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

  result = create_efd_node(EFD_NT_CONTAINER, EFD_ANON_NAME);

  child = create_efd_node(EFD_NT_CONTAINER, EFD_ANON_NAME);

  efd_add_child(result, child);

  efd_add_child(
    child,
    construct_efd_int_node(s_origin, (efd_int_t) mat->origin)
  );

  efd_add_child(
    child,
    construct_efd_int_node(s_solid_density, (efd_int_t) mat->solid_density)
  );
  efd_add_child(
    child,
    construct_efd_int_node(s_liquid_density, (efd_int_t) mat->liquid_density)
  );
  efd_add_child(
    child,
    construct_efd_int_node(s_gas_density, (efd_int_t) mat->gas_density)
  );

  efd_add_child(
    child,
    construct_efd_int_node(
      s_solid_specific_heat,
      (efd_int_t) mat->solid_specific_heat
    )
  );
  efd_add_child(
    child,
    construct_efd_int_node(
      s_liquid_specific_heat,
      (efd_int_t) mat->liquid_specific_heat
    )
  );
  efd_add_child(
    child,
    construct_efd_int_node(
      s_gas_specific_heat,
      (efd_int_t) mat->gas_specific_heat
    )
  );

  efd_add_child(
    child,
    construct_efd_int_node(
      s_cold_damage_temp,
      (efd_int_t) mat->cold_damage_temp
    )
  );

  efd_add_child(
    child,
    construct_efd_int_node(s_solidus, (efd_int_t) mat->solidus)
  );
  efd_add_child(
    child,
    construct_efd_int_node(s_liquidus, (efd_int_t) mat->liquidus)
  );
  efd_add_child(
    child,
    construct_efd_int_node(s_boiling_point, (efd_int_t) mat->boiling_point)
  );
  efd_add_child(
    child,
    construct_efd_int_node(s_ignition_point, (efd_int_t) mat->ignition_point)
  );
  efd_add_child(
    child,
    construct_efd_int_node(s_flash_point, (efd_int_t) mat->flash_point)
  );

  efd_add_child(
    child,
    construct_efd_int_node(
      s_cold_plastic_temp,
      (efd_int_t) mat->cold_plastic_temp
    )
  );
  efd_add_child(
    child,
    construct_efd_int_node(
      s_warm_plastic_temp,
      (efd_int_t) mat->warm_plastic_temp
    )
  );
  efd_add_child(
    child,
    construct_efd_int_node(s_cold_plasticity, (efd_int_t) mat->cold_plasticity)
  );
  efd_add_child(
    child,
    construct_efd_int_node(s_warm_plasticity, (efd_int_t) mat->warm_plasticity)
  );

  efd_add_child(
    child,
    construct_efd_int_node(s_viscosity, (efd_int_t) mat->viscosity)
  );
  efd_add_child(
    child,
    construct_efd_int_node(s_hardness, (efd_int_t) mat->hardness)
  );

  return result;
}

#endif // INCLUDE_EFD_CONV_MATERIAL_H
#endif // EFD_REGISTRATION
