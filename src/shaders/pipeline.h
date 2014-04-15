#ifndef PIPELINE_H
#define PIPELINE_H

// pipeline.h
// Shader functionality.

#include <GL/gl.h>

/**************
 * Structures *
 **************/

// A pipeline contains handles to a complete set of shaders (geometry, vertex,
// and fragment) used for rendering, as well as the combined program handle.
struct pipeline_s;
typedef struct pipeline_s pipeline;

/********************
 * Global variables *
 ********************/

extern pipeline RAW_PIPELINE;
extern pipeline CELL_PIPELINE;
extern pipeline TEXT_PIPELINE;

/*************************
 * Structure Definitions *
 *************************/

struct pipeline_s {
  char * vfile;
  char * gfile;
  char * ffile;
  GLuint vert;
  GLuint geom;
  GLuint frag;
  GLuint program;
};

/***************************
 * Setup/Cleanup Functions *
 ***************************/

// Note: core functions cribbed from:
// duriansoftware.com/joe/An-intro-to-modern-OpenGL.-Chapter-2.2:-Shaders.html

// Initializes the shaders subsystem, loading the default shaders onto the
// graphics card.
void setup_shaders(void);

// Cleans up the shaders subsystem.
void cleanup_shaders(void);

// Sets up the given pipeline, loading the shaders that it indicates, compiling
// them, and then linking them into a program.
void setup_pipeline(pipeline *p);

// Cleans up the given pipeline, freeing associated GPU resources.
void cleanup_pipeline(pipeline *p);

/*************
 * Functions *
 *************/

// Activates the given pipeline for rendering.
void use_pipeline(pipeline *p);

// Loads a shader from the given filename as the given shader type and returns
// a handle to it. If the shader fails to compile, it prints an error message
// and exits.
GLuint load_shader(GLenum type, const char * const filename);

// Checks that the given program was linked successfully and exits printing an
// error message if not.
void check_program(GLuint program);

#endif //ifndef PIPELINE_H
