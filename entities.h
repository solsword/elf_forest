#ifndef ENTITIES_H
#define ENTITIES_H

// entities.h
// Entity definitions.

#include <stdint.h>

#include <GL/gl.h>

#include "physics.h"
#include "vbo.h"

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

/*************************
 * Structure Definitions *
 *************************/

struct entity_s {
  vector size; // Dimensions.
  vector pos; // Position within a frame.
  vector vel; // Velocity.
  vector impulse; // Net impulse to be applied this tick.
  bbox box; // Bounding box.
  GLuint texture; // OpenGL texture.
  vertex_buffer model; // Model.
};

struct block_entity_s {
  uint16_t x, y, z; // Position within a chunk.
};

/*************
 * Functions *
 *************/

// Ticks all entities attached to the given frame.
void tick_entities(frame *f);

#endif //ifndef ENTITIES_H
