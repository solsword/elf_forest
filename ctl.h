#ifndef CONTROL_H
#define CONTROL_H

// ctl.h
// User input handling.

#include <stdint.h>

#include "entities.h"
#include "gfx.h"

/****************
 * Enumerations *
 ****************/

// Logical controls:
typedef enum {
  C_QUIT,
  C_PAUSE,
  C_JUMP, C_CROUCH,
  C_LEFT, C_RIGHT, C_FORWARD, C_REVERSE,
  C_CHANGE_VIEW, C_ZOOM_IN, C_ZOOM_OUT,
  N_CONTROLS
} control;

/********************
 * Global variables *
 ********************/

// This array defines the mapping from keys (GLFW constants should be used) to
// logical controls.
extern int KEYMAP[N_CONTROLS];

// The controls: these arrays are turned on/off according to keyboard inputs.
// The CONTROLS array is a toggle, while the other two are edge triggers. They
// persist until cleared by clear_edge_triggers();
extern uint8_t CONTROLS[N_CONTROLS];
extern uint8_t DOWN[N_CONTROLS];
extern uint8_t UP[N_CONTROLS];

// Pause states:
extern uint8_t PAUSED;
extern uint8_t PHYSICS_PAUSED;

// Controls how mouse motion translates into player rotation:
extern float MOUSE_SENSITIVITY;

// The player entity will receive control inputs:
extern entity * PLAYER;

// The pitch limits for looking up or down:
extern float MAX_PITCH;
extern float MIN_PITCH;

// How fast entities accelerate:
extern float ACCELERATION;
// Attenuation of strafing relative to forward motion:
extern float STRAFE_COEFFICIENT;
// Attenuation of backing up relative to forward motion:
extern float BACKUP_COEFFICIENT;

// Zoom level and limits (smaller numbers are closer):
extern float ZOOM;
extern float MIN_ZOOM;
extern float MAX_ZOOM;

/********************
 * Inline Functions *
 ********************/

static inline void enable_cursor(void) {
  glfwSetInputMode(WINDOW, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

static inline void disable_cursor(void) {
  glfwSetInputMode(WINDOW, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

/*************
 * Functions *
 *************/

// Sets up the control callbacks and other control configuration.
void setup_control(void);

// Reads the control state and updates state.
void tick_general_controls(void);

// The motion-related controls. Updates the PLAYER entity.
void tick_motion_controls(void);

// Clears the DOWN and UP arrays. Called at the end of each tick.
void clear_edge_triggers(void);

#endif //ifndef CONTROL_H
