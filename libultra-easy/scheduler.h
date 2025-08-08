#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

    /*********************************
               Definitions
    *********************************/

    // Max number of messages for the scheduler message queue
    #define SC_MSG_COUNT  12
    
    // Scheduler messages
	#define SC_MSG_IDLE    0
	#define SC_MSG_VSYNC   1 // Vertical retrace
	#define SC_MSG_DP      2 // Display processing complete (gDPFullSync)
	#define SC_MSG_SP      3 // RSP task complete
	#define SC_MSG_AUDIO   4 // Audio buffer consumption complete
	#define SC_MSG_PRENMI  5 // Trigger Pre-NMI (reset) event
	#define SC_MSG_RCPDEAD 6

    /*********************************
                 Structs
    *********************************/

    typedef struct {
		bool         initialized;

        bool         display;
		bool         is_changing_res;
        bool         reset;      // Whether the reset button was recently pressed
		bool         crash;      // Whether the crash thread was executed
		int          current_status;

        OSMesg       event_msg[SC_MSG_COUNT];
        OSMesgQueue  event_queue;

        OSTask*      task_gfx;    // Current executing graphics task
        OSTask*      task_audio;  // Current executing audio task

        OSMesgQueue* gfx_notify;
        OSMesgQueue* audio_notify;
    } Scheduler;

    /*********************************
                Functions
    *********************************/

    Scheduler* init_scheduler();

#endif