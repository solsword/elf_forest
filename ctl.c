// ctl.c
// User input handling.

#include <stdlib.h>
// DEBUG:
#include <stdio.h>
#include <math.h>

#include <GLFW/glfw3.h>

#include "util.h"
#include "ctl.h"
#include "gfx.h"
#include "world.h"
#include "entities.h"
#include "render.h"

/********************
 * Global variables *
 ********************/

int KEYMAP[N_CONTROLS] = {
  GLFW_KEY_Q, // quit
  GLFW_KEY_P, // pause
  GLFW_KEY_SPACE, GLFW_KEY_LEFT_CONTROL, // jump, crouch
  GLFW_KEY_A, GLFW_KEY_D, GLFW_KEY_W, GLFW_KEY_S, // movement
  GLFW_KEY_V, // swap views
  GLFW_KEY_KP_ADD, GLFW_KEY_KP_SUBTRACT, // zoom in/out
};

uint8_t CONTROLS[N_CONTROLS];
uint8_t DOWN[N_CONTROLS];
uint8_t UP[N_CONTROLS];

uint8_t PAUSED = 0;
uint8_t PHYSICS_PAUSED = 0;

float MOUSE_SENSITIVITY = 1.0/6.0;

entity * PLAYER = NULL;

float MAX_PITCH = M_PI_2;
float MIN_PITCH = -M_PI_2;

float ACCELERATION = 100.0;
float STRAFE_COEFFICIENT = 0.7;
float BACKUP_COEFFICIENT = 0.4;

float ZOOM = 1.0;
float MIN_ZOOM = 0.5;
float MAX_ZOOM = 2.5;

/*************
 * Callbacks *
 *************/

static void keyboard(
  GLFWwindow *window, int key, int scancode, int action, int mods
) {
  int i;
  if (key == KEYMAP[0]) {
    // We quit immediately here so that this won't be blocked by other things
    // that might filter user input or pause things.
    quit();
  }
  for (i = 0; i < N_CONTROLS; ++i) {
    if (key == KEYMAP[i]) {
      if (action == GLFW_PRESS) {
        CONTROLS[i] = 1;
        DOWN[i] = 1;
      } else if (action == GLFW_RELEASE) {
        CONTROLS[i] = 0;
        UP[i] = 1;
      } // TODO: handle repeat events?
    }
  }
}

static void mouse(GLFWwindow *window, int button, int action, int mods) {
  double x, y;
  glfwGetCursorPos(window, &x, &y);
}

/*************
 * Functions *
 *************/

static void mouse_moved(double dx, double dy) {
  if (PLAYER != NULL) {
    PLAYER->yaw -= dx*2*M_PI*MOUSE_SENSITIVITY;
    PLAYER->pitch -= dy*ASPECT*M_PI*MOUSE_SENSITIVITY;
    norm_angle(&(PLAYER->yaw));
    norm_angle(&(PLAYER->pitch));
    if (PLAYER->pitch > MAX_PITCH) {
      PLAYER->pitch = MAX_PITCH;
    }
    if (PLAYER->pitch < MIN_PITCH) {
      PLAYER->pitch = MIN_PITCH;
    }
  }
}

static void activate_ctl_callbacks(void) {
  glfwSetKeyCallback( WINDOW, keyboard );
  glfwSetMouseButtonCallback( WINDOW, mouse );
}

void setup_control(void) {
  disable_cursor();
  activate_ctl_callbacks();
}

void tick_general_controls(void) {
  // Handle mouse movement (TODO: switch this on/off):
  double mx, my;
  glfwGetCursorPos(WINDOW, &mx, &my);
  mx -= WINDOW_WIDTH/2.0;
  my -= WINDOW_HEIGHT/2.0;
  mx /= WINDOW_WIDTH;
  my /= WINDOW_HEIGHT;
  mouse_moved(mx, my);
  glfwSetCursorPos(WINDOW, WINDOW_WIDTH/2.0, WINDOW_HEIGHT/2.0);

  // Pausing:
  if (DOWN[C_PAUSE]) {
    PAUSED = !PAUSED;
    PHYSICS_PAUSED = !PHYSICS_PAUSED;
  }

  // Changing viewpoints:
  if (DOWN[C_CHANGE_VIEW]) {
    VIEW_MODE = (VIEW_MODE + 1) % NUM_VIEW_MODES;
  }

  // Zooming:
  if (CONTROLS[C_ZOOM_IN]) {
    ZOOM *= 0.9;
    ZOOM = ZOOM < MIN_ZOOM ? MIN_ZOOM : ZOOM;
  }
  if (CONTROLS[C_ZOOM_OUT]) {
    ZOOM *= 1.1;
    ZOOM = ZOOM > MAX_ZOOM ? MAX_ZOOM : ZOOM;
  }
}

void tick_motion_controls(void) {
  vector forward;
  vface(&forward, PLAYER->yaw, 0);
  vector v;
  // TODO: Limited air control?
  if (CONTROLS[C_FORWARD]) {
    vcopy(&v, &forward);
    vscale(&v, PLAYER->walk);
    vadd(&(PLAYER->control), &v);
  }
  if (CONTROLS[C_REVERSE]) {
    vcopy(&v, &forward);
    vscale(&v, -PLAYER->walk * BACKUP_COEFFICIENT);
    vadd(&(PLAYER->control), &v);
  }
  if (CONTROLS[C_LEFT]) {
    vcopy(&v, &forward);
    vyaw(&v, M_PI_2);
    vscale(&v, PLAYER->walk * STRAFE_COEFFICIENT);
    vadd(&(PLAYER->control), &v);
  }
  if (CONTROLS[C_RIGHT]) {
    vcopy(&v, &forward);
    vyaw(&v, -M_PI_2);
    vscale(&v, PLAYER->walk * STRAFE_COEFFICIENT);
    vadd(&(PLAYER->control), &v);
  }
  if (DOWN[C_JUMP] && PLAYER->on_ground) {
    vcopy(&v, &V_UP);
    vscale(&v, PLAYER->jump);
    vadd(&(PLAYER->control), &v);
    DOWN[C_JUMP] = 0;
  }
}

void clear_edge_triggers(void) {
  int i;
  for (i = 0; i < N_CONTROLS; ++i) {
    DOWN[i] = 0;
    UP[i] = 0;
  }
}
