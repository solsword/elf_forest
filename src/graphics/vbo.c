// vbo.c
// Vertex buffer objects.

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#include <GL/glew.h> // glDeleteBuffers etc.

#include "datatypes/list.h"

#include "vbo.h"

/****************************
 * Constructors/Destructors *
 ****************************/

void setup_vertex_buffer(vertex_buffer *vb) {
  vb->vdata = NULL;
  vb->idata = NULL;
  vb->allocated = 0;
  vb->vdata_size = 0;
  vb->idata_size = 0;
  vb->vbuffers = create_list();
  vb->ibuffers = create_list();
  vb->vcounts = create_list();
  vb->vertex_count = 0;
  vb->index_count = 0;
}

vertex_buffer* create_vertex_buffer(void) {
  vertex_buffer *result = (vertex_buffer*) malloc(sizeof(vertex_buffer));
  setup_vertex_buffer(result);
  return result;
}

void delete_gl_buffer_in_list(void *entry) {
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
  // TODO: Fix this!
  glDeleteBuffers(1, (GLuint) entry);
#pragma GCC diagnostic warning "-Wpointer-to-int-cast"
}

void cleanup_vertex_buffer(vertex_buffer *vb) {
  if (vb->allocated) {
    free(vb->vdata);
    free(vb->idata);
    vb->allocated = 0;
  }
  vb->vdata_size = 0;
  vb->idata_size = 0;
  vb->vertex_count = 0;
  vb->index_count = 0;
  l_foreach(vb->vbuffers, &delete_gl_buffer_in_list);
  l_foreach(vb->ibuffers, &delete_gl_buffer_in_list);
  cleanup_list(vb->vbuffers);
  cleanup_list(vb->ibuffers);
  cleanup_list(vb->vcounts);
}

void reset_vertex_buffer(vertex_buffer *vb) {
  cleanup_vertex_buffer(vb);
  vb->vbuffers = create_list();
  vb->ibuffers = create_list();
  vb->vcounts = create_list();
}

/*************
 * Functions *
 *************/

void vb_setup_cache(vertex_buffer *vb) {
  if (vb->allocated) {
    free(vb->vdata);
    free(vb->idata);
    vb->allocated = 0;
  }
  vb->vdata_size = MAX_INDICES;
  vb->vdata = (vertex *) malloc(MAX_INDICES*sizeof(vertex));
  if (vb->vdata == NULL) {
    perror("Failed to allocate vertex data cache");
    exit(errno);
  }
  vb->idata_size = MAX_INDICES;
  vb->idata = (vb_index *) malloc(MAX_INDICES*sizeof(vb_index));
  if (vb->idata == NULL) {
    perror("Failed to allocate vertex indices cache");
    exit(errno);
  }
  vb->vertex_count = 0;
  vb->index_count = 0;
  vb->allocated = 1;
}

void vb_add_vertex(vertex const * const v, vertex_buffer *vb) {
  assert(vb->allocated);
  if (vb->vertex_count == vb->vdata_size || vb->index_count == vb->idata_size) {
    // Compile what we've got and reset our data buffer.
    vb_compile_buffers(vb);
  }
  vb->vdata[vb->vertex_count].x = v->x;
  vb->vdata[vb->vertex_count].y = v->y;
  vb->vdata[vb->vertex_count].z = v->z;
  vb->vdata[vb->vertex_count].nx = v->nx;
  vb->vdata[vb->vertex_count].ny = v->ny;
  vb->vdata[vb->vertex_count].nz = v->nz;
  vb->vdata[vb->vertex_count].s = v->s;
  vb->vdata[vb->vertex_count].t = v->t;
  vb->idata[vb->index_count] = vb->vertex_count;
  vb->index_count += 1;
  vb->vertex_count += 1;
}

