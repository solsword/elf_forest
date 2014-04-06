// tex.c
// Texture loading and management.

#include <png.h>
#include <GLee.h> // glGenerateMipmaps

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "tex.h"

/********************
 * Global variables *
 ********************/

char const * const BLOCK_TEXTURE_FILE = "res/textures/blocks.png";

GLuint BLOCK_ATLAS = 0;
uint16_t BLOCK_ATLAS_WIDTH = 1;
uint16_t BLOCK_ATLAS_HEIGHT = 1;

/********
 * Data *
 ********/

uint16_t const BLOCK_TEXTURE_MAP[1024] = {
// VOID                                AIR
   0x000, 0x000, 0x000, 0x000,         0x000, 0x000, 0x000, 0x000,
// ETHER                               WATER
   0x000, 0x000, 0x000, 0x000,         0x003, 0x003, 0x003, 0x003,
// WATER_FLOW                          MIASMA
   0x004, 0x004, 0x004, 0x004,         0x005, 0x005, 0x005, 0x005,
// ___                                 ___
   0x006, 0x006, 0x006, 0x006,         0x007, 0x007, 0x007, 0x007,
// ___                                 ___
   0x008, 0x008, 0x008, 0x008,         0x009, 0x009, 0x009, 0x009,
// ___                                 ___
   0x00a, 0x00a, 0x00a, 0x00a,         0x00b, 0x00b, 0x00b, 0x00b,
// ___                                 ___
   0x00c, 0x00c, 0x00c, 0x00c,         0x00d, 0x00d, 0x00d, 0x00d,
// ___                                 ___
   0x00e, 0x00e, 0x00e, 0x00e,         0x00f, 0x00f, 0x00f, 0x00f,
// ___                                 ___
   0x010, 0x010, 0x010, 0x010,         0x011, 0x011, 0x011, 0x011,
// ___                                 ___
   0x012, 0x012, 0x012, 0x012,         0x013, 0x013, 0x013, 0x013,
// ___                                 ___
   0x014, 0x014, 0x014, 0x014,         0x015, 0x015, 0x015, 0x015,
// ___                                 ___
   0x016, 0x016, 0x016, 0x016,         0x017, 0x017, 0x017, 0x017,
// ___                                 ___
   0x018, 0x018, 0x018, 0x018,         0x019, 0x019, 0x019, 0x019,
// ___                                 ___
   0x01a, 0x01a, 0x01a, 0x01a,         0x01b, 0x01b, 0x01b, 0x01b,
// ___                                 ___
   0x01c, 0x01c, 0x01c, 0x01c,         0x01d, 0x01d, 0x01d, 0x01d,
// ___                                 ___
   0x01e, 0x01e, 0x01e, 0x01e,         0x01f, 0x01f, 0x01f, 0x01f,
// ___                                 ___
   0x020, 0x020, 0x020, 0x020,         0x021, 0x021, 0x021, 0x021,
// ___                                 ___
   0x022, 0x022, 0x022, 0x022,         0x023, 0x023, 0x023, 0x023,
// ___                                 ___
   0x024, 0x024, 0x024, 0x024,         0x025, 0x025, 0x025, 0x025,
// ___                                 ___
   0x026, 0x026, 0x026, 0x026,         0x027, 0x027, 0x027, 0x027,
// ___                                 ___
   0x028, 0x028, 0x028, 0x028,         0x029, 0x029, 0x029, 0x029,
// ___                                 ___
   0x02a, 0x02a, 0x02a, 0x02a,         0x02b, 0x02b, 0x02b, 0x02b,
// ___                                 ___
   0x02c, 0x02c, 0x02c, 0x02c,         0x02d, 0x02d, 0x02d, 0x02d,
// ___                                 ___
   0x02e, 0x02e, 0x02e, 0x02e,         0x02f, 0x02f, 0x02f, 0x02f,
// ___                                 ___
   0x030, 0x030, 0x030, 0x030,         0x031, 0x031, 0x031, 0x031,
// ___                                 ___
   0x032, 0x032, 0x032, 0x032,         0x033, 0x033, 0x033, 0x033,
// ___                                 ___
   0x034, 0x034, 0x034, 0x034,         0x035, 0x035, 0x035, 0x035,
// ___                                 ___
   0x036, 0x036, 0x036, 0x036,         0x037, 0x037, 0x037, 0x037,
// ___                                 ___
   0x038, 0x038, 0x038, 0x038,         0x039, 0x039, 0x039, 0x039,
// ___                                 ___
   0x03a, 0x03a, 0x03a, 0x03a,         0x03b, 0x03b, 0x03b, 0x03b,
// ___                                 LAVA
   0x03c, 0x03c, 0x03c, 0x03c,         0x03d, 0x03d, 0x03d, 0x03d,
// LAVA_FLOW                           QUICKSAND
   0x03e, 0x03e, 0x03e, 0x03e,         0x03f, 0x03f, 0x03f, 0x03f,
// BOUNDARY                            STONE
   0x040, 0x040, 0x040, 0x040,         0x041, 0x041, 0x041, 0x041,
// DIRT                                GRASS
   0x042, 0x042, 0x042, 0x042,         0x043, 0x042, 0x143, 0x143,
// SAND                                TRUNK
   0x044, 0x044, 0x044, 0x044,         0x145, 0x145, 0x045, 0x045,
// ___                                 ___
   0x046, 0x046, 0x046, 0x046,         0x047, 0x047, 0x047, 0x047,
// ___                                 ___
   0x048, 0x048, 0x048, 0x048,         0x049, 0x049, 0x049, 0x049,
// ___                                 ___
   0x04a, 0x04a, 0x04a, 0x04a,         0x04b, 0x04b, 0x04b, 0x04b,
// ___                                 ___
   0x04c, 0x04c, 0x04c, 0x04c,         0x04d, 0x04d, 0x04d, 0x04d,
// ___                                 ___
   0x04e, 0x04e, 0x04e, 0x04e,         0x04f, 0x04f, 0x04f, 0x04f,
// ___                                 ___
   0x050, 0x050, 0x050, 0x050,         0x051, 0x051, 0x051, 0x051,
// ___                                 ___
   0x052, 0x052, 0x052, 0x052,         0x053, 0x053, 0x053, 0x053,
// ___                                 ___
   0x054, 0x054, 0x054, 0x054,         0x055, 0x055, 0x055, 0x055,
// ___                                 ___
   0x056, 0x056, 0x056, 0x056,         0x057, 0x057, 0x057, 0x057,
// ___                                 ___
   0x058, 0x058, 0x058, 0x058,         0x059, 0x059, 0x059, 0x059,
// ___                                 ___
   0x05a, 0x05a, 0x05a, 0x05a,         0x05b, 0x05b, 0x05b, 0x05b,
// ___                                 ___
   0x05c, 0x05c, 0x05c, 0x05c,         0x05d, 0x05d, 0x05d, 0x05d,
// ___                                 ___
   0x05e, 0x05e, 0x05e, 0x05e,         0x05f, 0x05f, 0x05f, 0x05f,
// ___                                 ___
   0x060, 0x060, 0x060, 0x060,         0x061, 0x061, 0x061, 0x061,
// ___                                 ___
   0x062, 0x062, 0x062, 0x062,         0x063, 0x063, 0x063, 0x063,
// ___                                 ___
   0x064, 0x064, 0x064, 0x064,         0x065, 0x065, 0x065, 0x065,
// ___                                 ___
   0x066, 0x066, 0x066, 0x066,         0x067, 0x067, 0x067, 0x067,
// ___                                 ___
   0x068, 0x068, 0x068, 0x068,         0x069, 0x069, 0x069, 0x069,
// ___                                 ___
   0x06a, 0x06a, 0x06a, 0x06a,         0x06b, 0x06b, 0x06b, 0x06b,
// ___                                 ___
   0x06c, 0x06c, 0x06c, 0x06c,         0x06d, 0x06d, 0x06d, 0x06d,
// ___                                 ___
   0x06e, 0x06e, 0x06e, 0x06e,         0x06f, 0x06f, 0x06f, 0x06f,
// ___                                 ___
   0x070, 0x070, 0x070, 0x070,         0x071, 0x071, 0x071, 0x071,
// ___                                 ___
   0x072, 0x072, 0x072, 0x072,         0x073, 0x073, 0x073, 0x073,
// ___                                 ___
   0x074, 0x074, 0x074, 0x074,         0x075, 0x075, 0x075, 0x075,
// ___                                 ___
   0x076, 0x076, 0x076, 0x076,         0x077, 0x077, 0x077, 0x077,
// ___                                 ___
   0x078, 0x078, 0x078, 0x078,         0x079, 0x079, 0x079, 0x079,
// ___                                 ___
   0x07a, 0x07a, 0x07a, 0x07a,         0x07b, 0x07b, 0x07b, 0x07b,
// ___                                 ___
   0x07c, 0x07c, 0x07c, 0x07c,         0x07d, 0x07d, 0x07d, 0x07d,
// ___                                 ___
   0x07e, 0x07e, 0x07e, 0x07e,         0x07f, 0x07f, 0x07f, 0x07f,
// ___                                 ___
   0x080, 0x080, 0x080, 0x080,         0x081, 0x081, 0x081, 0x081,
// ___                                 ___
   0x082, 0x082, 0x082, 0x082,         0x083, 0x083, 0x083, 0x083,
// ___                                 ___
   0x084, 0x084, 0x084, 0x084,         0x085, 0x085, 0x085, 0x085,
// ___                                 ___
   0x086, 0x086, 0x086, 0x086,         0x087, 0x087, 0x087, 0x087,
// ___                                 ___
   0x088, 0x088, 0x088, 0x088,         0x089, 0x089, 0x089, 0x089,
// ___                                 ___
   0x08a, 0x08a, 0x08a, 0x08a,         0x08b, 0x08b, 0x08b, 0x08b,
// ___                                 ___
   0x08c, 0x08c, 0x08c, 0x08c,         0x08d, 0x08d, 0x08d, 0x08d,
// ___                                 ___
   0x08e, 0x08e, 0x08e, 0x08e,         0x08f, 0x08f, 0x08f, 0x08f,
// ___                                 ___
   0x090, 0x090, 0x090, 0x090,         0x091, 0x091, 0x091, 0x091,
// ___                                 ___
   0x092, 0x092, 0x092, 0x092,         0x093, 0x093, 0x093, 0x093,
// ___                                 ___
   0x094, 0x094, 0x094, 0x094,         0x095, 0x095, 0x095, 0x095,
// ___                                 ___
   0x096, 0x096, 0x096, 0x096,         0x097, 0x097, 0x097, 0x097,
// ___                                 ___
   0x098, 0x098, 0x098, 0x098,         0x099, 0x099, 0x099, 0x099,
// ___                                 ___
   0x09a, 0x09a, 0x09a, 0x09a,         0x09b, 0x09b, 0x09b, 0x09b,
// ___                                 ___
   0x09c, 0x09c, 0x09c, 0x09c,         0x09d, 0x09d, 0x09d, 0x09d,
// ___                                 ___
   0x09e, 0x09e, 0x09e, 0x09e,         0x09f, 0x09f, 0x09f, 0x09f,
// ___                                 ___
   0x0a0, 0x0a0, 0x0a0, 0x0a0,         0x0a1, 0x0a1, 0x0a1, 0x0a1,
// ___                                 ___
   0x0a2, 0x0a2, 0x0a2, 0x0a2,         0x0a3, 0x0a3, 0x0a3, 0x0a3,
// ___                                 ___
   0x0a4, 0x0a4, 0x0a4, 0x0a4,         0x0a5, 0x0a5, 0x0a5, 0x0a5,
// ___                                 ___
   0x0a6, 0x0a6, 0x0a6, 0x0a6,         0x0a7, 0x0a7, 0x0a7, 0x0a7,
// ___                                 ___
   0x0a8, 0x0a8, 0x0a8, 0x0a8,         0x0a9, 0x0a9, 0x0a9, 0x0a9,
// ___                                 ___
   0x0aa, 0x0aa, 0x0aa, 0x0aa,         0x0ab, 0x0ab, 0x0ab, 0x0ab,
// ___                                 ___
   0x0ac, 0x0ac, 0x0ac, 0x0ac,         0x0ad, 0x0ad, 0x0ad, 0x0ad,
// ___                                 ___
   0x0ae, 0x0ae, 0x0ae, 0x0ae,         0x0af, 0x0af, 0x0af, 0x0af,
// ___                                 ___
   0x0b0, 0x0b0, 0x0b0, 0x0b0,         0x0b1, 0x0b1, 0x0b1, 0x0b1,
// ___                                 ___
   0x0b2, 0x0b2, 0x0b2, 0x0b2,         0x0b3, 0x0b3, 0x0b3, 0x0b3,
// ___                                 ___
   0x0b4, 0x0b4, 0x0b4, 0x0b4,         0x0b5, 0x0b5, 0x0b5, 0x0b5,
// ___                                 ___
   0x0b6, 0x0b6, 0x0b6, 0x0b6,         0x0b7, 0x0b7, 0x0b7, 0x0b7,
// ___                                 ___
   0x0b8, 0x0b8, 0x0b8, 0x0b8,         0x0b9, 0x0b9, 0x0b9, 0x0b9,
// ___                                 ___
   0x0ba, 0x0ba, 0x0ba, 0x0ba,         0x0bb, 0x0bb, 0x0bb, 0x0bb,
// ___                                 ___
   0x0bc, 0x0bc, 0x0bc, 0x0bc,         0x0bd, 0x0bd, 0x0bd, 0x0bd,
// ___                                 ___
   0x0be, 0x0be, 0x0be, 0x0be,         0x0bf, 0x0bf, 0x0bf, 0x0bf,
// ___                                 ___
   0x0c0, 0x0c0, 0x0c0, 0x0c0,         0x0c1, 0x0c1, 0x0c1, 0x0c1,
// ___                                 ___
   0x0c2, 0x0c2, 0x0c2, 0x0c2,         0x0c3, 0x0c3, 0x0c3, 0x0c3,
// ___                                 ___
   0x0c4, 0x0c4, 0x0c4, 0x0c4,         0x0c5, 0x0c5, 0x0c5, 0x0c5,
// ___                                 ___
   0x0c6, 0x0c6, 0x0c6, 0x0c6,         0x0c7, 0x0c7, 0x0c7, 0x0c7,
// ___                                 ___
   0x0c8, 0x0c8, 0x0c8, 0x0c8,         0x0c9, 0x0c9, 0x0c9, 0x0c9,
// ___                                 ___
   0x0ca, 0x0ca, 0x0ca, 0x0ca,         0x0cb, 0x0cb, 0x0cb, 0x0cb,
// ___                                 ___
   0x0cc, 0x0cc, 0x0cc, 0x0cc,         0x0cd, 0x0cd, 0x0cd, 0x0cd,
// ___                                 ___
   0x0ce, 0x0ce, 0x0ce, 0x0ce,         0x0cf, 0x0cf, 0x0cf, 0x0cf,
// ___                                 ___
   0x0d0, 0x0d0, 0x0d0, 0x0d0,         0x0d1, 0x0d1, 0x0d1, 0x0d1,
// ___                                 ___
   0x0d2, 0x0d2, 0x0d2, 0x0d2,         0x0d3, 0x0d3, 0x0d3, 0x0d3,
// ___                                 ___
   0x0d4, 0x0d4, 0x0d4, 0x0d4,         0x0d5, 0x0d5, 0x0d5, 0x0d5,
// ___                                 ___
   0x0d6, 0x0d6, 0x0d6, 0x0d6,         0x0d7, 0x0d7, 0x0d7, 0x0d7,
// ___                                 ___
   0x0d8, 0x0d8, 0x0d8, 0x0d8,         0x0d9, 0x0d9, 0x0d9, 0x0d9,
// ___                                 ___
   0x0da, 0x0da, 0x0da, 0x0da,         0x0db, 0x0db, 0x0db, 0x0db,
// ___                                 ___
   0x0dc, 0x0dc, 0x0dc, 0x0dc,         0x0dd, 0x0dd, 0x0dd, 0x0dd,
// ___                                 ___
   0x0de, 0x0de, 0x0de, 0x0de,         0x0df, 0x0df, 0x0df, 0x0df,
// ___                                 ___
   0x0e0, 0x0e0, 0x0e0, 0x0e0,         0x0e1, 0x0e1, 0x0e1, 0x0e1,
// ___                                 ___
   0x0e2, 0x0e2, 0x0e2, 0x0e2,         0x0e3, 0x0e3, 0x0e3, 0x0e3,
// ___                                 ___
   0x0e4, 0x0e4, 0x0e4, 0x0e4,         0x0e5, 0x0e5, 0x0e5, 0x0e5,
// ___                                 ___
   0x0e6, 0x0e6, 0x0e6, 0x0e6,         0x0e7, 0x0e7, 0x0e7, 0x0e7,
// ___                                 ___
   0x0e8, 0x0e8, 0x0e8, 0x0e8,         0x0e9, 0x0e9, 0x0e9, 0x0e9,
// ___                                 ___
   0x0ea, 0x0ea, 0x0ea, 0x0ea,         0x0eb, 0x0eb, 0x0eb, 0x0eb,
// ___                                 ___
   0x0ec, 0x0ec, 0x0ec, 0x0ec,         0x0ed, 0x0ed, 0x0ed, 0x0ed,
// ___                                 ___
   0x0ee, 0x0ee, 0x0ee, 0x0ee,         0x0ef, 0x0ef, 0x0ef, 0x0ef,
// ___                                 ___
   0x0f0, 0x0f0, 0x0f0, 0x0f0,         0x0f1, 0x0f1, 0x0f1, 0x0f1,
// ___                                 ___
   0x0f2, 0x0f2, 0x0f2, 0x0f2,         0x0f3, 0x0f3, 0x0f3, 0x0f3,
// ___                                 ___
   0x0f4, 0x0f4, 0x0f4, 0x0f4,         0x0f5, 0x0f5, 0x0f5, 0x0f5,
// ___                                 ___
   0x0f6, 0x0f6, 0x0f6, 0x0f6,         0x0f7, 0x0f7, 0x0f7, 0x0f7,
// ___                                 ___
   0x0f8, 0x0f8, 0x0f8, 0x0f8,         0x0f9, 0x0f9, 0x0f9, 0x0f9,
// ___                                 ___
   0x0fa, 0x0fa, 0x0fa, 0x0fa,         0x0fb, 0x0fb, 0x0fb, 0x0fb,
// ___                                 BRANCHES
   0x0fc, 0x0fc, 0x0fc, 0x0fc,         0x0fd, 0x0fd, 0x0fd, 0x0fd,
// LEAVES                              GLASS
   0x0fe, 0x0fe, 0x0fe, 0x0fe,         0x0ff, 0x0ff, 0x0ff, 0x0ff,
};

