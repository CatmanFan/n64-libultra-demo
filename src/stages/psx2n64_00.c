#include <ultra64.h>

/* === Configuration === */
#include "config/video.h"

/* === Default libraries === */
#include "libultra-easy/types.h"
#include "libultra-easy/audio.h"
#include "libultra-easy/console.h"
#include "libultra-easy/controller.h"
#include "libultra-easy/display.h"
#include "libultra-easy/fault.h"
#include "libultra-easy/rcp.h"
#include "libultra-easy/gfx_2d.h"
#include "libultra-easy/gfx_3d.h"
#include "libultra-easy/fs.h"
#include "libultra-easy/time.h"

/* === Custom libraries === */
#include "psx_config.h"
#include "strings.h"
#include "stages.h"

/* [ASSETS]
========================================= */
#include "assets/models/diamond.h"

#include "assets/sprites/biosA_sce0.h"
#include "assets/sprites/biosA_sce1.h"
#include "assets/sprites/biosB_sce0.h"
#include "assets/sprites/biosB_sce1.h"
#include "assets/sprites/white.h"

/* [SEGMENTS]
========================================= */
SEGMENT_DECLARE(bgmCtl_playstation)
SEGMENT_DECLARE(bgmTbl_playstation)
SEGMENT_DECLARE(bgm_sce_logo_0)

/* [VARIABLES]
========================================= */
static vec3 init_pos_1;
static vec3 init_pos_2;
static vec3 end_pos_1;
static vec3 end_pos_2;
static double init_scale;
static double end_scale;

static f64 time_elapsed;
static f64 time_1;
static f64 time_2;
static f64 time_3;
static f64 time_logo_fade;
static f64 time_fadeout;
static f64 time_end;
static int time_4;

static sprite_t sce_text_0;
static sprite_t sce_text_1;
static sprite_t fade;
static bool fadeout;

static int scale_2d;
static int wait_transition;

static float BIOS = BIOS_VER;

/* [3D MODELS]
========================================= */
static simpleObj diamond_base_1 = { .dl = sce_diamond_mesh, .scale = 2.0 };
static simpleObj diamond_base_2 = { .dl = sce_diamond_mesh, .scale = 2.0 };
static simpleObj diamond_face_1 = { .dl = sce_diamond_mesh };
static simpleObj diamond_face_2 = { .dl = sce_diamond_mesh };

/* [STATIC FUNCTIONS]
========================================= */

/* [MAIN FUNCTIONS]
========================================= */
#include <PR/os_internal_error.h>
#include "config/global.h"

/* ==============================
 * Initializes stage.
 * ============================== */
void psx2n64_00_init()
{
	if (BIOS >= 5.0)
	{
		request_stage_change("psx2n64_01");
		return;
	}

	time_reset();

	display_set(1);
	scale_2d = display_highres() ? 1 : 2;

	time_elapsed = 0.0;
	time_1 = BIOS < 4.0 ? 1 : 0.5;
	time_2 = time_1 + 1;
	time_3 = time_2 + 1;
	time_logo_fade = BIOS < 4.0 ? 0 : 0.25;
	time_4 = 2;
	time_fadeout = 12.5 / 60.0;
	time_end = time_3 + time_4 + time_fadeout;

	diamond_base_2.pos.z = diamond_base_1.pos.z = -50;
	diamond_base_2.rot.z = diamond_face_2.rot.z = 180;

	diamond_face_1.pos.x = init_pos_1.x = 0;
	diamond_face_1.pos.y = init_pos_1.y = 0;
	diamond_face_1.pos.z = init_pos_1.z = diamond_base_1.pos.z + 0.5;
	diamond_face_2.pos.x = init_pos_2.x = init_pos_1.x * -1;
	diamond_face_2.pos.x = init_pos_2.y = init_pos_1.y * -1;
	diamond_face_2.pos.x = init_pos_2.z = diamond_base_1.pos.z + 0.1;

	end_pos_1.x = 9;
	end_pos_1.y = 48;
	end_pos_1.z = init_pos_1.z;
	end_pos_2.x = end_pos_1.x * -0.995;
	end_pos_2.y = end_pos_1.y * -0.995;
	end_pos_2.z = init_pos_2.z;

	init_scale = diamond_base_2.scale = diamond_base_1.scale;
	end_scale = 0.890001;
	diamond_face_1.scale = init_scale;
	diamond_face_2.scale = init_scale;

	sprite_create_tlut(&fade, white_tex, white_tlut, 32, 32, 1, G_IM_SIZ_4b);
	fade.r = fade.g = fade.b = 0;
	fade.a = 255;
	fade.scale_x = 600.0;
	fade.scale_y = 400.0;
	fadeout = FALSE;
	wait_transition = 0;

	// sprite_create_tlut(&sce_text_0, pyoro_tex, pyoro_tlut, 16, 16, 1, G_IM_SIZ_4b);
	if (BIOS < 4.0)
	{
		sprite_create_tlut(&sce_text_0, BIOS1_sceText0_tex, BIOS1_sceText0_tlut, 240, 43, 1, G_IM_SIZ_8b);
		sprite_create_tlut(&sce_text_1, BIOS1_sceText1_tex, BIOS1_sceText1_tlut, 240, 53, 1, G_IM_SIZ_8b);
	}
	else
	{
		sprite_create_tlut(&sce_text_0, BIOS2_sceText0_tex, BIOS2_sceText0_tlut, 200, 36, 1, G_IM_SIZ_4b);
		sprite_create_tlut(&sce_text_1, BIOS2_sceText1_tex, BIOS2_sceText1_tlut, 196, 43, 1, G_IM_SIZ_4b);
	}
	sce_text_0.x = (BIOS < 4.0 ? 200 : 220) / scale_2d;
	sce_text_0.y = (BIOS < 4.0 ? 57 + 1 : 64 + 1) / scale_2d;
	sce_text_1.x = (BIOS < 4.0 ? 200 : 222) / scale_2d;
	sce_text_1.y = (BIOS < 4.0 ? 384 + 1 : 376 + 1) / scale_2d;
	sce_text_1.a = sce_text_0.a = 0;
	sce_text_1.scale_x = sce_text_0.scale_x = 1.0 / scale_2d;
	sce_text_1.scale_y = sce_text_0.scale_y = 1.0 / scale_2d;

	if (__osGetCurrFaultedThread() != NULL && osGetThreadId(__osGetCurrFaultedThread()) == ID_AUDIO)
		crash_msg("Cannot continue, audio failed");

	music_load_bank(playstation);
	music_play(bgm_sce_logo_0, 0, 0, 0, 0);
}

