#ifndef CONTROL_H
#define CONTROL_H

// ctl.h
// User input handling.

#include <stdint.h>

#include "entities.h"

/****************
 * Enumerations *
 ****************/

// Control mapping:
typedef enum {
  C_PAUSE,
  C_JUMP,
  C_LEFT, C_RIGHT, C_FORWARD, C_REVERSE,
  C_ZOOM_IN, C_ZOOM_OUT,

  // Note: these three controls don't support edge triggers. They should only
  // be used as modifiers to other controls, since their state won't update if
  // they're pressed and no other key is down.
  C_SHIFT, C_CTRL, C_ALT,

  N_CONTROLS
} control;

/********************
 * Global variables *
 ********************/

extern uint8_t CONTROLS[N_CONTROLS];
extern uint8_t DOWN[N_CONTROLS];
extern uint8_t UP[N_CONTROLS];

extern uint8_t PAUSED;
extern uint8_t PHYSICS_PAUSED;

extern float STRAFE_COEFFICIENT;

extern int ZOOM;

/*************
 * Functions *
 *************/

// Sets up the control callbacks and other control configuration.
void setup_control(void);

// Reads the control state and updates state.
void tick_general_controls(void);

// The motion-related controls (not called from tick_general_controls). Updates
// the given entity.
void tick_motion_controls(entity *e);

// Clears the DOWN and UP arrays. Called by tick_general_controls at the end of
// each tick.
void clear_edge_triggers(void);

#endif //ifndef CONTROL_H
