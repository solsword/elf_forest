#if defined(EFD_REGISTER_DECLARATIONS)
// no declarations
#elif defined(EFD_REGISTER_FORMATS)
{
  .key = "rngtable",
  .unpacker = efd__rngtable,
  .packer = rngtable__efd,
  .copier = copy_v_rngtable,
  .destructor = cleanup_v_rngtable
},
#else
#ifndef INCLUDE_EFD_RNGTABLE_H
#define INCLUDE_EFD_RNGTABLE_H
// efd_rngtable.h
// Conversions efd <-> rngtable

#include <stdio.h>

#include "efd/efd.h"
#include "datatypes/rngtable.h"

void* efd__rngtable(efd_node *n) {
  size_t i, s, vcount, wcount;
  ptrdiff_t *values;
  float *weights;
  efd_node *field;
  SSTR(s_size, "size", 4);
  SSTR(s_values, "values", 6);
  SSTR(s_weights, "weights", 7);
  efd_assert_type(n, EFD_NT_CONTAINER);
  s = (size_t) *efd__i(efd_lookup(n, s_size));
  field = efd_lookup(n, s_values);
  values = *efd__ai(field);
  vcount = *efd__ai_count(field);
  if (vcount != s) {
    fprintf(
      stderr,
      "ERROR: rngtable node should have %zu values but has %zu instead.\n",
      s,
      vcount
    );
  }
  field = efd_lookup(n, s_weights);
  weights = *efd__an(field);
  wcount = *efd__an_count(field);
  if (wcount != s) {
    fprintf(
      stderr,
      "ERROR: rngtable node should have %zu weights but has %zu instead.\n",
      s,
      wcount
    );
  }
  rngtable *result = create_rngtable(s);
  for (i = 0; i < s; ++i) {
    result->values[i] = (void*) values[i];
    result->weights[i] = weights[i];
  }
  return (void*) result;
}

efd_node *rngtable__efd(void *v_t) {
  rngtable *t = (rngtable*) v_t;
  size_t i;
  efd_node *n;
  efd_node *result;
  SSTR(s_size, "size", 4);
  SSTR(s_values, "values", 6);
  SSTR(s_weights, "weights", 7);
  
  result = create_efd_node(EFD_NT_CONTAINER, EFD_ANON_NAME);

  n = create_efd_node(EFD_NT_INTEGER, s_size);
  *efd__i(n) = t->size;
  efd_add_child(result, n);

  n = create_efd_node(EFD_NT_ARRAY_INT, s_values);
  *efd__ai(n) = (ptrdiff_t*) malloc(t->size * sizeof(ptrdiff_t));
  for (i = 0; i < t->size; ++i) {
    (*efd__ai(n))[i] = (ptrdiff_t) t->values[i];
  }
  efd_add_child(result, n);

  n = create_efd_node(EFD_NT_ARRAY_NUM, s_weights);
  *efd__an(n) = (float*) malloc(t->size * sizeof(float));
  for (i = 0; i < t->size; ++i) {
    (*efd__an(n))[i] = t->weights[i];
  }
  efd_add_child(result, n);

  return result;
}

void* copy_v_rngtable(void *v_t) {
  return (void*) copy_rngtable((rngtable*) v_t);
}

#endif // INCLUDE_EFD_RNGTABLE_H
#endif // EFD_REGISTRATION
