#ifndef VIEWER_H
#define VIEWER_H

// viewer.h
// A program for viewing a single chunk.

#include "datatypes/vector.h"
#include "world/world.h"
#include "world/entities.h"

/*************
 * Constants *
 *************/

#define VB_MIN (-CHUNK_SIZE)
#define VB_MAX (2*CHUNK_SIZE)
#define VB_CTR ((VB_MIN + VB_MAX)/2)

// The corners of the viewing area:
extern vector VBOX_MIN;
extern vector VBOX_MAX;

// The fake chunk position of our observed chunk:
extern region_chunk_pos const OBSERVED_CHUNK;

// An interesting chunk:
extern region_chunk_pos const INTERESTING_CHUNK;

/********
 * Main *
 ********/

int main(int argc, char** argv);

/*************
 * Functions *
 *************/

// Spawns a player in the given area: 
void spawn_viewer(void);

// Fakes the move flags so that the player is always considered to be in a
// liquid.
void fake_player_floating(entity *e);

// Constrains the position of the given entity to the viewing area:
void constrain_player_position(entity *e);

// Loads a chunk from the world into the viewing area:
void view_chunk_from_world(region_chunk_pos const * const rcpos);

// Loads an empty chunk into the viewing area.
void view_empty_chunk();

// Returns a pointer to the chunk that the viewer is viewing.
chunk *get_observed_chunk();

// Sets the center block of the loaded chunk to the given block.
void set_center_block(block b);

// Draws the bounding box of the viewing area:
void draw_viewing_area(active_entity_area *ignore);

#endif // ifndef VIEWER_H
