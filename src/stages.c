#include <ultra64.h>
#include <string.h>
#include "libultra-easy.h"
#include "stages.h"

char *current_stage;
char *target_stage;
s32 current_stage_id;
s32 target_stage_id;
int current_stage_index;

/* *************************************************** */

#define STAGE_DECLARE(NAME)	extern void NAME##_init(); \
							extern void NAME##_update(); \
							extern void NAME##_render(); \
							extern void NAME##_destroy();

STAGE_DECLARE(test_menu)
STAGE_DECLARE(test_00)
STAGE_DECLARE(test_01)
STAGE_DECLARE(test_02)
STAGE_DECLARE(psx2n64_00)
STAGE_DECLARE(psx2n64_01)

#undef STAGE_DECLARE(NAME)

/* *************************************************** */

#define STAGE_DEFINE(id, NAME, RAW)		{ ##NAME##, id, RAW##_init, RAW##_update, RAW##_render, RAW##_destroy }
#define STAGE_DEFINE_NOD(id, NAME, RAW)	{ ##NAME##, id, RAW##_init, RAW##_update, RAW##_render, NULL }

StageInfo stages[] =
{
	STAGE_DEFINE_NOD(0, "test_menu", test_menu),
	STAGE_DEFINE_NOD(1, "test_00", test_00),
	STAGE_DEFINE_NOD(2, "test_01", test_01),
	STAGE_DEFINE_NOD(3, "test_02", test_02),
	STAGE_DEFINE_NOD(10, "psx2n64_00", psx2n64_00),
	STAGE_DEFINE(11, "psx2n64_01", psx2n64_01),
};

#undef STAGE_DEFINE(id, name)
#undef STAGE_DEFINE_NOD(id, name)

/* *************************************************** */

static int get_stage_index_with_id(s32 id)
{
	int index;
	for (index = 0; index < array_size(stages); index++) // size: 24
		if (id == stages[index].id)
			return index;

	return -1;
}

static int get_stage_index_with_name(const char *name)
{
	int index;
	for (index = 0; index < array_size(stages); index++)
		if (strcmp(stages[index].name, name) == 0)
			return index;

	return -1;
}

/**
 * @brief A function for exclusive use by the main thread.
 * 
 * This sets current_stage to the value specified in target and calls
 * the init function of the target stage.
 *
 * @param name    The name of the target stage.
 */
void change_stage(s32 id)
{
	int target_stage_index = get_stage_index_with_id(id);

	if (target_stage_index != -1)
	{
		if (stages[current_stage_index].destroy != NULL)
			stages[current_stage_index].destroy();

		current_stage = target_stage = stages[target_stage_index].name;
		current_stage_id = target_stage_id = stages[target_stage_index].id;
		current_stage_index = target_stage_index;

		stages[current_stage_index].init();
	}
}

/**
 * @brief A function for exclusive use by the main thread.
 *
 * This checks if target_stage is not equal to current_stage.
 */
bool change_stage_needed()
{
	return current_stage_id != target_stage_id;
}

void request_stage_change(const char *name)
{
	int target_stage_index = get_stage_index_with_name(name);

	if (target_stage_index != -1)
	{
		target_stage = stages[target_stage_index].name;
		target_stage_id = stages[target_stage_index].id;
	}
	else
	{
		char *msg_1 = "Requested stage not found: ";
		char msg_2[strlen(msg_1) + strlen(name) + 1];
		strcat(msg_2, msg_1);
		strcat(msg_2, name);

		crash_msg(msg_2);
	}
}