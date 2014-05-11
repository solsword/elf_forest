// txgen.c
// Texture generation.

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "datatypes/list.h"
#include "noise/noise.h"
#include "tex/tex.h"
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
    mixed_hash_1d((ptrdiff_t) head)
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
            hash_3d(hash_2d(mixed_hash_1d((ptrdiff_t) lit), i), col, row)
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

void ateach_scattered(
  size_t seed,
  size_t x_scale,
  size_t y_scale,
  int x_strength,
  int y_strength,
  ptrdiff_t x_min, ptrdiff_t x_max,
  ptrdiff_t y_min, ptrdiff_t y_max,
  void *arg,
  void (*f)(int, int, void *)
) {
  int x, y;
  int xi = 0, yi = 0; // grid indices
  for (xi = x_min/x_scale; xi <= x_max/x_scale; xi += 1) {
    for (yi = y_min/y_scale; yi <= y_max/y_scale; yi += 1) {
      x = xi*x_scale;
      x += (expanded_hash_1d(seed*xi + xi + yi) % (2*x_strength)) - x_strength;
      y = yi*y_scale;
      y += (expanded_hash_1d(seed*yi + xi + yi) % (2*y_strength))- y_strength;
      if (x >= x_min && x < x_max && y >= x_min && y < y_max) {
        f(x, y, arg);
      }
    }
  }
}

/******************
 * Filter Helpers *
 ******************/

// private helper function for fltr_scatter w/ its own argument structure
struct scatter_helper_args_s {
  texture *tx;
  pixel color;
};
void fltr_scatter_helper(int x, int y, void *arg) {
  struct scatter_helper_args_s *shargs = (struct scatter_helper_args_s *) arg;
  tx_set_px(
    shargs->tx,
    shargs->color,
    (size_t) ((x + shargs->tx->width) % shargs->tx->width),
    (size_t) ((y + shargs->tx->height) % shargs->tx->height)
  );
}

/********************
 * Filter Functions *
 ********************/

void fltr_scatter(texture *tx, void const * const fargs) {
  struct scatter_helper_args_s shargs;
  scatter_filter_args *sfargs = (scatter_filter_args *) fargs;
  shargs.tx = tx;
  shargs.color = sfargs->color;
  ateach_scattered(
    sfargs->seed,
    sfargs->x_freq, sfargs->y_freq,
    sfargs->x_freq/3, sfargs->y_freq/3,
    0, BLOCK_TEXTURE_SIZE,
    0, BLOCK_TEXTURE_SIZE,
    &shargs,
    &fltr_scatter_helper
  );
}

void fltr_apply_gradient_map(texture *tx, void const * const fargs) {
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

void fltr_worley(texture *tx, void const * const fargs) {
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

gradient_map test_test = {
  .colors = {
    0xff000000, 0xff111111, 0xff222222, 0xff333333,
    0xff444444, 0xff555555, 0xff666666, 0xff777777,
    0xff888888, 0xff999999, 0xffaaaaaa, 0xffbbbbbb,
    0xffcccccc, 0xffdddddd, 0xffeeeeee, 0xffffffff,
  },
  .thresholds = {
    1/16.0, 2/16.0, 3/16.0, 4/16.0,
    5/16.0, 6/16.0, 7/16.0, 8/16.0,
    9/16.0, 10/16.0, 11/16.0, 12/16.0,
    13/16.0, 14/16.0, 15/16.0, 16/16.0
  }
};
