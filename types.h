
/* = For graphics display purposes = */

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