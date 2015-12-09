#ifndef ADAPTATIONS_H
#define ADAPTATIONS_H

// adaptations.h
// Plant and animal adaptations.

#ifdef DEBUG
  #include <stdio.h>
#endif

/*********
 * Types *
 *********/

typedef unsigned int adaptation_id;

typedef uint64_t adpt_lentry;

/**************
 * Structures *
 **************/

// A list of adaptations of one particular kind:
struct adaptation_list_s;
typedef struct adaptation_list_s adaptation_list;

// Fungus adaptations:
struct fungus_adaptation_s;
typedef struct fungus_adaptation_s fungus_adaptation;

// (Terrestrial) plant adaptations:
struct plant_adaptation_s;
typedef struct plant_adaptation_s plant_adaptation;

// Aquatic plant adaptations:
struct aquatic_plant_adaptation_s;
typedef struct aquatic_plant_adaptation_s aquatic_plant_adaptation;

// Coral adaptations:
struct coral_adaptation_s;
typedef struct coral_adaptation_s coral_adaptation;

// TODO: Animal adaptations

/*************
 * Constants *
 *************/

#define ADAPTATION_LIST_SIZE 16

// The maximum number of distinct adaptations.
#define MAX_ADAPTATION_TYPES (64 * ADAPTATION_LIST_SIZE)

/*************************
 * Structure Definitions *
 *************************/

struct adaptation_list_s {
  adpt_lentry entries[ADAPTATION_LIST_SIZE];
}

struct fungus_adaptation_s {
  adaptation_id id;
  // TODO: stuff here
}

struct plant_adaptation_s {
  adaptation_id id;
  // TODO: stuff here
}

struct aquatic_plant_adaptation_s {
  adaptation_id id;
  // TODO: stuff here
}

struct coral_adaptation_s {
  adaptation_id id;
  // TODO: stuff here
}

/********************
 * Inline Functions *
 ********************/

static inline int includes_adaptation(adaptation_list *list, adaptation_id aid){
#ifdef DEBUG
  if ((aid / sizeof(adpt_lentry)) >= ADAPTATION_LIST_SIZE) {
    printf("Error: Adaptation ID %d is too large!\n", aid);
    exit(1);
  }
#endif
  return (
     list->entries[aid / sizeof(adpt_lentry)]
  >> (aid % sizeof(adpt_lentry))
  ) & 0x1;
}

static inline void set_adaptation(adaptation_list *list, adaptation_id aid){
#ifdef DEBUG
  if ((aid / sizeof(adpt_lentry)) >= ADAPTATION_LIST_SIZE) {
    printf("Error: Adaptation ID %d is too large!\n", aid);
    exit(1);
  }
#endif
  list->entries[aid / sizeof(adpt_lentry)] |= 1 << (aid % sizeof(adpt_lentry));
}

static inline void clear_adaptation(adaptation_list *list, adaptation_id aid){
#ifdef DEBUG
  if ((aid / sizeof(adpt_lentry)) >= ADAPTATION_LIST_SIZE) {
    printf("Error: Adaptation ID %d is too large!\n", aid);
    exit(1);
  }
#endif
  list->entries[aid / sizeof(adpt_lentry)] &= ~(1<<(aid % sizeof(adpt_lentry)));
}

/*************
 * Functions *
 *************/

#endif // ifndef ADAPTATIONS_H
