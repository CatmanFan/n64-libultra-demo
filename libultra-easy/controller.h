#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

	OSContPad controller[MAXCONTROLLERS];

	void init_controller();
	void controller_close();
	void read_controller();
	void reset_controller();
	bool joypad_is_pressed(int button, int cont);

#endif