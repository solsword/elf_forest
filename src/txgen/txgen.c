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

#include "util.h"

#include "txg_plants.h"
#include "txg_minerals.h"
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

static inline tx_grammar_literal* choose_child(
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
    hash_1d((ptrdiff_t) head)
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

tx_grammar_literal* create_grammar_literal(
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

texture* gen_block_texture(block b) {
  char filename[1024];
  block id = b_id(b);
  if (b_species(b) != 0) {
    switch (id) {
      default:
        // Fall out to the file base case below
        break;
      case B_DIRT:
        return gen_dirt_texture(b_species(b));

      case B_MUD:
        return gen_mud_texture(b_species(b));

      case B_SAND:
        return gen_sand_texture(b_species(b));

      case B_CLAY:
        return gen_clay_texture(b_species(b));

      case B_STONE:
        return gen_stone_texture(b_species(b));

      case B_MUSHROOM_SPORES:
      case B_MUSHROOM_SHOOTS:
      case B_MUSHROOM_GROWN:
        return gen_mushroom_texture(b);

      case B_GIANT_MUSHROOM_SPORES:
      case B_GIANT_MUSHROOM_CORE:
      case B_GIANT_MUSHROOM_MYCELIUM:
      case B_GIANT_MUSHROOM_SPROUT:
      case B_GIANT_MUSHROOM_STALK:
      case B_GIANT_MUSHROOM_CAP:
        return gen_giant_mushroom_texture(b);

      case B_MOSS_SPORES:
      case B_MOSS_SHOOTS:
      case B_MOSS_GROWN:
      case B_MOSS_FLOWERING:
      case B_MOSS_FRUITING:
        return gen_moss_texture(b);

      case B_GRASS_SEEDS:
      case B_GRASS_ROOTS:
      case B_GRASS_SHOOTS:
      case B_GRASS_GROWN:
      case B_GRASS_BUDDING:
      case B_GRASS_FLOWERING:
      case B_GRASS_FRUITING:
        return gen_grass_texture(b);

      case B_VINE_SEEDS:
      case B_VINE_CORE:
      case B_VINE_ROOTS:
      case B_VINE_SHOOTS:
      case B_VINE_SPROUTING:
      case B_VINE_GROWN:
      case B_VINE_BUDDING:
      case B_VINE_FLOWERING:
      case B_VINE_FRUITING:
      case B_VINE_SHEDDING:
      case B_VINE_DORMANT:
        return gen_vine_texture(b);

      case B_HERB_SEEDS:
      case B_HERB_CORE:
      case B_HERB_ROOTS:
      case B_HERB_SHOOTS:
      case B_HERB_STEMS:
      case B_HERB_LEAVES:
      case B_HERB_BUDDING:
      case B_HERB_FLOWERING:
      case B_HERB_FRUITING:
        return gen_herb_texture(b);

      case B_BUSH_SEEDS:
      case B_BUSH_CORE:
      case B_BUSH_ROOTS:
      case B_BUSH_SHOOTS:
      case B_BUSH_BRANCHES_SPROUTING:
      case B_BUSH_BRANCHES_GROWN:
      case B_BUSH_BRANCHES_BUDDING:
      case B_BUSH_BRANCHES_FLOWERING:
      case B_BUSH_BRANCHES_FRUITING:
      case B_BUSH_BRANCHES_SHEDDING:
      case B_BUSH_BRANCHES_DORMANT:
      case B_BUSH_LEAVES_SPROUTING:
      case B_BUSH_LEAVES_GROWN:
      case B_BUSH_LEAVES_BUDDING:
      case B_BUSH_LEAVES_FLOWERING:
      case B_BUSH_LEAVES_FRUITING:
      case B_BUSH_LEAVES_SHEDDING:
      case B_BUSH_LEAVES_DORMANT:
        return gen_bush_texture(b);

      case B_SHRUB_SEEDS:
      case B_SHRUB_CORE:
      case B_SHRUB_ROOTS:
      case B_SHRUB_THICK_ROOTS:
      case B_SHRUB_SHOOTS:
      case B_SHRUB_BRANCHES_SPROUTING:
      case B_SHRUB_BRANCHES_GROWN:
      case B_SHRUB_BRANCHES_BUDDING:
      case B_SHRUB_BRANCHES_FLOWERING:
      case B_SHRUB_BRANCHES_FRUITING:
      case B_SHRUB_BRANCHES_SHEDDING:
      case B_SHRUB_BRANCHES_DORMANT:
      case B_SHRUB_LEAVES_SPROUTING:
      case B_SHRUB_LEAVES_GROWN:
      case B_SHRUB_LEAVES_BUDDING:
      case B_SHRUB_LEAVES_FLOWERING:
      case B_SHRUB_LEAVES_FRUITING:
      case B_SHRUB_LEAVES_SHEDDING:
      case B_SHRUB_LEAVES_DORMANT:
        return gen_shrub_texture(b);

      case B_TREE_SEEDS:
      case B_TREE_CORE:
      case B_TREE_HEART_CORE:
      case B_TREE_ROOTS:
      case B_TREE_THICK_ROOTS:
      case B_TREE_HEART_ROOTS:
      case B_TREE_SHOOTS:
      case B_TREE_TRUNK:
      case B_TREE_BARE_BRANCHES:
      case B_TREE_BRANCHES_SPROUTING:
      case B_TREE_BRANCHES_GROWN:
      case B_TREE_BRANCHES_BUDDING:
      case B_TREE_BRANCHES_FLOWERING:
      case B_TREE_BRANCHES_FRUITING:
      case B_TREE_BRANCHES_SHEDDING:
      case B_TREE_BRANCHES_DORMANT:
      case B_TREE_LEAVES_SPROUTING:
      case B_TREE_LEAVES_GROWN:
      case B_TREE_LEAVES_BUDDING:
      case B_TREE_LEAVES_FLOWERING:
      case B_TREE_LEAVES_FRUITING:
      case B_TREE_LEAVES_SHEDDING:
      case B_TREE_LEAVES_DORMANT:
        return gen_tree_texture(b);

      case B_AQUATIC_GRASS_SEEDS:
      case B_AQUATIC_GRASS_ROOTS:
      case B_AQUATIC_GRASS_SHOOTS:
      case B_AQUATIC_GRASS_GROWN:
      case B_AQUATIC_GRASS_FLOWERING:
      case B_AQUATIC_GRASS_FRUITING:
        return gen_aquatic_grass_texture(b);

      case B_AQUATIC_PLANT_SEEDS:
      case B_AQUATIC_PLANT_CORE:
      case B_AQUATIC_PLANT_ANCHORS:
      case B_AQUATIC_PLANT_SHOOTS:
      case B_AQUATIC_PLANT_STEMS:
      case B_AQUATIC_PLANT_LEAVES_GROWN:
      case B_AQUATIC_PLANT_LEAVES_FLOWERING:
      case B_AQUATIC_PLANT_LEAVES_FRUITING:
        return gen_aquatic_plant_texture(b);

      case B_YOUNG_CORAL:
      case B_CORAL_CORE:
      case B_CORAL_BODY:
      case B_CORAL_FROND:
        return gen_coral_texture(b);

// TODO: More block types here!!

    }
  }
  // Base case if species is 0 or block type isn't gen-able: look for a file.
  // TODO: switch to using string.h...
  sprintf(
    filename, 
    "%s/%s.png",
    BLOCK_TEXTURE_DIR,
    BLOCK_NAMES[id]
  );
  if (access(filename, R_OK) != -1) {
    return load_texture_from_png(filename);
  }
  // Give up
  return NULL;
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
            hash_3d(hash_2d(hash_1d((ptrdiff_t) lit), i), col, row)
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
      x += (prng(seed*xi + xi + yi)) % (2*x_strength) - x_strength;
      y = yi*y_scale;
      y += (prng(seed*yi + xi - yi)) % (2*y_strength) - y_strength;
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
      tx_set_px(tx, gradient_map_result(grmap, p), col, row);
    }
  }
}

void fltr_worley(texture *tx, void const * const fargs) {
  int row, col;
  float noise;
  float dontcare;
  ptrdiff_t salt;
  worley_filter_args *wfargs = (worley_filter_args *) fargs;
  salt = prng(wfargs->seed);
  for (col = 0; col < tx->width; col += 1) {
    for (row = 0; row < tx->height; row += 1) {
      noise = wrnoise_2d_fancy(
        col * wfargs->freq, row * wfargs->freq, salt,
        32.0 * wfargs->freq, 32.0 * wfargs->freq,
        &dontcare, &dontcare,
        0
      );
      tx_set_px(
        tx,
        gradient_map_result(wfargs->grmap, noise),
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
