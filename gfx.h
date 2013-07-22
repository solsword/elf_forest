#ifndef GFX_H
#define GFX_H

#include <math.h>

#include <GL/glut.h>

/***************
 * Global vars *
 ***************/

// Stores the window ID when that gets set up.
extern int WINDOW;

// Default arguments to prepare(...)
extern const int DEFAULT_MODE;
extern const int DEFAULT_WIDTH;
extern const int DEFAULT_HEIGHT;
extern const char* DEFAULT_NAME;
extern const float DEFAULT_R;
extern const float DEFAULT_G;
extern const float DEFAULT_B;
extern const float DEFAULT_A;

// Camera parameters
extern const double FOV;
extern const double ASPECT;
extern const double NEAR;
extern const double FAR;

/*************
 * Functions *
 *************/

// Breaks out of the GLUT main loop and exits the program.
void quit(void);

// Breaks out of the GLUT main loop and exits the program, setting the return
// code as given.
void fail(int err);

// Call after prepare(...); starts the GLUT main loop.
void loop(void);

// Set up the GLUT window using the defaults declared in gfx.h.
void prepare_default(int* argc, char** argv);

// Set up the GLUT window including mode, width, height, and background color.
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
);

// Cleans up resources before exiting. Called by quit() and fail();
void cleanup(void);

// Clears the OpenGL color buffer.
void clear_color_buffer(void);

// Clears the OpenGL depth buffer.
void clear_depth_buffer(void);

// Clears the OpenGL stencil buffer.
void clear_stencil_buffer(void);

// Sets up OpenGL to draw into the stencil buffer.
void write_stencil_mode(void);

// Sets up OpenGL to draw using the stencil buffer.
void stencil_mode(void);

// Sets up OpenGL to draw into the normal color buffer.
void no_stencil_mode(void);

#endif // ifndef GFX_H
