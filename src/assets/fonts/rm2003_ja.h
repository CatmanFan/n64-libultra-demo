#ifndef rm2003_ja_header
#define rm2003_ja_header

	extern u8 rm2003_ja_img[][];
	extern u16 terminus_tlut[];
	extern Glyph rm2003_ja_glyphs[];

	Font rm2003_ja =
	{
		(u8 *)rm2003_ja_img,
		terminus_tlut,
		rm2003_ja_glyphs,
	};

#endif