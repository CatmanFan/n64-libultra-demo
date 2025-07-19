#ifndef __GFX_2D_H__
#define __GFX_2D_H__

#include <PR/gs2dex.h>
#include "libultra-easy/types.h"

/**
 * @brief Initializes S2DEX microcode
 */
extern void sprite_init();

/**
 * @brief Deinitializes S2DEX microcode
 */
extern void sprite_finish();

/**
 * @brief Creates a sprite from memory using GS2DEX types
 * 
 * @param tex          The defined texture tile(s)
 * @param pal          The TLUT palette. If it doesn't exist, set to NULL.
 * @param w            The width of each tile in pixels
 * @param h            The height of each tile in pixels
 * @param tiles        The total number of tiles
 * @param fmt          Image format
 * @param size         Image pixel size
 *
 * @return sprite_t*   The loaded sprite
 */
extern void sprite_create_tlut(sprite_t *spr, void *img, u16 *tlut, int w, int h, int frames, u8 img_siz);
extern void sprite_create(sprite_t *spr, void *img, int w, int h, int frames, u8 fmt, u8 siz);
extern void sprite_destroy(sprite_t *spr);

/**
 * @brief Draws the target sprite to the screen.
 */
extern void sprite_draw(sprite_t *spr);

extern void setup_mtx(uObjMtx *buf, int x, int y, int scale);

#endif