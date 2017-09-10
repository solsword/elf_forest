#if defined(ELFSCRIPT_REGISTER_DECLARATIONS)
// nothing to declare
#elif defined(ELFSCRIPT_REGISTER_FUNCTIONS)
{ .key = "color__pixel",         .function = &elfscript_fn_color__pixel       },
{ .key = "color__pixel",         .function = &elfscript_fn_pixel__color       },
#else
#ifndef INCLUDE_ELFSCRIPT_FUNC_COLOR_H
#define INCLUDE_ELFSCRIPT_FUNC_COLOR_H
// elfscript_color.h
// Functions for color handling and conversion.

#include "elfscript/elfscript.h"
#include "tex/color.h"

// Converts a color into a pixel:
es_var * elfscript_fn_color__pixel(es_var * color_var) {
  SSTR(s_color, "color", 5);

  es_obj *color_obj = es_as_obj(color_var);

  precise_color *color = (precise_color*) es_obj_as_fmt(color_obj, s_color);

  convert_to_xyz(color);

  return create_es_int_var(
    (es_int_t) xyz__rgb(color)
  );
}

es_var * elfscript_fn_pixel__color(es_var * pixel_var) {
  SSTR(s_color, "color", 5);

  es_int_t pixel = es_as_i(pixel_var);

  precise_color *color = create_precise_color();

  rgb__xyz(pixel, color);

  return create_es_obj_var(
    create_es_obj(
      es_lookup_format(s_color),
      color
    )
  );
}

#endif // INCLUDE_ELFSCRIPT_FUNC_COLOR_H
#endif // ELFSCRIPT_REGISTRATION
