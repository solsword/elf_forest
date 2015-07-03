#ifndef BIOLOGY_H
#define BIOLOGY_H

// biology.h
// General biology generation.

#include "world/world_map.h"
#include "world/species.h"

/*************
 * Constants *
 *************/

/*************
 * Functions *
 *************/

// Adds biology to the given chunk as part of chunk initialization. Should be
// called after the chunk's base cell contents (rocks, soil, air, water) have
// been added (see generate_chunk in worldgen). If data for the chunk's
// neighbors isn't available in detail, or if the given chunk's CF_HAS_BIOLOGY
// flag is already set, it will fail and return immediately. If it succeeds, it
// will set the chunk's CF_HAS_BIOLOGY flag.
void add_biology(chunk *c);

#endif // ifndef BIOLOGY_H
