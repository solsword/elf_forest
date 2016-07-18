// efd_setup.c
// Gathers Elf Forest Data conversion registrations and defines EFD setup.

#include <stdio.h>
#include <string.h>

#include "efd_setup.h"

#include "efd_parser.h"

#include "datatypes/dictionary.h"
#include "datatypes/dict_string_keys.h"
#include "datatypes/string.h"

#include "filesys/filesys.h"

/***********************
 * Plug-in Definitions *
 ***********************/

#include "func.list"
#include "gen.list"
#include "conv.list"

/***********
 * Globals *
 ***********/

#define EFD_REGISTER_FUNCTIONS
efd_function_declaration const EFD_FUNCTION_REGISTRY[] = {
  #include "func.list"
  { // for the trailing comma
    .key = NULL,
    .function = NULL
  }
};
#undef EFD_REGISTER_FUNCTIONS

#define EFD_REGISTER_GENERATORS
efd_generator_declaration const EFD_GENERATOR_REGISTRY[] = {
  #include "gen.list"
  { // for the trailing comma
    .key = NULL,
    .constructor = NULL
  }
};
#undef EFD_REGISTER_GENERATORS

#define EFD_REGISTER_FORMATS
efd_object_format const EFD_FORMAT_REGISTRY[] = {
  #include "conv.list"
  { // for the trailing comma
    .key = NULL,
    .unpacker = NULL,
    .packer = NULL,
    .copier = NULL,
    .destructor = NULL
  }
};
#undef EFD_REGISTER_FORMATS

size_t EFD_FUNCTION_REGISTRY_SIZE = (
  sizeof(EFD_FUNCTION_REGISTRY) / sizeof(efd_function_declaration)
) - 1; // -1 for the extra entry
size_t EFD_GENERATOR_REGISTRY_SIZE = (
  sizeof(EFD_GENERATOR_REGISTRY) / sizeof(efd_generator_declaration)
) - 1; // -1 for the extra entry
size_t EFD_FORMAT_REGISTRY_SIZE = (
  sizeof(EFD_FORMAT_REGISTRY) / sizeof(efd_object_format)
) - 1; // -1 for the extra entry


dictionary *EFD_FUNCTION_DICT = NULL;
dictionary *EFD_GENERATOR_DICT = NULL;
dictionary *EFD_FORMAT_DICT = NULL;

/*************
 * Functions *
 *************/

void setup_elf_forest_data(int track_error_contexts) {
  size_t i;
  string *s;
  efd_function_declaration const *fd;
  efd_generator_declaration const *gd;
  efd_object_format const *of;

  EFD_TRACK_ERROR_CONTEXTS = track_error_contexts;
  EFD_ERROR_CONTEXT = create_list();
  if (EFD_TRACK_ERROR_CONTEXTS) {
    efd_push_error_context(s_("Error context:"));
  } else {
    l_append_element(
      EFD_ERROR_CONTEXT,
      (void*) s_("(error contexts not tracked)")
    );
  }

  EFD_COMMON_DIR = fs_dirchild(FS_RES_DIR, EFD_COMMON_DIR_NAME);

  EFD_ROOT = create_efd_node(EFD_NT_CONTAINER, EFD_ROOT_NAME);

  EFD_GLOBALS = create_dictionary(EFD_GLOBALS_TABLE_SIZE);

  EFD_FUNCTION_DICT = create_dictionary(EFD_FUNCTION_REGISTRY_SIZE);
  EFD_GENERATOR_DICT = create_dictionary(EFD_GENERATOR_REGISTRY_SIZE);
  EFD_FORMAT_DICT = create_dictionary(EFD_FORMAT_REGISTRY_SIZE);

  for (i = 0; i < EFD_FUNCTION_REGISTRY_SIZE; ++i) {
    fd = &(EFD_FUNCTION_REGISTRY[i]);
    s = create_string_from_ntchars(fd->key);
    d_add_value_s(EFD_FUNCTION_DICT, s, (void*) fd->function);
    cleanup_string(s);
  }

  for (i = 0; i < EFD_GENERATOR_REGISTRY_SIZE; ++i) {
    gd = &(EFD_GENERATOR_REGISTRY[i]);
    s = create_string_from_ntchars(gd->key);
    d_add_value_s(EFD_GENERATOR_DICT, s, (void*) gd->constructor);
    cleanup_string(s);
  }

  for (i = 0; i < EFD_FORMAT_REGISTRY_SIZE; ++i) {
    of = &(EFD_FORMAT_REGISTRY[i]);
    s = create_string_from_ntchars(of->key);
    d_add_value_s(EFD_FORMAT_DICT, s, (void*) of);
    cleanup_string(s);
  }

#ifdef DEBUG
  if (sizeof(char) != sizeof(uint8_t)) {
    fprintf(stderr, "Warning: sizeof(char) != sizeof(uint8_t)\n");
  }
#endif
}

