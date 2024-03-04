/**
 * zlib License
 *
 * Copyright (C) 2024 Tharo
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 */

// Note about integration: This assumes there is at least 0xC750 bytes of free memory following each framebuffer for
// successively downscaled images. Ensure this is the case!

#include "global.h"

// Option to stop downscaling at 40x30 instead of 20x15, may improve visual clarity but bright patches won't bleed into
// surroundings as much
// #define SCALE_40_30

#define qu102(x) ((u32)((x) * (1 <<  2)))
#define qs105(x) ((s32)((x) * (1 <<  5)))
#define qs510(x) ((s32)((x) * (1 << 10)))

static u32 PreRender_Downscale4to1(Gfx** gfxP, u32 fbSegment, u32 width, u32 height) {
    Gfx* gfx = *gfxP;
    u32 newWidth = width / 2;
    u32 newHeight = height / 2;
    u32 newFbSegment = fbSegment + ALIGN64(width * height * G_IM_SIZ_16b_BYTES);
    u32 nRows;
    u32 rowsRemaining;
    u32 curRow;

    gDPPipeSync(gfx++);
    gDPSetColorImage(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, newWidth, newFbSegment);
    gDPSetScissor(gfx++, G_SC_NON_INTERLACE, 0, 0, newWidth, newHeight);

    nRows = TMEM_SIZE / (width * G_IM_SIZ_16b_BYTES);
    if (nRows & 1) {
        // We must load an even number of rows for texture sampling reasons, decrement so it fits in TMEM
        nRows--;
    }
    rowsRemaining = height;
    curRow = 0;

    while (rowsRemaining != 0) {
        u32 uls;
        u32 lrs;
        u32 ult;
        u32 lrt;

        // Make sure that we don't load past the end of the source image
        nRows = MIN(rowsRemaining, nRows);

        uls = 0;
        lrs = width;
        ult = curRow;
        lrt = curRow + nRows;

        // Load a horizontal strip of the source image in RGBA16 format
        gDPLoadTextureTile(gfx++, fbSegment, G_IM_FMT_RGBA, G_IM_SIZ_16b, width, height, uls, ult, lrs - 1, lrt - 1, 0,
                           G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                           G_TX_NOLOD);

        // Draw that horizontal strip to the destination image, downscaling 4-to-1 (2-to-1 in each axis)
        // The texture coordinates are offset by 0.5 for correct average filtering, dsdx and dtdy step by 2 input texels
        // as each output texel combines 2 pixels per row/column (4 pixels square)
        gSPTextureRectangle(gfx++, qu102(uls / 2), qu102(ult / 2), qu102(lrs / 2), qu102(lrt / 2), G_TX_RENDERTILE,
                            qs105(uls) + qs105(0.5), qs105(ult) + qs105(0.5), qs510(2.0), qs510(2.0));

        // Continue to next lines
        rowsRemaining -= nRows;
        curRow += nRows;
    }
    *gfxP = gfx;
    return newFbSegment;
}

