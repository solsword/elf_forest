#if defined(EFD_REGISTER_DECLARATIONS)
// no declarations
#elif defined(EFD_REGISTER_FORMATS)
{
  .key = "stone_species",
  .unpacker = efd__stone_species,
  .packer = stone_species__efd,
  .copier = copy_v_stone_species,
  .destructor = cleanup_v_stone_species
},
#else
#ifndef INCLUDE_EFD_CONV_STONE_SPECIES_H
#define INCLUDE_EFD_CONV_STONE_SPECIES_H
// efd_stone_species.h
// Conversions efd <-> stone_species

#include <stdio.h>

#include "efd/efd.h"
#include "world/species.h"

void* efd__stone_species(efd_node *n) {
  stone_species *result;
  size_t i;
  efd_node *val, *field, *subfield;

  SSTR(s_source, "source", 6);
  SSTR(s_composition, "composition", 11);
  SSTR(s_material, "material", 8);
  SSTR(s_appearance, "appearance", 10);

  SSTR(s_mineral_filter_args, "mineral_filter_args", 19);
  SSTR(s_trace_composition, "trace_composition", 17);
  SSTR(s_constituents, "constituents", 12);
  SSTR(s_traces, "traces", 6);

  efd_assert_type(n, EFD_NT_CONTAINER);

  val = efd_fresh_value(n);

  result = (stone_species*) malloc(sizeof(stone_species));
  result->id = SP_INVALID; // unregistered

  // Source:
  result->source = (geologic_source) efd_as_i(efd_lookup(val, s_source));

  // Material:
  field = efd_lookup(val, s_material);
  set_material(
    &(result->material),
    (material*) efd_as_o_fmt(field, s_material)
  );

  // Appearance:
  field = efd_lookup(val, s_appearance);
  set_mineral_filter_args(
    &(result->appearance),
    (mineral_filter_args*) efd_as_o_fmt(field, s_mineral_filter_args)
  );

  // Composition values:
  field = efd_lookup(val, s_composition);

  result->composition = (mineral_composition) efd_as_i(
    efd_lookup(field, s_composition)
  );

  result->trace_composition = (mineral_trace_composition) efd_as_i(
    efd_lookup(field, s_trace_composition)
  );

  subfield = efd_lookup(field, s_constituents);
  if (efd_normal_child_count(subfield) != MN_MAX_PRIMARY_CONSTITUENTS) {
    efd_report_error(
      s_sprintf(
        "ERROR: wrong number of 'constituents' (expected %zu, was %zu)",
        MN_MAX_PRIMARY_CONSTITUENTS,
        efd_normal_child_count(subfield)
      ),
      subfield
    );
    exit(EXIT_FAILURE);
  }
  for (i = 0; i < MN_MAX_PRIMARY_CONSTITUENTS; ++i) {
    result->constituents[i] = (species) efd_as_i(efd_nth(subfield, i));
  }

  subfield = efd_lookup(field, s_traces);
  if (efd_normal_child_count(subfield) != MN_MAX_TRACE_CONSTITUENTS) {
    efd_report_error(
      s_sprintf(
        "ERROR: wrong number of 'traces' (expected %zu, was %zu)",
        MN_MAX_TRACE_CONSTITUENTS,
        efd_normal_child_count(subfield)
      ),
      subfield
    );
    exit(EXIT_FAILURE);
  }
  for (i = 0; i < MN_MAX_TRACE_CONSTITUENTS; ++i) {
    result->traces[i] = (species) efd_as_i(efd_nth(subfield, i));
  }

  return (void*) result;
}

efd_node *stone_species__efd(void *v_sp) {
  stone_species *sp = (stone_species*) v_sp;
  size_t i;
  efd_node *result, *child, *grandchild;

  SSTR(s_source, "source", 6);
  SSTR(s_composition, "composition", 11);
  SSTR(s_material, "material", 8);
  SSTR(s_appearance, "appearance", 10);

  SSTR(s_mineral_filter_args, "mineral_filter_args", 19);
  SSTR(s_trace_composition, "trace_composition", 17);
  SSTR(s_constituents, "constituents", 12);
  SSTR(s_traces, "traces", 6);
  
  result = create_efd_node(EFD_NT_CONTAINER, EFD_ANON_NAME);

  // Source:
  child = construct_efd_int_node(s_source, (efd_int_t) sp->source);
  efd_add_child(result, child);

  // Material:
  child = construct_efd_obj_node(
    s_material,
    s_material,
    (void*) &(sp->material)
  );
  efd_add_child(result, child);

  // Appearance:
  child = construct_efd_obj_node(
    s_appearance,
    s_mineral_filter_args,
    (void*) &(sp->appearance)
  );
  efd_add_child(result, child);

  // Composition:
  child = create_efd_node(EFD_NT_CONTAINER, s_composition);

  grandchild = construct_efd_int_node(
    s_composition,
    (efd_int_t) sp->composition
  );
  efd_add_child(child, grandchild);

  grandchild = construct_efd_int_node(
    s_trace_composition,
    (efd_int_t) sp->trace_composition
  );
  efd_add_child(child, grandchild);

  grandchild = create_efd_node(EFD_NT_CONTAINER, s_constituents);
  for (i = 0; i < MN_MAX_PRIMARY_CONSTITUENTS; ++i) {
    efd_add_child(
      grandchild,
      construct_efd_int_node(EFD_ANON_NAME, (efd_int_t) sp->constituents[i])
    );
  }
  efd_add_child(child, grandchild);

  grandchild = create_efd_node(EFD_NT_CONTAINER, s_traces);
  for (i = 0; i < MN_MAX_TRACE_CONSTITUENTS; ++i) {
    efd_add_child(
      grandchild,
      construct_efd_int_node(EFD_ANON_NAME, (efd_int_t) sp->traces[i])
    );
  }
  efd_add_child(child, grandchild);

  efd_add_child(result, child);

  return result;
}

// TODO: Move these elsewhere!
stone_species *copy_stone_species(stone_species const * const src) {
  stone_species *result = (stone_species*) malloc(sizeof(stone_species));
  size_t i;

  result->id = src->id;
  result->source = src->source;
  set_material(&(result->material), &(src->material));
  set_mineral_filter_args(&(result->appearance), &(src->appearance));
  result->composition = src->composition;
  result->trace_composition = src->trace_composition;

  for (i = 0; i < MN_MAX_PRIMARY_CONSTITUENTS; ++i) {
    result->constituents[i] = src->constituents[i];
  }

  for (i = 0; i < MN_MAX_TRACE_CONSTITUENTS; ++i) {
    result->traces[i] = src->traces[i];
  }

  return result;
}

void* copy_v_stone_species(void *v_sp) {
  return (void*) copy_stone_species((stone_species*) v_sp);
}

void cleanup_stone_species(stone_species *sp) {
  free(sp);
}

void cleanup_v_stone_species(void *v_sp) {
  cleanup_stone_species((stone_species*) v_sp);
}

#endif // INCLUDE_EFD_CONV_STONE_SPECIES_H
#endif // EFD_REGISTRATION
