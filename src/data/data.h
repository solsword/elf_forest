#ifndef DATA_H
#define DATA_H

// data.h
// Loading from and saving to disk.

#include <stdint.h>

#include "datatypes/queue.h"
#include "world/blocks.h"
#include "world/world.h"
#include "world/exposure.h"

/*************
 * Constants *
 *************/

// Max chunks to load per tick:
extern const int LOAD_CAP;

/***********
 * Globals *
 ***********/

// Chunks that need to be reloaded/recompiled:
extern queue *CHUNKS_TO_RELOAD;
extern queue *CHUNKS_TO_RECOMPILE;

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
void mark_for_recompile(chunk *c);

// Ticks the chunk data system, loading/recompiling as many chunks as allowed.
void tick_data(void);

// Loads the given chunk. Uses the chunk's x/y/z coordinates to determine what
// contents it should have. Also calls compute_exposure and marks the chunk for
// recompilation.
void load_chunk(chunk_neighborhood *cnb);

#endif // ifndef DATA_H
