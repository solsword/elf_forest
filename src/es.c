// es.c
// Standalone executable for running ElfScript files.

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "es.h"

#include "datatypes/string.h"

#include "elfscript/elfscript.h"
#include "elfscript/elfscript_setup.h"

#include "util.h"

/*************
 * Constants *
 *************/

ptrdiff_t SEED = 6457802;

/********
 * Main *
 ********/

int main(int argc, char** argv) {
  // Iterate the prng a few times:
  SEED = prng(prng(prng(prng(SEED))));

  // Set up the strings system:
  init_strings();

  fprintf("%d %s\n", argc, argv[0]);
  // TODO: Proper option parsing include e.g., -h/--help, -v/--version, etc.
  if (argc == 0) { // read from stdin
    FILE* fin = stdin;
  } elif (argc == 1) { // load indicated file
    FILE* fin = fopen(argv[1]);
  }

  // This should never be reached.
  return 0;
}
