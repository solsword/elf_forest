// txgen.c
// Texture generation.

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


#include "datatypes/list.h"
#include "noise/noise.h"
#include "graphics/tex.h"

#include "txgen.h"

/*************
 * Constants *
 *************/

pixel const GRAMMAR_KEYS[N_GRAMMAR_KEYS] = {
  GRAMMAR_KEY_0,
  GRAMMAR_KEY_1,
  GRAMMAR_KEY_2,
  GRAMMAR_KEY_3,
  GRAMMAR_KEY_4,
  GRAMMAR_KEY_5
};

/********************
 * Inline Functions *
 ********************/

static inline tx_grammar_literal *choose_child(
  tx_grammar_disjunction *dis,
  int seed
) {
  float w_total, rnd;
  tx_grammar_disjunction *head = dis;
  w_total = 0;
  while (dis != NULL) {
    w_total += dis->weight;
    dis = dis->next;
  }
  rnd = (float) w_total;
  rnd *= (float) (
    ((seed & HASH_MASK) << HASH_BITS)
  +
    UPPER_HASH_OF(head)
  );
  rnd /= (float) ((HASH_MASK << HASH_BITS) + HASH_MASK);
  dis = head;
  while (dis != NULL) {
    rnd -= dis->weight;
    if (rnd < 0 || dis->next == NULL) {
      return dis->literal;
    }
    dis = dis->next;
  }
#ifdef DEBUG
  fprintf(
    stderr,
    "Reached unthinkable state: non-terminated disjunction list.\n"
  );
  exit(-1);
#else
  return NULL;
#endif
}

/*********************
 * Private Functions *
 *********************/

void expand_literal_into(void *element, void *arg) {
  tx_grammar_expansion_site *ges = (tx_grammar_expansion_site *) element;
  tx_grammar_literal *lit = (tx_grammar_literal *) arg;
  int left = (int) (ges->col) - (int) (ges->literal->anchor_x);
  int top = (int) (ges->row) - (int) (ges->literal->anchor_y);
  while (left < 0) { left += lit->result->width; }
  while (top < 0) { top += lit->result->height; }
  left %= lit->result->width;
  top %= lit->result->height;
  tx_draw_wrapped(lit->result, ges->literal->result, left, top);
}

/******************************
 * Constructors & Destructors *
 ******************************/

void cleanup_grammar(tx_grammar_literal *lit) {
  size_t i;
  tx_grammar_disjunction *dis = NULL;
  for (i = 0; i < N_GRAMMAR_KEYS; ++i) {
    dis = lit->children[i];
    while (dis != NULL) {
      cleanup_grammar(dis->literal);
      dis = dis->next;
    }
  }
  if (lit->result != NULL) {
    cleanup_texture(lit->result);
    lit->result = NULL;
  }
}

tx_grammar_expansion_site * create_expansion_site(void) {
  tx_grammar_expansion_site *ges = (tx_grammar_expansion_site *) malloc(
    sizeof(tx_grammar_expansion_site)
  );
  ges->literal = NULL;
  return ges;
}

// Cleans up a grammar expansion site:
void cleanup_expansion_site(tx_grammar_expansion_site *ges) {
  free(ges);
}

/*************
 * Functions *
 *************/

