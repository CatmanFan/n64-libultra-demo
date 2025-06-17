#include <ultra64.h>

#define NUM_SI_MSGS 4
static OSMesg msg_si[NUM_SI_MSGS];
static OSMesgQueue msgQ_si;

// This will hold the data read from the controllers at runtime
OSContPad controller[MAXCONTROLLERS];
static OSContStatus controller_status[MAXCONTROLLERS];

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
}

void reset_controller()
{
	osContReset(&msgQ_si, controller_status);
}