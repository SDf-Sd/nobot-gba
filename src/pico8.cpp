#include "pico8.h"

#include "map.h"

#include <stdlib.h>
#include <gba_video.h>
#include <gba_sprites.h>
#include <gba_systemcalls.h>


static OBJATTR obj_buffer[128] = { 0 };
static OBJAFFINE *obj_aff_buffer = (OBJAFFINE*) obj_buffer;

static const u16 palette[16] = {
	RGB8(0x00,0x00,0x00),	//black
	RGB8(0x1D,0x2B,0x53),	//dark blue
	RGB8(0x7E,0x25,0x53),	//dark purple
	RGB8(0x00,0x87,0x51),	//dark green
	RGB8(0xAB,0x52,0x36),	//brown
	RGB8(0x5F,0x57,0x4F),	//dark gray
	RGB8(0xC2,0xC3,0xC7),	//light gray
	RGB8(0xFF,0xF1,0xE8),	//white
	RGB8(0xFF,0x00,0x4D),	//red
	RGB8(0xFF,0xA3,0x00),	//orange
	RGB8(0xFF,0xEC,0x27),	//yellow
	RGB8(0x00,0xE4,0x36),	//green
	RGB8(0x29,0xAD,0xFF),	//blue
	RGB8(0x83,0x76,0x9C),	//indigo
	RGB8(0xFF,0x77,0xA8),	//pink
	RGB8(0xFF,0xCC,0xAA)	//peach
};

static u8 sprite_index = 0;
void spr(u16 n, s16 x, s16 y, u8 layer, u8 palette, bool flip_x, bool flip_y) {
	OBJATTR* obj = &(obj_buffer[sprite_index]);

	obj->attr0 = OBJ_Y(y) | ATTR0_COLOR_16 | ATTR0_SQUARE; // let height be modified
	obj->attr1 = OBJ_X(x) | ATTR1_SIZE_8; // let width be modified
	obj->attr2 = OBJ_CHAR(n) | OBJ_PRIORITY(layer) | OBJ_SQUARE | ATTR2_PALETTE(palette);

	if (flip_x)
		obj->attr1 |= ATTR1_FLIP_X;
	if (flip_y)
		obj->attr1 |= ATTR1_FLIP_Y;

	sprite_index += 1;
	if (sprite_index >= 128)
		sprite_index = 0;
}

void sspr(u16 n, s16 x, s16 y, int width_mode, int height_mode, u8 layer, u8 palette, bool double_size) {
	OBJATTR* obj = &(obj_buffer[sprite_index]);
	OBJAFFINE* obj_aff = &(obj_aff_buffer[sprite_index]);

	obj->attr0 = OBJ_Y(y) | ATTR0_COLOR_16 | height_mode; // let height be modified
	if (double_size) {
		obj->attr0 |= ATTR0_ROTSCALE_DOUBLE;
		obj_aff->pa = 0x80;
		obj_aff->pd = 0x80;
	}
	else {
		obj->attr0 |= ATTR0_ROTSCALE;
		obj_aff->pa = 0xFF;
		obj_aff->pd = 0xFF;
	}
	
	obj->attr1 = OBJ_X(x) | width_mode | ATTR1_ROTDATA(sprite_index); // let width be modified
	obj->attr2 = OBJ_CHAR(n) | OBJ_PRIORITY(layer) | ATTR2_PALETTE(0);

	obj_aff->pb = 0;
	obj_aff->pc = 0;

	sprite_index += 1;
	if (sprite_index >= 128)
		sprite_index = 0;
}

void pal(u8 c0, u8 c1, u8 p) {
	if (c0 == 0 && c1 == 0){
		for (int i = 0; i < 16; i++){
			if (p == 0 || p & PAL_BG)
				BG_PALETTE[i] = palette[i];
			if (p == 0 || p & PAL_SPR)
				SPRITE_PALETTE[i] = palette[i];
			if (p == 0 || p & PAL_SDW)
				SPRITE_PALETTE[i+16] = palette[i];
		}
	}
	else {
		if (p & PAL_BG)
			BG_PALETTE[c0] = palette[c1];
		if (p & PAL_SPR)
			SPRITE_PALETTE[c0] = palette[c1];
		if (p & PAL_SDW)
			SPRITE_PALETTE[c0 + 16] = palette[c1];
	}
}


void print(const char* str, u8 x, u8 y, u8 col) {
	x += 4;
	u8 xstart = x;

	u16 i = 0;
	while (str[i] != '\0') {
		if (str[i] == '#') {
			x = xstart;
			y += 8;
		}
		else if (str[i] == '^') {
			spr(375, x, y, 1, 0, 0, 0);
			x+=8;
		}
		else {
			spr(256+str[i]-' ', x, y, 1, 0, 0, 0);
			x+= 4;
		}
		i++;
	}
}

u8 print_char(char c, u8 x, u8 y, bool draw) {
	u8 xend = x;
	if (c == '^') {
		if (draw) spr(375, x, y, 1, 0, 0, 0);
		xend += 8;
	}
	else {
		if (draw) spr(256+c-' ', x, y, 1, 0, 0, 0);
		xend += 4;
	}
	return xend;
}

void camera(s16 x, s16 y) {
	camx = x - 56;
	camy = y - 16;

	// borderx = x + 8;
	// bordery = y + 8;
}

void update_screen() {
	REG_BG1HOFS = camx;
	REG_BG1VOFS = camy;

	// for my game specifically
	REG_BG2HOFS = -56;
	REG_BG2VOFS = camy;
	// // end

	REG_BG0HOFS = -56;
	REG_BG0VOFS = camy;

	// OBJATTR
	// *(OAM)
	// CpuFastSet(OAM, OAM, ((sizeof(OBJATTR)*128)/4) | FILL | COPY32);
	CpuFastSet(obj_buffer, OAM, ((sizeof(OBJATTR)*128)/4) | COPY32);

	for (u8 i = 0; i < 128; i++)
		obj_buffer[i].attr0 = OBJ_DISABLE;
	sprite_index = 0;
}

void map(u16 startx, u16 starty, u16 lengthx, u16 lengthy, u8 map_base) {
	u16* map_adr = (u16*) MAP_BASE_ADR(map_base);

	*((u32 *)MAP_BASE_ADR(map_base)) = 0x00000000;
	CpuFastSet(MAP_BASE_ADR(map_base), MAP_BASE_ADR(map_base), (0x800/4) | FILL | COPY32);
	for (u8 ty = starty; ty < starty+lengthy; ty++) {
		u8 map_index = 0;
		for (u8 tx = startx; tx < startx+lengthx; tx++){
			u16 tile = MAP_DATA[tx + (ty*128)]; // 128 is the amount of nums in a row
			*(map_adr + map_index) = tile;
			map_index++;
		}
		map_adr += 0x20;
	}
} 

u8 mget(u8 x, u8 y, u8 map_base, u8 xoffset, u8 yoffset) {
	u16* map_adr = (u16*) MAP_BASE_ADR(4);
	return map_adr[(x - intfxround(fxdiv(int2fx(camx), int2fx(-8)))) + ((y-yoffset)*32)];
}

FIXED rnd(FIXED x)
{
	int v = fxmul(x, int2fx(100));
	if (v == 0)
		return 0;
	
	return fxdiv(rand() % v, int2fx(100));
}