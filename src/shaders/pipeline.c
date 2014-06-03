// pipeline.c
// Shader functionality.

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "pipeline.h"
#include "util.h"

/********************
 * Global variables *
 ********************/

pipeline RAW_PIPELINE = {
  .vfile = "res/shaders/vert.default.glsl",
  .gfile = NULL,
  .ffile = "res/shaders/frag.rawcolor.glsl"
};

pipeline CELL_PIPELINE = {
  .vfile = "res/shaders/vert.default.glsl",
  .gfile = NULL,
  .ffile = "res/shaders/frag.textured.glsl"
};

pipeline TEXT_PIPELINE = {
  .vfile = "res/shaders/vert.default.glsl",
  .gfile = NULL,
  .ffile = "res/shaders/frag.txalpha.glsl"
};

/*********************
 * Private Functions *
 *********************/

static inline void show_shader_log(GLuint object) {
  GLint log_length = 1024;
  char *log;
  glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
  log = malloc(log_length);
  sprintf(log, "No log found.\n");
  glGetShaderInfoLog(object, log_length, NULL, log);
  fprintf(stderr, "%s", log);
  free(log);
}

static inline void show_program_log(GLuint object) {
  GLint log_length = 1024;
  char *log;
  glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
  log = malloc(log_length);
  sprintf(log, "No log found.\n");
  glGetProgramInfoLog(object, log_length, NULL, log);
  fprintf(stderr, "%s", log);
  free(log);
}

/***************************
 * Setup/Cleanup Functions *
 ***************************/

void setup_shaders() {
  if (report_opengl_error("Pre-shaders-setup error.\n")) { exit(1); }
  setup_pipeline(&RAW_PIPELINE);
  if (report_opengl_error("Raw pipeline error.\n")) { exit(1); }
  setup_pipeline(&CELL_PIPELINE);
  if (report_opengl_error("Cell pipeline error.\n")) { exit(1); }
  setup_pipeline(&TEXT_PIPELINE);
  if (report_opengl_error("Text pipeline error.\n")) { exit(1); }
}

void cleanup_shaders() {
  cleanup_pipeline(&RAW_PIPELINE);
  cleanup_pipeline(&CELL_PIPELINE);
  cleanup_pipeline(&TEXT_PIPELINE);
}

void setup_pipeline(pipeline *p) {
  p->vert = load_shader(GL_VERTEX_SHADER, p->vfile);
  if (p->gfile != NULL) {
    p->geom = load_shader(GL_GEOMETRY_SHADER_ARB, p->gfile);
  } else {
    p->geom = 0;
  }
  p->frag = load_shader(GL_FRAGMENT_SHADER, p->ffile);

  p->program = glCreateProgram();
  if (report_opengl_error("Program creation error.\n")) { exit(1); }

  glAttachShader(p->program, p->vert);
  if (report_opengl_error("Vertex shader attachment error.\n")) { exit(1); }
  if (p->geom) {
    glAttachShader(p->program, p->geom);
    if (report_opengl_error("Geometry shader attachment error.\n")) { exit(1); }
  }
  glAttachShader(p->program, p->frag);
  if (report_opengl_error("Fragment shader attachment error.\n")) { exit(1); }

  glLinkProgram(p->program);
  if (report_opengl_error("Program linking error.\n")) { exit(1); }

  check_program(p->program);

  // Bind the program as active in order to set up the texture uniform:
  glUseProgram(p->program);
  if (report_opengl_error("Unable to use fresh program.\n")) { exit(1); }

  // If the "texture" uniform exists, set it up to use texture unit 0.
  GLint txloc = glGetUniformLocation(p->program, "texture");
  if (report_opengl_error("Texture location error.\n")) { exit(1); }
  if (txloc != -1) {
    glUniform1i(txloc, 0);
    if (report_opengl_error("Texture setup error.\n")) { exit(1); }
    // Set the OpenGL active texture to 0.
    glActiveTexture( GL_TEXTURE0 );
    if (report_opengl_error("Texture activation error.\n")) { exit(1); }
  }
}

void cleanup_pipeline(pipeline *p) {
  glDeleteProgram(p->program);
  glDeleteShader(p->vert);
  glDeleteShader(p->geom);
  glDeleteShader(p->frag);
}

/*************
 * Functions *
 *************/

void use_pipeline(pipeline *p) {
  glUseProgram(p->program);
  if (report_opengl_error("Unable to use program.\n")) { exit(1); }
}

GLuint load_shader(GLenum type, const char * const filename) {
  size_t filesize;
  GLint sourcesize;
  GLchar *source;
  GLuint shader;
  GLint shader_ok = 0;

  if (report_opengl_error("Pre-loading error.\n")) { exit(1); }
  source = load_file(filename, &filesize);
  if (!source) {
    fprintf(stderr, "Error: could not find shader file '%s'!\n", filename);
    exit(1);
  }
  sourcesize = filesize;
  shader = glCreateShader(type);
  if (report_opengl_error("Shader creation error.\n")) { exit(1); }
  glShaderSource(shader, 1, (GLchar const **) &source, &sourcesize);
  if (report_opengl_error("Shader sourcing error.\n")) { exit(1); }
  free(source);
  glCompileShader(shader);
  if (report_opengl_error("Shader compilation error.\n")) { exit(1); }
  glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);
  if (shader_ok != GL_TRUE) {
    fprintf(stderr, "Failed to compile shader '%s':\n", filename);
    if (!report_opengl_error("Compile status check error.\n")) {
      show_shader_log(shader);
    }
    glDeleteShader(shader);
    exit(1);
  }
  return shader;
}

void check_program(GLuint program) {
  GLint program_ok;

  glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
  if (report_opengl_error("Link status reporting error.\n")) { exit(1); }
  if (!program_ok) {
      fprintf(stderr, "Failed to link shader program:\n");
      show_program_log(program);
      glDeleteProgram(program);
      exit(1);
  }
}
