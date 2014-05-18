// worldgen.c
// World map generation.

#include "worldgen.h"

/******************************
 * Constructors & Destructors *
 ******************************/

world_map *create_world_map(size_t width, size_t height) {
  world_map *result = (world_map*) malloc(sizeof(world_map));
  result->width = width;
  result->height = height;
  result->regions = (world_region *) malloc(sizeof(world_region)*width*height);
}
