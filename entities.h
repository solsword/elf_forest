#ifndef ENTITIES_H
#define ENTITIES_H

// entities.h
// Entity definitions.

#include <stdint.h>
#include <string.h>

#include <GL/gl.h>

#include "vbo.h"
#include "list.h"
#include "vector.h"
#include "bbox.h"
#include "world.h"

/**************
 * Structures *
 **************/

// An entity is a non-voxel object that can move around freely in the world.
struct entity_s;
typedef struct entity_s entity;

// A block entity is basically extra data associated with a block. Every block
// of a given ID should either have a block entity or not; but because blocks
// with block entities are relatively rare, it's more efficient to store their
// data separately than to make each block store extra data.
struct block_entity_s;
typedef struct block_entity_s block_entity;

/***********
 * Globals *
 ***********/

extern list *ENTITY_PROTOTYPES;

/*************************
 * Structure Definitions *
 *************************/

struct entity_s {
  char type[24]; // Entity type.
  vector size; // Dimensions.
  GLuint texture; // OpenGL texture.
  vertex_buffer * model; // Model.

  float mass; // Mass.
  float walk; // Movement impulse.
  float jump; // Jump impulse.

  vector pos; // Position within a frame.
  float yaw, pitch; // Facing.
  uint8_t on_ground; // Whether the entity is resting on ground.

  vector control; // Control inputs.
  vector vel; // Velocity.
  vector impulse; // Net impulse to be applied this tick.

  bbox box; // Bounding box.
  frame *fr; // The frame that this entity is in.
};

struct block_entity_s {
  uint16_t x, y, z; // Position within a chunk.
};

/********************
 * Inline Functions *
 ********************/

static inline void copy_entity_data(entity *from, entity *to) {
  strcpy(to->type, from->type);
  to->size.x = from->size.x;
  to->size.y = from->size.y;
  to->size.z = from->size.z;
  to->texture = from->texture;
  to->model = from->model;
  to->mass = from->mass;
  to->walk = from->walk;
  to->jump = from->jump;
}

static inline void copy_entity_pos(entity *from, entity *to) {
  to->pos.x = from->pos.x;
  to->pos.y = from->pos.y;
  to->pos.z = from->pos.z;
  to->yaw = from->yaw;
  to->pitch = from->pitch;
}

// Adds the given entity to the given frame.
static inline void add_entity(frame *f, entity *e) {
  e->fr = f;
  oct_insert(f->oct, (void *) e, &(e->box));
  append_element(f->entities, (void *)e);
}

// Zeroes out the velocity and impulse fields of the given entity.
static inline void clear_kinetics(entity *e) {
  e->vel.x = 0;
  e->vel.y = 0;
  e->vel.z = 0;
  e->impulse.x = 0;
  e->impulse.y = 0;
  e->impulse.z = 0;
}

// Recomputes the bounding box of the given entity based on its position and
// size.
static inline void compute_bb(entity *e) {
  float s2 = e->size.x/2.0;
  e->box.min.x = e->pos.x - s2;
  e->box.max.x = e->pos.x + s2;
  s2 = e->size.y/2.0;
  e->box.min.y = e->pos.y - s2;
  e->box.max.y = e->pos.y + s2;
  s2 = e->size.z/2.0;
  e->box.min.z = e->pos.z - s2;
  e->box.max.z = e->pos.z + s2;
}

/*************
 * Functions *
 *************/

// Adds a new entity prototype to the ENTITY_PROTOTYPES list.
void add_entity_type(entity *e);

// Sets up the entities system, which defines the ENTITY_PROTOTYPES list.
void setup_entities(void);

// Cleans up the memory used for the entities system.
void cleanup_entities(void);

// Ticks all entities attached to the given frame.
void tick_entities(frame *f);

// Ticks the given entity (it's assumed to be an entity):
void tick_entity(void *thing);

// Copies an entity from the types list, sets it up, and adds it to the given
// frame at the given position. This involves allocating space for the new
// entity. Returns a pointer to the entity spawned.
entity * spawn_entity(const char *type, vector *pos, frame *f);

// Scans the given list of entities and returns the first one with the given
// type. Returns NULL if no such entity exists.
entity * find_by_type(const char *type, list *l);

#endif //ifndef ENTITIES_H
