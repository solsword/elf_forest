// es_setup.c
// Gathers elfscript conversion registrations and defines elfscript setup.

#include <stdio.h>
#include <string.h>

#include "elfscript_setup.h"

#include "elfscript_parser.h"

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

#define ES_REGISTER_FUNCTIONS
es_function_declaration const ES_FUNCTION_REGISTRY[] = {
  #include "func.list"
  { // for the trailing comma
    .key = NULL,
    .function = NULL
  }
};
#undef ES_REGISTER_FUNCTIONS

#define ES_REGISTER_GENERATORS
es_generator_declaration const ES_GENERATOR_REGISTRY[] = {
  #include "gen.list"
  { // for the trailing comma
    .key = NULL,
    .constructor = NULL
  }
};
#undef ES_REGISTER_GENERATORS

#define ES_REGISTER_FORMATS
es_object_format const ES_FORMAT_REGISTRY[] = {
  #include "conv.list"
  { // for the trailing comma
    .key = NULL,
    .unpacker = NULL,
    .packer = NULL,
    .copier = NULL,
    .destructor = NULL
  }
};
#undef ES_REGISTER_FORMATS

size_t ES_FUNCTION_REGISTRY_SIZE = (
  sizeof(ES_FUNCTION_REGISTRY) / sizeof(es_function_declaration)
) - 1; // -1 for the extra entry
size_t ES_GENERATOR_REGISTRY_SIZE = (
  sizeof(ES_GENERATOR_REGISTRY) / sizeof(es_generator_declaration)
) - 1; // -1 for the extra entry
size_t ES_FORMAT_REGISTRY_SIZE = (
  sizeof(ES_FORMAT_REGISTRY) / sizeof(es_object_format)
) - 1; // -1 for the extra entry


dictionary *ES_FUNCTION_DICT = NULL;
dictionary *ES_GENERATOR_DICT = NULL;
dictionary *ES_FORMAT_DICT = NULL;

/*************
 * Functions *
 *************/

void setup_elfscript(int track_error_contexts) {
  size_t i;
  string *s;
  es_function_declaration const *fd;
  es_generator_declaration const *gd;
  es_object_format const *of;

  ELFSCRIPT_TRACK_ERROR_CONTEXTS = track_error_contexts;
  ELFSCRIPT_ERROR_CONTEXT = create_list();
  if (ELFSCRIPT_TRACK_ERROR_CONTEXTS) {
    es_push_error_context(s_("Error context:"));
  } else {
    l_append_element(
      ELFSCRIPT_ERROR_CONTEXT,
      (void*) s_("(error contexts not tracked)")
    );
  }

  ES_DATA_DIR = fs_dirchild(FS_RES_DIR, ES_DATA_DIR_NAME);
  ES_GLOBALS_DIR = fs_dirchild(ES_DATA_DIR, ES_GLOBALS_DIR_NAME);
  ES_COMMON_DIR = fs_dirchild(ES_DATA_DIR, ES_COMMON_DIR_NAME);

  ES_ROOT = create_es_node(ES_NT_CONTAINER, ES_ROOT_NAME, NULL);

  ES_GLOBALS = create_dictionary(ES_GLOBALS_TABLE_SIZE);

  ES_FUNCTION_DICT = create_dictionary(ES_FUNCTION_REGISTRY_SIZE);
  ES_GENERATOR_DICT = create_dictionary(ES_GENERATOR_REGISTRY_SIZE);
  ES_FORMAT_DICT = create_dictionary(ES_FORMAT_REGISTRY_SIZE);

  for (i = 0; i < ES_FUNCTION_REGISTRY_SIZE; ++i) {
    fd = &(ES_FUNCTION_REGISTRY[i]);
    s = create_string_from_ntchars(fd->key);
    d_add_value_s(ES_FUNCTION_DICT, s, (void*) fd->function);
    cleanup_string(s);
  }

  for (i = 0; i < ES_GENERATOR_REGISTRY_SIZE; ++i) {
    gd = &(ES_GENERATOR_REGISTRY[i]);
    s = create_string_from_ntchars(gd->key);
    d_add_value_s(ES_GENERATOR_DICT, s, (void*) gd->constructor);
    cleanup_string(s);
  }

  for (i = 0; i < ES_FORMAT_REGISTRY_SIZE; ++i) {
    of = &(ES_FORMAT_REGISTRY[i]);
    s = create_string_from_ntchars(of->key);
    d_add_value_s(ES_FORMAT_DICT, s, (void*) of);
    cleanup_string(s);
  }

#ifdef DEBUG
  if (sizeof(char) != sizeof(uint8_t)) {
    fprintf(stderr, "Warning: sizeof(char) != sizeof(uint8_t)\n");
  }
#endif
}

