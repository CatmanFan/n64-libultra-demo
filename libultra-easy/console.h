#ifndef __CONSOLE_H__
#define __CONSOLE_H__

	#include "libultra-easy/gfx.h"

	void console_clear();
	void console_puts(const char *txt, ...);
	void console_draw_dl();
	void console_draw_raw();
	bool console_set_fb();

#endif