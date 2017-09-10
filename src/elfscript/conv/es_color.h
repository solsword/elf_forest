#if defined(ELFSCRIPT_REGISTER_DECLARATIONS)
// no declarations
#elif defined(ELFSCRIPT_REGISTER_FORMATS)
{
  .key = "color",
  .unpacker = elfscript__color,
  .packer = color__elfscript,
  .copier = copy_v_color,
  .destructor = cleanup_v_color
},
#else
#ifndef INCLUDE_ELFSCRIPT_CONV_COLOR_H
#define INCLUDE_ELFSCRIPT_CONV_COLOR_H
// elfscript_color.h
// Conversions elfscript <-> precise_color

#include <stdio.h>

#include "elfscript/elfscript.h"
#include "tex/color.h"

void* elfscript__color(es_scope *sc) {
  es_var *alpha;
  precise_color *result;
  pixel px = PX_BLACK;

  SSTR(s_fmt, "fmt", 3);
  SSTR(s_R, "R", 1);
  SSTR(s_G, "G", 1);
  SSTR(s_B, "B", 1);
  SSTR(s_A, "A", 1);
  SSTR(s_X, "X", 1);
  SSTR(s_Y, "Y", 1);
  SSTR(s_Z, "Z", 1);
  SSTR(s_L, "L", 1);
  SSTR(s_a, "a", 1);
  SSTR(s_b, "b", 1);
  SSTR(s_c, "c", 1);
  SSTR(s_h, "h", 1);

  result = (precise_color*) malloc(sizeof(precise_color));

  result->format = es_as_i(es_read_var(sc, s_fmt));

  // Lookup components according to the specified format:
  switch (result->format) {
    default:
    case CFMT_INVALID:
      es_report_error(s_("ERROR: 'color' node has invalid 'fmt' argument:"));
      exit(EXIT_FAILURE);

    case CFMT_RGB:
      px_set_red(&px, (channel) es_as_i(es_read_var(sc, s_R)));
      px_set_green(&px, (channel) es_as_i(es_read_var(sc, s_G)));
      px_set_blue(&px, (channel) es_as_i(es_read_var(sc, s_B)));
      result->format = CFMT_XYZ;
      rgb__xyz(px, result);
      break;

    case CFMT_XYZ:
      result->x = es_as_n(es_read_var(sc, s_X));
      result->y = es_as_n(es_read_var(sc, s_Y));
      result->z = es_as_n(es_read_var(sc, s_Z));
      break;

    case CFMT_LCH:
      result->x = es_as_n(es_read_var(sc, s_L));
      result->y = es_as_n(es_read_var(sc, s_c));
      result->z = es_as_n(es_read_var(sc, s_h));
      break;

    case CFMT_LAB:
      result->x = es_as_n(es_read_var(sc, s_L));
      result->y = es_as_n(es_read_var(sc, s_a));
      result->z = es_as_n(es_read_var(sc, s_b));
      break;
  }

  // Lookup alpha if it's present or assume the default of 1.0:
  alpha = es_read_var(sc, s_A);
  if (alpha != NULL) {
    result->alpha = es_as_n(alpha);
  } else {
    result->alpha = 1.0;
  }

  // Normalize the result to XYZ:
  convert_to_xyz(result);

  return (void*) result;
}

es_scope * color__elfscript(void *v_color) {
  precise_color *color = (precise_color*) v_color;
  elfscript_scope *result;

  SSTR(s_fmt, "fmt", 3);
  SSTR(s_X, "X", 1);
  SSTR(s_Y, "Y", 1);
  SSTR(s_Z, "Z", 1);
  SSTR(s_A, "A", 1);

  result = create_es_scope();

  convert_to_xyz(color);

  es_write_var(result, s_fmt, create_es_int_var(color->format));

  es_write_var(result, s_X, create_es_num_var(color->x));
  es_write_var(result, s_Y, create_es_num_var(color->y));
  es_write_var(result, s_Z, create_es_num_var(color->z));
  es_write_var(result, s_A, create_es_num_var(color->alpha));

  return result;
}

// TODO: Put these elsewhere?
void * copy_v_color(void *v_color) {
  precise_color *color = (precise_color*) v_color;
  precise_color *result = (precise_color*) malloc(sizeof(precise_color));
  result->format = color->format;
  result->x = color->x;
  result->y = color->y;
  result->z = color->z;
  result->alpha = color->alpha;
  return result;
}

void cleanup_v_color(void *v_color) {
  free(v_color);
}

#endif // INCLUDE_ELFSCRIPT_CONV_COLOR_H
#endif // ELFSCRIPT_REGISTRATION
