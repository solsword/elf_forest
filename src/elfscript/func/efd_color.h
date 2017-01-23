#if defined(EFD_REGISTER_DECLARATIONS)
// nothing to declare
#elif defined(EFD_REGISTER_FUNCTIONS)
{ .key = "color__pixel",         .function = &efd_fn_color__pixel         },
#else
#ifndef INCLUDE_EFD_FUNC_COLOR_H
#define INCLUDE_EFD_FUNC_COLOR_H
// efd_color.h
// Functions for color handling and conversion.

#include "efd/efd.h"
#include "tex/color.h"

// Converts a color into a pixel:
efd_node * efd_fn_color__pixel(efd_node const * const node) {
  SSTR(s_color, "color", 5);

  precise_color *color;

  efd_assert_return_type(node, EFD_NT_INTEGER);
  efd_assert_child_count(node, 1, 1);

  color = (precise_color*) efd_as_o_fmt(efd_nth(node, 0), s_color);
  convert_to_xyz(color);
  return construct_efd_int_node(
    node->h.name,
    node,
    (efd_int_t) xyz__rgb(color)
  );
}

#endif // INCLUDE_EFD_FUNC_COLOR_H
#endif // EFD_REGISTRATION
