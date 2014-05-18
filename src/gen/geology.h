#ifndef GEOLOGY_H
#define GEOLOGY_H

// geology.h
// Stone types and strata generation.

#include <stdint.h>

/************************
 * Types and Structures *
 ************************/

// A layer of stone that encompasses some region of the world.
struct stratum_s;
typedef struct stratum_s stratum;

/*************
 * Constants *
 *************/

// Maximum number of stone layers per world region
#define MAX_STRATA_LAYERS 32

/*************************
 * Structure Definitions *
 *************************/

struct stratum_s {
  size_t width, height;
  world_region *regions;
};

/********************
 * Inline Functions *
 ********************/


/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates and returns a new world map of the given size.
world_map *create_world_map(size_t width, size_t height);

/*************
 * Functions *
 *************/

#endif // ifndef GEOLOGY_H
