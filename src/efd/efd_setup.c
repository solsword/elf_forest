// efd_setup.c
// Gathers Elf Forest Data conversion registrations and defines EFD setup.

#include <stdio.h>
#include <string.h>

#include "efd_setup.h"

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
  EFD_INT_GLOBALS = create_map(EFD_GLOBALS_KEY_ARITY, EFD_GLOBALS_TABLE_SIZE);
  EFD_NUM_GLOBALS = create_map(EFD_GLOBALS_KEY_ARITY, EFD_GLOBALS_TABLE_SIZE);
  EFD_STR_GLOBALS = create_map(EFD_GLOBALS_KEY_ARITY, EFD_GLOBALS_TABLE_SIZE);
  if (sizeof(char) != sizeof(uint8_t)) {
    fprintf(stderr, "Warning: sizeof(char) != sizeof(uint8_t)\n");
  }
  if (EFD_NODE_NAME_SIZE != 32) {
    fprintf(
      stderr,
      "Warning: unexpected EFD node name size: %ld\n",
      EFD_NODE_NAME_SIZE
    );
  }
  if (EFD_ANNOTATION_SIZE != 8) {
    fprintf(
      stderr,
      "Warning: unexpected EFD object format size: %ld\n",
      EFD_ANNOTATION_SIZE
    );
  }
}

// cleanup helper:
// TODO: Aggregate these!!
void _cleanup_string_in_map(void* v_string) {
  cleanup_string((string*) v_string);
}

void cleanup_elf_forest_data(void) {
  cleanup_efd_node(EFD_ROOT);
  cleanup_map(EFD_INT_GLOBALS);
  cleanup_map(EFD_NUM_GLOBALS);
  m_foreach(EFD_STR_GLOBALS, _cleanup_string_in_map);
  cleanup_map(EFD_STR_GLOBALS);
}

// Private helper for looking up string keys:
ptrdiff_t _efd_lookup_key(char const * const key) {
  size_t result = 0;
  for (result = 0; result < EFD_OBJECT_REGISTRY_SIZE; ++result) {
    char *n = EFD_OBJECT_NAME_REGISTRY[result];
    if (strncmp(key, n, EFD_ANNOTATION_SIZE) == 0) {
      return result;
    }
  }
  return -1;
}

// Note that these lookup functions are declared earlier, in efd.h

efd_unpack_function efd_lookup_unpacker(char const * const key) {
  ptrdiff_t idx = _efd_lookup_key(key);
  if (idx < 0) {
    fprintf(
      stderr,
      "Error: no unpack function found for format '%.*s'.\n",
      (int) EFD_ANNOTATION_SIZE,
      key
    );
    return NULL;
  }
  return EFD_OBJECT_UNPACKER_REGISTRY[idx];
}

efd_pack_function efd_lookup_packer(char const * const key) {
  ptrdiff_t idx = _efd_lookup_key(key);
  if (idx < 0) {
    fprintf(
      stderr,
      "Error: no pack function found for format '%.*s'.\n",
      (int) EFD_ANNOTATION_SIZE,
      key
    );
    return NULL;
  }
  return EFD_OBJECT_PACKER_REGISTRY[idx];
}

efd_copy_function efd_lookup_copier(char const * const key) {
  ptrdiff_t idx = _efd_lookup_key(key);
  if (idx < 0) {
    fprintf(
      stderr,
      "Error: no copy function found for format '%.*s'.\n",
      (int) EFD_ANNOTATION_SIZE,
      key
    );
    return NULL;
  }
  return EFD_OBJECT_COPIER_REGISTRY[idx];
}

efd_destroy_function efd_lookup_destructor(char const * const key) {
  ptrdiff_t idx = _efd_lookup_key(key);
  if (idx < 0) {
    fprintf(
      stderr,
      "Error: no destroy function found for format '%.*s'.\n",
      (int) EFD_ANNOTATION_SIZE,
      key
    );
    return NULL;
  }
  return EFD_OBJECT_DESTRUCTOR_REGISTRY[idx];
}
