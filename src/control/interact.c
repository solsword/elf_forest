// interact.c
// Player interaction with the environment.

#include "interact.h"

#include "ctl.h"

#include "world/world.h"

/********************
 * Global variables *
 ********************/

cell_cursor PLAYER_CURSOR;

/*************
 * Functions *
 *************/

float set_player_cursor(
  void* data,
  global_pos* pos,
  vector origin,
  vector heading,
  float length
) {
  cell_cursor* cursor = (cell_cursor*) data;
  refresh_cell_at_cache();
  cell* cell = cell_at(pos);
  cursor->valid = 0;
  if (cell == NULL) {
    return 0;
  }
  if (b_is_solid(cell->blocks[0]) || b_is_solid(cell->blocks[1])) {
    cursor->valid = 1;
    copy_glpos(pos, &(cursor->pos));
    return 0;
  }
  return CHUNK_SIZE;
}

void tick_interaction(void) {
  vector head_pos;
  vector look_direction;

  get_head_vec(PLAYER, &head_pos);
  vface(&look_direction, PLAYER->yaw, PLAYER->pitch);

  iter_ray(
    &(PLAYER->area->origin),
    head_pos,
    look_direction,
    (void*) &PLAYER_CURSOR,
    &set_player_cursor
  );
}
