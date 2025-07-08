#ifndef __GFX_3D_H__
#define __GFX_3D_H__

extern void init_camera_2d();
extern void init_camera_3d(simpleObj camera, float src_x, float src_y, float src_z, float dest_x, float dest_y, float dest_z, float fov);
extern void render_object(Gfx* obj_dl, vec3_t* obj_pos, vec3_t* obj_rot, float size);

#endif