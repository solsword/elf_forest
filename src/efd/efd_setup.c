// efd_setup.c
// Gathers Elf Forest Data conversion registrations and defines EFD setup.

#include <stdio.h>
#include <string.h>

#include "efd_setup.h"

#include "datatypes/dictionary.h"
#include "datatypes/dict_string_keys.h"
#include "datatypes/string.h"

/**************************
 * Conversion Definitions *
 **************************/

#include "conv.list"

/***********
 * Globals *
 ***********/

#define EFD_REGISTER_FUNCTIONS
efd_function_declaration const EFD_FUNCTION_REGISTRY[] = {
  #include "func.list"
  { // for the trailing comma
    .key = NULL;
    .function = NULL;
  }
};
#undef EFD_REGISTER_FUNCTIONS

#define EFD_REGISTER_FORMATS
efd_object_format const EFD_FORMAT_REGISTRY[] = {
  #include "conv.list"
  { // for the trailing comma
    .key = NULL;
    .unpacker = NULL;
    .packer = NULL;
    .copier = NULL;
    .destructor = NULL;
  }
};
#undef EFD_REGISTER_FORMATS

size_t EFD_FUNCTION_REGISTRY_SIZE = (
  sizeof(EFD_FUNCTION_REGISTRY) / sizeof(efd_function_declaration)
) - 1; // -1 for the extra entry
size_t EFD_FORMAT_REGISTRY_SIZE = (
  sizeof(EFD_FORMAT_REGISTRY) / sizeof(efd_object_format)
) - 1; // -1 for the extra entry


dictionary *EFD_FUNCTION_DICT = NULL;
dictionary *EFD_FORMAT_DICT = NULL;

/*************
 * Functions *
 *************/

void setup_elf_forest_data(void) {
  size_t i;
  string *s;
  efd_function_declaration *fd;
  efd_object_format *of;

  EFD_ROOT = create_efd_node(EFD_NT_CONTAINER, EFD_ROOT_NAME);

  EFD_INT_GLOBALS = create_dictionary(EFD_GLOBALS_TABLE_SIZE);
  EFD_NUM_GLOBALS = create_dictionary(EFD_GLOBALS_TABLE_SIZE);
  EFD_STR_GLOBALS = create_dictionary(EFD_GLOBALS_TABLE_SIZE);

  EFD_FUNCTION_DICT = create_dictionary(EFD_GLOBALS_TABLE_SIZE);
  EFD_FORMAT_DICT = create_dictionary(EFD_GLOBALS_TABLE_SIZE);

  for (i = 0; i < EFD_FUNCTION_REGISTRY_SIZE; ++i) {
    fd = &(EFD_FUNCTION_REGISTRY[i]);
    s = create_string_from_ntchars(fd->key);
    d_add_value_s(EFD_FUNCTION_DICT, s, (void*) fd->function);
  }

  for (i = 0; i < EFD_FORMAT_REGISTRY_SIZE; ++i) {
    of = &(EFD_FORMAT_REGISTRY[i]);
    s = create_string_from_ntchars(of->key);
    d_add_value_s(EFD_FORMAT_DICT, s, (void*) of);
  }

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
  d_foreach(EFD_STR_GLOBALS, &cleanup_v_string);
  cleanup_dictionary(EFD_STR_GLOBALS);

  cleanup_dictionary(EFD_FUNCTION_DICT);
  cleanup_dictionary(EFD_FORMAT_DICT);
}

// Note that these lookup functions are declared in efd.h

efd_eval_function efd_lookup_function(string const * const key) {
  efd_eval_function result;
  result = (efd_eval_function) d_get_value_s(EFD_FUNCTION_DICT, key);
  if (result == NULL) {
    fprintf(
      stderr,
      "Error: function '%.*s' not found.\n",
      (int) s_get_length(key),
      s_raw(key)
    );
    return NULL;
  }
  return result;
}

efd_object_format * efd_lookup_format(string const * const key) {
  efd_object_format* result;
  result = (efd_object_format*) d_get_value_s(EFD_FORMAT_DICT, key);
  if (result == NULL) {
    fprintf(
      stderr,
      "Error: object format '%.*s' not found.\n",
      (int) s_get_length(key),
      s_raw(key)
    );
    return NULL;
  }
  return result;
}

efd_unpack_function efd_lookup_unpacker(string const * const key) {
  return efd_lookup_format(key)->unpacker;
}

efd_pack_function efd_lookup_packer(string const * const key) {
  return efd_lookup_format(key)->packer;
}

efd_copy_function efd_lookup_copier(string const * const key) {
  return efd_lookup_format(key)->copier;
}

efd_destroy_function efd_lookup_destructor(string const * const key) {
  return efd_lookup_format(key)->destructor;
}
