#ifndef __TYPES_H__
#define __TYPES_H__

/* ================================= */
/*              Helpers              */
/* ================================= */

// Boolean
typedef int bool;
#define TRUE 1
#define FALSE 0

/* ================================= */
/*            3D Graphics            */
/* ================================= */

// 3-point vector
typedef float vec3_t[3];

// 2-point vector
typedef struct {
	float x;
	float y;
} vec2;

// Simple 3D object
typedef struct {
	vec3_t pos;
	vec3_t mov; // For camera interpolation
	vec3_t rot;
	Gfx*   dl;
} simpleObj;

#define vecSet(V,X,Y,Z) V[0] = X; V[1] = Y; V[2] = Z;

/* ================================= */
/*            2D Graphics            */
/* ================================= */

#include <PR/gs2dex.h>

typedef struct sprite_data {
	uObjTxtr *txtr;
	u32 *pal;
	uObjMtx *mtx;
	uObjSprite *obj;

	int tile_w;
	int tile_h;

	int x;
	int y;

	int r;
	int g;
	int b;
	int a;
} SpriteData;

typedef struct {
	unsigned long chr;
	int glyph_index;
	int width;
	int height;
} Glyph;

typedef struct {
	u8 img;
	Glyph glyphs;
} Font;

#endif