/* ==============================
 * Updates variables based on
 * controller input, time, etc.
 * ============================== */
void psx2n64_00_update()
{
	if (BIOS < 5.0)
	{
		time_elapsed = time_current();

		if (time_elapsed >= time_2 && time_elapsed < time_3)
		{
			f64 percA = -1.0 * (time_elapsed - time_3);
			f64 percB = 1.0 * (time_elapsed - time_2);

			diamond_face_1.scale = diamond_face_2.scale = (init_scale * percA) + (end_scale * percB);

			diamond_face_1.pos.x = (init_pos_1.x * percA) + (end_pos_1.x * percB);
			diamond_face_1.pos.y = (init_pos_1.y * percA) + (end_pos_1.y * percB);
			diamond_face_1.pos.z = (init_pos_1.z * percA) + (end_pos_1.z * percB);

			diamond_face_2.pos.x = (init_pos_2.x * percA) + (end_pos_2.x * percB);
			diamond_face_2.pos.y = (init_pos_2.y * percA) + (end_pos_2.y * percB);
			diamond_face_2.pos.z = (init_pos_2.z * percA) + (end_pos_2.z * percB);
		}

		else if (time_elapsed >= time_3)
		{
			diamond_face_1.pos.x = end_pos_1.x;
			diamond_face_1.pos.y = end_pos_1.y;
			diamond_face_1.pos.z = end_pos_1.z;

			diamond_face_2.pos.x = end_pos_2.x;
			diamond_face_2.pos.y = end_pos_2.y;
			diamond_face_2.pos.z = end_pos_2.z;

			diamond_face_1.scale = diamond_face_2.scale = end_scale;

			sce_text_1.a = sce_text_0.a = time_elapsed < time_3 ? 0
										: time_elapsed > time_3 + time_logo_fade || time_logo_fade <= 0 ? 255
										: 255 * ((time_elapsed - time_3) / time_logo_fade);
		}

		if (time_elapsed >= time_3 + time_4 && !fadeout)
		{
			fadeout = TRUE;
		}

		if (!fadeout)
			fade.a = time_elapsed < time_1 ? 255 : time_elapsed < time_2 ? round(255.0 * (time_1 - time_elapsed)) : 0;
		else
		{
			fade.a = round(255.0 * ((time_elapsed - (time_3 + time_4)) / time_fadeout));
			if (fade.a > 255 || time_elapsed >= time_end)
			{
				fade.a = 255;
				wait_transition++;
				if (wait_transition == 2)
					request_stage_change("psx2n64_01");
			}
		}
	}
}

extern Acmd *cl_start, *cl_end;
extern s32 cl_length;

/* ==============================
 * Renders frame.
 * ============================== */
void psx2n64_00_render()
{
	if (BIOS < 5.0)
	{
		vec3 cam = {4, 4, 609};
		vec3 dest = {0, 0, -1};

		clear_zfb();
		gDPSetColorDither(glistp++, G_CD_DISABLE);
		clear_cfb(fade.a < 255 ? 176 : 0, fade.a < 255 ? 176 : 0, fade.a < 255 ? 176 : 0);

		if (!fadeout)
		{
			if (time_elapsed >= time_2)
			{
				init_camera_3d(cam, dest, 40);
				render_object(&diamond_base_1);
				render_object(&diamond_base_2);
				render_object(&diamond_face_2);
				render_object(&diamond_face_1);
				gDPPipeSync(glistp++);
			}

			if (time_elapsed >= time_3)
			{
				sprite_init();
				sprite_draw(&sce_text_0);
				sprite_draw(&sce_text_1);
				sprite_finish();
			}
		}

		sprite_init();
		sprite_draw(&fade);
		sprite_finish();
	}

	#ifdef VERBOSE
	console_clear();
	// console_puts("Audio command list length dynamic: %d, %d", cl_end - cl_start, cl_length * sizeof(Acmd));
	// console_puts("Audio command list length direct: %d", cl_length);
	// console_puts("Audio command list type size: %d", sizeof(Acmd));

	console_puts("Stage time: %0.2f / %0.2f", time_elapsed, time_end);
	console_puts("Next marker in %0.2f\n", (time_elapsed < time_1 ? time_1 : time_elapsed < time_2 ? time_2 : time_elapsed < time_3 ? time_3 : time_elapsed < time_3 + time_4 ? time_3 + time_4 : time_end) - time_elapsed);
	console_puts("FPS: %2d", fps());
	if (display_tvtype() == 0) { console_puts("Video mode: PAL"); }
	if (display_tvtype() == 1) { console_puts("Video mode: NTSC"); }
	if (display_tvtype() == 2) { console_puts("Video mode: MPAL"); }
	console_draw_dl();
	#endif
}