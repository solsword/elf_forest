// snek/summon.c
// Main Python interface file used to call Python code from Elf Forest.

#include <stdio.h>
#include <string.h>

#include "python.h"

#include "summon.h"

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

void summon_snek() {
}

void dismiss_snek() {
}

void load_common_scripts(void) {
}
