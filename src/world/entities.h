#ifndef ENTITIES_H
#define ENTITIES_H

// entities.h
// Entity definitions.

#include <stdint.h>
#include <string.h>

#include <GL/gl.h>

#include <GLFW/glfw3.h> // glfwGetTime

#include "world.h"

#include "datatypes/list.h"
#include "datatypes/vector.h"
#include "datatypes/bbox.h"
#include "graphics/vbo.h"

/*********
 * Types *
 *********/

typedef uint8_t move_flag;

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

// An active entity area is a cubic region within which entities can be active.
// It is specified by a base region position and a dimension.
struct active_entity_area_s;
typedef struct active_entity_area_s active_entity_area;

/***********
 * Globals *
 ***********/

// The prototypes for each entity type:
extern list *ENTITY_PROTOTYPES;

// A single main active entity area:
extern size_t const ACTIVE_AREA_SIZE;
extern active_entity_area *ACTIVE_AREA;

/*************************
 * Structure Definitions *
 *************************/

struct entity_s {
  char type[24]; // Entity type.
  vector size; // Dimensions.
  vector head_pos; // Head position relative to overall position.
  GLuint texture; // OpenGL texture.
  vertex_buffer *model; // Model.

  float mass; // Mass.
  float walk; // Walk speed.
    float jump; // Upward jump impulse.
    float leap; // Forwards jump impulse.
  float swim; // Swim speed (double-swimming is not allowed).
    float buoyancy; // Buoyancy in liquids.
  float fly; // Fly speed.
    float flap; // Flap impulse.
    float lift; // Buoyancy in air.
    double flap_duration; // Minimum time between flaps.

  double last_flap; // Last time this entity flapped.

  vector pos; // Position relative to the origin of this entity's active area.
  float yaw, pitch; // Facing.
  move_flag move_flags; // Movement flags.

  vector control; // Control inputs.
  vector vel; // Velocity.
  vector impulse; // Net impulse to be applied this tick.

  bbox box; // Bounding box.
  active_entity_area *area; // Area within which this entity can move.
};

struct block_entity_s {
  ch_idx_t x, y, z; // Position within a chunk.
};

struct active_entity_area_s {
  // Origin position within a region (bottom-south-west corner of this block):
  region_pos origin;
  size_t size; // Total width, height, and length.
  list *list; // list for holding entities.
  octree *tree; // octree for holding entities.
};

/********************
 * Inline Functions *
 ********************/

static inline void copy_entity_data(entity *from, entity *to) {
  strcpy(to->type, from->type);
  to->size.x = from->size.x;
  to->size.y = from->size.y;
  to->size.z = from->size.z;
  to->head_pos.x = from->head_pos.x;
  to->head_pos.y = from->head_pos.y;
  to->head_pos.z = from->head_pos.z;
  to->texture = from->texture;
  to->model = from->model;
  to->mass = from->mass;
  to->walk = from->walk;
    to->jump = from->jump;
    to->leap = from->leap;
  to->swim = from->swim;
    to->buoyancy = from->buoyancy;
  to->fly = from->fly;
    to->flap = from->flap;
    to->lift = from->lift;
    to->flap_duration = from->flap_duration;
}

