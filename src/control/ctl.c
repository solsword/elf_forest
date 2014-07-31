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
#include "gen/terrain.h"
#include "tick/tick.h"
#include "ui/ui.h"

#include "util.h"

/********************
 * Global variables *
 ********************/

int KEYMAP[N_CONTROLS] = {
  GLFW_KEY_Q, // quit
  GLFW_KEY_P, // pause
  GLFW_KEY_SPACE, GLFW_KEY_Z, GLFW_KEY_LEFT_SHIFT, // jump, dive, crouch
  GLFW_KEY_A, GLFW_KEY_D, GLFW_KEY_W, GLFW_KEY_S, // movement
  GLFW_KEY_V, // swap views
  GLFW_KEY_KP_ADD, GLFW_KEY_KP_SUBTRACT, // zoom in/out
  GLFW_KEY_F1, // draw debug info
  GLFW_KEY_L, // teleport
};

uint8_t CONTROLS[N_CONTROLS];
uint8_t DOWN[N_CONTROLS];
uint8_t UP[N_CONTROLS];

uint8_t PAUSED = 0;
uint8_t PHYSICS_PAUSED = 0;

float MOUSE_SENSITIVITY = 1.0/8.0;

entity * PLAYER = NULL;

float MAX_PITCH = M_PI_2;
float MIN_PITCH = -M_PI_2;

float ZOOM = 1.0;
float MIN_ZOOM = 0.5;
float MAX_ZOOM = 2.5;

float TELEPORT_DISTANCE = 200000;

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
    core_shutdown(0);
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
  float dontcare, th;
  vector teleport_vector;
  region_pos teleport_destination;
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

  // Displaying debug info:
  if (DOWN[C_DRAW_DEBUG_INFO]) {
    DRAW_DEBUG_INFO = !DRAW_DEBUG_INFO;
  }

  // Teleporting:
  if (DOWN[C_TELEPORT]) {
    vface(&teleport_vector, PLAYER->yaw, 0);
    vscale(&teleport_vector, TELEPORT_DISTANCE);
    vadd(&(PLAYER->pos), &teleport_vector);
    get_head_rpos(PLAYER, &teleport_destination);
    compute_terrain_height(&teleport_destination, &dontcare, &th);
    PLAYER->pos.z = (
      (r_pos_t) ffloor(th+15) -
      PLAYER->area->origin.z
    );
  }
}

void tick_motion_controls(void) {
  vzero(&(PLAYER->control));
  // Reset and recompute the crouching flag:
  clear_crouching(PLAYER);
  if (CONTROLS[C_CROUCH]) {
    set_crouching(PLAYER);
  }
  // Add directional inputs to our control vector:
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
  if (CONTROLS[C_DIVE]) {
    vadd(&(PLAYER->control), &V_DOWN);
  }
  // Set the do_jump or do_flap flag if the jump key is pressed (or add to
  // control if we're in water)
  clear_do_jump(PLAYER);
  clear_do_flap(PLAYER);
  if (DOWN[C_JUMP] || CONTROLS[C_JUMP]) {
    if (in_liquid(PLAYER)) {
      vadd(&(PLAYER->control), &V_UP);
    } else {
      // Jump if we're on the ground, otherwise try to flap:
      if (on_ground(PLAYER)) {
        set_do_jump(PLAYER);
        fake_flap(PLAYER);
      } else if (is_airborne(PLAYER) && PLAYER->flap != 0 && try_flap(PLAYER)) {
        set_do_flap(PLAYER);
      }
      DOWN[C_JUMP] = 0; // we'll only jump/flap once per tick max
    }
  }
}

void clear_edge_triggers(void) {
  int i;
  for (i = 0; i < N_CONTROLS; ++i) {
    DOWN[i] = 0;
    UP[i] = 0;
  }
}
