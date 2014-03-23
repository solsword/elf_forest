#ifndef TEST_H
#define TEST_H

// test.h
// Testing functions.

#include "world/world.h"

/********
 * Main *
 ********/

int main(int argc, char** argv);

/*************
 * Functions *
 *************/

// Puts some sparse junk data into the main frame for testing.
void test_setup_world_junk(frame *f);

// Uses noise to create a test frame.
void test_setup_world_terrain(frame *f);

// Computes exposure for and compiles every chunk in the given frame.
void test_compile_frame(frame *f);

// Spawns a player in the given frame: 
void test_spawn_player(frame *f);

#endif // ifndef TEST_H
