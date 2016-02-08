#ifndef BOILERPLATE_H
#define BOILERPLATE_H

// boilerplate.h
// Macros for boilerplate code.

#define CLEANUP_DECL(typ) \
  void cleanup_v_ ## typ(void* v_doomed); \
  void cleanup_ ## typ(typ* doomed)

#define CLEANUP_IMPL(typ) \
  void cleanup_v_ ## typ(void* v_doomed) { cleanup_ ## typ((typ*) v_doomed); } \
  void cleanup_ ## typ(typ* doomed)

#endif // BOILERPLATE_H