void cleanup_elf_forest_data(void) {
  cleanup_string(ES_DATA_DIR);
  cleanup_string(ES_COMMON_DIR);

  cleanup_es_node(ES_ROOT);

  d_foreach(ES_GLOBALS, &cleanup_v_es_node);
  cleanup_dictionary(ES_GLOBALS);

  cleanup_dictionary(ES_FUNCTION_DICT);
  cleanup_dictionary(ES_FORMAT_DICT);

  l_foreach(ELFSCRIPT_ERROR_CONTEXT, &cleanup_v_string);
  cleanup_list(ELFSCRIPT_ERROR_CONTEXT);
}

// Note that these lookup functions are declared in efd.h

es_eval_function es_lookup_function(string const * const key) {
  es_eval_function result;
  result = (es_eval_function) d_get_value_s(ES_FUNCTION_DICT, key);
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

es_generator_constructor es_lookup_generator(string const * const key) {
  es_generator_constructor result;
  result = (es_generator_constructor) d_get_value_s(ES_GENERATOR_DICT, key);
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

es_object_format * es_lookup_format(string const * const key) {
  es_object_format* result;
  result = (es_object_format*) d_get_value_s(ES_FORMAT_DICT, key);
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

es_unpack_function es_lookup_unpacker(string const * const key) {
  es_object_format *format = es_lookup_format(key);
  if (format == NULL) {
    fprintf(stderr, "Error while looking up unpack function.\n");
    return NULL;
  }
  return format->unpacker;
}

es_pack_function es_lookup_packer(string const * const key) {
  es_object_format *format = es_lookup_format(key);
  if (format == NULL) {
    fprintf(stderr, "Error while looking up pack function.\n");
    return NULL;
  }
  return format->packer;
}

es_copy_function es_lookup_copier(string const * const key) {
  es_object_format *format = es_lookup_format(key);
  if (format == NULL) {
    fprintf(stderr, "Error while looking up copy function.\n");
    return NULL;
  }
  return format->copier;
}

es_destroy_function es_lookup_destructor(string const * const key) {
  es_object_format *format = es_lookup_format(key);
  if (format == NULL) {
    fprintf(stderr, "Error while looking up destroy function.\n");
    return NULL;
  }
  return format->destructor;
}

// Helper function for loading a single file:
void _load_common_es_file(
  string const * const filename,
  struct stat const * const st,
  void* v_context
) {
  // TODO: consider return value?
  es_parse_file(ES_ROOT, s_raw(filename));
}

void load_common_es(void) {
  walk_dir_tree(
    ES_GLOBALS_DIR,
    &fs_walk_filter_handle_all,
    NULL,
    &fs_walk_filter_ignore_hidden,
    NULL,
    &fs_walk_filter_by_extension,
    (void*) s_raw(ES_FILE_EXTENSION),
    &_load_common_es_file,
    NULL
  );

  walk_dir_tree(
    ES_COMMON_DIR,
    &fs_walk_filter_handle_all,
    NULL,
    &fs_walk_filter_ignore_hidden,
    NULL,
    &fs_walk_filter_by_extension,
    (void*) s_raw(ES_FILE_EXTENSION),
    &_load_common_es_file,
    NULL
  );
}

void save_common_es(void) {
  // TODO: HERE!
}
