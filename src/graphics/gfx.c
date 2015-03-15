// gfx.c
// Graphics environment wrangling.

#include <GL/glew.h>
#include <GL/glu.h>

#include <GLFW/glfw3.h>

#include <math.h>

#include "render.h"
#include "display.h"
#include "gfx.h"

#include "shaders/pipeline.h"

#include "ui/ui.h"
#include "world/world.h"
#include "world/entities.h"
#include "prof/ptime.h"
#include "data/data.h"
#include "control/ctl.h"
#include "gen/worldgen.h"
#include "jobs/jobs.h"
#include "util.h"

/***************
 * Global vars *
 ***************/

// Stores the window ID when that gets set up.
GLFWwindow * WINDOW;

// Should we render to the screen or not?
int RENDER = 1;

// Callback to call before each render call.
render_callback PRE_RENDER_CALLBACK = NULL;

// The current width/height of the window.
int WINDOW_WIDTH = 0;
int WINDOW_HEIGHT = 0;

// Default arguments to prepare(...)
int const DEFAULT_WIDTH = 800;
int const DEFAULT_HEIGHT = 600;
char const * const DEFAULT_NAME = "Elf Forest";
float const DEFAULT_R = 0.65;
float const DEFAULT_G = 0.71;
float const DEFAULT_B = 1.0;
float const DEFAULT_A = 1.0;

// Camera parameters
double const FOV = M_PI/3.0;
double const ASPECT = 4.0/3.0;
double const NEAR = 0.05;
double const FAR = 1024.0;

/***************************
 * GLFW Callback Functions *
 ***************************/

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

void set_active(GLFWwindow *window, int active) {
  if (active) {
    // Start rendering if we had stopped:
    RENDER = 1;
  } else {
    // Pause and stop rendering, but re-draw once to put up the "PAUSED"
    // message:
    PAUSED = 1;
    PHYSICS_PAUSED = 1;
    RENDER = 0;

    render(window);
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
  // Get the player's head position:
  get_head_vec(PLAYER, &head_pos);

  // Clear the buffers:
  //clear_color_buffer();
  //clear_depth_buffer();

  // Set up a fresh model view:
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // Call the pre-render callback if it exists:
  if (PRE_RENDER_CALLBACK != NULL) {
    (*PRE_RENDER_CALLBACK)();
  }

  // Render the active area:
#ifdef PROFILE_TIME
  start_duration(&RENDER_AREA_TIME);
#endif
    render_area(
      ACTIVE_AREA,
      &head_pos, PLAYER->yaw, PLAYER->pitch,
      FOV*ASPECT, FOV
    );
#ifdef PROFILE_TIME
  end_duration(&RENDER_AREA_TIME);
#endif

  // Render the UI:
#ifdef PROFILE_TIME
  start_duration(&RENDER_UI_TIME);
#endif
  render_ui();
#ifdef PROFILE_TIME
  end_duration(&RENDER_UI_TIME);
#endif

  // Pop our matrix:
  glPopMatrix();

  // Swap buffers and then clear the fresh ones:
  glfwSwapBuffers(window);
  clear_color_buffer();
  clear_depth_buffer();

  // Update the framerate counter:
  update_rate(&FRAMERATE);
}

/************************
 * GLFW Setup Functions *
 ************************/

void init_context(int* argc, char** argv) {
  if (!glfwInit()) {
    exit(-1);
  }
}

void activate_gfx_callbacks(void) {
  // Setup the callback functions:
  glfwSetWindowRefreshCallback(WINDOW, &render);
  // We don't care about window resize events as long as they don't affect our
  // framebuffer.
  glfwSetFramebufferSizeCallback( WINDOW, &resize );
  glfwSetWindowIconifyCallback( WINDOW, &minmaximize );
  glfwSetWindowFocusCallback( WINDOW, &focus );
}

// Sets various OpenGL settings:
void glsettings(void) {
  glEnable( GL_CULL_FACE );
  glFrontFace( GL_CW );
  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glAlphaFunc(GL_GREATER, 0.5);
  glBlendColor(1.0, 1.0, 1.0, 1.0);
  glEnable( GL_STENCIL_TEST ); // Do we need this?
  glEnable( GL_TEXTURE_2D );
  glEnable( GL_DEPTH_TEST );
  glDepthFunc( GL_LESS );
  glDepthMask( GL_TRUE );
  glEnable( GL_FOG );
  glHint( GL_FOG_HINT, GL_FASTEST );
  glFogi( GL_FOG_MODE, GL_EXP2 );
  glEnable( GL_ALPHA_TEST );
  glEnable( GL_POLYGON_OFFSET_FILL );
}

// Sets up the OpenGL perspective:
void glperspective(void) {
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  gluPerspective(FOV*R2D, ASPECT, NEAR, FAR);
  glMatrixMode( GL_MODELVIEW );
}

/*************
 * Functions *
 *************/

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
  char const * const name,
  float r,
  float g,
  float b,
  float a
) {
  // Initialize GLFW
  init_context(argc, argv);

  // Set up the window
  WINDOW = glfwCreateWindow( w, h, name, NULL, NULL );
  if (!WINDOW) {
    glfwTerminate();
    exit(-1);
  }
  glfwGetWindowSize(WINDOW, &WINDOW_WIDTH, &WINDOW_HEIGHT);
  glfwMakeContextCurrent(WINDOW);

  // Initialize GLEW
  GLenum err = glewInit();
  if (err != GLEW_OK) {
    printf("Failed to initialize GLEW:\n");
    printf("  %s\n", glewGetErrorString(err));
    exit(-1);
  }

  // Settings
  set_bg_color( r, g, b, a);
  glsettings();
  glperspective();

  // Activate callbacks
  activate_gfx_callbacks();
}
