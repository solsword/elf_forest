// ctl.c
// User input handling.

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <GL/glut.h>

#include "ctl.h"
#include "gfx.h"
#include "world.h"

/********************
 * Global variables *
 ********************/

uint8_t CONTROLS[N_CONTROLS];
uint8_t DOWN[N_CONTROLS];
uint8_t UP[N_CONTROLS];

uint8_t PAUSED = 0;
uint8_t PHYSICS_PAUSED = 0;

int ZOOM = 0;

/*************
 * Functions *
 *************/

static void keyboard(unsigned char key, int x, int y) {
  if (key == 'q') {
    quit();
  } else if (key == 'p') {
    CONTROLS[C_PAUSE] = 1;
    DOWN[C_PAUSE] = 1;
  } else if (key == ' ') {
    CONTROLS[C_JUMP] = 1;
    DOWN[C_JUMP] = 1;
  } else if (key == 'w') {
    CONTROLS[C_FORWARD] = 1;
    DOWN[C_FORWARD] = 1;
  } else if (key == 'a') {
    CONTROLS[C_LEFT] = 1;
    DOWN[C_LEFT] = 1;
  } else if (key == 's') {
    CONTROLS[C_REVERSE] = 1;
    DOWN[C_REVERSE] = 1;
  } else if (key == 'd') {
    CONTROLS[C_RIGHT] = 1;
    DOWN[C_RIGHT] = 1;
  } else if (key == 'k') {
    CONTROLS[C_ZOOM_IN] = 1;
    DOWN[C_ZOOM_IN] = 1;
  } else if (key == 'j') {
    CONTROLS[C_ZOOM_OUT] = 1;
    DOWN[C_ZOOM_OUT] = 1;
  }
}

static void keyboard_up(unsigned char key, int x, int y) {
  if (key == 'p') {
    CONTROLS[C_PAUSE] = 0;
    UP[C_PAUSE] = 1;
  } else if (key == ' ') {
    CONTROLS[C_JUMP] = 0;
    UP[C_JUMP] = 1;
  } else if (key == 'w') {
    CONTROLS[C_FORWARD] = 0;
    UP[C_FORWARD] = 1;
  } else if (key == 'a') {
    CONTROLS[C_LEFT] = 0;
    UP[C_LEFT] = 1;
  } else if (key == 's') {
    CONTROLS[C_REVERSE] = 0;
    UP[C_REVERSE] = 1;
  } else if (key == 'd') {
    CONTROLS[C_RIGHT] = 0;
    UP[C_RIGHT] = 1;
  } else if (key == 'k') {
    CONTROLS[C_ZOOM_IN] = 0;
    UP[C_ZOOM_IN] = 1;
  } else if (key == 'j') {
    CONTROLS[C_ZOOM_OUT] = 0;
    UP[C_ZOOM_OUT] = 1;
  }
}

static void special(int key, int x, int y) {
}

static void special_up(int key, int x, int y) {
}

static void mouse(int button, int state, int x, int y) {
}

static void motion(int x, int y) {
}

static void activate_ctl_callbacks(void) {
  glutKeyboardFunc( keyboard );
  glutKeyboardUpFunc( keyboard_up );
  glutSpecialFunc( special );
  glutSpecialUpFunc( special_up );
  glutMouseFunc( mouse );
  glutMotionFunc( motion );
}

void setup_control(void) {
  glutIgnoreKeyRepeat(1);
  activate_ctl_callbacks();
}

void tick_general_controls(void) {
  if (DOWN[C_PAUSE]) {
    PAUSED = !PAUSED;
    PHYSICS_PAUSED = !PHYSICS_PAUSED;
  }
  if (CONTROLS[C_ZOOM_IN]) {
    ZOOM += 1;
    ZOOM = ZOOM > HALF_FRAME *1.3 ? HALF_FRAME*1.3 : ZOOM;
  }
  if (CONTROLS[C_ZOOM_OUT]) {
    ZOOM -= 1;
    ZOOM = ZOOM < 0 ? 0 : ZOOM;
  }
  clear_edge_triggers();
}

void tick_motion_controls(void) {
  // TODO: HERE
}

void clear_edge_triggers(void) {
  int i;
  for (i = 0; i < N_CONTROLS; ++i) {
    DOWN[i] = 0;
    UP[i] = 0;
  }
}
