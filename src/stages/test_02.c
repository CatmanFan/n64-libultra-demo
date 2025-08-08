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
#include "libultra-easy/fs.h"
#include "libultra-easy/rcp.h"
// #include "libultra-easy/gfx_2d.h"
// #include "libultra-easy/gfx_3d.h"
#include "libultra-easy/time.h"

/* === Custom libraries === */
#include "stages.h"
#include "strings.h"

/* [VARIABLES]
========================================= */
static u8 header[0x30];
static u8 *header_ptr;
static bool isReadHeader;

/* [STATIC FUNCTIONS]
========================================= */
static void read_header()
{
	header_ptr = header;
	load_from_rom(header_ptr, (char *)0x00, 0x30);
	isReadHeader = header_ptr[0x2B] != 0;
}

/* [MAIN FUNCTIONS]
========================================= */

/* ==============================
 * Initializes stage.
 * ============================== */
void test_02_init()
{
	read_header();
}

/* ==============================
 * Updates variables based on
 * controller input, time, etc.
 * ============================== */
void test_02_update()
{
	if (joypad_button(B, 0))
		request_stage_change("test_menu");
}

/* ==============================
 * Renders frame.
 * ============================== */
void test_02_render()
{
	clear_zfb();
	clear_cfb(64, 64, 64);

	console_clear();
	console_puts(strings[9]);
	if (isReadHeader)
	{
		// int i;
		char code[4];
		code[0] = header_ptr[0x2B];
		code[1] = header_ptr[0x2C];
		code[2] = header_ptr[0x2D];
		code[3] = header_ptr[0x2E];

		console_puts(strings[10]);

		/*for (i = 0; i < 64; i+=16)
		{
			console_puts("%2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x ",

			header_ptr[i+0], header_ptr[i+1], header_ptr[i+2], header_ptr[i+3],
			header_ptr[i+4], header_ptr[i+5], header_ptr[i+6], header_ptr[i+7],
			header_ptr[i+8], header_ptr[i+9], header_ptr[i+10], header_ptr[i+11],
			header_ptr[i+12], header_ptr[i+13], header_ptr[i+14], header_ptr[i+15]);
		}*/

		console_puts(strings[11], code);

		switch (header_ptr[0x2E])
		{
			default:
				console_puts("Region: ?");
				break;
			case '7':
				console_puts("Region: Beta");
				break;
			case 'A':
				console_puts("Region: NTSC-Asia");
				break;
			case 'C':
				console_puts("Region: China");
				break;
			case 'G':
				console_puts("Region: Gateway-NTSC");
				break;
			case 'L':
				console_puts("Region: Gateway-PAL");
				break;
			case 'J':
				console_puts("Region: Japan");
				break;
			case 'K':
				console_puts("Region: Korea");
				break;
			case 'E':
				console_puts("Region: America");
				break;
			case 'B':
				console_puts("Region: Brasil");
				break;
			case 'P':
			case 'X':
			case 'Y':
				console_puts("Region: PAL");
				break;
			case 'D':
				console_puts("Region: Deutschland");
				break;
			case 'H':
				console_puts("Region: Nederland");
				break;
			case 'I':
				console_puts("Region: Italia");
				break;
			case 'F':
				console_puts("Region: France");
				break;
			case 'S':
				console_puts("Region: España");
				break;
			case 'U':
				console_puts("Region: Australia");
				break;
			case 'W':
				console_puts("Region: Norden");
				break;
		}
	}
	else { console_puts(strings[12]); }

	console_puts("\n");
	console_puts(strings[13]);
	console_draw_dl();
}