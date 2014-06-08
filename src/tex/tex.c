// tex.c
// Texture loading and management.

#include <png.h>
#include <GL/glew.h> // glGenerateMipmaps

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "tex.h"

#include "dta.h"

#include "datatypes/bitmap.h"
#include "datatypes/map.h"

#include "prof/pmem.h"

#include "world/blocks.h"

/********************
 * Global variables *
 ********************/

char const * const BLOCK_TEXTURE_DIR = "res/textures/static";

/******************************
 * Constructors & Destructors *
 ******************************/

texture *create_texture(size_t width, size_t height) {
  texture * tx = (texture *) malloc(sizeof(texture));
  tx->width = width;
  tx->height = height;
  tx->pixels = (pixel *) calloc(width * height, sizeof(pixel));
#ifdef PROFILE_MEM
  md_add_size(&TEXTURE_RAM_USAGE, width*height*sizeof(pixel), sizeof(texture));
#endif
  return tx;
}

texture *duplicate_texture(texture *original) {
  texture * tx = (texture *) malloc(sizeof(texture));
  tx->width = original->width;
  tx->height = original->height;
  tx->pixels = (pixel *) calloc(tx->width * tx->height, sizeof(pixel));
  memcpy(
    tx->pixels,
    original->pixels,
    tx->width * tx->height * sizeof(pixel)
  );
#ifdef PROFILE_MEM
  md_add_size(
    &TEXTURE_RAM_USAGE,
    tx->width*tx->height*sizeof(pixel),
    sizeof(texture)
  );
#endif
  return tx;
}

void cleanup_texture(texture *tx) {
#ifdef PROFILE_MEM
  md_sub_size(
    &TEXTURE_RAM_USAGE,
    tx->width*tx->height*sizeof(pixel),
    sizeof(texture)
  );
#endif
  free(tx->pixels);
  free(tx);
}

/*************
 * Functions *
 *************/

void setup_textures(void) {
  size_t i;
  for (i = 0; i < N_LAYERS; ++i) {
    LAYER_ATLASES[i] = create_dynamic_atlas(DYNAMIC_ATLAS_SIZE);
  }
}

void cleanup_textures(void) {
  size_t i;
  for (i = 0; i < N_LAYERS; ++i) {
    cleanup_dynamic_atlas(LAYER_ATLASES[i]);
    LAYER_ATLASES[i] = NULL;
  }
}

texture* load_texture_from_png(char const * const filename) {
  FILE *fp;

  fp = fopen(filename, "rb");
  if (fp == NULL) {
    perror(filename);
    exit(errno);
  }

  png_structp png_ptr = png_create_read_struct(
    PNG_LIBPNG_VER_STRING,
    NULL, NULL, NULL // We won't worry about PNG errors for now.
  );
  if (png_ptr == NULL) {
    fprintf(stderr, "Failed to create PNG read struct.\n");
    exit(-1);
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL) {
    png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
    fprintf(stderr, "Failed to create PNG info struct.\n");
    exit(-1);
  }

  png_infop end_info = png_create_info_struct(png_ptr);
  if (end_info == NULL) {
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    fprintf(stderr, "Failed to create PNG end info struct.\n");
    exit(-1);
  }

  // Get ready to read data:
  png_init_io(png_ptr, fp);

  // Read the png data:
  png_read_png(
    png_ptr,
    info_ptr,
    PNG_TRANSFORM_SCALE_16 \
    | PNG_TRANSFORM_PACKING \
    | PNG_TRANSFORM_EXPAND,
    NULL
  );

  // Create our result texture:
  texture *result = (texture*) malloc(sizeof(texture));
  if (result == NULL) {
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    perror("Couldn't allocate space for texture info.");
    exit(errno);
  }

  // Get the size info:

  result->width = png_get_image_width(png_ptr, info_ptr);
  result->height = png_get_image_height(png_ptr, info_ptr);

  // Note that we don't care about bit depth, color type, or any of that stuff.
  // We assume that it'll be correct (32 bpp RGBA). The transforms that we did
  // when reading should help with this.

  // Find out the number of bytes/row:
  uint32_t row_bytes = png_get_rowbytes(png_ptr, info_ptr);

  // Malloc our pixels array:
#ifdef DEBUG
  if (row_bytes != sizeof(pixel)*result->width) {
    fprintf(
      stderr,
      "Error: PNG row size doesn't match width (non-RGBA-32 pixel format?).\n"
      "Fatal error loading file '%s'.\n",
      filename
    );
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    exit(1);
  }
#endif
  result->pixels = (pixel*) malloc(
    result->width * result->height * sizeof(pixel)
  );
  if (result->pixels == NULL) {
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    free(result);
    perror("Couldn't allocate space for texture pixels.");
    exit(errno);
  }

  // Ask libpng for the array of byte arrays that corresponds to the image data:
  png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);

  int i;
  for (i = 0; i < result->height; ++i) {
    memcpy(
      &(result->pixels[result->width * i]),
      row_pointers[i],
      result->width * sizeof(pixel)
    );
  }

  // Clean up:
  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
  fclose(fp);

  // Profile the allocated memory:
