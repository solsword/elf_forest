#if defined(EFD_REGISTER_DECLARATIONS)
// no declarations
#elif defined(EFD_REGISTER_FORMATS)
{
  .key = "stratum",
  .unpacker = efd__stratum,
  .packer = stratum__efd,
  .copier = copy_v_stratum,
  .destructor = cleanup_v_stratum
},
#else
#ifndef INCLUDE_EFD_CONV_STRATUM_H
#define INCLUDE_EFD_CONV_STRATUM_H
// efd_stratum.h
// Conversions efd <-> stratum

#include <stdio.h>

#include "efd/efd.h"
#include "world/species.h"

void* efd__stratum(efd_node *n) {
  stratum *result;
  size_t i, childcount;
  efd_node *val, *field, *subfield;

  SSTR(s_seed, "seed", 4);
  SSTR(s_source, "source", 6);
  SSTR(s_base_species, "base_species", 12);

  SSTR(s_cx, "cx", 2);
  SSTR(s_cy, "cy", 2);
  SSTR(s_size, "size", 4);
  SSTR(s_thickness, "thickness", 9);
  SSTR(s_profile, "profile", 7);

  SSTR(s_dependent_parameters, "dependent_parameters", 20);

  SSTR(s_persistence, "persistence", 11);
  SSTR(s_scale_bias, "scale_bias", 10);

  SSTR(s_radial_frequency, "radial_frequency", 16);
  SSTR(s_radial_variance, "radial_variance", 15);

  SSTR(s_gross_distortion, "gross_distortion", 16);
  SSTR(s_fine_distortion, "fine_distortion", 15);

  SSTR(s_large_var, "large_var", 9);
  SSTR(s_med_var, "med_var", 7);
  SSTR(s_small_var, "small_var", 9);
  SSTR(s_tiny_var, "tiny_var", 8);

  SSTR(s_detail_var, "detail_var", 10);
  SSTR(s_ridges, "ridges", 6);

  SSTR(s_smoothing, "smoothing", 9);

  SSTR(s_vein_species, "vein_species", 12);
  SSTR(s_vein_scales, "vein_scales", 11);
  SSTR(s_vein_strengths, "vein_strengths", 14);

  SSTR(s_inclusion_species, "inclusion_species", 17);
  SSTR(s_inclusion_frequencies, "inclusion_frequencies", 21);

  efd_assert_type(n, EFD_NT_CONTAINER);

  val = efd_get_value(n);

  result = (stratum*) malloc(sizeof(stratum));

  // Seed:
  result->seed = efd_as_i(efd_lookup_expected(val, s_seed));

  // Source:
  result->source = (geologic_source) efd_as_i(
    efd_lookup_expected(val, s_source)
  );

  // Base species:
  result->base_species = (species) efd_as_i(
    efd_lookup_expected(val, s_base_species)
  );

  // Shape parameters:
  result->cx = efd_as_n(efd_lookup_expected(val, s_cx));
  result->cy = efd_as_n(efd_lookup_expected(val, s_cy));
  result->size = efd_as_n(efd_lookup_expected(val, s_size));
  result->thickness = efd_as_n(efd_lookup_expected(val, s_thickness));
  result->profile = (map_function) efd_as_i(
    efd_lookup_expected(val, s_profile)
  );

  // Dependent parameters:
  field = efd_lookup_expected(val, s_dependent_parameters);

  result->persistence = efd_as_n(efd_lookup_expected(field, s_persistence));
  result->scale_bias = efd_as_n(efd_lookup_expected(field, s_scale_bias));

  result->radial_frequency = efd_as_n(
    efd_lookup_expected(field, s_radial_frequency)
  );
  result->radial_variance = efd_as_n(
    efd_lookup_expected(field, s_radial_variance)
  );
  result->gross_distortion = efd_as_n(
    efd_lookup_expected(field, s_gross_distortion)
  );
  result->fine_distortion = efd_as_n(
    efd_lookup_expected(field, s_fine_distortion)
  );
  result->large_var = efd_as_n(efd_lookup_expected(field, s_large_var));
  result->med_var = efd_as_n(efd_lookup_expected(field, s_med_var));
  result->small_var = efd_as_n(efd_lookup_expected(field, s_small_var));
  result->tiny_var = efd_as_n(efd_lookup_expected(field, s_tiny_var));
  result->detail_var = efd_as_n(efd_lookup_expected(field, s_detail_var));
  result->ridges = efd_as_n(efd_lookup_expected(field, s_ridges));
  result->smoothing = efd_as_n(efd_lookup_expected(field, s_smoothing));

  subfield = efd_lookup_expected(field, s_vein_species);
  childcount = efd_normal_child_count(subfield);
  if (childcount > WM_N_VEIN_TYPES) {
    efd_report_error(
      s_sprintf(
        "ERROR: wrong number of vein species "
        "(expected <= %zu, found %zu)",
        WM_N_VEIN_TYPES,
        childcount
      ),
      subfield
    );
    exit(EXIT_FAILURE);
  }
  for (i = 0; i < childcount; ++i) {
    result->vein_species[i] = (species) efd_as_i(efd_nth(subfield, i));
  }
  for (; i < WM_N_VEIN_TYPES; ++i) {
    result->vein_species[i] = SP_INVALID;
  }

  subfield = efd_lookup_expected(field, s_vein_scales);
  childcount = efd_normal_child_count(subfield);
  if (childcount > WM_N_VEIN_TYPES) {
    efd_report_error(
      s_sprintf(
        "ERROR: wrong number of vein scales (expected <= %zu, found %zu)",
        WM_N_VEIN_TYPES,
        childcount
      ),
      subfield
    );
    exit(EXIT_FAILURE);
  }
  for (i = 0; i < childcount; ++i) {
    result->vein_scales[i] = efd_as_n(efd_nth(subfield, i));
  }
  for (; i < WM_N_VEIN_TYPES; ++i) {
    result->vein_scales[i] = 0;
  }

  subfield = efd_lookup_expected(field, s_vein_strengths);
  childcount = efd_normal_child_count(subfield);
  if (childcount > WM_N_VEIN_TYPES) {
    efd_report_error(
      s_sprintf(
        "ERROR: wrong number of vein strengths (expected <= %zu, found %zu)",
        WM_N_VEIN_TYPES,
        childcount
      ),
      subfield
    );
    exit(EXIT_FAILURE);
  }
  for (i = 0; i < childcount; ++i) {
    result->vein_strengths[i] = efd_as_n(efd_nth(subfield, i));
  }
  for (; i < WM_N_VEIN_TYPES; ++i) {
    result->vein_strengths[i] = 0;
  }

  subfield = efd_lookup_expected(field, s_inclusion_species);
  childcount = efd_normal_child_count(subfield);
  if (childcount > WM_N_INCLUSION_TYPES) {
    efd_report_error(
      s_sprintf(
        "ERROR: wrong number of inclusion species "
        "(expected <= %zu, found %zu)",
        WM_N_INCLUSION_TYPES,
        childcount
      ),
      subfield
    );
    exit(EXIT_FAILURE);
  }
  for (i = 0; i < childcount; ++i) {
    result->inclusion_species[i] = (species) efd_as_i(efd_nth(subfield, i));
  }
  for (; i < WM_N_INCLUSION_TYPES; ++i) {
    result->inclusion_species[i] = SP_INVALID;
  }

  subfield = efd_lookup_expected(field, s_inclusion_frequencies);
  childcount = efd_normal_child_count(subfield);
  if (childcount > WM_N_INCLUSION_TYPES) {
    efd_report_error(
      s_sprintf(
        "ERROR: wrong number of inclusion frequencies "
        "(expected <= %zu, found %zu)",
        WM_N_INCLUSION_TYPES,
        childcount
      ),
      subfield
    );
    exit(EXIT_FAILURE);
  }
  for (i = 0; i < childcount; ++i) {
    result->inclusion_frequencies[i] = efd_as_n(efd_nth(subfield, i));
  }
  for (; i < WM_N_INCLUSION_TYPES; ++i) {
    result->inclusion_frequencies[i] = 0;
  }

  return (void*) result;
}

