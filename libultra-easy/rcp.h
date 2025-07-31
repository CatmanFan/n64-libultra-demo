#ifndef __GFX_RCP_H__
#define __GFX_RCP_H__

	#include "libultra-easy/types.h"

	Gfx *glistp;

	void rcp_start();
	void rcp_init_rdp();
	void rcp_init_rsp();
	void rcp_finish();

	void clear_zfb();
	void clear_cfb(int r, int g, int b);

	void draw_rectangle(int x, int y, int w, int h, int color);
	void draw_gradient(int x, int y, int w, int h, int color1, int color2, bool horizontal);

#endif