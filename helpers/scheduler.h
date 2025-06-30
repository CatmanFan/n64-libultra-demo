#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include <PR/sched.h>

extern OSMesgQueue msgQ_pi, msgQ_gfx, msgQ_dma;
extern OSIoMesg msgIo_dma;

extern OSSched scheduler;
extern OSScTask sched_task;
extern OSScMsg *sched_msg; 	 // Received message

#endif