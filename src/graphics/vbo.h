#ifndef VBO_H
#define VBO_H

// vbo.h
// Vertex buffer objects.

#include <stdint.h>

#include <GL/gl.h>

#include "util.h"

#include "datatypes/list.h"

/**********************
 * Structures & Types *
 **********************/

// An index into a vertex buffer.
typedef GLushort vb_index;

// All of the data needed to define a vertex: 3D position, 3D normal, s and t
// texture coordinates, and brightness.
struct vertex_s;
typedef struct vertex_s vertex;

// An abstraction of OpenGL's buffer objects. Uses an index buffer along with a
// vertex buffer.
struct vertex_buffer_s;
typedef struct vertex_buffer_s vertex_buffer;

/*************
 * Constants *
 *************/

// Maximum number of indices in a vertex/index buffer.
static vb_index const MAX_INDICES = 3*(umaxof(vb_index)/4);

/*************************
 * Structure Definitions *
 *************************/

struct vertex_s {
  GLfloat x, y, z;
  GLfloat s, t;
  GLshort nx, ny, nz;
  GLubyte brightness, _1, _2; // TODO: Use these extra bytes
};

struct vertex_buffer_s {
  vertex *vdata; // The vertex data cache (temporary storage).
  vb_index *idata; // The index data cache (temporary storage).
  uint8_t allocated; // Whether the data caches are allocated or not.
  vb_index vdata_size; // size of the vertex data cache (in vertices).
  vb_index idata_size; // size of the index data cache (in indices).
  list *vbuffers; // The vertex buffer handle(s) on the GPU.
  list *ibuffers; // The index buffer handle(s) on the GPU.
  list *vcounts; // The list of vertex counts for the GPU buffers.
  // # of vertices in the data array:
  vb_index vertex_count;
  // # of indices in the index array:
  vb_index index_count;
};

/****************************
 * Constructors/Destructors *
 ****************************/

// Sets up the given vertex buffer, but just does minimal initialization.
void setup_vertex_buffer(vertex_buffer *vb);

// Allocates and returns a new vertex buffer.
vertex_buffer* create_vertex_buffer();

// Frees memory and deletes OpenGL array objects associated with the given
// vertex buffer. Does not free the buffer itself.
void cleanup_vertex_buffer(vertex_buffer *vb);

// Frees memory internal to the given vertex buffer and resets it to a blank
// state.
void reset_vertex_buffer(vertex_buffer *vb);

/*************
 * Functions *
 *************/

// Sets up the data cache of the given buffer. Should be called before any
// calls to add_vertex or reuse_vertex. If the buffer is already allocated,
// this will free the old buffer and set up a new one. This function also
// deletes (if necessary) and unbinds the OpenGL arrays associated with the
// given vertex buffer object.
void vb_setup_cache(vertex_buffer *buf);

// Copies the given vertex into the given buffer's data cache.
void vb_add_vertex(vertex const * const v, vertex_buffer *buf);

// Reuses an existing vertex from the data cache. index is an offset backwards
// in the index list from the last given index, with 0 being that last index).
// Note that the data is periodically reset, and if a vertex would be reused
// from the reset data it gets re-added instead. Reuses longer than 1/2
// MAX_INDICES might corrupt the vertex caches.
void vb_reuse_vertex(int i, vertex_buffer *buf);

// Compiles the given vertex buffer into a pair of OpenGL array objects,
// freeing the data cache after doing so. If the buffer already has OpenGL
// array(s) associated with it, these will be deleted first.
void vb_compile_buffers(vertex_buffer *buf);

// Frees the data cache of the given vertex buffer. Can safely be called
// immediately after vb_compile_buffers without losing data (although vertex
// reuse will be impossible after the cache is freed even once it's set up
// again).
void vb_free_cache(vertex_buffer *vb);

// Draws the given vertex buffer with the given texture bound (unless txid is
// 0, in which case no texture is used). Should only be called after
// vb_compile_buffers().
void draw_vertex_buffer(vertex_buffer *buf, GLuint txid);

#endif //ifndef VBO_H
