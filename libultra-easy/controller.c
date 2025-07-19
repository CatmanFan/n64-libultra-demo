#include <ultra64.h>
#include "libultra-easy/types.h"
#include "libultra-easy/crash.h"

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

/* ============= PROTOTYPES ============ */

#define NUM_SI_MSGS 4
static OSMesg msg_si[NUM_SI_MSGS];
static OSMesgQueue msgQ_si;

// This will hold the data read from the controllers at runtime
OSContPad controller[MAXCONTROLLERS];
static bool joypad_has_data = FALSE;

static OSContStatus controller_status[MAXCONTROLLERS];

/* ============= FUNCTIONS ============== */

bool joypad_is_pressed(Button button, int cont)
{
	if (!joypad_has_data)
		crash_msg("Controller not read");
	return controller[cont].button == button;
}

void init_controller()
{
	u8 bitp;

	// Init message queue
	osCreateMesgQueue(&msgQ_si, msg_si, NUM_SI_MSGS);
	osSetEventMesg(OS_EVENT_SI, &msgQ_si, NULL);

	// Init SI
	osContInit(&msgQ_si, &bitp, controller_status);
}

void read_controller()
{
	osContStartReadData(&msgQ_si);
	osRecvMesg(&msgQ_si, NULL, OS_MESG_BLOCK); // Wait for controller data
	osContGetReadData(controller);
	joypad_has_data = TRUE;
}

void reset_controller()
{
	osContReset(&msgQ_si, controller_status);
	joypad_has_data = FALSE;
}