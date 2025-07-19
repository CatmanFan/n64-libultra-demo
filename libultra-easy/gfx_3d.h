#ifndef __GFX_3D_H__
#define __GFX_3D_H__

	void init_camera_2d();
	void init_camera_3d(vec3 src, vec3 dest, float fov);
	void render_object(simpleObj *obj);

#endif