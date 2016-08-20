#include <stdio.h>

#include <GL/glew.h>
#include <GL/gl.h>

#include <GLFW/glfw3.h>

#include "tex/dta.h"
#include "tex/tex.h"

int main(int argc, char **argv) {
  GLFWwindow *window;
  GLint mtxs;
  int status;
  GLenum err;

  printf("Checking OpenGL features...\n");

  glfwInit();
  window = glfwCreateWindow( 10, 10, "GLEW Version Test", NULL, NULL );
  glfwMakeContextCurrent(window);
  err = glewInit();

  if (err != GLEW_OK) {
    printf("Failed to initialize GLEW:\n");
    printf("  %s\n", glewGetErrorString(err));
    return 2;
  }

  status = EXIT_SUCCESS;
  if (GLEW_VERSION_1_2) {
    printf("OpenGL core version 1.2 supported.\n");
  } else {
    printf("### No support for OpenGL core version 1.2.\n");
    status = EXIT_FAILURE;
  }
  if (GLEW_VERSION_1_3) {
    printf("OpenGL core version 1.3 supported.\n");
  } else {
    printf("### No support for OpenGL core version 1.3.\n");
    status = EXIT_FAILURE;
  }
  if (GLEW_VERSION_1_4) {
    printf("OpenGL core version 1.4 supported.\n");
  } else {
    printf("### No support for OpenGL core version 1.4.\n");
    status = EXIT_FAILURE;
  }
  if (GLEW_VERSION_1_5) {
    printf("OpenGL core version 1.5 supported.\n");
  } else {
    printf("### No support for OpenGL core version 1.5.\n");
    status = EXIT_FAILURE;
  }
  if (GLEW_VERSION_2_0) {
    printf("OpenGL core version 2.0 supported.\n");
  } else {
    printf("### No support for OpenGL core version 2.0.\n");
    status = EXIT_FAILURE;
  }
  if (GLEW_VERSION_2_1) {
    printf("OpenGL core version 2.1 supported.\n");
  } else {
    printf("### No support for OpenGL core version 2.1.\n");
    status = EXIT_FAILURE;
  }
  if (GLEW_VERSION_3_0) {
    printf("OpenGL core version 3.0 supported.\n");
  } else {
    printf("### No support for OpenGL core version 3.0.\n");
    status = EXIT_FAILURE;
  }
  if (GLEW_VERSION_4_0) {
    printf("OpenGL core version 4.0 supported.\n");
  } else {
    printf("### No support for OpenGL core version 4.0.\n");
    status = EXIT_FAILURE;
  }
  if (GLEW_VERSION_4_5) {
    printf("*** OpenGL core version 4.5 supported.\n");
  }
  if (GLEW_ARB_vertex_buffer_object) {
    printf("OpenGL vertex buffer object ARB supported.\n");
  } else {
    printf("### No support for OpenGL vertex buffer object ARB.\n");
    status = EXIT_FAILURE;
  }
  if (GLEW_ARB_geometry_shader4) {
    printf("OpenGL geometry shader ARB supported.\n");
  } else {
    printf("### No support for OpenGL geometry shader ARB.\n");
    status = EXIT_FAILURE;
  }
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &mtxs);
  if (mtxs >= BLOCK_TEXTURE_SIZE * DYNAMIC_ATLAS_SIZE) {
    printf(
      "OpenGL max texture size is sufficient (%d >= %ld).\n",
      mtxs,
      BLOCK_TEXTURE_SIZE * DYNAMIC_ATLAS_SIZE
    );
  } else {
    printf(
      "### OpenGL max texture size is too small (%d < %ld).\n",
      mtxs,
      BLOCK_TEXTURE_SIZE * DYNAMIC_ATLAS_SIZE
    );
    status = EXIT_FAILURE;
  }
  glfwTerminate();
  return status;
}
