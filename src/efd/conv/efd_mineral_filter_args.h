#if defined(EFD_REGISTER_DECLARATIONS)
// no declarations
#elif defined(EFD_REGISTER_FORMATS)
{
  .key = "mineral_filter_args",
  .unpacker = efd__mineral_filter_args,
  .packer = mineral_filter_args__efd,
  .copier = copy_v_mineral_filter_args,
  .destructor = cleanup_v_mineral_filter_args
},
#else
#ifndef INCLUDE_EFD_CONV_MINERAL_FILTER_ARGS_H
#define INCLUDE_EFD_CONV_MINERAL_FILTER_ARGS_H
// efd_mineral_filter_args.h
// Conversions efd <-> mineral_filter_args

#include <stdio.h>

#include "efd/efd.h"
#include "txgen/txg_minerals.h"

void* efd__mineral_filter_args(efd_node *n) {
  mineral_filter_args *result;
  efd_node *val;
  precise_color color;

  SSTR(s_seed, "seed", 4);
  SSTR(s_scale, "scale", 5);
  SSTR(s_gritty, "gritty", 6);
  SSTR(s_contoured, "contoured", 9);
  SSTR(s_porous, "porous", 6);
  SSTR(s_bumpy, "bumpy", 5);
  SSTR(s_layered, "layered", 7);
  SSTR(s_layerscale, "layerscale", 10);
  SSTR(s_layerwaves, "layerwaves", 10);
  SSTR(s_wavescale, "wavescale", 9);
  SSTR(s_inclusions, "inclusions", 10);
  SSTR(s_dscale, "dscale", 6);
  SSTR(s_distortion, "distortion", 10);
  SSTR(s_squash, "squash", 6);
  SSTR(s_color, "color", 5);
  SSTR(s_base_color, "base_color", 10);
  SSTR(s_alt_color, "alt_color", 9);
  SSTR(s_sat_noise, "sat_noise", 9);
  SSTR(s_desaturate, "desaturate", 10);
  SSTR(s_brightness, "brightness", 10);

  efd_assert_type(n, EFD_NT_CONTAINER);

  if (efd_normal_child_count(n) != 1) {
    efd_report_error(
      s_("ERROR: 'mineral_filter_args' proto must have exactly 1 child."),
      n
    );
    exit(EXIT_FAILURE);
  }

  val = efd_concrete(efd_fresh_value(efd_nth(n, 0)));

  efd_assert_type(val, EFD_NT_CONTAINER);

  result = create_mineral_filter_args();

  result->seed = efd_as_i(efd_lookup_expected(val, s_seed));
  result->scale = efd_as_n(efd_lookup_expected(val, s_scale));
  result->gritty = efd_as_n(efd_lookup_expected(val, s_gritty));
  result->contoured = efd_as_n(efd_lookup_expected(val, s_contoured));
  result->porous = efd_as_n(efd_lookup_expected(val, s_porous));
  result->bumpy = efd_as_n(efd_lookup_expected(val, s_bumpy));
  result->layered = efd_as_n(efd_lookup_expected(val, s_layered));
  result->layerscale = efd_as_n(efd_lookup_expected(val, s_layerscale));
  result->layerwaves = efd_as_n(efd_lookup_expected(val, s_layerwaves));
  result->wavescale = efd_as_n(efd_lookup_expected(val, s_wavescale));
  result->inclusions = efd_as_n(efd_lookup_expected(val, s_inclusions));
  result->dscale = efd_as_n(efd_lookup_expected(val, s_dscale));
  result->distortion = efd_as_n(efd_lookup_expected(val, s_distortion));
  result->squash = efd_as_n(efd_lookup_expected(val, s_squash));
  // TODO: treat these as colors and convert to a pixel value.
  color = efd_as_o_fmt(efd_lookup_expected(val, s_base_color), s_color);
  result->base_color = xyz__rgb(color);
  color = efd_as_o_fmt(efd_lookup_expected(val, s_alt_color), s_color);
  result->alt_color = xyz__rgb(color);
  result->sat_noise = efd_as_n(efd_lookup_expected(val, s_sat_noise));
  result->desaturate = efd_as_n(efd_lookup_expected(val, s_desaturate));
  result->brightness = efd_as_n(efd_lookup_expected(val, s_brightness));

  return (void*) result;
}

