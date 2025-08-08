#ifndef __DISPLAY_H__
#define __DISPLAY_H__

	void display_off();
	void display_on();
	void display_set(int highres);
	int display_width();
	int display_height();
	bool display_highres();
	float display_yscale();
	int display_tvtype();
	int display_framerate();

#endif