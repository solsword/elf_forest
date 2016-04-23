#ifndef INCLUDE_EFD_SETUP_H
#define INCLUDE_EFD_SETUP_H

// efd_setup.h
// Gathers Elf Forest Data conversion registrations and defines EFD setup.

#include "efd.h"

/*************
 * Constants *
 *************/

extern char * const EFD_OBJECT_NAME_REGISTRY[];
extern efd_unpack_function EFD_OBJECT_UNPACKER_REGISTRY[];
extern efd_pack_function EFD_OBJECT_PACKER_REGISTRY[];
extern efd_copy_function EFD_OBJECT_COPIER_REGISTRY[];
extern efd_destroy_function EFD_OBJECT_DESTRUCTOR_REGISTRY[];

/*************************
 * Included Declarations *
 *************************/

#define EFD_REGISTER_DECLARATIONS
#include "conv/efd_null.h"
#include "conv/efd_tests.h"
#include "conv/efd_rngtable.h"
#undef EFD_REGISTER_DECLARATIONS

/*************
 * Functions *
 *************/

// Gets the data system set up.
void setup_elf_forest_data(void);

// Cleans up the data system.
void cleanup_elf_forest_data(void);

// Note that the lookup functions declared earlier in efd.h are defined in
// efd_setup.c

#endif // INCLUDE_EFD_SETUP_H
