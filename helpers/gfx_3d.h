#ifndef __RENDERER_H__
#define __RENDERER_H__

	extern void init_world(simpleObj camera, int x, int y, int z, float fov);
	extern void render_object(Gfx* obj_dl, vec3_t* obj_pos, vec3_t* obj_rot, float size);

#endif