void run_grammar(tx_grammar_literal *lit) {
  size_t i;
  int row, col;
  pixel px;
  tx_grammar_literal *chosen = NULL;
  tx_grammar_expansion_site *ges = NULL;
  // Create a list of expansion sites:
  list *expansion_sites = create_list();
  // Start by loading our source file into our result texture and calling our
  // pre-processing filter if we have one:
  if (lit->filename == NULL) {
    if (lit->anchor_x == 0 || lit->anchor_y == 0) {
      fprintf(
        stderr,
        "Error: grammar with NULL filename must have nonzero anchor values.\n"
      );
      fprintf(stderr, "(found: %zu, %zu)\n", lit->anchor_x, lit->anchor_y);
      exit(1);
    }
    lit->result = create_texture(lit->anchor_x, lit->anchor_y);
  } else {
    lit->result = load_texture_from_png(lit->filename);
  }
  if (lit->preprocess != NULL) {
    (*(lit->preprocess))(lit->result, lit->preargs);
  }
  // Now expand each child reference at random. Note that due to the low
  // resolution of the pseudo-random process, any item with a weight of less
  // than 1/65536th of the total weight of all items in a disjunction will
  // never be picked.
  for (col = 0; col < lit->result->width; ++col) {
    for (row = 0; row < lit->result->height; ++row) {
      px = tx_get_px(lit->result, col, row);
      for (i = 0; i < N_GRAMMAR_KEYS; ++i) {
        if (px == GRAMMAR_KEYS[i] && lit->children[i] != NULL) {
          chosen = choose_child(
            lit->children[i],
            hash_3d(hash_2d(UPPER_HASH_OF(lit), i), col, row)
          );
          if (chosen->result == NULL) {
            run_grammar(chosen);
          }
          ges = create_expansion_site();
          ges->col = col;
          ges->row = row;
          ges->literal = chosen;
          l_append_element(expansion_sites, ges);
          ges = NULL; // destroy_list later on takes care of the memory used.
        }
      }
    }
  }
  l_witheach(expansion_sites, (void *) lit, expand_literal_into);
  // Clean up our list of expansion sites:
  destroy_list(expansion_sites);
  // Before wrapping up call our post-processing function if it exists:
  if (lit->postprocess != NULL) {
    (*(lit->postprocess))(lit->result, lit->postargs);
  }
}

/********************
 * Filter Functions *
 ********************/

void fltr_scatter(texture *tx, void *fargs) {
  int row, col;
  scatter_filter_args *sfargs = (scatter_filter_args *) fargs;
  int max_dist_x = sfargs->x_freq / 3;
  int max_dist_y = sfargs->y_freq / 3;
  int dx, dy;
  int starting_col = UPPER_HASH_OF(tx) % (sfargs->x_freq/2);
  int starting_row = UPPER_HASH_OF(sfargs) % (sfargs->y_freq/2);
  for (
    col = starting_col % tx->width;
    col < tx->width;
    col += sfargs->x_freq
  ) {
    for (
      row = starting_row % tx->height;
      row < tx->height;
      row += sfargs->y_freq
    ) {
      dx = hash_3d(UPPER_HASH_OF(tx), col, row);
      dx %= (2*max_dist_x + 1);
      dx -= max_dist_x;

      dy = hash_3d(UPPER_HASH_OF(tx), row, col);
      dy %= (2*max_dist_y + 1);
      dy -= max_dist_y;

      tx_set_px(
        tx,
        sfargs->color,
        (size_t) ((col + dx + tx->width) % tx->width),
        (size_t) ((row + dy + tx->height) % tx->height)
      );
    }
  }
}

void fltr_apply_gradient_map(texture *tx, void *fargs) {
  int row, col;
  pixel p;
  gradient_map *grmap = (gradient_map *) fargs;
  for (col = 0; col < tx->width; col += 1) {
    for (row = 0; row < tx->height; row += 1) {
      p = tx_get_px(tx, col, row);
      tx_set_px(tx, gradient_result(grmap, p), col, row);
    }
  }
}

void fltr_worley(texture *tx, void *fargs) {
  int row, col;
  float noise;
  worley_filter_args *wfargs = (worley_filter_args *) fargs;
  for (col = 0; col < tx->width; col += 1) {
    for (row = 0; row < tx->height; row += 1) {
      //noise = wrnoise_2d(col * wfargs->freq, row * wfargs->freq);
      //*
      noise = wrnoise_2d_wrapped(
        //col * wfargs->freq,
        //row * wfargs->freq,
        3 + col / 8.0,
        row / 8.0,
        4, 4
      );
      // */
      /*
      noise = 0.5*sqrtf(
        2*wrnoise_2d_wrapped(
          col * wfargs->freq, row * wfargs->freq,
          32*wfargs->freq, 32*wfargs->freq
        )
      );
      // */
      //noise = 0.5*sqrtf(2*wrnoise_2d(col * wfargs->freq, row * wfargs->freq));
      tx_set_px(
        tx,
        gradient_result(wfargs->grmap, noise*noise),
        col,
        row
      );
    }
  }
}
