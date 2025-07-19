#include <ultra64.h>
#include "libultra-easy.h"

u32 current_stage;
u32 target_stage;
int current_stage_index;

typedef struct stage_info
{
	u32 id;
	void (*init)();
	void (*update)();
	void (*render)();
	void (*destroy)();
} StageInfo;

/* *************************************************** */

#define STAGE(NAME)	extern void NAME##_init(); \
					extern void NAME##_update(); \
					extern void NAME##_render(); \
					extern void NAME##_destroy();

STAGE(test_menu)
STAGE(test_00)
STAGE(test_01)
STAGE(psx2n64_00)
STAGE(psx2n64_01)

#undef STAGE(NAME)

/* *************************************************** */

StageInfo stages[] =
{
   // ID | Init           | Update           | Render           | Destroy //
	{ -1, test_menu_init,  test_menu_update,  test_menu_render,  NULL },
	{ 0,  test_00_init,    test_00_update,    test_00_render,    NULL },
	{ 1,  test_01_init,    test_01_update,    test_01_render,    NULL },
	{ 10, psx2n64_00_init, psx2n64_00_update, psx2n64_00_render, NULL },
	{ 11, psx2n64_01_init, psx2n64_01_update, psx2n64_01_render, psx2n64_01_destroy },
};

/* *************************************************** */

int get_stage_index(u32 id)
{
	int index;
	for (index = 0; index < sizeof(stages); index++)
		if (id == stages[index].id)
			return index;

	return -1;
}

void change_stage(u32 id)
{
	int target_stage_index = get_stage_index(id);
	if (target_stage_index != -1)
	{
		if (stages[current_stage_index].destroy != NULL)
			stages[current_stage_index].destroy();

		current_stage = target_stage = id;
		current_stage_index = target_stage_index;

		stages[current_stage_index].init();
	}
}

bool change_stage_needed()
{
	return current_stage != target_stage;
}