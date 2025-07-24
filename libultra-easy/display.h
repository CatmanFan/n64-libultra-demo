#ifndef __DISPLAY_H__
#define __DISPLAY_H__

	extern void display_off();
	extern void display_on();
	extern void display_set(int highres);
	extern int display_width();
	extern int display_height();
	extern bool display_highres();
	extern float display_yscale();
	extern int display_tvtype();

#endif