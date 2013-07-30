// gfx.c
// Graphics environment wrangling.

#include <GL/glu.h>

#include <GLFW/glfw3.h>

#include <math.h>

#include "util.h"
#include "vbo.h"
#include "world.h"
#include "tick.h"
#include "gfx.h"
#include "render.h"
#include "ui.h"
#include "display.h"
#include "data.h"
#include "ctl.h"

/***************
 * Global vars *
 ***************/

// Stores the window ID when that gets set up.
GLFWwindow * WINDOW;

// Should we render to the screen or not?
int RENDER = 1;

// The current width/height of the window.
int WINDOW_WIDTH = 0;
int WINDOW_HEIGHT = 0;

// Default arguments to prepare(...)
const int DEFAULT_WIDTH = 800;
const int DEFAULT_HEIGHT = 600;
const char* DEFAULT_NAME = "Elf Forest";
const float DEFAULT_R = 0.65;
const float DEFAULT_G = 0.71;
const float DEFAULT_B = 1.0;
const float DEFAULT_A = 1.0;

// Camera parameters
const double FOV = M_PI/3.0;
const double ASPECT = 4.0/3.0;
const double NEAR = 0.2;
const double FAR = 362.0; // 256x256 diagonal

/*************
 * Functions *
 *************/

// GLFW callback functions:

void resize(GLFWwindow *window, int w, int h) {
  int dw = ASPECT*h;
  if (w < dw) {
    int dh = w/ASPECT;
    glViewport(0, (h + dh)/2.0, w, dh);
  } else {
    glViewport((w - dw)/2.0, 0, dw, h);
  }
  WINDOW_WIDTH = w;
  WINDOW_HEIGHT = h;
}

static void set_active(GLFWwindow *window, int active) {
  if (active) {
    // Start rendering if we had stopped:
    RENDER = 1;
  } else {
    // Pause and stop rendering:
    RENDER = 0;
    PAUSED = 1;
    PHYSICS_PAUSED = 1;
  }
}

void focus(GLFWwindow *window, int focus) {
  set_active(window, focus);
}

void minmaximize(GLFWwindow *window, int minimized) {
  set_active(window, !minimized);
}

void render(GLFWwindow *window) {
  vector head_pos;
  get_head_pos(PLAYER, &head_pos);
  render_frame(&MAIN_FRAME, &head_pos, PLAYER->yaw, PLAYER->pitch);
  render_ui();
  glfwSwapBuffers(window);
  glClear( GL_COLOR_BUFFER_BIT );
}

// Individual setup functions:

// Initialize the GLFW context:
static void init_context(int* argc, char** argv) {
  if (!glfwInit()) {
    exit(-1);
  }
}

static void activate_gfx_callbacks(void) {
  // Setup the callback functions:
  glfwSetWindowRefreshCallback(WINDOW, &render);
  // We don't care about window resize events as long as they don't affect our
  // framebuffer.
  glfwSetFramebufferSizeCallback( WINDOW, &resize );
  glfwSetWindowIconifyCallback( WINDOW, &minmaximize );
  glfwSetWindowFocusCallback( WINDOW, &focus );
}

// Sets various OpenGL settings:
void glsettings() {
  glEnable( GL_CULL_FACE );
  glFrontFace( GL_CW );
  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glBlendColor(1.0, 1.0, 1.0, 1.0);
  glEnable( GL_STENCIL_TEST ); // Do we need this?
  glEnable( GL_TEXTURE_2D );
  glEnable( GL_DEPTH_TEST );
  glDepthFunc( GL_LESS );
  glDepthMask( GL_TRUE );
  glEnable( GL_FOG );
  glHint( GL_FOG_HINT, GL_FASTEST );
  glFogi( GL_FOG_MODE, GL_EXP2 );
}

// Sets up the OpenGL perspective:
void glperspective() {
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  gluPerspective(FOV*R2D, ASPECT, NEAR, FAR);
  glMatrixMode( GL_MODELVIEW );
}

// Main interface functions:

void prepare_default(int* argc, char** argv) {
  prepare(
    argc, argv,
    DEFAULT_WIDTH, DEFAULT_HEIGHT, 
    DEFAULT_NAME,
    DEFAULT_R,
    DEFAULT_G,
    DEFAULT_B,
    DEFAULT_A
  );
}

void prepare(
  int* argc,
  char** argv,
  int w,
  int h,
  const char* name,
  float r,
  float g,
  float b,
  float a
) {
  init_context(argc, argv);
  WINDOW = glfwCreateWindow( w, h, name, NULL, NULL );
  if (!WINDOW) {
    glfwTerminate();
    exit(-1);
  }
  glfwGetWindowSize(WINDOW, &WINDOW_WIDTH, &WINDOW_HEIGHT);
  glfwMakeContextCurrent(WINDOW);
  set_bg_color( r, g, b, a);
  glsettings();
  glperspective();
  activate_gfx_callbacks();
}

void cleanup(void) {
  cleanup_frame(&MAIN_FRAME);
  cleanup_entities();
  cleanup_data();
}

void loop(void) {
  while (!glfwWindowShouldClose(WINDOW)) {
    if (RENDER) {
      render(WINDOW);
    }
    glfwPollEvents();
    tick(ticks_expected());
  }
  glfwTerminate();
}

void quit(void) {
  cleanup();
  glfwTerminate();
  exit(0);
}

void fail(int err) {
  cleanup();
  glfwTerminate();
  exit(err);
}
