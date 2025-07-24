#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

	extern OSContPad controller[];

	extern void init_controller();
	extern void read_controller();
	extern void reset_controller();
	extern bool joypad_is_pressed(int button, int cont);

#endif