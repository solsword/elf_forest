// dta.c
// Dynamic texture atlas functionality.

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "dta.h"

#include "tex.h"

#include "txgen/txgen.h"

#include "datatypes/bitmap.h"
#include "datatypes/map.h"

#include "world/blocks.h"

/********************
 * Global variables *
 ********************/

dynamic_texture_atlas *LAYER_ATLASES[N_LAYERS] = {
  NULL,
  NULL,
  NULL
};

/******************************
 * Constructors & Destructors *
 ******************************/

dynamic_texture_atlas *create_dynamic_atlas(size_t size) {
  dynamic_texture_atlas *dta = (dynamic_texture_atlas *) malloc(
    sizeof(dynamic_texture_atlas)
  );
  dta->size = size;
  dta->vacancies = create_bitmap(size*size);
  dta->tcmap = create_map(1, size*size*4);
  dta->atlas = create_texture(
    BLOCK_TEXTURE_SIZE * size,
    BLOCK_TEXTURE_SIZE * size
  );
  dta->handle = 0;

  // Reserve indices 0 through 3 as 'invalid' textures so that failed map
  // lookups (which return NULL) will be obvious.
  texture *invalid = load_texture_from_png("res/textures/invalid.png");
  // Mark 0 through 4 as used:
  bm_set_bits(dta->vacancies, 0, 5);
  // Add B_VOID -> 1 to our block id/variant -> index mapping:
  dta_set_index(dta, b_make_block(B_VOID), 1);
  // Copy the invalid texture five times into our texture atlas:
  tx_paste(dta->atlas, invalid, 0, 0);
  tx_paste(dta->atlas, invalid, BLOCK_TEXTURE_SIZE, 0);
  tx_paste(dta->atlas, invalid, BLOCK_TEXTURE_SIZE*2, 0);
  tx_paste(dta->atlas, invalid, BLOCK_TEXTURE_SIZE*3, 0);
  tx_paste(dta->atlas, invalid, BLOCK_TEXTURE_SIZE*4, 0);
  // Clean up the loaded texture as it's no longer needed:
  cleanup_texture(invalid);

  // Initialize the OpenGL texture:
  dta_update_texture(dta);

  return dta;
}

void cleanup_dynamic_atlas(dynamic_texture_atlas *dta) {
  cleanup_bitmap(dta->vacancies);
  cleanup_map(dta->tcmap);
  cleanup_texture(dta->atlas);
  // TODO: Destroy the OpenGL texture!
  free(dta);
}

/*************
 * Functions *
 *************/

void ensure_mapped(block b) {
  if (b_is_invisible(b)) {
    return;
  }
  dynamic_texture_atlas *dta = LAYER_ATLASES[block_layer(b)];
  size_t i = dta_get_index(dta, b);
  texture *tx;
  if (i == 0) {
    // We need to load the block's texture:
    // TODO: Real error checking/reporting!!
#ifdef DEBUG
    printf(
      "Loading texture for block %s:%d...\n",
      BLOCK_NAMES[b_id(b)],
      b_species(b)
    );
#endif
    tx = gen_block_texture(b);
    if (tx) {
#ifdef DEBUG
      printf("  ...done.\n");
#endif
      dta_add_block(dta, b, tx);
      dta_update_texture(dta);
    } else {
#ifdef DEBUG
      printf("  ...failed (no texture found).\n");
#endif
      // If there's no texture for the block, we'll mark it as
      // invalid-no-texture, and it will use the default "invalid" texture.
      dta_set_index(dta, b, 1);
    }
  }
}

void dta_update_texture(dynamic_texture_atlas *dta) {
  if (dta->handle == 0) {
    dta->handle = upload_texture(dta->atlas);
  } else {
    upload_texture_to(dta->atlas, dta->handle);
  }
}

ptrdiff_t dta_add_block(
  dynamic_texture_atlas *dta,
  block b,
  texture *facemap
) {
  size_t spots_needed = 1;
  size_t index;
  if (bi_anis(b)) {
    spots_needed = 4;
  }
  index = bm_find_space(dta->vacancies, spots_needed);
  if (index == -1) {
#ifdef DEBUG
    fprintf(
      stderr,
      "Warning: failed to add block to dynamic texture atlas: no space.\n"
    );
#endif
    // TODO: Something else here?
    return -1;
  }
  // Mark the vacant space as filled:
  bm_set_bits(dta->vacancies, index, spots_needed);
  // Add to our block id/variant -> index mapping:
#ifdef DEBUG
  assert(dta_set_index(dta, b, index) == 0);
#else
  dta_set_index(dta, b, index);
#endif
  // Copy the relevant textures into our texture atlas:
  tx_paste_region(
    dta->atlas,
    facemap,
    BLOCK_TEXTURE_SIZE * (index % dta->size), // destination left/top
      BLOCK_TEXTURE_SIZE * (index / dta->size),
    0, // source left/top
      0,
    BLOCK_TEXTURE_SIZE, // region width/height
      BLOCK_TEXTURE_SIZE
  );
  if (bi_anis(b)) {
    size_t i;
    for (i = 1; i < 4; ++i) {
      tx_paste_region(
        dta->atlas,
        facemap,
        BLOCK_TEXTURE_SIZE * ((index + i) % dta->size), // destination left/top
          BLOCK_TEXTURE_SIZE * ((index + i) / dta->size),
        BLOCK_TEXTURE_SIZE * i, // source left/top
          0,
        BLOCK_TEXTURE_SIZE, // region width/height
          BLOCK_TEXTURE_SIZE
      );
    }
  }
  // Return the index used:
  return index;
}
