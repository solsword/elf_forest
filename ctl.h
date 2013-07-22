#ifndef CONTROL_H
#define CONTROL_H

// ctl.h
// User input handling.

#include <stdint.h>

/****************
 * Enumerations *
 ****************/

// Control mapping:
typedef enum {
  C_PAUSE,
  C_JUMP, C_LEFT, C_RIGHT, C_FORWARD, C_REVERSE,
  C_ZOOM_IN, C_ZOOM_OUT,
  N_CONTROLS
} control;

/********************
 * Global variables *
 ********************/

extern uint8_t CONTROLS[N_CONTROLS];
extern uint8_t DOWN[N_CONTROLS];
extern uint8_t UP[N_CONTROLS];

extern uint8_t PAUSED;

extern int ZOOM;

/*************
 * Functions *
 *************/

// Sets up the control callbacks and other control configuration.
void setup_control(void);

// Reads the control state and updates state.
void tick_general_controls(void);

// The motion-related controls (not called from tick_general_controls):
void tick_motion_controls(void);

// Clears the DOWN and UP arrays. Called by tick_general_controls at the end of
// each tick.
void clear_edge_triggers(void);

#endif //ifndef CONTROL_H
