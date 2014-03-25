#ifndef GFX_H
#define GFX_H

// gfx.h
// Graphics environment wrangling.

#include <math.h>

#include <GLFW/glfw3.h>

/***************
 * Global vars *
 ***************/

// Stores the window ID when that gets set up.
extern GLFWwindow * WINDOW;

// Should we render to the screen or not?
extern int RENDER;

// The current width/height of the window.
extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;

// Default arguments to prepare(...)
extern int const DEFAULT_WIDTH;
extern int const DEFAULT_HEIGHT;
extern char const * const DEFAULT_NAME;
extern float const DEFAULT_R;
extern float const DEFAULT_G;
extern float const DEFAULT_B;
extern float const DEFAULT_A;

// Camera parameters
extern double const FOV;
extern double const ASPECT;
extern double const NEAR;
extern double const FAR;

/********************
 * Inline Functions *
 ********************/

static inline void set_bg_color(float r, float g, float b, float a) {
  float color[4];
  color[0] = r; color[1] = g; color[2] = b; color[3] = a;
  glClearColor( r, g, b, a );
  glFogfv( GL_FOG_COLOR, color );
  glClearColor( r, g, b, a );
}

static inline void clear_color_buffer(void) {
  glClear( GL_COLOR_BUFFER_BIT );
}

static inline void clear_depth_buffer(void) {
  glClear( GL_DEPTH_BUFFER_BIT );
}

static inline void clear_stencil_buffer(void) {
  glClear( GL_STENCIL_BUFFER_BIT );
}

static inline void write_stencil_mode(void) {
  glColorMask(0, 0, 0, 0); // Don't update the color buffer
  glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE ); // Write to the stencil buffer
  glStencilFunc( GL_ALWAYS, 0x1, 0x1); // Ignore the stencil buffer
}

static inline void stencil_mode(void) {
  glColorMask(1, 1, 1, 1); // Write to the color buffer
  glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP ); // Don't change the stencil buffer
  glStencilFunc( GL_EQUAL, 0x1, 0x1); // Respect the stencil buffer
}

static inline void no_stencil_mode(void) {
  glColorMask(1, 1, 1, 1); // Write to the color buffer
  glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP ); // Don't change the stencil buffer
  glStencilFunc( GL_ALWAYS, 0x1, 0x1); // Ignore the stencil buffer
}

static inline void set_fog_density(float density) {
  glFogf( GL_FOG_DENSITY, density );
}

/*************
 * Functions *
 *************/

// Breaks out of the main loop and exits the program.
void quit(void);

// Breaks out of the main loop and exits the program, setting the return code
// as given.
void fail(int err);

// Call after prepare(...); starts the main loop.
void loop(void);

// Set up the window using the defaults declared in gfx.h.
void prepare_default(int* argc, char** argv);

// Set up the window including title, width, height, and background color.
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
);

// Cleans up resources before exiting. Called by quit() and fail();
void cleanup(void);

#endif // ifndef GFX_H
