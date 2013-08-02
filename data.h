#ifndef DATA_H
#define DATA_H

// data.h
// Loading from and saving to disk.

#include <stdint.h>

#include "blocks.h"
#include "list.h"
#include "world.h"

/**************
 * Structures *
 **************/

// Holds 7 chunk pointers: a main chunk and its neighbors.
struct chunk_neighborhood_s;
typedef struct chunk_neighborhood_s chunk_neighborhood;

/*************
 * Constants *
 *************/

// Max chunks to load per tick:
extern const int LOAD_CAP;

/***********
 * Globals *
 ***********/

// Chunks that need to be reloaded/recompiled:
extern list *CHUNKS_TO_RELOAD;
extern list *CHUNKS_TO_RECOMPILE;

/*************************
 * Structure Definitions *
 *************************/

struct chunk_neighborhood_s {
  chunk *c, *above, *below, *north, *south, *east, *west;
};

/*************
 * Functions *
 *************/

// Allocates and returns a chunk neighborhood object centered at the given
// position.
chunk_neighborhood * get_neighborhood(frame *f, frame_chunk_index fcidx);

// Sets up the data subsytem.
void setup_data(void);

// Cleans up the data subsytem.
void cleanup_data(void);

// Marks the given chunk for reloading.
void mark_for_reload(frame *f, frame_chunk_index fcidx);

// Marks the given chunk for recompilation.
void mark_for_recompile(chunk *c);

// Ticks the chunk data system, loading/recompiling as many chunks as allowed.
void tick_data(void);

// Computes block exposure for the given chunk. Might add exposure information
// to neighboring chunks, in which case it will mark them for recompilation.
void compute_exposure(chunk_neighborhood *cnb);

// Loads the given chunk. Uses the chunk's x/y/z coordinates to determine what
// contents it should have. Also calls compute_exposure and marks the chunk for
// recompilation.
void load_chunk(chunk_neighborhood *cnb);

#endif // ifndef DATA_H
