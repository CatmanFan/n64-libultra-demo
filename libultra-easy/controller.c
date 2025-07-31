#include <ultra64.h>
#include "libultra-easy.h"

/* Available OR values for "button":
   - START_BUTTON
   - A_BUTTON
   - B_BUTTON

   - U_CBUTTONS
   - D_CBUTTONS
   - L_C_BUTTONS
   - R_CBUTTONS

   - U_JPAD
   - D_JPAD
   - L_JPAD
   - R_JPAD

   - L_TRIG
   - R_TRIG
   - Z_TRIG

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
static int current_status;

static OSMesg			msg_si[NUM_SI_MSGS];
static OSMesgQueue		msgQ_si;
static OSMesg			msg_controller[NUM_CONTROLLER_MSGS];
static OSMesgQueue		msg_queue_controller;

// This will hold the data read from the controllers at runtime
OSContPad controller[MAXCONTROLLERS];
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

bool joypad_is_pressed(int button, int cont)
{
	if (!controller_initialized) return FALSE;
	if (!joypad_has_data) crash_msg("Controller not read");

	return controller[cont].button == button;
}