#ifdef PROFILE_MEM
  md_add_size(
    &TEXTURE_RAM_USAGE,
    result->width*result->height*sizeof(pixel),
    sizeof(texture)
  );
#endif

  return result;
}

void write_texture_to_ppm(texture *tx, char const * const filename) {
  FILE *fp;
  pixel px;
  int i, j;
  fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Error: couldn't open destination file '%s'.\n", filename);
    exit(1);
  }
  fprintf(fp, "P3\n");
  fprintf(fp, "# texture ppm\n");
  fprintf(fp, "# Generated from a texture by code in graphics/tex.c.\n");
  fprintf(fp, "%d %d\n", tx->width, tx->height);
  fprintf(fp, "255\n");
  for (i = 0; i < tx->width; ++i) {
    for (j = 0; j < tx->height; ++j) {
      px = tx_get_px(tx, i, j);
      fprintf(
        fp,
        "%3d %3d %3d ",
        px_red(px),
        px_green(px),
        px_blue(px)
      );
    }
    fprintf(fp, "\n");
  }
  fprintf(fp, "\n");
  fclose(fp);
}

void write_texture_to_png(texture *tx, char const * const filename) {
  FILE *fp;
  int i;
  png_structp png_ptr;
  png_infop info_ptr;
  // Get various file and/or PNG struct pointers:
  fp = fopen(filename, "wb");
  if (!fp) {
    fprintf(stderr, "Error: couldn't open destination file '%s'.\n", filename);
    exit(1);
  }
  png_ptr = png_create_write_struct(
    PNG_LIBPNG_VER_STRING,
    NULL, NULL, NULL // We won't worry about PNG errors for now.
  );
  if (!png_ptr) {
    fprintf(stderr, "Error: couldn't create png write struct.\n");
    exit(-1);
  }
  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
    fprintf(stderr, "Error: couldn't create png write struct.\n");
    exit(-1);
  }
  // Setup the IO process:
  png_init_io(png_ptr, fp);
  // Set the PNG header info:
  png_set_IHDR(
    png_ptr, info_ptr,
    tx->width, tx->height,
    CHANNEL_BITS, PNG_COLOR_TYPE_RGB_ALPHA,
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_DEFAULT,
    PNG_FILTER_TYPE_DEFAULT
  );
  // Build an array of row pointers:
  png_bytepp row_pointers = (png_bytepp) malloc(tx->width * sizeof(png_bytep));
  for (i = 0; i < tx->height; ++i) {
    row_pointers[i] = ((png_bytep) tx->pixels) + tx->width*sizeof(pixel) * i;
  }
  // Feed libpng the image data:
  png_set_rows(png_ptr, info_ptr, row_pointers);
  // Write out the PNG file:
  png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
  // Clean up the writing apparatus and return:
  png_destroy_write_struct(&png_ptr, &info_ptr);
  free(row_pointers);
  fclose(fp);
}

void upload_texture_to(texture* source, GLuint handle) {
  glBindTexture( GL_TEXTURE_2D, handle);

  // Set parameters:
  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameterf(
    GL_TEXTURE_2D,
    GL_TEXTURE_MIN_FILTER,
    GL_NEAREST
    //GL_NEAREST_MIPMAP_NEAREST
    //TODO: disable or use mipmapping!
  );

  // Load texture data:
  glTexImage2D(
    GL_TEXTURE_2D, // target
    0, // level
    GL_RGBA8, // internal format
    source->width, source->height, // dimensions
    0, // border
    GL_RGBA, // incoming data ordering
    GL_UNSIGNED_BYTE, // incoming data size
    source->pixels // texture data
  );

#ifdef PROFILE_MEM
  md_add_size(&TEXTURE_GPU_USAGE, source->width*source->height*sizeof(pixel),0);
#endif

  // Generate mipmaps:
  // TODO: use this?
  //glGenerateMipmap( GL_TEXTURE_2D );
}

GLuint upload_png(char const * const filename) {
  texture* tx = load_texture_from_png(filename);
  GLuint result = upload_texture(tx);
  cleanup_texture(tx);
  return result;
}

