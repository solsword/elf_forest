// climate.c
// Hydrology and climate.

#include <stdint.h>

#include "world/world.h"
#include "climate.h"

/******************************
 * Constructors & Destructors *
 ******************************/

body_of_water* create_body_of_water(float level, salinity salt) {
  body_of_water *result = (body_of_water*) malloc(sizeof(body_of_water));
  result->level = level;
  result->salt = salt;
  return result;
}

void cleanup_body_of_water(body_of_water *body) {
  free(body);
}

/*************
 * Functions *
 *************/
