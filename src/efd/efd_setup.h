#ifndef INCLUDE_EFD_SETUP_H
#define INCLUDE_EFD_SETUP_H

// efd_setup.h
// Gathers Elf Forest Data conversion registrations and defines EFD setup.

#include "efd.h"

/*************
 * Constants *
 *************/

extern char * const * const EFD_DATATYPE_NAME_REGISTRY;
extern efd_unpack_function* EFD_DATATYPE_UNPACKER_REGISTRY;
extern efd_pack_function* EFD_DATATYPE_PACKER_REGISTRY;
extern efd_destroy_function* EFD_DATATYPE_DESTRUCTOR_REGISTRY;

/********************
 * Inline Functions *
 ********************/

/*************
 * Functions *
 *************/

// Gets the data system set up.
void init_elf_forest_data(void);

// Note that the lookup functions are declared earlier, in efd.h

#endif // INCLUDE_EFD_SETUP_H
