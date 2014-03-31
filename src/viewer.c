// viewer.c
// A program for viewing a single chunk.
//
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "viewer.h"

#include "graphics/gfx.h"

#include "world/world.h"
#include "world/entities.h"
#include "world/chunk_data.h"

#include "control/ctl.h"
#include "gen/terrain.h"
#include "graphics/tex.h"
#include "graphics/display.h"
#include "graphics/render.h"
#include "physics/physics.h"
#include "data/data.h"
#include "tick/tick.h"
#include "ui/ui.h"


/*************
 * Constants *
 *************/

vector VBOX_MIN = { .x = VB_MIN, .y = VB_MIN, .z = VB_MIN };
vector VBOX_MAX = { .x = VB_MAX, .y = VB_MAX, .z = VB_MAX };

region_chunk_pos const OBSERVED_CHUNK = { .x = 0, .y = 0, .z = 0 };

region_chunk_pos const INTERESTING_CHUNK = { .x = 0, .y = 4500, .z = 0 };

/********
 * Main *
 ********/

int main(int argc, char** argv) {
  region_pos origin = { .x = 0, .y = 0, .z = 0 };

  // Seed the random number generator:
  srand(545438);

  // Prepare the window context:
  prepare_default(&argc, argv);

  // Set up the viewing area:
  printf("Setting up viewing area...\n");

  // Initialize stateless subsystems:
  init_control();
  init_textures();
  init_tick(0);

  // Setup stateful subsystems:
  setup_ui();
  setup_data();
  setup_entities(&origin);

  printf("...done.\n");

  // load a single chunk:
  view_chunk_from_world(&INTERESTING_CHUNK);

  // Spawn the player:
  spawn_viewer();

  // Set the pre-display callback:
  AREA_PRE_RENDER_CALLBACK = &(draw_viewing_area);

  // Start the main loop:
  loop();

  return 0;
}

/*************
 * Functions *
 *************/

void spawn_viewer() {
  vector pos = { .x=-3, .y=-3, .z=4 };
  PLAYER = spawn_entity("viewer", &pos, ACTIVE_AREA);
  // Set up the physics update end-step callback to constrain our position:
  PHYS_SUBSTEP_START_CALLBACK = &(fake_player_floating);
  PHYS_SUBSTEP_END_CALLBACK = &(constrain_player_position);
}

void fake_player_floating(entity *e) {
  if (e == PLAYER) {
    set_in_liquid(e);
  }
}

void constrain_player_position(entity *e) {
  if (e == PLAYER) {
    if (e->area->origin.x + e->pos.x < VBOX_MIN.x) {
      e->pos.x = VBOX_MIN.x - e->area->origin.x;
      e->vel.x = 0;
    }
    if (e->area->origin.y + e->pos.y < VBOX_MIN.y) {
      e->pos.y = VBOX_MIN.y - e->area->origin.y;
      e->vel.y = 0;
    }
    if (e->area->origin.z + e->pos.z < VBOX_MIN.z) {
      e->pos.z = VBOX_MIN.z - e->area->origin.z;
      e->vel.z = 0;
    }

    if (e->area->origin.x + e->pos.x > VBOX_MAX.x) {
      e->pos.x = VBOX_MAX.x - e->area->origin.x;
      e->vel.x = 0;
    }
    if (e->area->origin.y + e->pos.y > VBOX_MAX.y) {
      e->pos.y = VBOX_MAX.y - e->area->origin.y;
      e->vel.y = 0;
    }
    if (e->area->origin.z + e->pos.z > VBOX_MAX.z) {
      e->pos.z = VBOX_MAX.z - e->area->origin.z;
      e->vel.z = 0;
    }
  }
}

