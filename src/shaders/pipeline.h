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

extern pipeline MAIN_PIPELINE;

/*************************
 * Structure Definitions *
 *************************/

struct pipeline_s {
  GLuint vert;
  GLuint geom;
  GLuint frag;
  GLuint program;
};

/*************
 * Functions *
 *************/

// Note: core functions cribbed from:
// duriansoftware.com/joe/An-intro-to-modern-OpenGL.-Chapter-2.2:-Shaders.html

// Initializes the shaders subsystem, loading the default shaders onto the
// graphics card (no cleanup necessary).
void init_shaders();

// TODO: cleanup_shaders which will destroy the shader program

// Loads a shader from the given filename and returns a handle to it.
GLuint load_shader(GLenum type, const char * const filename);

// Links the three given shaders into a program and returns a handle to it.
GLuint link_program(
  GLuint vertex_shader,
  GLuint geometry_shader,
  GLuint fragment_shader
);
#endif //ifndef PIPELINE_H
