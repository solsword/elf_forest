#ifndef INTERACT_H
#define INTERACT_H

// interact.h
// Player interaction with the environment.

#include "world/world.h"

/**************
 * Structures *
 **************/

struct cell_cursor_s;
typedef struct cell_cursor_s cell_cursor;

/*************************
 * Structure Definitions *
 *************************/

struct cell_cursor_s {
  int valid;
  global_pos pos;
};

/********************
 * Global variables *
 ********************/

extern cell_cursor PLAYER_CURSOR;

/*************
 * Functions *
 *************/

// A ray iteration function for setting the PLAYER_CELL_CURSOR global.
float set_player_cursor(
  void* data,
  global_pos* pos,
  vector origin,
  vector heading,
  float length
);

// Ticks player/world interaction
void tick_interaction(void);

#endif // #ifndef INTERACT_H
