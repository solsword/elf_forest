#ifndef TXGEN_H
#define TXGEN_H

// txgen.h
// Texture generation.

#include <stdint.h>

#include "graphics/tex.h"

/************************
 * Types and Structures *
 ************************/

/*************
 * Constants *
 *************/

// A table of special pixel values used for grammar expansion:
#define N_COLOR_SLOTS 6
extern pixel const GRAMMAR_SLOT[N_COLOR_SLOTS];

/*************************
 * Structure Definitions *
 *************************/

/********************
 * Inline Functions *
 ********************/

/******************************
 * Constructors & Destructors *
 ******************************/

/*************
 * Functions *
 *************/

#endif // ifndef TXGEN_H
