#ifndef TEST_H
#define TEST_H

// test.h
// Testing functions.

#include "world/world.h"

/*************
 * Constants *
 *************/

extern region_chunk_pos const CHUNK_ORIGIN;

/********
 * Main *
 ********/

int main(int argc, char** argv);

/*************
 * Functions *
 *************/

// Spawns a player in the given area: 
void test_spawn_player(active_entity_area *area);

#endif // ifndef TEST_H