/******************************
 * Constructors & Destructors *
 ******************************/

texture *create_texture(size_t width, size_t height) {
  texture * tx = (texture *) malloc(sizeof(texture));
  tx->width = width;
  tx->height = height;
  tx->pixels = (pixel *) calloc(width * height, sizeof(pixel));
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
  return tx;
}

void cleanup_texture(texture *tx) {
  free(tx->pixels);
  free(tx);
}

/*************
 * Functions *
 *************/

void init_textures(void) {
  texture* tx = load_texture_from_png(BLOCK_TEXTURE_FILE);
  BLOCK_ATLAS = upload_texture(tx);
  BLOCK_ATLAS_WIDTH = tx->width / BLOCK_TEXTURE_SIZE;
  BLOCK_ATLAS_HEIGHT = tx->height / BLOCK_TEXTURE_SIZE;
  cleanup_texture(tx);
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

GLuint upload_texture(texture* source) {
  GLuint result = 0;
  // Generate and bind a texture:
  glGenTextures(1, &result);
  glBindTexture( GL_TEXTURE_2D, result);

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

  // Generate mipmaps:
  glGenerateMipmap( GL_TEXTURE_2D );

  return result;
}

GLuint upload_png(char const * const filename) {
  texture* tx = load_texture_from_png(filename);
  GLuint result = upload_texture(tx);
  free(tx->pixels);
  free(tx);
  return result;
}

void tx_copy_region(
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