void cleanup_elf_forest_data(void) {
  cleanup_string(EFD_COMMON_DIR);

  cleanup_efd_node(EFD_ROOT);

  d_foreach(EFD_GLOBALS, &cleanup_v_efd_node);
  cleanup_dictionary(EFD_GLOBALS);

  cleanup_dictionary(EFD_FUNCTION_DICT);
  cleanup_dictionary(EFD_FORMAT_DICT);

  l_foreach(EFD_ERROR_CONTEXT, &cleanup_v_string);
  cleanup_list(EFD_ERROR_CONTEXT);
}

// Note that these lookup functions are declared in efd.h

efd_eval_function efd_lookup_function(string const * const key) {
  efd_eval_function result;
  result = (efd_eval_function) d_get_value_s(EFD_FUNCTION_DICT, key);
  if (result == NULL) {
    fprintf(
      stderr,
      "ERROR: function '%.*s' not found.\n",
      (int) s_count_bytes(key),
      s_raw(key)
    );
    return NULL;
  }
  return result;
}

efd_generator_constructor efd_lookup_generator(string const * const key) {
  efd_generator_constructor result;
  result = (efd_generator_constructor) d_get_value_s(EFD_GENERATOR_DICT, key);
  if (result == NULL) {
    fprintf(
      stderr,
      "ERROR: constructor for generator '%.*s' not found.\n",
      (int) s_count_bytes(key),
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
      "ERROR: object format '%.*s' not found.\n",
      (int) s_count_bytes(key),
      s_raw(key)
    );
    return NULL;
  }
  return result;
}

efd_unpack_function efd_lookup_unpacker(string const * const key) {
  efd_object_format *format = efd_lookup_format(key);
  if (format == NULL) {
    fprintf(stderr, "Error while looking up unpack function.\n");
    return NULL;
  }
  return format->unpacker;
}

efd_pack_function efd_lookup_packer(string const * const key) {
  efd_object_format *format = efd_lookup_format(key);
  if (format == NULL) {
    fprintf(stderr, "Error while looking up unpack function.\n");
    return NULL;
  }
  return format->packer;
}

efd_copy_function efd_lookup_copier(string const * const key) {
  efd_object_format *format = efd_lookup_format(key);
  if (format == NULL) {
    fprintf(stderr, "Error while looking up unpack function.\n");
    return NULL;
  }
  return format->copier;
}

efd_destroy_function efd_lookup_destructor(string const * const key) {
  efd_object_format *format = efd_lookup_format(key);
  if (format == NULL) {
    fprintf(stderr, "Error while looking up unpack function.\n");
    return NULL;
  }
  return format->destructor;
}

void load_common_efd_file(
  string const * const filename,
  struct stat const * const st,
  void* v_context
) {
  // TODO: consider return value?
  efd_parse_file(EFD_ROOT, s_raw(filename));
}

void load_common_efd(void) {
  walk_dir_tree(
    EFD_COMMON_DIR,
    &fs_walk_filter_handle_all,
    NULL,
    &fs_walk_filter_ignore_hidden,
    NULL,
    &fs_walk_filter_by_extension,
    (void*) s_raw(EFD_FILE_EXTENSION),
    &load_common_efd_file,
    NULL
  );

  // Unpack prototypes into objects:
  efd_unpack_node(EFD_ROOT);
  d_foreach(EFD_GLOBALS, &efd_unpack_v_node);
}

void save_common_efd(void) {
  // TODO: HERE!
}
