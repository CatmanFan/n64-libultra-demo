#ifndef __GFX_H__
#define __GFX_H__

#include "config/video.h"

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
void init_gfx();

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
 * @brief The number of active framebuffers, as specified by CFB_COUNT.
 */
FrameBuffer framebuffers[CFB_COUNT];

/**
 * @brief Returns the framebuffer currently in use.
 */
FrameBuffer* current_framebuffer();

/**
 * @brief The Z-Buffer. Remains stationary.
 */
u16 zbuffer[SCREEN_W_HD * SCREEN_H_HD];

#endif