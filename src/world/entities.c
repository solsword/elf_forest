// entities.c
// Entity definitions.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "world.h"
#include "static_entities.h"
#include "entities.h"

#include "datatypes/list.h"
#include "graphics/gfx.h"
#include "physics/physics.h"
#include "data/data.h"

/***********
 * Globals *
 ***********/

list *ENTITY_PROTOTYPES = NULL;

// TODO: How big should this be?
size_t const ACTIVE_AREA_SIZE = CHUNK_SIZE * 16;
active_entity_area *ACTIVE_AREA = NULL;

/*********************
 * Private Variables *
 *********************/

// Warp counts:
int WARP_X, WARP_Y, WARP_Z;

/*********************
 * Private Functions *
 *********************/

// Function for scanning a list of entities and finding the first of the given
// type. Returns 1 if the type of the entity pointer passed matches the given
// reference, and 0 otherwise.
int scan_type(void *thing, void *ref) {
  entity *e = (entity *) thing;
  return !strcmp(e->type, (char const *) ref);
}

// Function for handling entities that fall out of an active area by destroying
// them.
void handle_out_of_bounds_entity(active_entity_area *area, entity *e) {
  // TODO: Something else here?
  cleanup_entity(e);
}

// warp_space iteration function:
static inline void warp_entity(void *thing) {
  entity *e = (entity *) thing;
  e->pos.x -= WARP_X * CHUNK_SIZE;
  e->pos.y -= WARP_Y * CHUNK_SIZE;
  e->pos.z -= WARP_Z * CHUNK_SIZE;
  //printf("entity-warp: %0.2f, %0.2f, %0.2f\n", e->pos.x, e->pos.y, e->pos.z);
  oct_remove(e->area->tree, (void *) e);
  compute_bb(e);
  //printf("entity-warp minz: %0.2f\n", e->box.min.z);
  if (!oct_insert(e->area->tree, (void *) e, &(e->box))) {
    handle_out_of_bounds_entity(e->area, e);
  }
}

/******************************
 * Constructors & Destructors *
 ******************************/

void setup_entities(global_pos *origin) {
  ENTITY_PROTOTYPES = create_list();
  ACTIVE_AREA = create_active_entity_area(ACTIVE_AREA_SIZE);
  // TODO: management of multiple active entity areas?
  int i;
  entity *e;
  for (i = 0; i < NUM_STATIC_ENTITY_TYPES; ++i) {
    e = &(STATIC_ENTITY_TYPES[i]);
    add_entity_type(e);
  }
}

void cleanup_entities(void) {
  cleanup_active_entity_area(ACTIVE_AREA);
  destroy_list(ENTITY_PROTOTYPES);
}

// Allocates and returns a new raw entity:
entity * create_entity(void) {
  entity *e = (entity *) malloc(sizeof(entity));
  if (e == NULL) {
    perror("Not enough memory to spawn entity.");
    exit(errno);
  }
  e->move_flags = 0;
  e->model = NULL;
  e->area = NULL;
  e->last_flap = 0;
  return e;
}

void cleanup_entity(entity *e) {
  if (e->area != NULL) {
    l_remove_element(e->area->list, (void *) e);
    oct_remove(e->area->tree, (void *) e);
  }
  free(e);
}

active_entity_area * create_active_entity_area(size_t size) {
  active_entity_area *area = (active_entity_area *) malloc(
    sizeof(active_entity_area)
  );
  if (area == NULL) {
    perror("Not enough memory to create active entity area.");
    exit(errno);
  }
  area->origin.x = 0;
  area->origin.y = 0;
  area->origin.z = 0;
  area->size = size;
  area->list = create_list();
  area->tree = create_octree(size);
  return area;
}

void cleanup_active_entity_area(active_entity_area *area) {
  cleanup_octree(area->tree);
  destroy_list(area->list);
  free(area);
}

/*************
 * Functions *
 *************/

void add_entity_type(entity *e) {
  entity *e_copy = create_entity();
  copy_entity_data(e, e_copy);
  l_append_element(ENTITY_PROTOTYPES, (void *)e_copy);
}

void tick_active_entities() {
  l_foreach(ACTIVE_AREA->list, &tick_entity);
}

void tick_entity(void *thing) {
  entity *e = (entity *) thing;
  tick_physics(e);
}

cell * head_cell(entity *e) {
  chunk_or_approx coa;
  global_pos glpos;
  global_chunk_pos glcpos;
  block_index idx;

  get_head_glpos(e, &glpos);
  glpos__glcpos(&glpos, &glcpos);
  glpos__cidx(&glpos, &idx);

  get_best_data(&glcpos, &coa);
  if (coa.type == CA_TYPE_CHUNK) {
    return c_cell((chunk *) (coa.ptr), idx);
  } else if (coa.type == CA_TYPE_APPROXIMATION) {
    return ca_cell((chunk_approximation *) (coa.ptr), idx);
  } else {
    return NULL;
  }
}

void warp_space(active_entity_area *area, vector *pos) {
  // TODO: Handle loading and unloading entities
  // Warp offset coordiantes:
  WARP_X = fastfloor(pos->x / CHUNK_SIZE);
  WARP_Y = fastfloor(pos->y / CHUNK_SIZE);
  WARP_Z = fastfloor(pos->z / CHUNK_SIZE);
  if (WARP_X || WARP_Y || WARP_Z) {
    // DEBUG:
    /*
    printf("pre-warp pos: %0.2f, %0.2f, %0.2f\n", pos->x, pos->y, pos->z);
    printf("pre-warp: %d, %d, %d\n", WARP_X, WARP_Y, WARP_Z);
    // */
    area->origin.x += WARP_X * CHUNK_SIZE;
    area->origin.y += WARP_Y * CHUNK_SIZE;
    area->origin.z += WARP_Z * CHUNK_SIZE;
    // Warp entities if we changed offsets:
    l_foreach(area->list, &warp_entity);
    // DEBUG:
    /*
    WARP_X = fastfloor(pos->x / CHUNK_SIZE);
    WARP_Y = fastfloor(pos->y / CHUNK_SIZE);
    WARP_Z = fastfloor(pos->z / CHUNK_SIZE);
    printf("post-warp: %d, %d, %d\n", WARP_X, WARP_Y, WARP_Z);
    // */
  }
  WARP_X = 0;
  WARP_Y = 0;
  WARP_Z = 0;
}

entity * spawn_entity(
  char const * const type,
  vector *pos,
  active_entity_area *area
) {
  entity *e = create_entity();
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
  // Add the entity as an active entity:
  add_active_entity(area, e);
  // Return the entity:
  return e;
}

entity * find_by_type(char const * const type, list *l) {
  return (entity *) l_scan_elements(l, (void *) type, &scan_type);
}
