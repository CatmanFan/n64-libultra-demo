#ifndef psx_bios_font_header
#define psx_bios_font_header

	extern u8 psx_bios_font_img[];
	extern u16 psx_bios_font_tlut[];
	extern Glyph psx_bios_font_glyphs[];

	Font psx_bios_font =
	{
		.bmp			= psx_bios_font_img,
		.bmp_width		= 640,
		.bmp_height		= 54,
		.multi_bmp		= FALSE,
		.tlut			= psx_bios_font_tlut,
		.glyphs			= psx_bios_font_glyphs,
		.glyph_count	= 100
	};

#endif