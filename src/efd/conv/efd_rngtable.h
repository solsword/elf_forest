#if defined(EFD_REGISTER_NAME)
"rngtable"
#elif defined(EFD_REGISTER_UNPACKER)
efd__rngtable
#elif defined(EFD_REGISTER_PACKER)
rngtable__efd
#elif defined(EFD_REGISTER_DESTRUCTOR)
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
  s = (size_t) efd__i(efd(n, "size"));
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
  efd_node *result = create_efd_node(EFD_NT_CONTAINER, "-")
}

void cleanup_v_rngtable(void *v_rngtable) {
  cleanup_rngtable((rngtable*) v_rngtable);
  free(v_rngtable);
}

#endif // INCLUDE_EFD_RNGTABLE_H
#endif // EFD_REGISTRATION
