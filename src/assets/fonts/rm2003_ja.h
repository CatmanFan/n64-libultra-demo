#ifndef rm2003_ja_header
#define rm2003_ja_header

	extern u8 rm2003_ja_img[][];
	extern u16 terminus_tlut[];
	extern Glyph rm2003_ja_glyphs[];

	Font rm2003_ja =
	{
		.bmp			= (u8 *)rm2003_ja_img,
		.bmp_width		= 12,
		.bmp_height		= 12,
		.multi_bmp		= TRUE,
		.tlut			= terminus_tlut,
		.glyphs			= rm2003_ja_glyphs,
		.glyph_count	= 274
	};

#endif