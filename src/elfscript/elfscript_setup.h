#ifndef INCLUDE_ELFSCRIPT_SETUP_H
#define INCLUDE_ELFSCRIPT_SETUP_H

// elfscript_setup.h
// Gathers elfscript conversion registrations and defines elfscript setup.

#include "elfscript.h"

/***********
 * Globals *
 ***********/

extern es_function_declaration const ES_FUNCTION_REGISTRY[];
extern es_generator_declaration const ES_GENERATOR_REGISTRY[];
extern es_object_format const ES_FORMAT_REGISTRY[];

extern size_t ES_FUNCTION_REGISTRY_SIZE;
extern size_t ES_GENERATOR_REGISTRY_SIZE;
extern size_t ES_FORMAT_REGISTRY_SIZE;

extern dictionary *ES_FUNCTION_DICT;
extern dictionary *ES_GENERATOR_DICT;
extern dictionary *ES_FORMAT_DICT;

/*************************
 * Included Declarations *
 *************************/

#define ES_REGISTER_DECLARATIONS
#include "func.list"
#include "gen.list"
#include "conv.list"
#undef ES_REGISTER_DECLARATIONS

/*************
 * Functions *
 *************/

// Gets the elfscript system set up.
void setup_elfscript(int track_error_contexts);

// Cleans up the elfscript system.
void cleanup_elfscript(void);

// Note that the lookup functions declared earlier in elfscript.h are defined
// in elfscript_setup.c

// Loads common elfscript code from res/script, recursively looking for
// *.es files and loading each.
void load_common_elfscript(void);

// TODO: This function!
void save_common_elfscript(void);

#endif // INCLUDE_ELFSCRIPT_SETUP_H
