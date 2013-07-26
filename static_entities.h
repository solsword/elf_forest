#ifndef STATIC_ENTITIES_H
#define STATIC_ENTITIES_H

// static_entities.h
// System-defined static entity information.

#define NUM_STATIC_ENTITY_TYPES 4

entity STATIC_ENTITY_TYPES[NUM_STATIC_ENTITY_TYPES] = {
  {
    .type="elf",
    .size={.x=0.7,.y=0.7,.z=1.9},
    .mass=45,
    .walk=500,
    .jump=20000,
  },
  {
    .type="dwarf",
    .size={.x=0.8,.y=0.8,.z=1.2},
    .mass=90,
    .walk=750,
    .jump=24000,
  },
  {
    .type="human",
    .size={.x=0.8,.y=0.8,.z=1.7},
    .mass=70,
    .walk=750,
    .jump=24000,
  },
  {
    .type="sparrow",
    .size={.x=0.2,.y=0.2,.z=0.2},
    .mass=0.03,
    .walk=0.6,
    .jump=50,
  }
};

#endif //ifndef STATIC_ENTITIES_H
