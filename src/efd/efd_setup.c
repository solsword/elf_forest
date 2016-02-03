// efd_setup.c
// Gathers Elf Forest Data conversion registrations and defines EFD setup.

#include "efd_setup.h"

/**************************
 * Conversion Definitions *
 **************************/

#include "conv/efd_rngtable.h"

/*************
 * Constants *
 *************/

#define EFD_REGISTER_NAMES
extern char * const EFD_OBJECT_NAME_REGISTRY[] = {
  #include "conv/efd_rngtable.h",
  #include "conv/efd_tests.h"
};
#undef EFD_REGISTER_NAMES

// Get the size of our registry
size_t EFD_OBJECT_REGISTRY_SIZE = (
  sizeof(EFD_OBJECT_NAME_REGISTRY) / sizeof(char*)
);

#define EFD_REGISTER_UNPACKERS
extern efd_unpack_function EFD_OBJECT_UNPACKER_REGISTRY[] = {
  #include "conv/efd_rngtable.h",
  #include "conv/efd_tests.h"
};
#undef EFD_REGISTER_UNPACKERS

#define EFD_REGISTER_PACKERS
extern efd_pack_function EFD_OBJECT_PACKER_REGISTRY[] = {
  #include "conv/efd_rngtable.h",
  #include "conv/efd_tests.h"
};
#undef EFD_REGISTER_PACKERS

#define EFD_REGISTER_DESTRUCTORS
extern efd_destroy_function EFD_OBJECT_DESTRUCTOR_REGISTRY[] = {
  #include "conv/efd_rngtable.h",
  #include "conv/efd_tests.h"
};
#undef EFD_REGISTER_DESTRUCTORS

/*************
 * Functions *
 *************/

void setup_elf_forest_data(void) {
  EFD_ROOT = create_efd_node(EFD_NT_CONTAINER, "_");
}

void cleanup_elf_forest_data(void) {
  cleanup_efd_node(EFD_ROOT);
}

// Private helper for looking up string keys:
ptrdiff_t _efd_lookup_key(char const * const key) {
  size_t result = 0;
  for (result = 0; result < EFD_OBJECT_REGISTRY_SIZE; ++result) {
    char *n = EFD_OBJECT_NAME_REGISTRY[result];
    if (strncmp(key, n, EFD_OBJECT_FORMAT_SIZE) == 0) {
      return result;
    }
  }
  return -1;
}

// Note that these lookup functions are declared earlier, in efd.h

efd_unpack_function efd_lookup_unpacker(char const * const key) {
  ptrdiff_t idx = _efd_lookup_key(key);
  if (idx < 0) { return NULL; }
  return EFD_OBJECT_UNPACKER_REGISTRY[idx];
}

efd_pack_function efd_lookup_packer(char const * const key) {
  ptrdiff_t idx = _efd_lookup_key(key);
  if (idx < 0) { return NULL; }
  return EFD_OBJECT_PACKER_REGISTRY[idx];
}

efd_destroy_function efd_lookup_destructor(char const * const key) {
  ptrdiff_t idx = _efd_lookup_key(key);
  if (idx < 0) { return NULL; }
  return EFD_OBJECT_DESTRUCTOR_REGISTRY[idx];
}
