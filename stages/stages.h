#ifndef __STAGES_H__
#define __STAGES_H__

	int current_stage;
	int target_stage;

	// MENU
	extern void menu_init();
	extern void menu_update();
	extern void menu_render();

	// STAGE 00
	extern void stage00_init();
	extern void stage00_update();
	extern void stage00_render();

	// STAGE 01
	extern void stage01_init();
	extern void stage01_update();
	extern void stage01_render();

#endif