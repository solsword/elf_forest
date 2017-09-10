#if defined(ELFSCRIPT_REGISTER_DECLARATIONS)
// no declarations
#elif defined(ELFSCRIPT_REGISTER_FORMATS)
{
  .key = "stone_species",
  .unpacker = elfscript__stone_species,
  .packer = stone_species__elfscript,
  .copier = copy_v_stone_species,
  .destructor = cleanup_v_stone_species
},
#else
#ifndef INCLUDE_ELFSCRIPT_CONV_STONE_SPECIES_H
#define INCLUDE_ELFSCRIPT_CONV_STONE_SPECIES_H
// elfscript_stone_species.h
// Conversions elfscript <-> stone_species

#include <stdio.h>

#include "elfscript/elfscript.h"
#include "world/species.h"

void* elfscript__stone_species(es_scope *sc) {
  stone_species *result;
  size_t i, subchildcount;
  es_scope *field, *subfield;
  material *mat;
  mineral_filter_args *mfargs;

  SSTR(s_source, "source", 6);
  SSTR(s_composition, "composition", 11);
  SSTR(s_material, "material", 8);
  SSTR(s_appearance, "appearance", 10);

  SSTR(s_mineral_filter_args, "mineral_filter_args", 19);
  SSTR(s_trace_composition, "trace_composition", 17);
  SSTR(s_constituents, "constituents", 12);
  SSTR(s_traces, "traces", 6);

  result = (stone_species*) malloc(sizeof(stone_species));
  result->id = SP_INVALID; // unregistered

  // Source:
  result->source = (geologic_source) es_as_i(
    es_read_var(sc, s_source)
  );

  // Material:
  field = es_read_var(sc, s_material);
  mat = (material*) es_pack(field, s_material);
  set_material(&(result->material), mat);
  cleanup_material(mat);

  // Appearance:
  field = es_read_var(sc, s_appearance);
  mfargs = (mineral_filter_args*) es_pack(field, s_mineral_filter_args);
  set_mineral_filter_args(&(result->appearance), mfargs);
  cleanup_mineral_filter_args(mfargs);

  // Composition values:
  field = es_read_var(sc, s_composition);

  result->composition = (mineral_composition) es_as_i(
    es_read_var(field, s_composition)
  );


  result->trace_composition = (mineral_trace_composition) es_as_i(
    es_read_var(field, s_trace_composition)
  );

  subfield = es_read_var(field, s_constituents);
  subchildcount = es_scope_size(subfield);
  if (subchildcount > MN_MAX_PRIMARY_CONSTITUENTS) {
    es_report_error(
      s_sprintf(
        "ERROR: wrong number of 'constituents' (expected <= %zu, found %zu)",
        MN_MAX_PRIMARY_CONSTITUENTS,
        subchildcount
      )
      // TODO: Way to give extra context here?
    );
    exit(EXIT_FAILURE);
  }
  for (i = 0; i < subchildcount; ++i) {
    result->constituents[i] = (species) es_as_i(es_read_nth(subfield, i));
  }
  for (; i < MN_MAX_PRIMARY_CONSTITUENTS; ++i) {
    result->constituents[i] = SP_INVALID;
  }

  subfield = es_read_var(field, s_traces);
  subchildcount = es_scope_size(subfield);
  if (subchildcount > MN_MAX_TRACE_CONSTITUENTS) {
    elfscript_report_error(
      s_sprintf(
        "ERROR: wrong number of 'traces' (expected <= %zu, found %zu)",
        MN_MAX_TRACE_CONSTITUENTS,
        subchildcount
      )
      // TODO: Way to give extra context here?
    );
    exit(EXIT_FAILURE);
  }
  for (i = 0; i < subchildcount; ++i) {
    result->traces[i] = (species) es_as_i(es_read_nth(subfield, i));
  }
  for (; i < MN_MAX_TRACE_CONSTITUENTS; ++i) {
    result->traces[i] = SP_INVALID;
  }

  return (void*) result;
}

es_scope *stone_species__elfscript(void *v_sp) {
  stone_species *sp = (stone_species*) v_sp;
  size_t i;
  es_scope *result, *child, *grandchild;

  SSTR(s_source, "source", 6);
  SSTR(s_composition, "composition", 11);
  SSTR(s_material, "material", 8);
  SSTR(s_appearance, "appearance", 10);

  SSTR(s_mineral_filter_args, "mineral_filter_args", 19);
  SSTR(s_trace_composition, "trace_composition", 17);
  SSTR(s_constituents, "constituents", 12);
  SSTR(s_traces, "traces", 6);
  
  result = create_es_scope();

  // Source:
  es_write_var(result, s_source, create_es_int_var((es_int_t) sp->source));

  // Material:
  es_write_var(
    result,
    s_material,
    create_es_scp_var(es_unpack((void*) &(sp->material), s_material))
  );

  // Appearance:
  es_write_var(
    result,
    s_appearance,
    create_es_scp_var(
      es_unpack((void*) &(sp->appearance), s_mineral_filter_args)
    )
  );

  // Composition:
  child = create_es_scope();

  es_write_var(result, s_composition, create_es_scp_var(child));

  es_write_var(
    child,
    s_composition,
    create_es_int_var((es_int_t) sp->composition)
  );

  es_write_var(
    child,
    s_trace_composition,
    create_es_int_var((es_int_t) sp->trace_composition)
  );

  //   Constituents:
  grandchild = create_es_scope();

  es_write_var(child, s_constituents, create_es_scp_var(grandchild));

  for (i = 0; i < MN_MAX_PRIMARY_CONSTITUENTS; ++i) {
    es_write_last(
      grandchild,
      create_es_int_var((es_int_t) sp->constituents[i])
    );
  }

  //   Trace Constituents:
  grandchild = create_es_scope();

  es_write_var(child, s_traces, create_es_scp_var(grandchild));

  for (i = 0; i < MN_MAX_PRIMARY_CONSTITUENTS; ++i) {
    es_write_last(
      grandchild,
      create_es_int_var((es_int_t) sp->traces[i])
    );
  }

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

#endif // INCLUDE_ELFSCRIPT_CONV_STONE_SPECIES_H
#endif // ELFSCRIPT_REGISTRATION