void vb_reuse_vertex(int i, vertex_buffer *vb) {
  assert(vb->allocated);
  if (vb->index_count == vb->idata_size) {
    vb_compile_buffers(vb);
  }
  if (((int) vb->index_count) - i - 1 < 0) {
    // We need to re-add an old vertex:
    vertex *v = &(
      vb->vdata[vb->idata[vb->idata_size + vb->index_count - i - 1]]
    );
    vb_add_vertex(v, vb);
  } else {
    // Just look up the index we want and add it:
    vb->idata[vb->index_count] = vb->idata[vb->index_count - i - 1];
    vb->index_count += 1;
  }
}

void vb_compile_buffers(vertex_buffer *vb) {
  assert(vb->allocated);
  GLuint vertex_buffer;
  GLuint index_buffer;
  // Generate new buffers:
  glGenBuffers(1, &vertex_buffer);
  glGenBuffers(1, &index_buffer);

  // Bind them and read from our caches:
  glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer );
  glBufferData(
    GL_ARRAY_BUFFER,
    sizeof(vertex) * vb->vertex_count,
    (GLvoid const *) vb->vdata,
    GL_STATIC_DRAW
  );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );

  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, index_buffer );
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER,
    sizeof(vb_index) * vb->index_count,
    (GLvoid const *) vb->idata,
    GL_STATIC_DRAW
  );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

  // Add the newly-created GPU-side buffers to our list:
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
  l_append_element(vb->vbuffers, (void*) vertex_buffer);
  l_append_element(vb->ibuffers, (void*) index_buffer);
  l_append_element(vb->vcounts, (void*) (vb->index_count));
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"

  // Reset the cache state:
  vb->vertex_count = 0;
  vb->index_count = 0;
}

void vb_free_cache(vertex_buffer *vb) {
  free(vb->vdata);
  free(vb->idata);
  vb->allocated = 0;
}

void draw_vertex_buffer(vertex_buffer *vb, GLuint txid) {
  size_t i = 0;
  vb_index count;

  // Check for an empty buffer:
  if (l_is_empty(vb->vcounts)) {
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

  // Iterate through our list of vertex buffers:
  for (i = 0; i < l_get_length(vb->vcounts); ++i) {
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
    count = (vb_index) l_get_item(vb->vcounts, i);
#pragma GCC diagnostic warning "-Wpointer-to-int-cast"

    // Check for an empty buffer:
    if (count == 0) {
      continue;
    }

    // Bind the buffer object holding vertex data:
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
    glBindBuffer( GL_ARRAY_BUFFER, (GLuint) l_get_item(vb->vbuffers, i) );
#pragma GCC diagnostic warning "-Wpointer-to-int-cast"

    // Set the vertex/normal/texture data strides & offsets:
    glVertexPointer(
      3,
      GL_FLOAT,
      sizeof(vertex),
      (GLvoid const *)0
    );
    glTexCoordPointer(
      2,
      GL_FLOAT,
      sizeof(vertex),
      (GLvoid const *) (3*sizeof(GLfloat))
    );
    //*
    glNormalPointer(
      GL_SHORT,
      sizeof(vertex),
      (GLvoid const *) (5*sizeof(GLfloat))
    );
    // */
    // DEBUG!
    //glNormal3f(0.0, 0.0, 1.0);

    // Bind the buffer object holding index data:
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
    glBindBuffer(
      GL_ELEMENT_ARRAY_BUFFER,
      (GLuint) l_get_item(vb->ibuffers, i)
    );
#pragma GCC diagnostic warning "-Wpointer-to-int-cast"

    // Draw the entire index array as triangles:
    glDrawElements( GL_TRIANGLES, count, GL_UNSIGNED_SHORT, 0 );

    // Unbind the buffers:
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
  }

  // Unbind our texture:
  glBindTexture( GL_TEXTURE_2D, 0 );

  // Disable the array functionality:
  glDisableClientState( GL_COLOR_ARRAY );
  glDisableClientState( GL_VERTEX_ARRAY );
  glDisableClientState( GL_NORMAL_ARRAY );
  glDisableClientState( GL_TEXTURE_COORD_ARRAY );
}