static inline void copy_entity_pos(entity *from, entity *to) {
  to->pos.x = from->pos.x;
  to->pos.y = from->pos.y;
  to->pos.z = from->pos.z;
  to->yaw = from->yaw;
  to->pitch = from->pitch;
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

// Adds the given entity to the list of active entities. If it cannot (because
// the entity isn't within the area) it returns 0, otherwise it returns 1.
static inline int add_active_entity(active_entity_area *area, entity *e) {
  compute_bb(e); // (Re)compute the bounding box for the given entity.
  if (!oct_insert(area->tree, (void *) e, &(e->box))) {
    return 0;
  }
  l_append_element(area->list, (void *)e);
  e->area = area;
  return 1;
}

// Removes the given entity from the given active entity area, returning 1 if
// it succeeds and 0 if the given entity wasn't present.
static inline int remove_active_entity(active_entity_area *area, entity *e) {
  if (!oct_remove(area->tree, (void *) e)) {
    return 0;
  }
  l_remove_element(area->list, (void *) e);
  e->area = NULL;
  return 1;
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

static inline void get_head_vec(entity *e, vector *result) {
  vcopy(result, &(e->head_pos));
  //vyaw(result, e->yaw);
  // TODO: 3-d rotating creatures like birds/fish?
  //vpitch(&hp, e->pitch);
  vadd(result, &(e->pos));
}

// Note: if this is called on an entity that isn't in any area, it writes
// (0, 0, 0) into the result argument.
static inline void get_head_rpos(entity *e, region_pos *rpos) {
  vector hp;
  get_head_vec(e, &hp);
  if (e->area == NULL) {
    rpos->x = 0;
    rpos->y = 0;
    rpos->z = 0;
  } else {
    vec__rpos(&(e->area->origin), &hp, rpos);
  }
}

// Functions to quickly get rounded bounding box block coordinates:
static inline r_pos_t e_rp_min_x(entity *e) {
  return e->area->origin.x + fastfloor(e->box.min.x);
}
static inline r_pos_t e_rp_max_x(entity *e) {
  return e->area->origin.x + fastfloor(e->box.max.x);
}
static inline r_pos_t e_rp_min_y(entity *e) {
  return e->area->origin.y + fastfloor(e->box.min.y);
}
static inline r_pos_t e_rp_max_y(entity *e) {
  return e->area->origin.y + fastfloor(e->box.max.y);
}
static inline r_pos_t e_rp_min_z(entity *e) {
  return e->area->origin.z + fastfloor(e->box.min.z);
}
static inline r_pos_t e_rp_max_z(entity *e) {
  return e->area->origin.z + fastfloor(e->box.max.z);
}

static inline void e_min__rpos(entity *e, region_pos *rpos) {
  rpos->x = e_rp_min_x(e);
  rpos->y = e_rp_min_y(e);
  rpos->z = e_rp_min_z(e);
}

static inline void e_max__rpos(entity *e, region_pos *rpos) {
  rpos->x = e_rp_max_x(e);
  rpos->y = e_rp_max_y(e);
  rpos->z = e_rp_max_z(e);
}

// Movement functions:

// Try to flap. Returns 1 if flapping is okay, and 0 if not (because our
// previous flap was too recent). If it returns 1, it also automatically
// records the current time as the last flap time.
static inline int try_flap(entity *e) {
  double t = glfwGetTime();
  if (t - e->last_flap > e->flap_duration) {
    e->last_flap = t;
    return 1;
  } else {
    return 0;
  }
}

// Fake a flap: set the "last flap time" to now.
static inline void fake_flap(entity *e) {
  e->last_flap = glfwGetTime();
}

/******************************
 * Constructors & Destructors *
 ******************************/

// Sets up the entities system, which defines the ENTITY_PROTOTYPES list and
// sets up the main active entity area (which gets the given origin).
void setup_entities(region_pos *origin);

// Cleans up the memory used for the entities system.
void cleanup_entities(void);

// Allocates and returns a new blank entity. In most cases, you should use
// spawn_entity instead.
entity * create_entity(void);

// Cleans up and destroys an entity, removing it from any active area it may be
// a part of first.
void cleanup_entity(entity *e);

// Allocates and returns an active entity area.
active_entity_area * create_active_entity_area(region_pos *origin, size_t size);

// Cleans up the memory used by an active entity area, also freeing memory used
// by  all entities within the area. Remove the entities first if you don't
// want them to be destroyed.
void cleanup_active_entity_area(active_entity_area *area);

/*************
 * Functions *
 *************/

// Adds a new entity prototype to the ENTITY_PROTOTYPES list.
// This function allocates a new entity and copies data to it from the given
// entity, putting the new entity onto the prototypes list, so freeing the
// entity given as an argument is still the caller's responsibility. Make sure
// to call setup_entities() first.
void add_entity_type(entity *e);

// Ticks all of the active entities.
void tick_active_entities();

// Ticks the given entity (it's assumed to be an entity):
void tick_entity(void *thing);

// Returns the block that the entity's head is in, which is B_VOID if the
// entity isn't in a loaded chunk.
block head_block(entity *e);

// Warps space in the given active entity area such that the given entity is
// somewhere in its central chunk. This updates the area's origin and all
// entities in the space have their position vectors adjusted to be relative to
// the new origin instead of the old one. This also causes the bounding boxes
// of each entity to be recomputed and they're each re-inserted into the area's
// octree.
void warp_space(active_entity_area *area, entity *e);

// Copies an entity from the types list, sets it up, and adds it to the given
// active entity area at the given position. This involves allocating space for
// the new entity. Returns a pointer to the entity spawned.
entity * spawn_entity(
  char const * const type,
  vector *pos,
  active_entity_area *area
);

// Scans the given list of entities and returns the first one with the given
// type. Returns NULL if no such entity exists.
entity * find_by_type(char const * const type, list *l);

#endif //ifndef ENTITIES_H
