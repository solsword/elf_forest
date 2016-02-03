#ifndef INCLUDE_EFD_SETUP_H
#define INCLUDE_EFD_SETUP_H

// efd_setup.h
// Gathers Elf Forest Data conversion registrations and defines EFD setup.

#include "efd.h"

/**************
 * Structures *
 **************/

struct efd_int_test_s;
typedef struct efd_int_test_s efd_int_test;

struct efd_num_test_s;
typedef struct efd_num_test_s efd_num_test;

/*************
 * Constants *
 *************/

extern char * const * const EFD_DATATYPE_NAME_REGISTRY;
extern efd_unpack_function* EFD_DATATYPE_UNPACKER_REGISTRY;
extern efd_pack_function* EFD_DATATYPE_PACKER_REGISTRY;
extern efd_destroy_function* EFD_DATATYPE_DESTRUCTOR_REGISTRY;

/*************************
 * Structure Definitions *
 *************************/

struct efd_int_test_s {
  string* input;
  string* expect;
  ptrdiff_t output;
  string* remainder;
};

struct efd_num_test_s {
  string* input;
  string* expect;
  float output;
  string* remainder;
};

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
