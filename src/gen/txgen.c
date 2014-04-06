// txgen.c
// Texture generation.

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
  rnd *= (float) (((seed & HASH_MASK) << HASH_BITS) + UPPER_HASH_OF(head));
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

/*************
 * Functions *
 *************/

// DEBUG:
extern tx_grammar_literal moss_clump_10;

void run_grammar(tx_grammar_literal *lit) {
  size_t i;
  int row, col;
  pixel px;
  tx_grammar_literal *chosen = NULL;
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
            HASH[
              HASH[
                HASH[
                  UPPER_HASH_OF(lit) + HASH_OF(i)
                ] + HASH_OF(col)
              ] + HASH_OF(row)
            ]
          );
          if (chosen->result == NULL) {
            run_grammar(chosen);
          }
          tx_draw_wrapped(
            lit->result,
            chosen->result,
            (col - chosen->anchor_x) % (lit->result->width),
            (row - chosen->anchor_y) % (lit->result->height)
          );
        }
      }
    }
  }
  // Before wrapping up call our post-processing function if it exists:
  if (lit->postprocess != NULL) {
    (*(lit->postprocess))(lit->result, lit->postargs);
  }
}

/********************
 * Filter Functions *
 ********************/

void fltr_scatter(texture *tx, void *args) {
  int row, col;
  scatter_filter_args *sfargs = (scatter_filter_args *) args;
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
      dx = UPPER_HASH_OF(tx);
      dx = HASH[dx + (col & HASH_MASK)];
      dx = HASH[dx + (row & HASH_MASK)];
      dx %= (2*max_dist_x + 1);
      dx -= max_dist_x;

      dy = UPPER_HASH_OF(tx);
      dy = HASH[dy + (row & HASH_MASK)];
      dy = HASH[dy + (col & HASH_MASK)];
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
