/**
 * @file z_viscvg.c
 * Description: Visualise Coverage in various ways.
 *
 * Coverage is roughly how much of a pixel is covered by a primitive; it is used for antialiasing, see PreRender.c and
 * §15 of the programming manual for details.
 *
 * To understand this file, it is helpful to remember that A_MEM is essentially synonymous with coverage, and that
 * `GBL_c1/2(p, a, m, b)` are usually `(p * a + m * b) / (a + b)`.
 *
 * Coverage is full when not on an edge, while on an edge it is usually lower, and since coverage is treated as an
 * alpha value, edges with lower coverage will show up as darker than interiors in all of the available modes.
 *
 * Coverage is abbreviated to "cvg"; "FB RGB" ("framebuffer red/green/blue") is the color the pixel had before the
 * filter is applied.
 */

#include "global.h"

/**
 * Draws only coverage: does not retain any of the original pixel RGB, primColor is used as background color.
 */
Gfx sCoverageOnlyDL[] = {
    gsDPSetOtherMode(G_AD_PATTERN | G_CD_MAGICSQ | G_CK_NONE | G_TC_CONV | G_TF_POINT | G_TT_NONE | G_TL_TILE |
                         G_TD_CLAMP | G_TP_NONE | G_CYC_1CYCLE | G_PM_NPRIMITIVE,
                     G_AC_NONE | G_ZS_PRIM | G_RM_VISCVG | G_RM_VISCVG2),
    // (blendColor RGB) * (cvg)
    gsDPFillRectangle(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1),
    gsDPPipeSync(),
    gsDPSetBlendColor(0, 0, 0, 8),
    gsSPEndDisplayList(),
};

/**
 * Draws fog + coverage * RGB of pixels
 *
 * @bug This easily overflows the blender because the fog value is added to the coverage value.
 */
Gfx sCoverageRGBFogDL[] = {
    gsDPSetOtherMode(G_AD_PATTERN | G_CD_MAGICSQ | G_CK_NONE | G_TC_CONV | G_TF_POINT | G_TT_NONE | G_TL_TILE |
                         G_TD_CLAMP | G_TP_NONE | G_CYC_1CYCLE | G_PM_NPRIMITIVE,
                     G_AC_NONE | G_ZS_PRIM | IM_RD | CVG_DST_CLAMP | ZMODE_OPA | FORCE_BL |
                         GBL_c1(G_BL_CLR_FOG, G_BL_A_FOG, G_BL_CLR_MEM, G_BL_A_MEM) |
                         GBL_c2(G_BL_CLR_FOG, G_BL_A_FOG, G_BL_CLR_MEM, G_BL_A_MEM)),
    // (fog RGB) * (fog alpha) + (FB RGB) * (cvg)
    gsDPFillRectangle(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1),
    gsSPEndDisplayList(),
};

/**
 * Draws coverage and RGB of pixels
 */
Gfx sCoverageRGBDL[] = {
    gsDPSetOtherMode(G_AD_PATTERN | G_CD_MAGICSQ | G_CK_NONE | G_TC_CONV | G_TF_POINT | G_TT_NONE | G_TL_TILE |
                         G_TD_CLAMP | G_TP_NONE | G_CYC_1CYCLE | G_PM_NPRIMITIVE,
                     G_AC_NONE | G_ZS_PRIM | IM_RD | CVG_DST_CLAMP | ZMODE_OPA | FORCE_BL |
                         GBL_c1(G_BL_CLR_IN, G_BL_0, G_BL_CLR_MEM, G_BL_A_MEM) |
                         GBL_c2(G_BL_CLR_IN, G_BL_0, G_BL_CLR_MEM, G_BL_A_MEM)),
    // (FB RGB) * (cvg)
    gsDPFillRectangle(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1),
    gsSPEndDisplayList(),
};

/**
 * Two stage filtering:
 *
 * 1. Apply a uniform color filter by transparently blending primColor with original frame. The "cloud surface"
 * RenderMode is used to preserve the coverage for the second stage.
 * 2. Second half is the same as sCoverageRGBDL's, i.e. (RGB from stage 1) * cvg
 */
Gfx sCoverageRGBPrimFogDL[] = {
    gsDPSetCombineMode(G_CC_PRIMITIVE, G_CC_PRIMITIVE),
    gsDPSetOtherMode(G_AD_NOTPATTERN | G_CD_DISABLE | G_CK_NONE | G_TC_CONV | G_TF_POINT | G_TT_NONE | G_TL_TILE |
                         G_TD_CLAMP | G_TP_NONE | G_CYC_1CYCLE | G_PM_NPRIMITIVE,
                     G_AC_NONE | G_ZS_PRIM | G_RM_CLD_SURF | G_RM_CLD_SURF2),
    // stage 1 color = (primColor RGB) * (primColor Alpha) + (FB RGB) * (1 - primColor Alpha)
    gsDPFillRectangle(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1),

    gsDPSetOtherMode(G_AD_PATTERN | G_CD_MAGICSQ | G_CK_NONE | G_TC_CONV | G_TF_POINT | G_TT_NONE | G_TL_TILE |
                         G_TD_CLAMP | G_TP_NONE | G_CYC_1CYCLE | G_PM_NPRIMITIVE,
                     G_AC_NONE | G_ZS_PRIM | IM_RD | CVG_DST_CLAMP | ZMODE_OPA | FORCE_BL |
                         GBL_c1(G_BL_CLR_IN, G_BL_0, G_BL_CLR_MEM, G_BL_A_MEM) |
                         GBL_c2(G_BL_CLR_IN, G_BL_0, G_BL_CLR_MEM, G_BL_A_MEM)),
    // final color = (stage 1 RGB) * (cvg)
    gsDPFillRectangle(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1),
    gsSPEndDisplayList(),
};

void VisCvg_Init(VisCvg* this) {
    this->type = FB_FILTER_NONE;
    this->setScissor = false;
    this->primColor.r = 255;
    this->primColor.g = 255;
    this->primColor.b = 255;
    this->primColor.a = 255;
}

void VisCvg_Destroy(VisCvg* this) {
}

void VisCvg_Draw(VisCvg* this, Gfx** gfxp) {
    Gfx* gfx = *gfxp;

    gDPPipeSync(gfx++);
    gDPSetPrimDepth(gfx++, 0xFFFF, 0xFFFF);

    if (this->setScissor == true) {
        gDPSetScissor(gfx++, G_SC_NON_INTERLACE, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    }

    switch (this->type) {
        case FB_FILTER_CVG_RGB:
            gSPDisplayList(gfx++, sCoverageRGBDL);
            break;

        case FB_FILTER_CVG_RGB_PRIMFOG:
            // Set primitive color for uniform fogging in custom RenderMode
            gDPSetColor(gfx++, G_SETPRIMCOLOR, this->primColor.rgba);
            gSPDisplayList(gfx++, sCoverageRGBPrimFogDL);
            break;

        case FB_FILTER_CVG:
            // Set background color for G_RM_VISCVG
            gDPSetColor(gfx++, G_SETBLENDCOLOR, this->primColor.rgba);
            gSPDisplayList(gfx++, sCoverageOnlyDL);
            break;

        case FB_FILTER_CVG_RGB_FOG:
            // Set fogging color for custom RenderMode, needs to be close to 0 to not overflow
            gDPSetColor(gfx++, G_SETFOGCOLOR, this->primColor.rgba);
            gSPDisplayList(gfx++, sCoverageRGBFogDL);
            break;

        default:
            break;
    }

    gDPPipeSync(gfx++);
    *gfxp = gfx;
}