efd_node *stratum__efd(void *v_stratum) {
  stratum *src = (stratum*) v_stratum;
  size_t i;
  efd_node *result, *child, *grandchild;

  SSTR(s_seed, "seed", 4);
  SSTR(s_source, "source", 6);
  SSTR(s_base_species, "base_species", 12);

  SSTR(s_cx, "cx", 2);
  SSTR(s_cy, "cy", 2);
  SSTR(s_size, "size", 4);
  SSTR(s_thickness, "thickness", 9);
  SSTR(s_profile, "profile", 7);

  SSTR(s_dependent_parameters, "dependent_parameters", 20);

  SSTR(s_persistence, "persistence", 11);
  SSTR(s_scale_bias, "scale_bias", 10);

  SSTR(s_radial_frequency, "radial_frequency", 16);
  SSTR(s_radial_variance, "radial_variance", 15);

  SSTR(s_gross_distortion, "gross_distortion", 16);
  SSTR(s_fine_distortion, "fine_distortion", 15);

  SSTR(s_large_var, "large_var", 9);
  SSTR(s_med_var, "med_var", 7);
  SSTR(s_small_var, "small_var", 9);
  SSTR(s_tiny_var, "tiny_var", 8);

  SSTR(s_detail_var, "detail_var", 10);
  SSTR(s_ridges, "ridges", 6);

  SSTR(s_smoothing, "smoothing", 9);

  SSTR(s_vein_species, "vein_species", 12);
  SSTR(s_vein_scales, "vein_scales", 11);
  SSTR(s_vein_strengths, "vein_strengths", 14);

  SSTR(s_inclusion_species, "inclusion_species", 17);
  SSTR(s_inclusion_frequencies, "inclusion_frequencies", 21);
  
  result = create_efd_node(EFD_NT_CONTAINER, EFD_ANON_NAME, NULL);

  // Seed:
  efd_add_child(
    result,
    construct_efd_int_node(s_seed, NULL, (efd_int_t) src->seed)
  );

  // Source:
  efd_add_child(
    result,
    construct_efd_int_node(s_source, NULL, (efd_int_t) src->source)
  );

  // Base species:
  efd_add_child(
    result,
    construct_efd_int_node(s_base_species, NULL, (efd_int_t) src->base_species)
  );

  // Shape parameters:
  efd_add_child(
    result,
    construct_efd_num_node(s_cx, NULL, (efd_num_t) src->cx)
  );
  efd_add_child(
    result,
    construct_efd_num_node(s_cy, NULL, (efd_num_t) src->cy)
  );
  efd_add_child(
    result,
    construct_efd_num_node(s_size, NULL, (efd_num_t) src->size)
  );
  efd_add_child(
    result,
    construct_efd_num_node(s_thickness, NULL, (efd_num_t) src->thickness)
  );
  efd_add_child(
    result,
    construct_efd_int_node(s_profile, NULL, (efd_int_t) src->profile)
  );

  // Derived parameters:
  child = create_efd_node(EFD_NT_CONTAINER, s_dependent_parameters, NULL);
  efd_add_child(result, child);

  efd_add_child(
    child,
    construct_efd_num_node(s_persistence, NULL, (efd_num_t) src->persistence)
  );
  efd_add_child(
    child,
    construct_efd_num_node(s_scale_bias, NULL, (efd_num_t) src->scale_bias)
  );

  efd_add_child(
    child,
    construct_efd_num_node(
      s_radial_frequency,
      NULL,
      (efd_num_t) src->radial_frequency
    )
  );
  efd_add_child(
    child,
    construct_efd_num_node(
      s_radial_variance,
      NULL,
      (efd_num_t) src->radial_variance
    )
  );

  efd_add_child(
    child,
    construct_efd_num_node(
      s_gross_distortion,
      NULL,
      (efd_num_t) src->gross_distortion
    )
  );
  efd_add_child(
    child,
    construct_efd_num_node(
      s_fine_distortion,
      NULL,
      (efd_num_t) src->fine_distortion
    )
  );

  efd_add_child(
    child,
    construct_efd_num_node(s_large_var, NULL, (efd_num_t) src->large_var)
  );
  efd_add_child(
    child,
    construct_efd_num_node(s_med_var, NULL, (efd_num_t) src->med_var)
  );
  efd_add_child(
    child,
    construct_efd_num_node(s_small_var, NULL, (efd_num_t) src->small_var)
  );
  efd_add_child(
    child,
    construct_efd_num_node(s_tiny_var, NULL, (efd_num_t) src->tiny_var)
  );
  efd_add_child(
    child,
    construct_efd_num_node(s_detail_var, NULL, (efd_num_t) src->detail_var)
  );
  efd_add_child(
    child,
    construct_efd_num_node(s_ridges, NULL, (efd_num_t) src->ridges)
  );

  efd_add_child(
    child,
    construct_efd_num_node(s_smoothing, NULL, (efd_num_t) src->smoothing)
  );

  grandchild = create_efd_node(EFD_NT_CONTAINER, s_vein_species, NULL);
  efd_add_child(child, grandchild);

  for (i = 0; i < WM_N_VEIN_TYPES; ++i) {
    if (src->vein_species[i] == SP_INVALID) {
      break;
    }
    efd_add_child(
      grandchild,
      construct_efd_int_node(
        EFD_ANON_NAME,
        NULL,
        (efd_int_t) src->vein_species[i]
      )
    );
  }

  grandchild = create_efd_node(EFD_NT_CONTAINER, s_vein_scales, NULL);
  efd_add_child(child, grandchild);

  for (i = 0; i < WM_N_VEIN_TYPES; ++i) {
    if (src->vein_species[i] == SP_INVALID) {
      break;
    }
    efd_add_child(
      grandchild,
      construct_efd_num_node(
        EFD_ANON_NAME,
        NULL,
        (efd_num_t) src->vein_scales[i]
      )
    );
  }

  grandchild = create_efd_node(EFD_NT_CONTAINER, s_vein_strengths, NULL);
  efd_add_child(child, grandchild);

  for (i = 0; i < WM_N_VEIN_TYPES; ++i) {
    if (src->vein_species[i] == SP_INVALID) {
      break;
    }
    efd_add_child(
      grandchild,
      construct_efd_num_node(
        EFD_ANON_NAME,
        NULL,
        (efd_num_t) src->vein_strengths[i]
      )
    );
  }

  grandchild = create_efd_node(EFD_NT_CONTAINER, s_inclusion_species, NULL);
  efd_add_child(child, grandchild);

  for (i = 0; i < WM_N_INCLUSION_TYPES; ++i) {
    if (src->inclusion_species[i] == SP_INVALID) {
      break;
    }
    efd_add_child(
      grandchild,
      construct_efd_int_node(
        EFD_ANON_NAME,
        NULL,
        (efd_int_t) src->inclusion_species[i]
      )
    );
  }

  grandchild = create_efd_node(EFD_NT_CONTAINER, s_inclusion_frequencies, NULL);
  efd_add_child(child, grandchild);

  for (i = 0; i < WM_N_INCLUSION_TYPES; ++i) {
    if (src->inclusion_species[i] == SP_INVALID) {
      break;
    }
    efd_add_child(
      grandchild,
      construct_efd_num_node(
        EFD_ANON_NAME,
        NULL,
        (efd_num_t) src->inclusion_frequencies[i]
      )
    );
  }

  return result;
}

