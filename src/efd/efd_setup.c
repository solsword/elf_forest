// efd_setup.c
// Gathers Elf Forest Data conversion registrations and defines EFD setup.

#include <stdio.h>
#include <string.h>

#include "efd_setup.h"

#include "datatypes/dictionary.h"
#include "datatypes/string.h"

/**************************
 * Conversion Definitions *
 **************************/

#include "conv/efd_null.h"
#include "conv/efd_tests.h"
#include "conv/efd_rngtable.h"

/*************
 * Constants *
 *************/

#define EFD_REGISTER_NAMES
char * const EFD_OBJECT_NAME_REGISTRY[] = {
  #include "conv/efd_null.h"
  #include "conv/efd_tests.h"
  #include "conv/efd_rngtable.h"
  "INVALID"
};
#undef EFD_REGISTER_NAMES

// Get the size of our registry
size_t EFD_OBJECT_REGISTRY_SIZE = (
  sizeof(EFD_OBJECT_NAME_REGISTRY) / sizeof(char*)
);

#define EFD_REGISTER_UNPACKERS
efd_unpack_function EFD_OBJECT_UNPACKER_REGISTRY[] = {
  #include "conv/efd_null.h"
  #include "conv/efd_tests.h"
  #include "conv/efd_rngtable.h"
  NULL
};
#undef EFD_REGISTER_UNPACKERS

#define EFD_REGISTER_PACKERS
efd_pack_function EFD_OBJECT_PACKER_REGISTRY[] = {
  #include "conv/efd_null.h"
  #include "conv/efd_tests.h"
  #include "conv/efd_rngtable.h"
  NULL
};
#undef EFD_REGISTER_PACKERS

#define EFD_REGISTER_COPIERS
efd_copy_function EFD_OBJECT_COPIER_REGISTRY[] = {
  #include "conv/efd_null.h"
  #include "conv/efd_tests.h"
  #include "conv/efd_rngtable.h"
  NULL
};
#undef EFD_REGISTER_COPIERS

#define EFD_REGISTER_DESTRUCTORS
efd_destroy_function EFD_OBJECT_DESTRUCTOR_REGISTRY[] = {
  #include "conv/efd_null.h"
  #include "conv/efd_tests.h"
  #include "conv/efd_rngtable.h"
  NULL
};
#undef EFD_REGISTER_DESTRUCTORS

/*************
 * Functions *
 *************/

void setup_elf_forest_data(void) {
  EFD_ROOT = create_efd_node(EFD_NT_CONTAINER, EFD_ROOT_NAME);
  EFD_INT_GLOBALS = create_dictionary(EFD_GLOBALS_TABLE_SIZE);
  EFD_NUM_GLOBALS = create_dictionary(EFD_GLOBALS_TABLE_SIZE);
  EFD_STR_GLOBALS = create_dictionary(EFD_GLOBALS_TABLE_SIZE);
#ifdef DEBUG
  if (sizeof(char) != sizeof(uint8_t)) {
    fprintf(stderr, "Warning: sizeof(char) != sizeof(uint8_t)\n");
  }
#endif
}

void cleanup_elf_forest_data(void) {
  cleanup_efd_node(EFD_ROOT);
  cleanup_dictionary(EFD_INT_GLOBALS);
  cleanup_dictionary(EFD_NUM_GLOBALS);
  d_foreach(EFD_STR_GLOBALS, cleanup_v_string);
  cleanup_dictionary(EFD_STR_GLOBALS);
}

// Private helper for looking up string keys:
ptrdiff_t _efd_lookup_key(string const * const key) {
  size_t result = 0;
  for (result = 0; result < EFD_OBJECT_REGISTRY_SIZE; ++result) {
    char *n = EFD_OBJECT_NAME_REGISTRY[result];
    if (s_check_bytes(key, n)) {
      return result;
    }
  }
  return -1;
}

// Note that these lookup functions are declared earlier, in efd.h

efd_unpack_function efd_lookup_unpacker(string const * const key) {
  ptrdiff_t idx = _efd_lookup_key(key);
  if (idx < 0) {
    fprintf(
      stderr,
      "Error: no unpack function found for format '%.*s'.\n",
      (int) s_get_length(key),
      s_raw(key)
    );
    return NULL;
  }
  return EFD_OBJECT_UNPACKER_REGISTRY[idx];
}

efd_pack_function efd_lookup_packer(string const * const key) {
  ptrdiff_t idx = _efd_lookup_key(key);
  if (idx < 0) {
    fprintf(
      stderr,
      "Error: no pack function found for format '%.*s'.\n",
      (int) s_get_length(key),
      s_raw(key)
    );
    return NULL;
  }
  return EFD_OBJECT_PACKER_REGISTRY[idx];
}

efd_copy_function efd_lookup_copier(string const * const key) {
  ptrdiff_t idx = _efd_lookup_key(key);
  if (idx < 0) {
    fprintf(
      stderr,
      "Error: no copy function found for format '%.*s'.\n",
      (int) s_get_length(key),
      s_raw(key)
    );
    return NULL;
  }
  return EFD_OBJECT_COPIER_REGISTRY[idx];
}

efd_destroy_function efd_lookup_destructor(string const * const key) {
  ptrdiff_t idx = _efd_lookup_key(key);
  if (idx < 0) {
    fprintf(
      stderr,
      "Error: no destroy function found for format '%.*s'.\n",
      (int) s_get_length(key),
      s_raw(key)
    );
    return NULL;
  }
  return EFD_OBJECT_DESTRUCTOR_REGISTRY[idx];
}
