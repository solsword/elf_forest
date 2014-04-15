// txgen.c
// Texture generation.

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "datatypes/list.h"
#include "noise/noise.h"
#include "graphics/tex.h"
#include "world/blocks.h"

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

tx_grammar_literal * create_grammar_literal(
  char const * const fn,
  size_t ax, size_t ay,
  texture_filter prp, void *prargs,
  texture_filter psp, void *psargs,
  tx_grammar_disjunction *child0,
  tx_grammar_disjunction *child1
) {
  size_t i;
  tx_grammar_literal *lit = (tx_grammar_literal *) malloc(
    sizeof(tx_grammar_literal)
  );
  lit->filename = fn;
  lit->anchor_x = ax;
  lit->anchor_y = ay;
  lit->preprocess = prp;
  lit->postprocess = psp;
  lit->preargs = prargs;
  lit->postargs = psargs;
  for (i = 0; i < N_GRAMMAR_KEYS; ++i) {
    lit->children[i] = NULL;
  }
  lit->children[0] = child0;
  lit->children[1] = child1;
  lit->result = NULL;
  return lit;
}

void cleanup_grammar_literal(tx_grammar_literal *lit) {
  size_t i;
  tx_grammar_disjunction *dis = NULL;
  for (i = 0; i < N_GRAMMAR_KEYS; ++i) {
    dis = lit->children[i];
    if (dis != NULL) {
      cleanup_grammar_disjunction(dis);
    }
  }
  if (lit->result != NULL) {
    cleanup_texture(lit->result);
  }
  free(lit);
}

tx_grammar_disjunction * create_grammar_disjunction(
  tx_grammar_disjunction *nxt,
  tx_grammar_literal *lit,
  float w
) {
  tx_grammar_disjunction * dis = (tx_grammar_disjunction *) malloc(
    sizeof(tx_grammar_disjunction)
  );
  dis->next = nxt;
  dis->literal = lit;
  dis->weight = w;
  return dis;
}

void cleanup_grammar_disjunction(tx_grammar_disjunction *dis) {
  if (dis->next != NULL) { cleanup_grammar_disjunction(dis->next); }
  cleanup_grammar_literal(dis->literal);
  free(dis);
}

void cleanup_grammar_results(tx_grammar_literal *lit) {
  size_t i;
  tx_grammar_disjunction *dis = NULL;
  for (i = 0; i < N_GRAMMAR_KEYS; ++i) {
    dis = lit->children[i];
    while (dis != NULL) {
      cleanup_grammar_results(dis->literal);
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

texture* get_block_texture(block b) {
  char filename[1024];
  block id = b_id(b);
  texture *tx;
  switch (id) {
    /*
    case B_GRASS:
      // TODO: Fix me!
      tx = NULL;
      break;
    */
    default:
      sprintf(
        filename, 
        "%s/%s.png",
        BLOCK_TEXTURE_DIR,
        BLOCK_NAMES[id]
      );
      if (access(filename, R_OK) != -1) {
        tx = load_texture_from_png(filename);
      } else {
        tx = NULL;
      }
  }
  return tx;
}

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
      noise = wrnoise_2d(
        col * wfargs->freq, row * wfargs->freq,
        32.0 * wfargs->freq, 32.0 * wfargs->freq,
        0
      );
      tx_set_px(
        tx,
        gradient_result(wfargs->grmap, noise),
        col,
        row
      );
    }
  }
}

void fltr_branches(texture *tx, void *fargs) {
  int row, col;
  float noise, dsx, dsy;
  gradient_map grmap;
  branch_filter_args *bfargs = (branch_filter_args *) fargs;
  grmap.colors[0] = bfargs->center_color;
  grmap.colors[1] = bfargs->mid_color;
  grmap.colors[2] = bfargs->outer_color;
  grmap.colors[3] = 0x00000000;
  if (bfargs->rough) {
    grmap.thresholds[0] = 0.43;
    grmap.thresholds[1] = 0.61;
    grmap.thresholds[2] = 0.72;
  } else {
    grmap.thresholds[0] = 0.06;
    grmap.thresholds[1] = 0.14;
    grmap.thresholds[2] = 0.2;
  }
  grmap.thresholds[3] = 1.0;
#ifdef DEBUG
  // Use orange for out-of-range noise results:
  grmap.colors[4] = 0xff0088ff;
  grmap.thresholds[4] = 1000.0;
#endif
  for (col = 0; col < tx->width; col += 1) {
    for (row = 0; row < tx->height; row += 1) {
      // TODO: property wrapped simplex noise.
      dsx = sxnoise_2d(col * bfargs->scale / 2.0, row * bfargs->scale / 2.0);
      dsy = sxnoise_2d(col * bfargs->scale / 2.0, row * bfargs->scale / 2.0);
      noise = wrnoise_2d(
        (col + dsx * bfargs->distortion) * bfargs->scale,
        (row + dsy * bfargs->distortion) * bfargs->scale,
        32.0 * bfargs->scale, 32.0 * bfargs->scale,
        (!bfargs->rough) * WORLEY_FLAG_INCLUDE_NEXTBEST
      );
      if (bfargs->rough) {
        noise = 1 - noise;
        // TODO: a sigmoid for organizing branches
      }
      noise *= (2 - bfargs->width);
      if (noise > 1) { noise = 1; }
      tx_set_px(
        tx,
        gradient_result(&grmap, noise),
        col,
        row
      );
    }
  }
}
