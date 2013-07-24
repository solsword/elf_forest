#ifndef VBO_H
#define VBO_H

// vbo.h
// Vertex buffer objects.

#include <stdint.h>

#include <GL/gl.h>

/**********************
 * Structures & Types *
 **********************/

// An index into a vertex buffer.
typedef uint32_t vb_index;

// All of the data needed to define a vertex: 3D position, 3D normal, and s and
// t texture coordinates, each stored as a 16-bit unsigned int.
struct vertex_s;
typedef struct vertex_s vertex;

// An abstraction of OpenGL's buffer objects. Uses 
struct vertex_buffer_s;
typedef struct vertex_buffer_s vertex_buffer;

/*************************
 * Structure Definitions *
 *************************/

struct vertex_s {
  GLshort x, y, z;
  GLshort nx, ny, nz;
  GLshort s, t;
};

struct vertex_buffer_s {
  vertex *vdata; // The vertex data cache (temporary storage).
  vb_index *idata; // The index data cache (temporary storage).
  uint8_t allocated; // Whether the data caches are allocated or not.
  GLuint vertices; // The vertices buffer.
  GLuint indices; // The indices buffer.
  // # of vertices in the data array:
  vb_index stored_vertex_count;
  // # of vertices in the shape (some may be referenced more than once by the
  // index list):
  vb_index vertex_count;
};

/*************
 * Functions *
 *************/

// Sets up the data cache of the given buffer. Should be called before any
// calls to add_vertex or reuse_vertex. If the buffer is already allocated,
// this will free the old buffer and set up a new one. This function also
// deletes (if necessary) and unbinds the OpenGL arrays associated with the
// given vertex buffer object.
void setup_cache(vb_index vsize, vb_index isize, vertex_buffer *buf);

// Copies the given vertex into the given buffer's data cache.
void add_vertex(const vertex *v, vertex_buffer *buf);

// Reuses an existing vertex from the data cache. index can either be positive
// (an absolute index) or negative (an offset backwards in the index list from
// the last given index, with -1 being that last index).
void reuse_vertex(int i, vertex_buffer *buf);

// Compiles the given vertex buffer into a pair of OpenGL array objects,
// freeing the data cache after doing so. If the buffer already has OpenGL
// array(s) associated with it, these will be deleted first.
void compile_buffers(vertex_buffer *buf);

// Frees memory and deletes OpenGL array objects associated with the given
// vertex buffer.
void cleanup_vertex_buffer(vertex_buffer *vb);

// Draws the given vertex buffer with the given texture bound (unless txid is
// 0, in which case no texture is used). Should only be called after
// compile_buffers().
void draw_vertex_buffer(vertex_buffer *buf, GLuint txid);

#endif //ifndef VBO_H
