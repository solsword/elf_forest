// pipeline.c
// Shader functionality.

#include <GLee.h>
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
  .gfile = "res/shaders/geom.pass.glsl",
  .ffile = "res/shaders/frag.rawcolor.glsl"
};

pipeline CELL_PIPELINE = {
  .vfile = "res/shaders/vert.default.glsl",
  .gfile = "res/shaders/geom.pass.glsl",
  .ffile = "res/shaders/frag.textured.glsl"
};

pipeline TEXT_PIPELINE = {
  .vfile = "res/shaders/vert.default.glsl",
  .gfile = "res/shaders/geom.pass.glsl",
  .ffile = "res/shaders/frag.txalpha.glsl"
};

/*********************
 * Private Functions *
 *********************/

static inline void show_shader_log(GLuint object) {
  GLint log_length;
  char *log;
  glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
  log = malloc(log_length);
  glGetShaderInfoLog(object, log_length, NULL, log);
  fprintf(stderr, "%s", log);
  free(log);
}

static inline void show_program_log(GLuint object) {
  GLint log_length;
  char *log;
  glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
  log = malloc(log_length);
  glGetProgramInfoLog(object, log_length, NULL, log);
  fprintf(stderr, "%s", log);
  free(log);
}

/***************************
 * Setup/Cleanup Functions *
 ***************************/

void setup_shaders() {
  setup_pipeline(&RAW_PIPELINE);
  setup_pipeline(&CELL_PIPELINE);
  setup_pipeline(&TEXT_PIPELINE);
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

  glAttachShader(p->program, p->vert);
  if (p->geom) {
    glAttachShader(p->program, p->geom);
  }
  glAttachShader(p->program, p->frag);

  glLinkProgram(p->program);

  check_program(p->program);

  // If the "texture" uniform exists, set it up to use texture unit 0.
  GLint txloc = glGetUniformLocation(p->program, "texture");
  if (txloc != -1) {
    glUniform1i(txloc, 0);
    // Set the OpenGL active texture to 0.
    glActiveTexture( GL_TEXTURE0 );
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
}

GLuint load_shader(GLenum type, const char * const filename) {
  size_t filesize;
  GLint sourcesize;
  GLchar *source;
  GLuint shader;
  GLint shader_ok;

  source = load_file(filename, &filesize);
  if (!source) {
    fprintf(stderr, "Error: could not find shader file '%s'!\n", filename);
    exit(1);
  }
  sourcesize = filesize;
  shader = glCreateShader(type);
  glShaderSource(shader, 1, (GLchar const **) &source, &sourcesize);
  free(source);
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);
  if (!shader_ok) {
      fprintf(stderr, "Failed to compile shader '%s':\n", filename);
      show_shader_log(shader);
      glDeleteShader(shader);
      exit(1);
  }
  return shader;
}

void check_program(GLuint program) {
  GLint program_ok;

  glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
  if (!program_ok) {
      fprintf(stderr, "Failed to link shader program:\n");
      show_program_log(program);
      glDeleteProgram(program);
      exit(1);
  }
}
