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

pipeline MAIN_PIPELINE;

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

/*************
 * Functions *
 *************/

void init_shaders() {
  MAIN_PIPELINE.vert = load_shader(GL_VERTEX_SHADER, "res/shaders/vert.glsl");
  MAIN_PIPELINE.geom = load_shader(
    GL_GEOMETRY_SHADER_ARB,
    "res/shaders/geom.glsl"
  );
  MAIN_PIPELINE.frag = load_shader(GL_FRAGMENT_SHADER, "res/shaders/frag.glsl");
  MAIN_PIPELINE.program = link_program(
    MAIN_PIPELINE.vert,
    MAIN_PIPELINE.geom,
    MAIN_PIPELINE.frag
  );
  glUseProgram(MAIN_PIPELINE.program);
  // Set up the "texture" uniform to use texture unit 0.
  GLint txloc = glGetUniformLocation(MAIN_PIPELINE.program, "texture");
  glUniform1i(txloc, 0);
  glActiveTexture( GL_TEXTURE0 );
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
      return 0;
  }
  return shader;
}

GLuint link_program(
  GLuint geometry_shader,
  GLuint vertex_shader,
  GLuint fragment_shader
) {
  GLint program_ok;

  GLuint program = glCreateProgram();

  glAttachShader(program, vertex_shader);
  glAttachShader(program, geometry_shader);
  glAttachShader(program, fragment_shader);

  //glProgramParameteri(program, GL_GEOMETRY_INPUT_TYPE, GL_POINTS);
  //glProgramParameteri(program, GL_GEOMETRY_OUTPUT_TYPE, GL_TRIANGLES);

  glLinkProgram(program);

  glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
  if (!program_ok) {
      fprintf(stderr, "Failed to link shader program:\n");
      show_program_log(program);
      glDeleteProgram(program);
      exit(1);
  }
  return program;
}
