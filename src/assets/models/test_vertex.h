// The vertex coords
Vtx test_vertex_vtx[4] = {
	// Top-left
	{{ {-64, 64, -5},		0, {0, 0},	{0,		0xFF,	0,		0xFF} }},
	// Top-right
	{{ {64, 64, -5},		0, {0, 0},	{0,		0,		0,		0xFF} }},
	// Bottom-right
	{{ {64, -64, -5},		0, {0, 0},	{0,		0,		0xFF,	0xFF} }},
	// Bottom-left
	{{ {-64, -64, -5},		0, {0, 0},	{0xFF,	0,		0,		0xFF} }},
};

// Draw a colorful square using our vertex parameters.
Gfx test_vertex_mesh[] = {
	gsDPPipeSync(),
	gsDPSetCycleType(G_CYC_1CYCLE),
	gsDPSetRenderMode(G_RM_AA_OPA_SURF, G_RM_AA_OPA_SURF2),
    gsDPSetColorDither(G_CD_MAGICSQ),
	gsSPClearGeometryMode(0xFFFFFFFF),
	gsSPSetGeometryMode(G_SHADE | G_SHADING_SMOOTH),

	gsSPVertex(&(test_vertex_vtx[0]), 4, 0),
	gsSP1Triangle(0, 1, 2, 0),
	gsSP1Triangle(0, 2, 3, 0),
	gsSPEndDisplayList(),
};