efd_node *mineral_filter_args__efd(void *v_args) {
  mineral_filter_args *args = (mineral_filter_args*) v_args;
  efd_node *result, *child;

  SSTR(s_seed, "seed", 4);
  SSTR(s_scale, "scale", 5);
  SSTR(s_gritty, "gritty", 6);
  SSTR(s_contoured, "contoured", 9);
  SSTR(s_porous, "porous", 6);
  SSTR(s_bumpy, "bumpy", 5);
  SSTR(s_layered, "layered", 7);
  SSTR(s_layerscale, "layerscale", 10);
  SSTR(s_layerwaves, "layerwaves", 10);
  SSTR(s_wavescale, "wavescale", 9);
  SSTR(s_inclusions, "inclusions", 10);
  SSTR(s_dscale, "dscale", 6);
  SSTR(s_distortion, "distortion", 10);
  SSTR(s_squash, "squash", 6);
  SSTR(s_base_color, "base_color", 10);
  SSTR(s_alt_color, "alt_color", 9);
  SSTR(s_sat_noise, "sat_noise", 9);
  SSTR(s_desaturate, "desaturate", 10);
  SSTR(s_brightness, "brightness", 10);

  result = create_efd_node(EFD_NT_CONTAINER, EFD_ANON_NAME, NULL);

  child = create_efd_node(EFD_NT_CONTAINER, EFD_ANON_NAME, NULL);

  efd_add_child(result, child);

  efd_add_child(
    child,
    construct_efd_int_node(s_seed, NULL, (efd_int_t) args->seed)
  );

  efd_add_child(
    child,
    construct_efd_num_node(s_scale, NULL, (efd_num_t) args->scale)
  );
  efd_add_child(
    child,
    construct_efd_num_node(s_gritty, NULL, (efd_num_t) args->gritty)
  );
  efd_add_child(
    child,
    construct_efd_num_node(s_contoured, NULL, (efd_num_t) args->contoured)
  );
  efd_add_child(
    child,
    construct_efd_num_node(s_porous, NULL, (efd_num_t) args->porous)
  );
  efd_add_child(
    child,
    construct_efd_num_node(s_bumpy, NULL, (efd_num_t) args->bumpy)
  );
  efd_add_child(
    child,
    construct_efd_num_node(s_layered, NULL, (efd_num_t) args->layered)
  );
  efd_add_child(
    child,
    construct_efd_num_node(s_layerscale, NULL, (efd_num_t) args->layerscale)
  );
  efd_add_child(
    child,
    construct_efd_num_node(s_layerwaves, NULL, (efd_num_t) args->layerwaves)
  );
  efd_add_child(
    child,
    construct_efd_num_node(s_wavescale, NULL, (efd_num_t) args->wavescale)
  );
  efd_add_child(
    child,
    construct_efd_num_node(s_inclusions, NULL, (efd_num_t) args->inclusions)
  );
  efd_add_child(
    child,
    construct_efd_num_node(s_dscale, NULL, (efd_num_t) args->dscale)
  );
  efd_add_child(
    child,
    construct_efd_num_node(s_distortion, NULL, (efd_num_t) args->distortion)
  );
  efd_add_child(
    child,
    construct_efd_num_node(s_squash, NULL, (efd_num_t) args->squash)
  );
  // TODO: HERE
  efd_add_child(
    child,
    construct_efd_int_node(s_base_color, NULL, (efd_int_t) args->base_color)
  );
  efd_add_child(
    child,
    construct_efd_int_node(s_alt_color, NULL, (efd_int_t) args->alt_color)
  );
  efd_add_child(
    child,
    construct_efd_num_node(s_sat_noise, NULL, (efd_num_t) args->sat_noise)
  );
  efd_add_child(
    child,
    construct_efd_num_node(s_desaturate, NULL, (efd_num_t) args->desaturate)
  );
  efd_add_child(
    child,
    construct_efd_num_node(s_brightness, NULL, (efd_num_t) args->brightness)
  );

  return result;
}

#endif // INCLUDE_EFD_CONV_MINERAL_FILTER_ARGS_H
#endif // EFD_REGISTRATION
