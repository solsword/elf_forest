#if defined(ELFSCRIPT_REGISTER_DECLARATIONS)
// no declarations
#elif defined(ELFSCRIPT_REGISTER_FORMATS)
{
  .key = "rngtable",
  .unpacker = elfscript__rngtable,
  .packer = rngtable__elfscript,
  .copier = copy_v_rngtable,
  .destructor = cleanup_v_rngtable
},
#else
#ifndef INCLUDE_ELFSCRIPT_CONV_RNGTABLE_H
#define INCLUDE_ELFSCRIPT_CONV_RNGTABLE_H
// elfscript_rngtable.h
// Conversions elfscript <-> rngtable

#include <stdio.h>

#include "elfscript/elfscript.h"
#include "datatypes/rngtable.h"

void* elfscript__rngtable(es_scope *sc) {
  size_t i, s, vcount, wcount;
  ptrdiff_t *values;
  float *weights;
  es_scope *vfield, *wfield;
  SSTR(s_size, "size", 4);
  SSTR(s_values, "values", 6);
  SSTR(s_weights, "weights", 7);

  s = (size_t) es_as_i(es_read_var(sc, s_size));
  vfield = es_as_scope(es_read_var(sc, s_values));
  vcount = es_scope_size(vfield);
  if (vcount != s) {
    fprintf(
      stderr,
      "ERROR: rngtable node should have %zu values but has %zu instead.\n",
      s,
      vcount
    );
  }
  wfield = es_as_scope(es_read_var(sc, s_weights));
  wcount = es_scope_size(wfield);
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
    result->values[i] = es_raw_value(es_read_nth(vfield, i));
    result->weights[i] = es_as_n(es_read_nth(wfield, i));
  }
  return (void*) result;
}

elfscript_node *rngtable__elfscript(void *v_t) {
  rngtable *t = (rngtable*) v_t;
  size_t i;
  es_scope *vfield, *wfield;
  es_scope *result;
  SSTR(s_size, "size", 4);
  SSTR(s_values, "values", 6);
  SSTR(s_weights, "weights", 7);
  
  result = create_es_scope();

  es_write_var(result, s_size, create_es_int_var((es_int_t) t->size));

  vfield = create_es_scope();
  es_write_var(result, s_values, create_es_scp_var(vfield));
  wfield = create_es_scope();
  es_write_var(result, s_weights, create_es_scp_var(wfield));

  for (i = 0; i < t->size; ++i) {
    // TODO: Better format than ints here?
    es_write_last(vfield, create_es_int_var((es_int_t) t->values[i]));
    es_write_last(wfield, create_es_num_var((es_num_t) t->weights[i]));
  }

  return result;
}

void* copy_v_rngtable(void *v_t) {
  return (void*) copy_rngtable((rngtable*) v_t);
}

#endif // INCLUDE_ELFSCRIPT_CONV_RNGTABLE_H
#endif // ELFSCRIPT_REGISTRATION
