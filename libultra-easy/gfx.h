#ifndef __GFX_H__
#define __GFX_H__

#include "config/video.h"
#include "libultra-easy/scheduler.h"

enum FB_STATUS
{
	FB_FREE,
	FB_BUSY,
	FB_SHOWING,
	FB_READY
};

typedef struct
{
	int id;
	void *address;
	s32 size;
	Gfx *dl;
	enum FB_STATUS status;
} FrameBuffer;

typedef struct
{
	void *fb;
	void (*func)();
	OSTask *task;
	Gfx *dl;
} RenderTask;

// A struct which describes the render task from the main thread
typedef struct
{
	void (*func)();
	bool usecpu;
	bool swapbuffer;
	bool used;
} RenderMessage;

/**
 * @brief Initializes the graphics subsystem and thread.
 */
void init_gfx(Scheduler *sc);

/**
 * @brief Stops the graphics subsystem and thread.
 */
void gfx_close();

/**
 * @brief Sends a render task request to the graphics thread.
 *
 * @param[in] func
 *            Name of the render callback function
 * @param[in] usecpu
 *            Determines whether to render directly to the framebuffer using the CPU
 * @param[in] swapbuffer
 *            Determines whether to swap the current buffer after rendering
 */
void gfx_request_render(void (*func)(), bool usecpu, bool swapbuffer);

/**
 * @brief The number of total framebuffers, as specified by CFB_COUNT.
 */
FrameBuffer framebuffers[3];

/**
 * @brief The number of currently active framebuffers.
 */
int num_active_framebuffers();

/**
 * @brief The Z-Buffer. Remains stationary.
 */
u16 *zbuffer __attribute__((aligned(16)));

#endif