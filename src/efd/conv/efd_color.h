#if defined(EFD_REGISTER_DECLARATIONS)
// no declarations
#elif defined(EFD_REGISTER_FORMATS)
{
  .key = "color",
  .unpacker = efd__color,
  .packer = color__efd,
  .copier = copy_v_color,
  .destructor = cleanup_v_color
},
#else
#ifndef INCLUDE_EFD_CONV_COLOR_H
#define INCLUDE_EFD_CONV_COLOR_H
// efd_color.h
// Conversions efd <-> precise_color

#include <stdio.h>

#include "efd/efd.h"
#include "tex/color.h"

void* efd__color(efd_node *n) {
  efd_node *val, *alpha;
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

  efd_assert_type(n, EFD_NT_CONTAINER);
  efd_assert_child_count(n, 4, 4);

  val = efd_get_value(n);
  efd_assert_type(val, EFD_NT_CONTAINER);

  result = (precise_color*) malloc(sizeof(precise_color));

  result->format = efd_as_i(efd_lookup_expected(val, s_fmt));

  // Lookup components according to the specified format:
  switch (result->format) {
    default:
    case CFMT_INVALID:
      efd_report_error_full(
        s_("ERROR: 'color' node has invalid 'fmt' argument:"),
        n
      );
      exit(EXIT_FAILURE);

    case CFMT_RGB:
      px_set_red(&px, (channel) efd_as_i(efd_lookup_expected(val, s_R)));
      px_set_green(&px, (channel) efd_as_i(efd_lookup_expected(val, s_G)));
      px_set_blue(&px, (channel) efd_as_i(efd_lookup_expected(val, s_B)));
      result->format = CFMT_XYZ;
      rgb__xyz(px, result);
      break;

    case CFMT_XYZ:
      result->x = efd_as_n(efd_lookup_expected(val, s_X));
      result->y = efd_as_n(efd_lookup_expected(val, s_Y));
      result->z = efd_as_n(efd_lookup_expected(val, s_Z));
      break;

    case CFMT_LCH:
      result->x = efd_as_n(efd_lookup_expected(val, s_L));
      result->y = efd_as_n(efd_lookup_expected(val, s_c));
      result->z = efd_as_n(efd_lookup_expected(val, s_h));
      break;

    case CFMT_LAB:
      result->x = efd_as_n(efd_lookup_expected(val, s_L));
      result->y = efd_as_n(efd_lookup_expected(val, s_a));
      result->z = efd_as_n(efd_lookup_expected(val, s_b));
      break;
  }

  // Lookup alpha if it's present or assume the default of 1.0:
  alpha = efd_lookup(val, s_A);
  if (alpha != NULL) {
    result->alpha = efd_as_n(alpha);
  } else {
    result->alpha = 1.0;
  }

  // Normalize the result to XYZ:
  convert_to_xyz(result);

  return (void*) result;
}

efd_node *color__efd(void *v_color) {
  precise_color *color = (precise_color*) v_color;
  efd_node *result;

  SSTR(s_fmt, "fmt", 3);
  SSTR(s_X, "X", 1);
  SSTR(s_Y, "Y", 1);
  SSTR(s_Z, "Z", 1);
  SSTR(s_A, "A", 1);

  result = create_efd_node(EFD_NT_CONTAINER, EFD_ANON_NAME, NULL);

  convert_to_xyz(color);

  efd_add_child(result, construct_efd_int_node(s_fmt, NULL, color->format));
  efd_add_child(result, construct_efd_num_node(s_X, NULL, color->x));
  efd_add_child(result, construct_efd_num_node(s_Y, NULL, color->y));
  efd_add_child(result, construct_efd_num_node(s_Z, NULL, color->z));
  efd_add_child(result, construct_efd_num_node(s_A, NULL, color->alpha));

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

#endif // INCLUDE_EFD_CONV_COLOR_H
#endif // EFD_REGISTRATION
