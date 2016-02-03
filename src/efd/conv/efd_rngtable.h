#if defined(EFD_REGISTER_NAMES)
"rngtbl"
#elif defined(EFD_REGISTER_UNPACKERS)
efd__rngtable
#elif defined(EFD_REGISTER_PACKERS)
rngtable__efd
#elif defined(EFD_REGISTER_DESTRUCTORS)
cleanup_v_rngtable
#else
#ifndef INCLUDE_EFD_RNGTABLE_H
#define INCLUDE_EFD_RNGTABLE_H
// efd_rngtable.h
// Conversions efd <-> rngtable

#include "efd/efd.h"
#include "datatypes/rngtable.h"

void* efd__rngtable(efd_node *n) {
  size_t i, s, vcount, wcount;
  ptrdiff_t *values;
  float *weights;
  efd_node *field;
  efd_assert_type(n, EFD_NT_CONTAINER);
  s = (size_t) *efd__i(efd(n, "size"));
  field = efd(n, "values");
  values = efd__ai(field);
  vcount = efd_count__ai(field);
  if (vcount != s) {
    fprintf(
      stderr,
      "ERROR: rngtable node should have %zu values but has %zu instead.\n",
      s,
      vcount
    );
  }
  field = efd(n, "weights")
  weights = efd__an(field);
  wcount = efd_count__an(field);
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

efd_node *rngtable__efd(rngtable *t) {
  size_t i;
  efd_node *n;
  efd_node *result;
  
  result = create_efd_node(EFD_NT_CONTAINER, EFD_PROTO_NAME)

  n = create_efd_node(EFD_NT_INTEGER, "size");
  *efd__i(n) = t->size;
  efd_add_child(result, n);

  n = create_efd_node(EFD_NT_ARRAY_INT, "values");
  efd__ai(n) = (ptrdiff_t*) malloc(t->size * sizeof(ptrdiff_t));
  for (i = 0; i < t->size; ++i) {
    efd__ai(n)[i] = t->values[i];
  }
  efd_add_child(result, n);

  n = create_efd_node(EFD_NT_ARRAY_NUM, "weights");
  efd__an(n) = (float*) malloc(t->size * sizeof(float));
  for (i = 0; i < t->size; ++i) {
    efd__an(n)[i] = t->weights[i];
  }
  efd_add_child(result, n);

  return result;
}

void cleanup_v_rngtable(void *v_rngtable) {
  cleanup_rngtable((rngtable*) v_rngtable);
  free(v_rngtable);
}

#endif // INCLUDE_EFD_RNGTABLE_H
#endif // EFD_REGISTRATION
