// The vertex coords
Vtx test_vertex_vtx[4] = {
	{{ {-64, 64, -5},		0, {0, 0},	{0,		0xFF,	0,		0xFF} }},
	{{ {64, 64, -5},		0, {0, 0},	{0,		0,		0,		0xFF} }},
	{{ {64, -64, -5},		0, {0, 0},	{0,		0,		0xFF,	0xFF} }},
	{{ {-64, -64, -5},		0, {0, 0},	{0xFF,	0,		0,		0xFF} }},
};

// Draw a colorful square using our vertex parameters.
Gfx test_vertex_mesh[] = {
	gsSPVertex(&(test_vertex_vtx[0]), 4, 0),

	gsDPPipeSync(),
	gsDPSetCycleType(G_CYC_1CYCLE),
	gsDPSetRenderMode(G_RM_AA_OPA_SURF, G_RM_AA_OPA_SURF2),
	gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsDPSetColorDither(G_CD_BAYER),
	gsSPClearGeometryMode(0xFFFFFFFF),
	gsSPSetGeometryMode(G_SHADE | G_SHADING_SMOOTH),

	gsSP2Triangles
	(
		0, 1, 2, 0,
		0, 2, 3, 0
	),
	gsSPEndDisplayList(),
};