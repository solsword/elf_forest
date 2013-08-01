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

// TODO: HERE

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

// TODO: HERE

/*************
 * Functions *
 *************/

// Sets up the data subsytem.
void setup_data(void);

// Cleans up the data subsytem.
void cleanup_data(void);

// Marks the given chunk for reloading.
void mark_for_reload(chunk *c);

// Marks the given chunk for recompilation.
void mark_for_recompile(chunk *c);

// Ticks the chunk data system, loading/recompiling as many chunks as allowed.
void tick_data(void);

// Computes block exposure for the given chunk.
void compute_exposure(chunk *c);

// Loads the given chunk. Uses the chunk's x/y/z coordinates to determine what
// contents it should have. Also calls compute_exposure and compile_chunk.
void load_chunk(chunk *c);
#endif // ifndef DATA_H
