#ifndef terminus_header
#define terminus_header

	extern u8 terminus_img[][];
	extern u16 terminus_tlut[];
	extern Glyph terminus_glyphs[];

	Font terminus =
	{
		(u8 *)terminus_img,
		terminus_tlut,
		terminus_glyphs,
	};

#endif