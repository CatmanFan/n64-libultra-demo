#ifndef __GFX_2D_H__
#define __GFX_2D_H__

extern void sprite_init();
extern void sprite_finish();

extern SpriteData sprite_create_raw(void *img, int w, int h, u8 fmt, u8 siz);
extern SpriteData sprite_create(uObjSprite *obj, uObjTxtr *txtr, u32 *pal);
extern void sprite_draw(SpriteData *spr);

extern int get_nearest_multiple(int input);
extern void copy_img(void *img, int img_w, void *dest, int dest_w, int w, int h);

#endif