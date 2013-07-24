#include <GL/gl.h>
#include <GL/glut.h>

#include <math.h>

#include "util.h"
#include "world.h"
#include "tick.h"
#include "gfx.h"
#include "render.h"
#include "display.h"
#include "ctl.h"

/***************
 * Global vars *
 ***************/

// Stores the window ID when that gets set up.
int WINDOW;

// Default arguments to prepare(...)
const int DEFAULT_MODE = GLUT_RGBA | GLUT_DOUBLE | GLUT_STENCIL;
const int DEFAULT_WIDTH = 800;
const int DEFAULT_HEIGHT = 600;
const char* DEFAULT_NAME = "Elf Forest";
const float DEFAULT_R = 0;
const float DEFAULT_G = 0;
const float DEFAULT_B = 0;
const float DEFAULT_A = 1.0;

// Camera parameters
const double FOV = M_PI/2.0;
const double ASPECT = 4.0/3.0;
const double NEAR = 0.2;
const double FAR = 362.0; // 256x256 diagonal

/*************
 * Functions *
 *************/

// GLUT callback functions:

static void idle(void) {
  glutPostRedisplay();
}

static void resize(int w, int h) {
  int dw = ASPECT*h;
  if (w < dw) {
    int dh = w/ASPECT;
    glViewport(0, (h + dh)/2.0, w, dh);
  } else {
    glViewport((w - dw)/2.0, 0, dw, h);
  }
}

static void render(void) {
  vector eye_pos;
  eye_pos.x = 0.0;
  eye_pos.y = 0.0;
  eye_pos.z = 1.0;
  render_frame(&MAIN_FRAME, &eye_pos, 0, 0);
  glutSwapBuffers();
  glClear( GL_COLOR_BUFFER_BIT );
  tick(ticks_expected());
}

// Individual setup functions:

// Initialize the GLUT context:
static void init_context(int* argc, char** argv) {
  glutInit( argc, argv );
}

static void set_mode(int mode) {
  glutInitDisplayMode( mode );
}

static void set_size(int width, int height) {
  glutInitWindowSize( width, height );
}

static void create_window(const char* name) {
  WINDOW = glutCreateWindow( name );
}

static void set_bg_color(float r, float g, float b, float a) {
  glClearColor( r, g, b, a );
}

static void activate_gfx_callbacks(void) {
  // Setup the glut callback functions:
  glutIdleFunc( idle );
  glutReshapeFunc( resize );
  glutDisplayFunc( render );
}

// Sets various OpenGL settings:
void glsettings() {
  glEnable( GL_CULL_FACE );
  glFrontFace( GL_CW );
  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glEnable( GL_STENCIL_TEST );
  glEnable( GL_TEXTURE_2D );
  glEnable( GL_DEPTH_TEST );
  glDepthFunc( GL_LESS );
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
    DEFAULT_MODE,
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
  int mode,
  int w,
  int h,
  const char* name,
  float r,
  float g,
  float b,
  float a
) {
  init_context(argc, argv);
  set_mode(mode);
  set_size(w, h);
  create_window(name);
  set_bg_color( r, g, b, a);
  glsettings();
  glperspective();
  activate_gfx_callbacks();
}

void cleanup(void) {
  cleanup_frame(&MAIN_FRAME);
}

void loop(void) {
  // Swap buffers and loop away:
  glutSwapBuffers();
  glutMainLoop();
}

void quit(void) {
  cleanup();
  glutDestroyWindow( WINDOW );
  exit(0);
}

void fail(int err) {
  cleanup();
  glutDestroyWindow(WINDOW);
  exit(err);
}

void clear_color_buffer(void) {
  glClear( GL_COLOR_BUFFER_BIT );
}

void clear_depth_buffer(void) {
  glClear( GL_DEPTH_BUFFER_BIT );
}

void clear_stencil_buffer(void) {
  glClear( GL_STENCIL_BUFFER_BIT );
}

void write_stencil_mode(void) {
  glColorMask(0, 0, 0, 0); // Don't update the color buffer
  glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE ); // Write to the stencil buffer
  glStencilFunc( GL_ALWAYS, 0x1, 0x1); // Ignore the stencil buffer
}

void stencil_mode(void) {
  glColorMask(1, 1, 1, 1); // Write to the color buffer
  glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP ); // Don't change the stencil buffer
  glStencilFunc( GL_EQUAL, 0x1, 0x1); // Respect the stencil buffer
}

void no_stencil_mode(void) {
  glColorMask(1, 1, 1, 1); // Write to the color buffer
  glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP ); // Don't change the stencil buffer
  glStencilFunc( GL_ALWAYS, 0x1, 0x1); // Ignore the stencil buffer
}
