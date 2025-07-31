Vtx sce_diamond_vtx[3] = {
	// Left
	{{ {-64, 0, 0},		0, {0, 0},	{0xB2, 0,    0, 0xFF} }},
	// Top
	{{ {0, 64, 0},		0, {0, 0},	{0xB2, 0x85, 0, 0xFF} }},
	// Bottom
	{{ {0, -64, 0},		0, {0, 0},	{0xB2, 0x85, 0, 0xFF} }},
};

Gfx sce_diamond_mesh[] = {
	gsDPPipeSync(),
	gsDPSetCycleType(G_CYC_1CYCLE),
	gsDPSetRenderMode(G_RM_AA_OPA_SURF, G_RM_AA_OPA_SURF2),
    gsDPSetColorDither(G_CD_BAYER),
	gsSPClearGeometryMode(0xFFFFFFFF),
	gsSPSetGeometryMode(G_SHADE | G_SHADING_SMOOTH),

	gsSPVertex(&(sce_diamond_vtx[0]), 3, 0),
	gsSP1Triangle(0, 1, 2, 0),
	gsSPEndDisplayList(),
};