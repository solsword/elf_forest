#ifndef STATIC_ENTITIES_H
#define STATIC_ENTITIES_H

// static_entities.h
// System-defined static entity information.

#include "entities.h"

#define NUM_STATIC_ENTITY_TYPES 5

entity STATIC_ENTITY_TYPES[NUM_STATIC_ENTITY_TYPES] = {
  {
    .type="tester",
    .size={.x=0.7,.y=0.7,.z=1.8},
    .head_pos={.x=0.0,.y=0.0,.z=0.7},
    .mass=45,
    .walk=6000,
    .swim=6000,
    .buoyancy=0.7,
    .fly=3000,
    .jump=60000,
  },
  {
    .type="elf",
    .size={.x=0.7,.y=0.7,.z=1.8},
    .head_pos={.x=0.0,.y=0.0,.z=0.7},
    .mass=45,
    .walk=1200,
    .swim=800,
    .buoyancy=0.7,
    .fly=30,
    .jump=24000,
  },
  {
    .type="dwarf",
    .size={.x=0.8,.y=0.8,.z=1.2},
    .head_pos={.x=0.0,.y=0.0,.z=0.4},
    .mass=90,
    .walk=200,
    .swim=200,
    .buoyancy=0.7,
    .fly=50,
    .jump=24000,
  },
  {
    .type="human",
    .size={.x=0.8,.y=0.8,.z=1.6},
    .head_pos={.x=0.0,.y=0.0,.z=0.6},
    .mass=70,
    .walk=200,
    .swim=200,
    .buoyancy=0.7,
    .fly=50,
    .jump=24000,
  },
  {
    .type="sparrow",
    .size={.x=0.2,.y=0.2,.z=0.2},
    .head_pos={.x=0.0,.y=0.0,.z=0.0},
    .mass=0.03,
    .walk=3,
    .swim=1,
    .buoyancy=0.9,
    .fly=30,
    .jump=50,
  }
};

#endif //ifndef STATIC_ENTITIES_H
