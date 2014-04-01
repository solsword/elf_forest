// txgen.c
// Texture generation.

#include <stdint.h>

#include "noise/noise.h"
#include "graphics/tex.h"

#include "txgen.h"

/*************
 * Constants *
 *************/

extern pixel const GRAMMAR_SLOT[N_COLOR_SLOTS] = {
  0xfe0000ff,
  0xfeff00ff,
  0x0000feff,
  0xfe8080ff,
  0xfeff80ff,
  0x8080feff
};

/********************
 * Inline Functions *
 ********************/

/******************************
 * Constructors & Destructors *
 ******************************/

/*************
 * Functions *
 *************/
