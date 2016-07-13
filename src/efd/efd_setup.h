#ifndef INCLUDE_EFD_SETUP_H
#define INCLUDE_EFD_SETUP_H

// efd_setup.h
// Gathers Elf Forest Data conversion registrations and defines EFD setup.

#include "efd.h"

/***********
 * Globals *
 ***********/

extern efd_function_declaration const EFD_FUNCTION_REGISTRY[];
extern efd_generator_declaration const EFD_GENERATOR_REGISTRY[];
extern efd_object_format const EFD_FORMAT_REGISTRY[];

extern size_t EFD_FUNCTION_REGISTRY_SIZE;
extern size_t EFD_GENERATOR_REGISTRY_SIZE;
extern size_t EFD_FORMAT_REGISTRY_SIZE;

extern dictionary *EFD_FUNCTION_DICT;
extern dictionary *EFD_GENERATOR_DICT;
extern dictionary *EFD_FORMAT_DICT;

/*************************
 * Included Declarations *
 *************************/

#define EFD_REGISTER_DECLARATIONS
#include "func.list"
#include "gen.list"
#include "conv.list"
#undef EFD_REGISTER_DECLARATIONS

/*************
 * Functions *
 *************/

// Gets the data system set up.
void setup_elf_forest_data(int track_error_contexts);

// Cleans up the data system.
void cleanup_elf_forest_data(void);

// Note that the lookup functions declared earlier in efd.h are defined in
// efd_setup.c

// Loads the common Elf Forest Data from res/data, recursively looking for
// *.efd files and loading each under EFD_ROOT, before unpacking EFD_ROOT.
void load_common_efd(void);

// TODO: This function!
void save_common_efd(void);

#endif // INCLUDE_EFD_SETUP_H
