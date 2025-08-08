#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

	enum CONTROLLER_BUTTON
	{
		NONE		= 0,
		DPAD_UP		= (1 << 0),
		DPAD_DOWN	= (1 << 1),
		DPAD_LEFT	= (1 << 2),
		DPAD_RIGHT	= (1 << 3),
		C_UP		= (1 << 4),
		C_DOWN		= (1 << 5),
		C_LEFT		= (1 << 6),
		C_RIGHT		= (1 << 7),
		A			= (1 << 8),
		B			= (1 << 9),
		L			= (1 << 10),
		R			= (1 << 11),
		Z			= (1 << 12),
		START		= (1 << 13),
	};

	void init_controller();
	void controller_close();
	void read_controller();
	void reset_controller();

	vec2 joypad_stick(int controller_num);
	bool joypad_button(enum CONTROLLER_BUTTON button, int controller_num);

#endif