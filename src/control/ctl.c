// ctl.c
// User input handling.

#include <stdlib.h>
// DEBUG:
#include <stdio.h>
#include <math.h>

#include <GLFW/glfw3.h>

#include "graphics/gfx.h"
#include "graphics/render.h"
#include "control/ctl.h"
#include "world/world.h"
#include "world/entities.h"
#include "util.h"

/********************
 * Global variables *
 ********************/

int KEYMAP[N_CONTROLS] = {
  GLFW_KEY_Q, // quit
  GLFW_KEY_P, // pause
  GLFW_KEY_SPACE, GLFW_KEY_LEFT_SHIFT, // jump, crouch
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

void init_control(void) {
  disable_cursor();
  glfwSetCursorPos(WINDOW, WINDOW_WIDTH/2.0, WINDOW_HEIGHT/2.0);
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
  vzero(&(PLAYER->control));
  if (CONTROLS[C_FORWARD]) {
    vadd(&(PLAYER->control), &V_NORTH);
  }
  if (CONTROLS[C_REVERSE]) {
    vadd(&(PLAYER->control), &V_SOUTH);
  }
  if (CONTROLS[C_LEFT]) {
    vadd(&(PLAYER->control), &V_WEST);
  }
  if (CONTROLS[C_RIGHT]) {
    vadd(&(PLAYER->control), &V_EAST);
  }
  // Note: unlike other control inputs, jump inputs are just impulses.
  if (DOWN[C_JUMP] || CONTROLS[C_JUMP]) {
    if (in_liquid(PLAYER)) {
      vadd(&(PLAYER->control), &V_UP);
    } else if (on_ground(PLAYER)) {
      vector v;
      vcopy(&v, &V_UP);
      vscale(&v, PLAYER->jump);
      vadd(&(PLAYER->impulse), &v);
      DOWN[C_JUMP] = 0; // we'll only jump once per frame max
    }
  }
  if (CONTROLS[C_CROUCH]) {
    if (in_liquid(PLAYER)) {
      vadd(&(PLAYER->control), &V_DOWN);
    } else {
      set_crouching(PLAYER);
    }
  } else {
    clear_crouching(PLAYER);
  }
}

void clear_edge_triggers(void) {
  int i;
  for (i = 0; i < N_CONTROLS; ++i) {
    DOWN[i] = 0;
    UP[i] = 0;
  }
}
