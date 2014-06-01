#ifndef MATERIALS_H
#define MATERIALS_H

// materials.h
// Material properties.

/*********
 * Enums *
 *********/

enum material_origin_e {
  MO_IGNEOUS, // basalt
  MO_METAMORPHIC, // granite
  MO_SEDIMENTARY, // sandstone
  MO_ERODED, // gravel
  MO_ORGANIC_DECOMPOSED, // dirt
  MO_ORGANIC_GROWTH, // wood
};
typedef enum material_origin_e material_origin;

/************************
 * Types and Structures *
 ************************/

// TODO: more material info (e.g., quartz vs. quartz sand vs. quartz gravel)?
typedef block_variant material;

/*************
 * Constants *
 *************/

/***********
 * Globals *
 ***********/

/*************************
 * Structure Definitions *
 *************************/

/********************
 * Inline Functions *
 ********************/

static inline float mt_erosion_rate(material m) {
};

/******************************
 * Constructors & Destructors *
 ******************************/

/*************
 * Functions *
 *************/

#endif // ifndef MATERIALS_H
