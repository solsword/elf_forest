#ifndef DATA_H
#define DATA_H

// data.h
// Loading from and saving to disk.

#include <stdint.h>

#include "datatypes/queue.h"
//#include "datatypes/map.h"
#include "world/blocks.h"
#include "world/world.h"
#include "world/chunk_data.h"

/**************
 * Structures *
 **************/

// An entity is a non-voxel object that can move around freely in the world.
struct chunk_cache_s;
typedef struct chunk_cache_s chunk_cache;

/*************
 * Constants *
 *************/

// Max chunks to load per tick:
extern int const LOAD_CAP;

/***********
 * Globals *
 ***********/

// Chunks that need to be reloaded/recompiled:
extern queue *CHUNKS_TO_RELOAD;
extern queue *CHUNKS_TO_RECOMPILE;

/*************************
 * Structure Definitions *
 *************************/

struct chunk_cache_s {
  list *chunks;
};

/*************
 * Functions *
 *************/

// Sets up the data subsytem.
void setup_data(void);

// Cleans up the data subsytem.
void cleanup_data(void);

// Marks the given chunk for reloading.
void mark_for_reload(frame *f, frame_chunk_index fcidx);

// Marks the given chunk for recompilation.
void mark_for_recompile(frame *f, frame_chunk_index fcidx);

// Ticks the chunk data system, loading/recompiling as many chunks as allowed.
void tick_data(void);

// Loads the given chunk. Uses the chunk's x/y/z coordinates to determine what
// contents it should have. Also calls compute_exposure and marks the chunk for
// recompilation.
void load_chunk(chunk_neighborhood *cnb);

#endif // ifndef DATA_H
