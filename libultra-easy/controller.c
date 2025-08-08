#include <ultra64.h>
#include "libultra-easy.h"

/*
   "stick_x" and "stick_y" have a maximum of -128 to 127.
   (Recommended values are: -/+61 X, -/+ 63 Y.)
   (Incline values are:     -/+45 X, -/+ 47 Y.)

   "errno" is used for the following errors:
   - CONT_NO_RESPONSE_ERROR (i.e. controller is not inserted)
   - CONT_OVERRUN_ERROR (i.e. data transfer rate overload, should ignore data)
*/

/* =================================================== *
 *                       MACROS                        *
 * =================================================== */

#define NUM_SI_MSGS					4
#define NUM_CONTROLLER_MSGS			8*MAXCONTROLLERS

#define MSG_CONTROLLER_IDLE			0
#define MSG_CONTROLLER_READ			1
#define MSG_CONTROLLER_RESET		2
#define MSG_CONTROLLER_RUMBLE_ALL	3

/* =================================================== *
 *                 FUNCTION PROTOTYPES                 *
 * =================================================== */

static OSThread controller_thread;
static void controller_threadfunc(void *arg) __attribute__ ((noreturn));

/* =================================================== *
 *                     PROTOTYPES                      *
 * =================================================== */

extern Scheduler scheduler;

static bool controller_initialized = FALSE;
static int controller_buttons[MAXCONTROLLERS];
static vec2 controller_stick[MAXCONTROLLERS];
static int current_status;

static OSMesg			msg_si[NUM_SI_MSGS];
static OSMesgQueue		msgQ_si;
static OSMesg			msg_controller[NUM_CONTROLLER_MSGS];
static OSMesgQueue		msg_queue_controller;

// This will hold the data read from the controllers at runtime
static OSContPad controller[MAXCONTROLLERS];
static bool joypad_has_data = FALSE;

static OSContStatus controller_status[MAXCONTROLLERS];

/* =================================================== *
 *                      FUNCTIONS                      *
 * =================================================== */

void controller_close()
{
	osStopThread(&controller_thread);
	controller_initialized = FALSE;
}

void init_controller()
{
	// Init message queues
	osCreateMesgQueue(&msg_queue_controller, msg_controller, NUM_CONTROLLER_MSGS);
	osCreateMesgQueue(&msgQ_si, msg_si, NUM_SI_MSGS);
	osSetEventMesg(OS_EVENT_SI, &msgQ_si, NULL);

	// Start thread
	osCreateThread(&controller_thread, ID_CONTROLLER, controller_threadfunc, NULL, REAL_STACK(CONTROLLER), PR_CONTROLLER);
	osStartThread(&controller_thread);
}

static void controller_threadfunc(void *arg)
{
	OSMesg dummy;
	u8 bitp;
	s32 result;

	// Init SI
	osContInit(&msgQ_si, &bitp, controller_status);
	controller_initialized = TRUE;

	while (1)
	{
		osRecvMesg(&msg_queue_controller, (OSMesg *)&dummy, OS_MESG_BLOCK);

		// At this point, a message has been received
		switch ((int)dummy)
		{
			case MSG_CONTROLLER_READ:
				result = osContStartReadData(&msgQ_si);

				if (result != 0)
				{
					debug_printf("[Controller] Failed to read controller data. Code: %d\n");
				}
				else
				{
					osRecvMesg(&msgQ_si, NULL, OS_MESG_BLOCK); // Wait for controller data
					osContGetReadData(controller);
					joypad_has_data = TRUE;
				}
				break;

			case MSG_CONTROLLER_RUMBLE_ALL:
				// osMotorInit(&msgQ_si, ...);
				break;

			case MSG_CONTROLLER_RESET:
				osContReset(&msgQ_si, controller_status);
				joypad_has_data = FALSE;
				break;
		}

		current_status = MSG_CONTROLLER_IDLE;
	}
}

void read_controller()
{
	int i;
	if (!controller_initialized) return;

	if (scheduler.crash)
	{
		osContStartReadData(&msgQ_si);
		osRecvMesg(&msgQ_si, NULL, OS_MESG_BLOCK); // Wait for controller data
		osContGetReadData(controller);
		joypad_has_data = TRUE;
	}
	else if (current_status != MSG_CONTROLLER_READ)
	{
		osSendMesg(&msg_queue_controller, (OSMesg)MSG_CONTROLLER_READ, OS_MESG_BLOCK);
		current_status = MSG_CONTROLLER_READ;
	}

	// Convert from LibUltra-defined values
	for (i = 0; i < MAXCONTROLLERS; i++)
	{
		controller_buttons[i] = 0;

		if ((controller[i].button & U_JPAD) != 0)
			controller_buttons[i] |= DPAD_UP;
		if ((controller[i].button & D_JPAD) != 0)
			controller_buttons[i] |= DPAD_DOWN;
		if ((controller[i].button & L_JPAD) != 0)
			controller_buttons[i] |= DPAD_LEFT;
		if ((controller[i].button & R_JPAD) != 0)
			controller_buttons[i] |= DPAD_RIGHT;

		if ((controller[i].button & U_CBUTTONS) != 0)
			controller_buttons[i] |= C_UP;
		if ((controller[i].button & D_CBUTTONS) != 0)
			controller_buttons[i] |= C_DOWN;
		if ((controller[i].button & L_CBUTTONS) != 0)
			controller_buttons[i] |= C_LEFT;
		if ((controller[i].button & R_CBUTTONS) != 0)
			controller_buttons[i] |= C_RIGHT;

		if ((controller[i].button & A_BUTTON) != 0)
			controller_buttons[i] |= A;
		if ((controller[i].button & B_BUTTON) != 0)
			controller_buttons[i] |= B;
		if ((controller[i].button & L_TRIG) != 0)
			controller_buttons[i] |= L;
		if ((controller[i].button & R_TRIG) != 0)
			controller_buttons[i] |= R;
		if ((controller[i].button & Z_TRIG) != 0)
			controller_buttons[i] |= Z;
		if ((controller[i].button & START_BUTTON) != 0)
			controller_buttons[i] |= START;

		controller_stick[i].x = controller[i].stick_x;
		controller_stick[i].y = controller[i].stick_y;
	}
}

void reset_controller()
{
	if (!controller_initialized) return;

	if (scheduler.crash)
	{
		osContReset(&msgQ_si, controller_status);
		joypad_has_data = FALSE;
	}
	else if (current_status != MSG_CONTROLLER_RESET)
	{
		osSendMesg(&msg_queue_controller, (OSMesg)MSG_CONTROLLER_RESET, OS_MESG_BLOCK);
		current_status = MSG_CONTROLLER_RESET;
	}
}

vec2 joypad_stick(int controller_num)
{
	if (!controller_initialized) return (vec2){0};
	if (!joypad_has_data) crash_msg("Controller not read");

	return controller_stick[controller_num];
}

bool joypad_button(enum CONTROLLER_BUTTON button, int controller_num)
{
	if (!controller_initialized) return FALSE;
	if (!joypad_has_data) crash_msg("Controller not read");

	return (controller_buttons[controller_num] & button) != 0;
}