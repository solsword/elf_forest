// entities.c
// Entity definitions.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "list.h"
#include "gfx.h"
#include "world.h"
#include "physics.h"
#include "static_entities.h"
#include "entities.h"
#include "data.h"

/***********
 * Globals *
 ***********/

list *ENTITY_PROTOTYPES;

/*********************
 * Private Variables *
 *********************/

// Warp counts:
int WARP_X, WARP_Y, WARP_Z;

/*********************
 * Private Functions *
 *********************/

// Function for searching a list of entities and finding the first of the given
// type. Returns 1 if the entity pointer passed has type SEARCH_TYPE, and 0
// otherwise.
const char *SEARCH_TYPE = NULL;
int scan_type(void *thing) {
  entity *e = (entity *) thing;
  return !strcmp(e->type, SEARCH_TYPE);
}

// warp_space iteration function:
static inline void warp_entity(void *thing) {
  entity *e = (entity *) thing;
  e->pos.x -= WARP_X * CHUNK_SIZE;
  e->pos.y -= WARP_Y * CHUNK_SIZE;
  e->pos.z -= WARP_Z * CHUNK_SIZE;
  compute_bb(e);
}

/*************
 * Functions *
 *************/

// Loads the data for the given entity type, populating the fields of the given
// entity object. Make sure to call setup_entities() first.
void add_entity_type(entity *e) {
  append_element(ENTITY_PROTOTYPES, (void *)e);
}

void setup_entities(void) {
  ENTITY_PROTOTYPES = create_list();
  int i;
  entity *e;
  for (i = 0; i < NUM_STATIC_ENTITY_TYPES; ++i) {
    e = &(STATIC_ENTITY_TYPES[i]);
    add_entity_type(e);
  }
}

void cleanup_entities(void) {
  // TODO: Use destroy on one list and cleanup on another?
  cleanup_list(ENTITY_PROTOTYPES);
}

void tick_entities(frame *f) {
  foreach(f->entities, &tick_entity);
}

void tick_entity(void *thing) {
  entity *e = (entity *) thing;
  tick_physics(e);
}

void warp_space(frame *f, entity *e) {
  frame_chunk_index fcidx;
  region_chunk_pos rcpos;
  chunk *c;
  // TODO: Handle loading and unloading entities
  // Warp offset coordiantes:
  WARP_X = fastfloor(e->pos.x / CHUNK_SIZE);
  WARP_Y = fastfloor(e->pos.y / CHUNK_SIZE);
  WARP_Z = fastfloor(e->pos.z / CHUNK_SIZE);
  f->chunk_offset.x = (f->chunk_offset.x + WARP_X) % FRAME_SIZE;
  f->chunk_offset.y = (f->chunk_offset.y + WARP_Y) % FRAME_SIZE;
  f->chunk_offset.z = (f->chunk_offset.z + WARP_Z) % FRAME_SIZE;
  f->region_offset.x += WARP_X;
  f->region_offset.y += WARP_Y;
  f->region_offset.z += WARP_Z;
  if (WARP_X || WARP_Y || WARP_Z) {
    // Warp entities if we changed offsets:
    foreach(f->entities, &warp_entity);
    // Also mark dirty chunks as necessary:
    for (fcidx.x = 0; fcidx.x < FRAME_SIZE; ++fcidx.x) {
      for (fcidx.y = 0; fcidx.y < FRAME_SIZE; ++fcidx.y) {
        for (fcidx.z = 0; fcidx.z < FRAME_SIZE; ++fcidx.z) {
          // Get the new intended region chunk position:
          fcidx__rcpos(&fcidx, f, &rcpos);
          c = chunk_at(f, fcidx);
          if (
            c->rpos.x != rcpos.x
          ||
            c->rpos.y != rcpos.y
          ||
            c->rpos.z != rcpos.z
          ) {
            c->rpos.x = rcpos.x;
            c->rpos.y = rcpos.y;
            c->rpos.z = rcpos.z;
            mark_for_reload(c);
          }
        }
      }
    }
  }
}

entity * spawn_entity(const char *type, vector *pos, frame *f) {
  entity *e = (entity *) malloc(sizeof(entity));
  if (e == NULL) {
    perror("Failed to spawn entity.");
    fail(errno);
  }
  // Find the prototype that matches the given name:
  entity *prototype = find_by_type(type, ENTITY_PROTOTYPES);
  if (prototype == NULL) {
    fprintf(stderr, "Failed to spawn entity: invalid type '%s'.\n", type);
    free(e);
    return NULL;
  }
  // Copy its data into our new entity:
  copy_entity_data(prototype, e);
  // Fill in the given position:
  e->pos.x = pos->x;
  e->pos.y = pos->y;
  e->pos.z = pos->z;
  // Set yaw and pitch to 0:
  e->yaw = 0;
  e->pitch = 0;
  // Zero out the velocity and impulse fields:
  clear_kinetics(e);
  // Compute the bounding box:
  compute_bb(e);
  // Put the object into the given frame:
  add_entity(f, e);
  // Return the entity:
  return e;
}

entity * find_by_type(const char *type, list *l) {
  SEARCH_TYPE = type;
  return (entity *) find_element(l, &scan_type);
}
