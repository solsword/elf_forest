#if defined(ELFSCRIPT_REGISTER_DECLARATIONS)
// no declarations
#elif defined(ELFSCRIPT_REGISTER_FORMATS)
{
  .key = "mineral_filter_args",
  .unpacker = elfscript__mineral_filter_args,
  .packer = mineral_filter_args__elfscript,
  .copier = copy_v_mineral_filter_args,
  .destructor = cleanup_v_mineral_filter_args
},
#else
#ifndef INCLUDE_ELFSCRIPT_CONV_MINERAL_FILTER_ARGS_H
#define INCLUDE_ELFSCRIPT_CONV_MINERAL_FILTER_ARGS_H
// elfscript_mineral_filter_args.h
// Conversions elfscript <-> mineral_filter_args

#include <stdio.h>

#include "elfscript/elfscript.h"
#include "txgen/txg_minerals.h"

void* elfscript__mineral_filter_args(es_scope *sc) {
  mineral_filter_args *result;

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

  result = create_mineral_filter_args();

  result->seed = es_as_i(es_read_var(sc, s_seed));
  result->scale = es_as_n(es_read_var(sc, s_scale));
  result->gritty = es_as_n(es_read_var(sc, s_gritty));
  result->contoured = es_as_n(es_read_var(sc, s_contoured));
  result->porous = es_as_n(es_read_var(sc, s_porous));
  result->bumpy = es_as_n(es_read_var(sc, s_bumpy));
  result->layered = es_as_n(es_read_var(sc, s_layered));
  result->layerscale = es_as_n(es_read_var(sc, s_layerscale));
  result->layerwaves = es_as_n(es_read_var(sc, s_layerwaves));
  result->wavescale = es_as_n(es_read_var(sc, s_wavescale));
  result->inclusions = es_as_n(es_read_var(sc, s_inclusions));
  result->dscale = es_as_n(es_read_var(sc, s_dscale));
  result->distortion = es_as_n(es_read_var(sc, s_distortion));
  result->squash = es_as_n(es_read_var(sc, s_squash));
  result->base_color = es_as_i(es_read_var(sc, s_base_color));
  result->alt_color = es_as_i(es_read_var(sc, s_alt_color));
  result->sat_noise = es_as_n(es_read_var(sc, s_sat_noise));
  result->desaturate = es_as_n(es_read_var(sc, s_desaturate));
  result->brightness = es_as_n(es_read_var(sc, s_brightness));

  return (void*) result;
}

elfscript_node *mineral_filter_args__elfscript(void *v_args) {
  mineral_filter_args *args = (mineral_filter_args*) v_args;
  es_scope *result;

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

  result = create_es_scope();

  es_write_var(
    result,
    s_seed,
    create_es_int_var((es_int_t) args->seed)
  );

  es_write_var(
    result,
    s_scale,
    construct_es_num_node((es_num_t) args->scale)
  );
  es_write_var(
    result,
    s_gritty,
    construct_es_num_node((es_num_t) args->gritty)
  );
  es_write_var(
    result,
    s_contoured,
    construct_es_num_node((es_num_t) args->contoured)
  );
  es_write_var(
    result,
    s_porous,
    construct_es_num_node((es_num_t) args->porous)
  );
  es_write_var(
    result,
    s_bumpy,
    construct_es_num_node((es_num_t) args->bumpy)
  );
  es_write_var(
    result,
    s_layered,
    construct_es_num_node((es_num_t) args->layered)
  );
  es_write_var(
    result,
    s_layerscale,
    construct_es_num_node((es_num_t) args->layerscale)
  );
  es_write_var(
    result,
    s_layerwaves,
    construct_es_num_node((es_num_t) args->layerwaves)
  );
  es_write_var(
    result,
    s_wavescale,
    construct_es_num_node((es_num_t) args->wavescale)
  );
  es_write_var(
    result,
    s_inclusions,
    construct_es_num_node((es_num_t) args->inclusions)
  );
  es_write_var(
    result,
    s_dscale,
    construct_es_num_node((es_num_t) args->dscale)
  );
  es_write_var(
    result,
    s_distortion,
    construct_es_num_node((es_num_t) args->distortion)
  );
  es_write_var(
    result,
    s_squash,
    construct_es_num_node((es_num_t) args->squash)
  );
  es_write_var(
    result,
    s_base_color,
    create_es_int_var((es_int_t) args->base_color)
  );
  es_write_var(
    result,
    s_alt_color,
    create_es_int_var((es_int_t) args->alt_color)
  );
  es_write_var(
    result,
    s_sat_noise,
    construct_es_num_node((es_num_t) args->sat_noise)
  );
  es_write_var(
    result,
    s_desaturate,
    construct_es_num_node((es_num_t) args->desaturate)
  );
  es_write_var(
    result,
    s_brightness,
    construct_es_num_node((es_num_t) args->brightness)
  );

  return result;
}

#endif // INCLUDE_ELFSCRIPT_CONV_MINERAL_FILTER_ARGS_H
#endif // ELFSCRIPT_REGISTRATION
