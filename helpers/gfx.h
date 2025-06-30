#ifndef __GFX_H__
#define __GFX_H__

#include "helpers/types.h"

extern Gfx *glistp;

extern void init_gfx();
extern void finish_gfx();
extern void clear_zfb();
extern void clear_cfb(int r, int g, int b);
extern void swap_cfb(int index);

extern void init_sprite();
extern void draw_sprite(Sprite *sp);
extern void finish_sprite();

extern void draw_pixel(int x, int y, unsigned int color);
extern void draw_rectangle(int x, int y, int w, int h);

#endif