#ifndef __TYPES_H__
#define __TYPES_H__

typedef enum joypad_button
{
	A = A_BUTTON,
	B = B_BUTTON,
	Start = START_BUTTON,
	Up = U_JPAD,
	Down = D_JPAD,
	Left = L_JPAD,
	Right = R_JPAD,
	C_Up = U_CBUTTONS,
	C_Down = D_CBUTTONS,
	C_Left = L_CBUTTONS,
	C_Right = R_CBUTTONS,
	L = L_TRIG,
	R = R_TRIG,
	Z = Z_TRIG,
} Button;

#define RGBA32(r,g,b,a) ((r << 24) | (g << 16) | (b << 8) | a)

/* ================================= */
/*              Helpers              */
/* ================================= */

// Boolean
typedef int bool;
#define TRUE 1
#define FALSE 0

#define round(n) (n < 0.0F ? ((s32)((f64)n - 0.5F)) : ((s32)((f64)n + 0.5F)))

#define array_size(x) (sizeof(x) / sizeof(*(x)))

/* ================================= */
/*            3D Graphics            */
/* ================================= */

// 3-point vector
typedef struct {
	float x;
	float y;
	float z;
} vec3;

// 2-point vector
typedef struct {
	float x;
	float y;
} vec2;

#define vec3_set(V,X,Y,Z) V.x = X; V.y = Y; V.z = Z;
#define vec2_set(V,X,Y) V.x = X; V.y = Y;

// Simple 3D object
typedef struct {
	Gfx* dl;
	vec3 pos;
	vec3 mov; // For camera interpolation
	vec3 rot;
	float scale;
} simpleObj;

/* ================================= */
/*            2D Graphics            */
/* ================================= */

#include <PR/gs2dex.h>

enum sprite_img_type {
	SPRITE_RGBA16,
	SPRITE_CI8,
	SPRITE_CI4,
};

typedef struct {
	void *tex;
	u16 *tlut;
	int frames;

	int w;
	int h;

	u8 img_fmt;
	u8 img_siz;
	u32 tlut_fmt;
} sprite_data;

typedef struct {
	sprite_data data;

	int x;
	int y;

	int r;
	int g;
	int b;
	int a;

	f64 scale_x;
	f64 scale_y;

	int frame;
} sprite_t;

typedef struct {
	unsigned long chr;
	int glyph_index;
	int width;
	int height;
	int x;
	int y;
} Glyph;

typedef struct {
	u8 *bmp;
	int bmp_width;
	int bmp_height;
	bool multi_bmp;
	u16 *tlut;
	Glyph *glyphs;
	int glyph_count;
} Font;

#endif