#ifndef __TYPES_H__
#define __TYPES_H__

/* ================================= */
/* =         3D Graphics           = */
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
} simpleObj;

#define vecSet(V,X,Y,Z) V[0] = X; V[1] = Y; V[2] = Z;

/* ================================= */
/* =         2D Graphics           = */
/* ================================= */

#endif