#ifndef ECOLOGY_H
#define ECOLOGY_H

// ecology.h
// Generation of ecology.

#include "world/world_map.h"
#include "world/species.h"

/*************
 * Constants *
 *************/

#define ECO_MAX_SPECIES_PER_BIOME 500

/*************
 * Functions *
 *************/

// Generates ecological information for the given world map.
void generate_ecology(world_map *wm);

#endif // ifndef ECOLOGY_H
