#if defined(EFD_REGISTER_DECLARATIONS)
// no declarations
#elif defined(EFD_REGISTER_NAMES)
"NULL",
#elif defined(EFD_REGISTER_UNPACKERS)
efd__null,
#elif defined(EFD_REGISTER_PACKERS)
null__efd,
#elif defined(EFD_REGISTER_COPIERS)
dont_copy,
#elif defined(EFD_REGISTER_DESTRUCTORS)
dont_cleanup,
#else
#ifndef INCLUDE_EFD_NULL_H
#define INCLUDE_EFD_NULL_H
// efd_null.h
// Creates NULL pointers

#include "efd/efd.h"

void* efd__null(efd_node *n) {
  return NULL;
}

efd_node *null__efd(void *v_t) {
  return create_efd_node(EFD_NT_CONTAINER, EFD_ANON_NAME);
}

#endif // INCLUDE_EFD_NULL_H
#endif // EFD_REGISTRATION
