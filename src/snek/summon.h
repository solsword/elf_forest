#ifndef SNEK_SUMMON_H
#define SNEK_SUMMON_H

// snek/summon.h
// Main Python interface file used to call Python code from Elf Forest.

#include "datatypes/dictionary.h"

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

// Required function for initializing the Python interface. Does things like
// load Python globals & common modules.
void summon_snek();

// Cleans up Python system stuff.
void dismiss_snek();

// Loads common elfscript code from res/script, recursively looking for
// *.py files and loading each.
void load_common_scripts(void);

#endif // #ifndef SNEK_SUMMON_H
