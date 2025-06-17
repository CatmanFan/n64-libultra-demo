#ifndef __GFX_H__
#define __GFX_H__

#include "helpers/types.h"

extern Gfx *glistp;

extern void init_gfx();
extern void finish_gfx();
extern void clear_zfb();
extern void clear_cfb(int r, int g, int b);

#endif