static_assert(SCREEN_WIDTH == 320, "Bloom shader expects 320x240 screen size");
static_assert(SCREEN_HEIGHT == 240, "Bloom shader expects 320x240 screen size");
void PreRender_BloomShader(Gfx** gfxP, u32 yl, u32 yh, u8 alpha1, u8 alpha2) {
#ifdef SCALE_40_30
    const u32 tmemAddr = TMEM_SIZE - ALIGN8(40 * 30 * G_IM_SIZ_16b_BYTES);
#else /* SCALE_20_15 */
    const u32 tmemAddr = TMEM_SIZE - ALIGN8(20 * 15 * G_IM_SIZ_16b_BYTES);
#endif
    Gfx* gfx = *gfxP;
    u32 fbSegment;
    u32 nRows;
    u32 rowsRemaining;
    u32 curRow;

    if (alpha1 == 0 || alpha2 == 0) {
        // Don't do anything if either alpha is 0
        return;
    }

    // Set up downscaling with average filtering
    gDPPipeSync(gfx++);
    gDPSetOtherMode(gfx++,
                    G_AD_DISABLE | G_CD_DISABLE | G_CK_NONE | G_TC_FILT | G_TF_AVERAGE | G_TT_NONE | G_TL_TILE |
                        G_TD_CLAMP | G_TP_NONE | G_CYC_1CYCLE | G_PM_NPRIMITIVE,
                    G_AC_NONE | G_ZS_PRIM | CVG_DST_FULL | G_RM_NOOP | G_RM_NOOP2);
    gDPSetCombineLERP(gfx++, 0, 0, 0, TEXEL0, 0, 0, 0, 1,
                             0, 0, 0, TEXEL0, 0, 0, 0, 1);

    // Execute downscaling
    fbSegment = 0x0F000000;
    // 320x240 -> 160x120
    fbSegment = PreRender_Downscale4to1(&gfx, fbSegment, 320, 240);
    // 160x120 -> 80x60
    fbSegment = PreRender_Downscale4to1(&gfx, fbSegment, 160, 120);
    // 80x60 -> 40x30
    fbSegment = PreRender_Downscale4to1(&gfx, fbSegment, 80, 60);
#ifndef SCALE_40_30
    // 40x30 -> 20x15
    fbSegment = PreRender_Downscale4to1(&gfx, fbSegment, 40, 30);
#endif

    // Upload downscaled 20x15 rgba16 image to TMEM
#ifdef SCALE_40_30
    gDPLoadMultiBlock(gfx++, fbSegment, tmemAddr / 8, 1, G_IM_FMT_RGBA, G_IM_SIZ_16b, 40, 30, 0,
                      G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, 3, 3);
                    // 3 = log2(320/40), 3 = log2(240/30)
#else /* SCALE_20_15 */
    gDPLoadMultiBlock(gfx++, fbSegment, tmemAddr / 8, 1, G_IM_FMT_RGBA, G_IM_SIZ_16b, 20, 15, 0,
                      G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, 4, 4);
                    // 4 = log2(320/20), 4 = log2(240/15)
#endif

    // Account for bilerp oversampling of the small image by shifting the tile size, this avoids shifting pixels
    // of the original framebuffer out of place while maintaining correct bilinear sampling
#ifdef SCALE_40_30
    gDPSetTileSize(gfx++, 1, qu102(0+0.5), qu102(0+0.5), qu102(40-1+0.5), qu102(30-1+0.5));
#else
    gDPSetTileSize(gfx++, 1, qu102(0+0.5), qu102(0+0.5), qu102(20-1+0.5), qu102(15-1+0.5));
#endif

    // Return to original framebuffer
    gDPSetColorImage(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, 0x0F000000);
    gDPSetScissor(gfx++, G_SC_NON_INTERLACE, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // For each FB line blend (using CC to avoid overflows) with the downscaled image

    gDPSetOtherMode(gfx++,
                    G_AD_DISABLE | G_CD_MAGICSQ | G_CK_NONE | G_TC_FILT | G_TF_BILERP | G_TT_NONE | G_TL_TILE |
                        G_TD_CLAMP | G_TP_NONE | G_CYC_2CYCLE | G_PM_NPRIMITIVE,
                    // IM_RD is needed to save coverage for the VI AA pass :(
                    G_AC_NONE | G_ZS_PRIM | CVG_DST_SAVE | ZMODE_OPA | IM_RD | G_RM_NOOP | G_RM_NOOP2);
    gDPSetCombineLERP(gfx++, TEXEL1, TEXEL0, PRIM_LOD_FRAC,   TEXEL0,   0, 0, 0, 1, 
                             TEXEL0, 0,      PRIMITIVE_ALPHA, COMBINED, 0, 0, 0, COMBINED);
    gDPSetPrimColor(gfx++, 0, alpha1, 255, 255, 255, alpha2);

    nRows = tmemAddr / (320 * G_IM_SIZ_16b_BYTES);
    rowsRemaining = yh - yl;
    curRow = yl;

    while (rowsRemaining != 0) {
        u32 uls;
        u32 lrs;
        u32 ult;
        u32 lrt;

        // Make sure that we don't load past the end of the source image
        nRows = MIN(rowsRemaining, nRows);

        uls = 0;
        lrs = 320;
        ult = curRow;
        lrt = curRow + nRows;

        // Load the framebuffer rows
        gDPLoadTextureTile(gfx++, 0x0F000000, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, 240, uls, ult, lrs - 1, lrt - 1, 0,
                           G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                           G_TX_NOLOD);

        // Redraw, blending in the resampled image
        gSPTextureRectangle(gfx++, qu102(uls), qu102(ult), qu102(lrs), qu102(lrt), G_TX_RENDERTILE,
                            qs105(uls), qs105(ult), qs510(1.0), qs510(1.0));

        rowsRemaining -= nRows;
        curRow += nRows;
    }

    *gfxP = gfx;
}
