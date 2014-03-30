// vbo.c
// Vertex buffer objects.

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#include <GLee.h> // glDeleteBuffers etc.

#include "vbo.h"

/*************
 * Functions *
 *************/

void setup_cache(vb_index vsize, vb_index isize, vertex_buffer *vb) {
  if (vb->vertices != 0) {
    glDeleteBuffers(1, &(vb->vertices));
    vb->vertices = 0;
  }
  if (vb->indices != 0) {
    glDeleteBuffers(1, &(vb->indices));
    vb->indices = 0;
  }
  if (vb->allocated) {
    free(vb->vdata);
    free(vb->idata);
    vb->allocated = 0;
  }
  vb->vdata = (vertex *) malloc(vsize*sizeof(vertex));
  if (vb->vdata == NULL) {
    perror("Failed to allocate vertex data cache.");
    exit(errno);
  }
  vb->idata = (vb_index *) malloc(isize*sizeof(vb_index));
  if (vb->idata == NULL) {
    perror("Failed to allocate vertex indices cache.");
    exit(errno);
  }
  vb->stored_vertex_count = 0;
  vb->vertex_count = 0;
  vb->allocated = 1;
}

void add_vertex(vertex const * const v, vertex_buffer *vb) {
  assert(vb->allocated);
  if (vb->allocated != 1) {
  }
  vb->vdata[vb->stored_vertex_count].x = v->x;
  vb->vdata[vb->stored_vertex_count].y = v->y;
  vb->vdata[vb->stored_vertex_count].z = v->z;
  vb->vdata[vb->stored_vertex_count].nx = v->nx;
  vb->vdata[vb->stored_vertex_count].ny = v->ny;
  vb->vdata[vb->stored_vertex_count].nz = v->nz;
  vb->vdata[vb->stored_vertex_count].s = v->s;
  vb->vdata[vb->stored_vertex_count].t = v->t;
  vb->idata[vb->vertex_count] = vb->stored_vertex_count;
  vb->vertex_count += 1;
  vb->stored_vertex_count += 1;
}

void reuse_vertex(int i, vertex_buffer *vb) {
  assert(vb->allocated);
  if (i >= 0) {
    vb->idata[vb->vertex_count] = (vb_index) i;
  } else {
    vb->idata[vb->vertex_count] = vb->idata[vb->vertex_count + i];
  }
  vb->vertex_count += 1;
}

void compile_buffers(vertex_buffer *vb) {
  assert(vb->allocated);
  // Clear out any old OpenGL buffers:
  if (vb->vertices != 0) {
    glDeleteBuffers(1, &(vb->vertices));
    vb->vertices = 0;
  }
  if (vb->indices != 0) {
    glDeleteBuffers(1, &(vb->indices));
    vb->indices = 0;
  }
  // Generate new buffers:
  glGenBuffers(1, &(vb->vertices));
  glGenBuffers(1, &(vb->indices));

  // Bind them and read from our caches:
  glBindBuffer( GL_ARRAY_BUFFER, vb->vertices );
  glBufferData(
    GL_ARRAY_BUFFER,
    sizeof(vertex) * vb->stored_vertex_count,
    (GLvoid const *) vb->vdata,
    GL_STATIC_DRAW
  );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );

  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vb->indices );
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER,
    sizeof(vb_index) * vb->vertex_count,
    (GLvoid const *) vb->idata,
    GL_STATIC_DRAW
  );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

  // Free our data cache:
  free(vb->vdata);
  free(vb->idata);
  vb->allocated = 0;
}

void setup_vertex_buffer(vertex_buffer *vb) {
  vb->allocated = 0;
  vb->vertices = 0;
  vb->indices = 0;
  vb->stored_vertex_count = 0;
  vb->vertex_count = 0;
}

void cleanup_vertex_buffer(vertex_buffer *vb) {
  if (vb->allocated) {
    free(vb->vdata);
    free(vb->idata);
    vb->allocated = 0;
  }
  if (vb->vertices != 0) {
    glDeleteBuffers(1, &(vb->vertices));
    vb->vertices = 0;
  }
  if (vb->indices != 0) {
    glDeleteBuffers(1, &(vb->indices));
    vb->indices = 0;
  }
  vb->stored_vertex_count = 0;
  vb->vertex_count = 0;
}

void draw_vertex_buffer(vertex_buffer *vb, GLuint txid) {
  // Check for an empty buffer:
  if (vb->vertices == 0 || vb->indices == 0) {
    return;
  }

  // Enable array functionality:
  glDisableClientState( GL_COLOR_ARRAY );
  glEnableClientState( GL_VERTEX_ARRAY );
  glEnableClientState( GL_NORMAL_ARRAY );
  //glDisableClientState( GL_NORMAL_ARRAY );
  glEnableClientState( GL_TEXTURE_COORD_ARRAY );

  // Bind the given texture:
  glBindTexture( GL_TEXTURE_2D, txid );

  // Bind the buffer object holding vertex data:
  glBindBuffer( GL_ARRAY_BUFFER, vb->vertices );

  // Set the vertex/normal/texture data strides & offsets:
  glVertexPointer(
    3,
    GL_SHORT,
    sizeof(vertex),
    (GLvoid const *)0
  );
  //*
  glNormalPointer(
    GL_SHORT,
    sizeof(vertex),
    (GLvoid const *) (3*sizeof(GLshort))
  );
  // */
  // DEBUG!
  //glNormal3f(0.0, 0.0, 1.0);
  glTexCoordPointer(
    2,
    GL_SHORT,
    sizeof(vertex),
    (GLvoid const *) (6*sizeof(GLshort))
  );

  // Bind the buffer object holding index data:
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vb->indices );

  // Draw the entire index array as triangles:
  glDrawElements( GL_TRIANGLES, vb->vertex_count, GL_UNSIGNED_INT, 0 );

  // Unbind the buffers:
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

  // Unbind our texture:
  glBindTexture( GL_TEXTURE_2D, 0 );

  // Disable the array functionality:
  glDisableClientState( GL_COLOR_ARRAY );
  glDisableClientState( GL_VERTEX_ARRAY );
  glDisableClientState( GL_NORMAL_ARRAY );
  glDisableClientState( GL_TEXTURE_COORD_ARRAY );
}
