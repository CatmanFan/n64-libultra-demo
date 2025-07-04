#ifndef __CONSOLE_H__
#define __CONSOLE_H__

extern void console_clear();
extern void console_puts(const char *txt, ...);
extern Gfx *console_draw_dl(Gfx *dl);
extern void console_draw_raw();

#endif