void tx_paste_region(
  texture *dst,
  texture const * const src,
  size_t dst_left,
    size_t dst_top,
  size_t src_left,
    size_t src_top,
    size_t region_width,
    size_t region_height
) {
  size_t row;
#ifdef DEBUG
  // Some bounds checking:
  if (dst_left + region_width > dst->width) {
    fprintf(
      stderr,
      "Error: region overruns destination width: %zu + %zu > %d\n",
      dst_left, region_width, dst->width
    );
    exit(1);
  }
  if (dst_top + region_height > dst->height) {
    fprintf(
      stderr,
      "Error: region overruns destination height: %zu + %zu > %d\n",
      dst_top, region_height, dst->height
    );
    exit(1);
  }
  if (src_left + region_width > src->width) {
    fprintf(
      stderr,
      "Error: region exceeds source width: %zu + %zu > %d\n",
      src_left, region_width, src->width
    );
    exit(1);
  }
  if (src_top + region_height > src->height) {
    fprintf(
      stderr,
      "Error: region exceeds source height: %zu + %zu > %d\n",
      src_top, region_height, src->height
    );
    exit(1);
  }
#endif
  for (row = 0; row < region_height; ++row) {
    memcpy(
      (void *) tx_get_addr(dst, dst_left, dst_top + row), // dst
      (void *) tx_get_addr(src, src_left, src_top + row), // src
      (region_width)*sizeof(pixel) // bytes
    );
  }
}

void tx_draw_region(
  texture *dst,
  texture const * const src,
  size_t dst_left,
    size_t dst_top,
  size_t src_left,
    size_t src_top,
    size_t region_width,
    size_t region_height
) {
  size_t row, column;
  pixel src_px, dst_px;
  float alpha;
#ifdef DEBUG
  // Some bounds checking:
  if (dst_left + region_width >= dst->width) {
    fprintf(stderr, "Error: region overruns destination width.\n");
    exit(1);
  }
  if (dst_top + region_height >= dst->height) {
    fprintf(stderr, "Error: region overruns destination height.\n");
    exit(1);
  }
  if (src_left + region_width >= src->width) {
    fprintf(stderr, "Error: region exceeds source width.\n");
    exit(1);
  }
  if (src_top + region_height >= src->height) {
    fprintf(stderr, "Error: region exceeds source height.\n");
    exit(1);
  }
#endif
  for (row = 0; row < region_height; ++row) {
    for (column = 0; column < region_width; ++column) {
      src_px = tx_get_px(src, src_left + column, src_top + row);
      dst_px = tx_get_px(dst, dst_left + column, dst_top + row);
      alpha = px_alpha(src_px) / ((float) CHANNEL_MAX);
      px_set_red(
        &dst_px,
        (px_red(src_px) * alpha) + (px_red(dst_px) * (1 - alpha))
      );
      px_set_green(
        &dst_px,
        (px_green(src_px) * alpha) + (px_green(dst_px) * (1 - alpha))
      );
      px_set_blue(
        &dst_px,
        (px_blue(src_px) * alpha) + (px_blue(dst_px) * (1 - alpha))
      );
      tx_set_px(dst, dst_px, dst_left + column, dst_top + row);
    }
  }
}

void tx_draw_region_wrapped(
  texture *dst,
  texture const * const src,
  size_t dst_left,
    size_t dst_top,
  size_t src_left,
    size_t src_top,
    size_t region_width,
    size_t region_height
) {
  size_t row, column, dr, dc;
  pixel src_px, dst_px;
  float alpha;
  for (row = 0; row < region_height; ++row) {
    for (column = 0; column < region_width; ++column) {
      src_px = tx_get_px(
        src,
        (src_left + column) % src->width,
        (src_top + row) % src->height
      );
      dr = (dst_top + row) % dst->height;
      dc = (dst_left + column) % dst->width;
      dst_px = tx_get_px(dst, dc, dr);
      alpha = (
        px_alpha(src_px) / ((float) CHANNEL_MAX)
      +
        (1 - (px_alpha(dst_px) / ((float) CHANNEL_MAX)))
      );
      if (alpha > 1) { alpha = 1; }
      px_set_red(
        &dst_px,
        (px_red(src_px) * alpha) + (px_red(dst_px) * (1 - alpha))
      );
      px_set_green(
        &dst_px,
        (px_green(src_px) * alpha) + (px_green(dst_px) * (1 - alpha))
      );
      px_set_blue(
        &dst_px,
        (px_blue(src_px) * alpha) + (px_blue(dst_px) * (1 - alpha))
      );
      alpha = px_alpha(src_px) / ((float) CHANNEL_MAX) + px_alpha(dst_px);
      if (alpha > 1) { alpha = 1; }
      px_set_alpha(&dst_px, alpha * CHANNEL_MAX);
      tx_set_px(dst, dst_px, dc, dr);
    }
  }
}
