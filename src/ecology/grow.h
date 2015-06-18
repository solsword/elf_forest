#ifndef GROW_H
#define GROW_H

// grow.h
// Plant growth.

#include <stdint.h>

#include "world/blocks.h"
#include "world/world_map.h"

#include "util.h"

/*************
 * Constants *
 *************/

/*************
 * Functions *
 *************/

// Grows plants in the given chunk for the given number of cycles. It returns 1
// on success, or 0 if the chunk's full neighborhood isn't available.
// Neighboring chunks will be edited directly when plants grow into them, but
// growth will stop at the border (writing an appropriate number of pending
// growth cycles in the new/updated blocks).
ptrdiff_t grow_plants(chunk *c, size_t cycles);

#endif // ifndef GROW_H