void draw_viewing_area(active_entity_area *area) {
  GLfloat top_vs[12],
          bot_vs[12],
          top_center[3],
          bot_center[3],
          north_center[3],
          south_center[3],
          east_center[3],
          west_center[3];
  top_vs[ 0] = VB_MIN - 1 - area->origin.x;
  top_vs[ 1] = VB_MIN - 1 - area->origin.y;
  top_vs[ 2] = VB_MAX + 1 - area->origin.z;

  top_vs[ 3] = VB_MAX + 1 - area->origin.x;
  top_vs[ 4] = VB_MIN - 1 - area->origin.y;
  top_vs[ 5] = VB_MAX + 1 - area->origin.z;

  top_vs[ 6] = VB_MAX + 1 - area->origin.x;
  top_vs[ 7] = VB_MAX + 1 - area->origin.y;
  top_vs[ 8] = VB_MAX + 1 - area->origin.z;

  top_vs[ 9] = VB_MIN - 1 - area->origin.x;
  top_vs[10] = VB_MAX + 1 - area->origin.y;
  top_vs[11] = VB_MAX + 1 - area->origin.z;

  top_vs[ 9] = VB_MIN - 1 - area->origin.x;
  top_vs[10] = VB_MAX + 1 - area->origin.y;
  top_vs[11] = VB_MAX + 1 - area->origin.z;

  top_center[ 0] = VB_CTR - area->origin.x;
  top_center[ 1] = VB_CTR - area->origin.y;
  top_center[ 2] = VB_MAX + 1 - area->origin.z;

  bot_vs[ 0] = VB_MIN - 1 - area->origin.x;
  bot_vs[ 1] = VB_MIN - 1 - area->origin.y;
  bot_vs[ 2] = VB_MIN - 1 - area->origin.z;

  bot_vs[ 3] = VB_MAX + 1 - area->origin.x;
  bot_vs[ 4] = VB_MIN - 1 - area->origin.y;
  bot_vs[ 5] = VB_MIN - 1 - area->origin.z;

  bot_vs[ 6] = VB_MAX - area->origin.x;
  bot_vs[ 7] = VB_MAX + 1 - area->origin.y;
  bot_vs[ 8] = VB_MIN - 1 - area->origin.z;

  bot_vs[ 9] = VB_MIN - 1 - area->origin.x;
  bot_vs[10] = VB_MAX + 1 - area->origin.y;
  bot_vs[11] = VB_MIN - 1 - area->origin.z;

  bot_vs[ 9] = VB_MIN - 1 - area->origin.x;
  bot_vs[10] = VB_MAX + 1 - area->origin.y;
  bot_vs[11] = VB_MIN - 1 - area->origin.z;

  bot_center[ 0] = VB_CTR - area->origin.x;
  bot_center[ 1] = VB_CTR - area->origin.y;
  bot_center[ 2] = VB_MIN - 1 - area->origin.z;

  north_center[ 0] = VB_CTR - area->origin.x;
  north_center[ 1] = VB_MAX + 1 - area->origin.y;
  north_center[ 2] = VB_CTR - area->origin.z;

  south_center[ 0] = VB_CTR - area->origin.x;
  south_center[ 1] = VB_MIN - 1 - area->origin.y;
  south_center[ 2] = VB_CTR - area->origin.z;

  east_center[ 0] = VB_MAX + 1 - area->origin.x;
  east_center[ 1] = VB_CTR - area->origin.y;
  east_center[ 2] = VB_CTR - area->origin.z;

  west_center[ 0] = VB_MIN - 1 - area->origin.x;
  west_center[ 1] = VB_CTR - area->origin.y;
  west_center[ 2] = VB_CTR - area->origin.z;

  glPushMatrix();

  glDisable( GL_CULL_FACE );

  glBegin( GL_TRIANGLES );

  // The bottom:
  set_color(DARK_SHADOW);
  glVertex3fv(&(bot_vs[0])); glVertex3fv(&(bot_vs[3]));
  set_color(LIGHT_SHADOW); glVertex3fv(bot_center);

  set_color(DARK_SHADOW);
  glVertex3fv(&(bot_vs[3])); glVertex3fv(&(bot_vs[6]));
  set_color(LIGHT_SHADOW); glVertex3fv(bot_center);

  set_color(DARK_SHADOW);
  glVertex3fv(&(bot_vs[6])); glVertex3fv(&(bot_vs[9]));
  set_color(LIGHT_SHADOW); glVertex3fv(bot_center);

  set_color(DARK_SHADOW);
  glVertex3fv(&(bot_vs[9])); glVertex3fv(&(bot_vs[0]));
  set_color(LIGHT_SHADOW); glVertex3fv(bot_center);

  // The top:
  set_color(DARK_SHADOW);
  glVertex3fv(&(top_vs[0])); glVertex3fv(&(top_vs[3]));
  set_color(LIGHT_SHADOW); glVertex3fv(top_center);

  set_color(DARK_SHADOW);
  glVertex3fv(&(top_vs[3])); glVertex3fv(&(top_vs[6]));
  set_color(LIGHT_SHADOW); glVertex3fv(top_center);

  set_color(DARK_SHADOW);
  glVertex3fv(&(top_vs[6])); glVertex3fv(&(top_vs[9]));
  set_color(LIGHT_SHADOW); glVertex3fv(top_center);

  set_color(DARK_SHADOW);
  glVertex3fv(&(top_vs[9])); glVertex3fv(&(top_vs[0]));
  set_color(LIGHT_SHADOW); glVertex3fv(top_center);

  // The north side:
  set_color(DARK_SHADOW);
  glVertex3fv(&(bot_vs[9])); glVertex3fv(&(bot_vs[6]));
  set_color(LIGHT_SHADOW); glVertex3fv(north_center);

  set_color(DARK_SHADOW);
  glVertex3fv(&(bot_vs[6])); glVertex3fv(&(top_vs[6]));
  set_color(LIGHT_SHADOW); glVertex3fv(north_center);

  set_color(DARK_SHADOW);
  glVertex3fv(&(top_vs[6])); glVertex3fv(&(top_vs[9]));
  set_color(LIGHT_SHADOW); glVertex3fv(north_center);

  set_color(DARK_SHADOW);
  glVertex3fv(&(top_vs[9])); glVertex3fv(&(bot_vs[9]));
  set_color(LIGHT_SHADOW); glVertex3fv(north_center);

  // The south side:
  set_color(DARK_SHADOW);
  glVertex3fv(&(bot_vs[0])); glVertex3fv(&(bot_vs[3]));
  set_color(LIGHT_SHADOW); glVertex3fv(south_center);

  set_color(DARK_SHADOW);
  glVertex3fv(&(bot_vs[3])); glVertex3fv(&(top_vs[3]));
  set_color(LIGHT_SHADOW); glVertex3fv(south_center);

  set_color(DARK_SHADOW);
  glVertex3fv(&(top_vs[3])); glVertex3fv(&(top_vs[0]));
  set_color(LIGHT_SHADOW); glVertex3fv(south_center);

  set_color(DARK_SHADOW);
  glVertex3fv(&(top_vs[0])); glVertex3fv(&(bot_vs[0]));
  set_color(LIGHT_SHADOW); glVertex3fv(south_center);

  // The west side:
  set_color(DARK_SHADOW);
  glVertex3fv(&(bot_vs[0])); glVertex3fv(&(bot_vs[9]));
  set_color(LIGHT_SHADOW); glVertex3fv(west_center);

  set_color(DARK_SHADOW);
  glVertex3fv(&(bot_vs[9])); glVertex3fv(&(top_vs[9]));
  set_color(LIGHT_SHADOW); glVertex3fv(west_center);

  set_color(DARK_SHADOW);
  glVertex3fv(&(top_vs[9])); glVertex3fv(&(top_vs[0]));
  set_color(LIGHT_SHADOW); glVertex3fv(west_center);

  set_color(DARK_SHADOW);
  glVertex3fv(&(top_vs[0])); glVertex3fv(&(bot_vs[0]));
  set_color(LIGHT_SHADOW); glVertex3fv(west_center);

  // The east side:
  set_color(DARK_SHADOW);
  glVertex3fv(&(bot_vs[3])); glVertex3fv(&(bot_vs[6]));
  set_color(LIGHT_SHADOW); glVertex3fv(east_center);

  set_color(DARK_SHADOW);
  glVertex3fv(&(bot_vs[6])); glVertex3fv(&(top_vs[6]));
  set_color(LIGHT_SHADOW); glVertex3fv(east_center);

  set_color(DARK_SHADOW);
  glVertex3fv(&(top_vs[6])); glVertex3fv(&(top_vs[3]));
  set_color(LIGHT_SHADOW); glVertex3fv(east_center);

  set_color(DARK_SHADOW);
  glVertex3fv(&(top_vs[3])); glVertex3fv(&(bot_vs[3]));
  set_color(LIGHT_SHADOW); glVertex3fv(east_center);

  glEnd();

  glEnable( GL_CULL_FACE );

  glPopMatrix();
}

void view_chunk_from_world(region_chunk_pos const * const rcpos) {
  chunk *c = create_chunk(rcpos);
  chunk_or_approx coa;
  coa.type = CA_TYPE_CHUNK;
  coa.ptr = (void *) c;

  // Set up initial flags:
  c->chunk_flags &= ~CF_COMPILE_ON_LOAD;
  c->chunk_flags &= ~CF_LOADED;
  c->chunk_flags &= ~CF_COMPILED;

  // Load the chunk:
  load_chunk(c);

  // Compile the chunk:
  compute_exposure(&coa);
  compile_chunk_or_approx(&coa);

  // Overwrite the chunk's position information and stick it in the chunk cache
  // at the correct position:
  c->rcpos.x = OBSERVED_CHUNK.x;
  c->rcpos.y = OBSERVED_CHUNK.y;
  c->rcpos.z = OBSERVED_CHUNK.z;
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
  m3_put_value(
    CHUNK_CACHE->levels[LOD_BASE],
    (void *) c,
    (map_key_t) c->rcpos.x,
    (map_key_t) c->rcpos.y,
    (map_key_t) c->rcpos.z
  );
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
}
