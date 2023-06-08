#ifndef PICO8_H
#define PICO8_H

#include "fixed.h"

#include <gba_input.h>
#include <gba_types.h>

#define PAL_BG (1 << 0)
#define PAL_SPR (1 << 1)
#define PAL_SDW (1 << 2)

#define fget(n,f) ((FLAGS_DATA[(n)] >> (f)) & 1)

static s16 camx = 0;
static s16 camy = 0;
// static s16 borderx = 0;
// static s16 bordery = 0;

// vfx
void camera(s16 x, s16 y);
void pal (u8 col, u8 c1, u8 p);
void print(const char* str, u8 x, u8 y, u8 col);
u8 print_char(char c, u8 x, u8 y, bool draw);
void spr(u16 n, s16 x, s16 y, u8 layer, u8 palette, bool flip_x, bool flip_y);
void sspr(u16 n, s16 x, s16 y, int width_mode, int height_mode, u8 layer, u8 palette, bool double_size = true);
void update_screen();

// sfx
void music(s8 n, u16 fade_len, u8 channel_mask);
void sfx(u8 n);

// math
#define max(X,Y) (((X) > (Y)) ? (X): (Y))
#define min(X,Y) (((X) < (Y)) ? (X): (Y))
#define clamp(X,Y,Z) (min(Z, max(X, Y)))

#define flr(X) ((s16(X)))

int rndi(int x);
FIXED rnd(FIXED x);

// map
void map(u16 startx, u16 starty, u16 lengthx = 16, u16 lengthy = 16, u8 map_base = 4);
u8 mget(u8 x, u8 y, u8 map_base = 4, u8 x_offset = 7, u8 yoffset = 2);

#endif