// TODO: Move these elsewhere!
stratum *copy_stratum(stratum const * const src) {
  stratum *result = (stratum*) malloc(sizeof(stratum));
  size_t i;

  result->seed = src->seed;
  result->source = src->source;
  result->cx = src->cx;
  result->cy = src->cy;
  result->size = src->size;
  result->thickness = src->thickness;
  result->profile = src->profile;
  result->persistence = src->persistence;
  result->scale_bias = src->scale_bias;
  result->radial_frequency = src->radial_frequency;
  result->radial_variance = src->radial_variance;
  result->gross_distortion = src->gross_distortion;
  result->fine_distortion = src->fine_distortion;
  result->large_var = src->large_var;
  result->med_var = src->med_var;
  result->small_var = src->small_var;
  result->tiny_var = src->tiny_var;
  result->detail_var = src->detail_var;
  result->ridges = src->ridges;
  result->smoothing = src->smoothing;

  for (i = 0; i < WM_N_VEIN_TYPES; ++i) {
    result->vein_scales[i] = src->vein_scales[i];
    result->vein_strengths[i] = src->vein_strengths[i];
    result->vein_species[i] = src->vein_species[i];
  }

  for (i = 0; i < WM_N_INCLUSION_TYPES; ++i) {
    result->inclusion_frequencies[i] = src->inclusion_frequencies[i];
    result->inclusion_species[i] = src->inclusion_species[i];
  }

  result->base_species = src->base_species;

  return result;
}

void* copy_v_stratum(void *v_st) {
  return (void*) copy_stratum((stratum*) v_st);
}

void cleanup_stratum(stratum *st) {
  free(st);
}

void cleanup_v_stratum(void *v_st) {
  cleanup_stratum((stratum*) v_st);
}

#endif // INCLUDE_EFD_CONV_STRATUM_H
#endif // EFD_REGISTRATION
