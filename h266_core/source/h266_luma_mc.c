/***************************************************************************//**
 *
 * @file          h266_luma_mc.c
 * @brief         h.266 luma motion compensation subroutines.
 *
 * @authors       Chao Zhou
 *
 * Xin26x Video Codec Library
 *
 * Copyright (C) 2020-2026 Chao Zhou <czhou2@qq.com>
 *
 * This file is part of Xin26x.
 *
 * Licensed under the GNU General Public License, Version 3 or later
 * (GPL-3.0-or-later). See the LICENSE file for details.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 *******************************************************************************/
#include "xin_typedef.h"
#include "h266_motion_comp.h"
#include "h266_inter_pred_context.h"
#include "basic_macro.h"
#include "h26x_definition.h"
#include "h266_constant.h"

static const SINT16 g_lumaFilter[XIN_INTERP_SUB_POS][LUMA_INTERP_TAPS] =
{
    {  0, 0,   0, 64,  0,   0,  0,  0 },
    {  0, 1,  -3, 63,  4,  -2,  1,  0 },
    { -1, 2,  -5, 62,  8,  -3,  1,  0 },
    { -1, 3,  -8, 60, 13,  -4,  1,  0 },
    { -1, 4, -10, 58, 17,  -5,  1,  0 },
    { -1, 4, -11, 52, 26,  -8,  3, -1 },
    { -1, 3,  -9, 47, 31, -10,  4, -1 },
    { -1, 4, -11, 45, 34, -10,  4, -1 },
    { -1, 4, -11, 40, 40, -11,  4, -1 },
    { -1, 4, -10, 34, 45, -11,  4, -1 },
    { -1, 4, -10, 31, 47,  -9,  3, -1 },
    { -1, 3,  -8, 26, 52, -11,  4, -1 },
    {  0, 1,  -5, 17, 58, -10,  4, -1 },
    {  0, 1,  -4, 13, 60,  -8,  3, -1 },
    {  0, 1,  -3,  8, 62,  -5,  2, -1 },
    {  0, 1,  -2,  4, 63,  -3,  1,  0 }
};

static const SINT16 g_lumaFilter4x4[XIN_INTERP_SUB_POS][LUMA_INTERP_TAPS] =
{
    {  0, 0,   0, 64,  0,   0,  0,  0 },
    {  0, 1,  -3, 63,  4,  -2,  1,  0 },
    {  0, 1,  -5, 62,  8,  -3,  1,  0 },
    {  0, 2,  -8, 60, 13,  -4,  1,  0 },
    {  0, 3, -10, 58, 17,  -5,  1,  0 }, //1/4
    {  0, 3, -11, 52, 26,  -8,  2,  0 },
    {  0, 2,  -9, 47, 31, -10,  3,  0 },
    {  0, 3, -11, 45, 34, -10,  3,  0 },
    {  0, 3, -11, 40, 40, -11,  3,  0 }, //1/2
    {  0, 3, -10, 34, 45, -11,  3,  0 },
    {  0, 3, -10, 31, 47,  -9,  2,  0 },
    {  0, 2,  -8, 26, 52, -11,  3,  0 },
    {  0, 1,  -5, 17, 58, -10,  3,  0 }, //3/4
    {  0, 1,  -4, 13, 60,  -8,  2,  0 },
    {  0, 1,  -3,  8, 62,  -5,  1,  0 },
    {  0, 1,  -2,  4, 63,  -3,  1,  0 }
};

static const SINT16 g_lumaAltHpelIFilter[LUMA_INTERP_TAPS] =
{
    0, 3, 9, 20, 20, 9, 3, 0
};

void Xin266InterpCopy (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex)
{
    UINT32 row, col;

    (void)frac;
    (void)filterIndex;

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col++)
        {
            dst[col] = src[col];
        }

        src += srcStride;
        dst += dstStride;

    }
}

void Xin266LumaInterpHor (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex)
{
    const SINT16 *fltCoeff;
    SINT32 value;
    UINT32 row, col;
    SINT32 sum;

    frac = frac & XIN_MV_FRAC_MASK;

    if ((filterIndex == XIN_INTERP_ALT_FILTER) && frac == 8)
    {
        fltCoeff = g_lumaAltHpelIFilter;
    }
    else if (filterIndex == XIN_INTERP_4x4_FILTER)
    {
        fltCoeff = g_lumaFilter4x4[frac];
    }
    else
    {
        fltCoeff = g_lumaFilter[frac];
    }

    src -= (LUMA_INTERP_TAPS/2 - 1);

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col++)
        {
            sum  = src[col + 0] * fltCoeff[0];
            sum += src[col + 1] * fltCoeff[1];
            sum += src[col + 2] * fltCoeff[2];
            sum += src[col + 3] * fltCoeff[3];
            sum += src[col + 4] * fltCoeff[4];
            sum += src[col + 5] * fltCoeff[5];
            sum += src[col + 6] * fltCoeff[6];
            sum += src[col + 7] * fltCoeff[7];

            value    = (sum + INTERP_OFFSET) >> INTERP_SHIFT;
            value    = XIN_CLIP(value, MIN_PIXEL_VALUE, MAX_PIXEL_VALUE);
            dst[col] = (PIXEL)value;
        }

        src += srcStride;
        dst += dstStride;

    }

}

void Xin266LumaInterpVet (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex)
{
    const SINT16 *fltCoeff;
    SINT32 value;
    UINT32 row, col;
    SINT32 sum;

    frac = frac >> XIN_MV_FRAC_BITS;

    if ((filterIndex == XIN_INTERP_ALT_FILTER) && frac == 8)
    {
        fltCoeff = g_lumaAltHpelIFilter;
    }
    else if (filterIndex == XIN_INTERP_4x4_FILTER)
    {
        fltCoeff = g_lumaFilter4x4[frac];
    }
    else
    {
        fltCoeff = g_lumaFilter[frac];
    }

    src -= (LUMA_INTERP_TAPS / 2 - 1) * srcStride;

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col++)
        {
            sum  = src[col + 0 * srcStride] * fltCoeff[0];
            sum += src[col + 1 * srcStride] * fltCoeff[1];
            sum += src[col + 2 * srcStride] * fltCoeff[2];
            sum += src[col + 3 * srcStride] * fltCoeff[3];

            sum += src[col + 4 * srcStride] * fltCoeff[4];
            sum += src[col + 5 * srcStride] * fltCoeff[5];
            sum += src[col + 6 * srcStride] * fltCoeff[6];
            sum += src[col + 7 * srcStride] * fltCoeff[7];


            value    = (sum + INTERP_OFFSET) >> INTERP_SHIFT;
            value    = XIN_CLIP(value, MIN_PIXEL_VALUE, MAX_PIXEL_VALUE);
            dst[col] = (PIXEL)value;
        }

        src += srcStride;
        dst += dstStride;

    }

}

void Xin266LumaInterpHorU8S16 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex)
{
    const SINT16 *fltCoeff;
    UINT32 row, col;
    SINT32 sum;
    SINT32 shift;
    SINT32 offset;

    src   -= LUMA_INTERP_TAPS / 2 - 1;
    shift  = INTERP_SHIFT - XIN_MAX (2, INTERP_PRECISION - XIN_INTERNAL_BIT_DEPTH);
    offset = -INTERP_PREC_OFFSET << shift;
    frac   = frac & XIN_MV_FRAC_MASK;

    if ((filterIndex == XIN_INTERP_ALT_FILTER) && frac == 8)
    {
        fltCoeff = g_lumaAltHpelIFilter;
    }
    else if (filterIndex == XIN_INTERP_4x4_FILTER)
    {
        fltCoeff = g_lumaFilter4x4[frac];
    }
    else
    {
        fltCoeff = g_lumaFilter[frac];
    }

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col++)
        {
            sum  = src[col + 0] * fltCoeff[0];
            sum += src[col + 1] * fltCoeff[1];
            sum += src[col + 2] * fltCoeff[2];
            sum += src[col + 3] * fltCoeff[3];
            sum += src[col + 4] * fltCoeff[4];
            sum += src[col + 5] * fltCoeff[5];
            sum += src[col + 6] * fltCoeff[6];
            sum += src[col + 7] * fltCoeff[7];
            sum += offset;
            sum  = sum >> shift;

            dst[col] = (SINT16)sum;
        }

        src += srcStride;
        dst += dstStride;

    }

}

void Xin266LumaInterpVetS16U8 (
    const SINT16 *src,
    intptr_t     srcStride,
    PIXEL        *dst,
    intptr_t     dstStride,
    SINT32       frac,
    UINT32       width,
    UINT32       height,
    UINT32       filterIndex)
{
    const SINT16 *fltCoeff;
    UINT32 row, col;
    SINT32 value;
    SINT32 sum;
    SINT32 offset;
    SINT32 shift;

    src    -= (LUMA_INTERP_TAPS/2 - 1) * srcStride;
    shift   = INTERP_SHIFT + XIN_MAX (2, (INTERP_PRECISION - XIN_INTERNAL_BIT_DEPTH));
    offset  = 1 << (shift - 1);
    offset += INTERP_PREC_OFFSET << INTERP_SHIFT;
    frac    = frac >> XIN_MV_FRAC_BITS;

    if ((filterIndex == XIN_INTERP_ALT_FILTER) && frac == 8)
    {
        fltCoeff = g_lumaAltHpelIFilter;
    }
    else if (filterIndex == XIN_INTERP_4x4_FILTER)
    {
        fltCoeff = g_lumaFilter4x4[frac];
    }
    else
    {
        fltCoeff = g_lumaFilter[frac];
    }

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col++)
        {
            sum  = src[col + 0 * srcStride] * fltCoeff[0];
            sum += src[col + 1 * srcStride] * fltCoeff[1];
            sum += src[col + 2 * srcStride] * fltCoeff[2];
            sum += src[col + 3 * srcStride] * fltCoeff[3];

            sum += src[col + 4 * srcStride] * fltCoeff[4];
            sum += src[col + 5 * srcStride] * fltCoeff[5];
            sum += src[col + 6 * srcStride] * fltCoeff[6];
            sum += src[col + 7 * srcStride] * fltCoeff[7];


            value    = (sum + offset) >> shift;
            value    = XIN_CLIP(value, MIN_PIXEL_VALUE, MAX_PIXEL_VALUE);
            dst[col] = (PIXEL)value;
        }

        src += srcStride;
        dst += dstStride;

    }

}

void  Xin266LumaInterpVetS16S16 (
    const SINT16 *src,
    intptr_t     srcStride,
    SINT16       *dst,
    intptr_t     dstStride,
    SINT32       frac,
    UINT32       width,
    UINT32       height,
    UINT32       filterIndex)
{
    const SINT16 *fltCoeff;
    UINT32 row, col;
    SINT32 value;
    SINT32 sum;

    src -= (LUMA_INTERP_TAPS/2 - 1) * srcStride;
    frac = frac>>XIN_MV_FRAC_BITS;

    if ((filterIndex == XIN_INTERP_ALT_FILTER) && frac == 8)
    {
        fltCoeff = g_lumaAltHpelIFilter;
    }
    else if (filterIndex == XIN_INTERP_4x4_FILTER)
    {
        fltCoeff = g_lumaFilter4x4[frac];
    }
    else
    {
        fltCoeff = g_lumaFilter[frac];
    }

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col++)
        {
            sum  = src[col + 0 * srcStride] * fltCoeff[0];
            sum += src[col + 1 * srcStride] * fltCoeff[1];
            sum += src[col + 2 * srcStride] * fltCoeff[2];
            sum += src[col + 3 * srcStride] * fltCoeff[3];

            sum += src[col + 4 * srcStride] * fltCoeff[4];
            sum += src[col + 5 * srcStride] * fltCoeff[5];
            sum += src[col + 6 * srcStride] * fltCoeff[6];
            sum += src[col + 7 * srcStride] * fltCoeff[7];

            value    = sum >> INTERP_SHIFT;
            dst[col] = (SINT16)value;
        }

        src += srcStride;
        dst += dstStride;

    }

}

void  Xin266LumaInterpHorVet (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex)
{
    SINT16  firstPassDst[128*(128+8)];

    src -= (LUMA_INTERP_TAPS / 2 - 1) * srcStride;

    Xin266LumaInterpHorU8S16 (
        src,
        srcStride,
        firstPassDst,
        width,
        frac,
        width,
        height + LUMA_INTERP_TAPS - 1,
        filterIndex);

    Xin266LumaInterpVetS16U8 (
        firstPassDst + (LUMA_INTERP_TAPS / 2 - 1) * width,
        width,
        dst,
        dstStride,
        frac,
        width,
        height,
        filterIndex);

}

void  Xin266InterpCopyU8S16 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex)
{
    UINT32 row, col;
    SINT16 offset;
    SINT32 shift;

    (void)frac;
    (void)filterIndex;
    offset = INTERP_PREC_OFFSET;
    shift  = XIN_MAX (2, (INTERP_PRECISION - XIN_INTERNAL_BIT_DEPTH));

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col++)
        {
            dst[col] = src[col] << shift;
            dst[col] = dst[col] - offset;

        }

        src += srcStride;
        dst += dstStride;

    }

}

void  Xin266LumaInterpVetU8S16 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex)
{
    const SINT16 *fltCoeff;
    UINT32 row, col;
    SINT32 sum;
    SINT32 shift;
    SINT32 offset;

    src   -= (LUMA_INTERP_TAPS / 2 - 1) * srcStride;
    shift  = INTERP_SHIFT - XIN_MAX (2, INTERP_PRECISION - XIN_INTERNAL_BIT_DEPTH);
    offset = -INTERP_PREC_OFFSET << shift;
    frac   = frac >> XIN_MV_FRAC_BITS;

    if ((filterIndex == XIN_INTERP_ALT_FILTER) && frac == 8)
    {
        fltCoeff = g_lumaAltHpelIFilter;
    }
    else if (filterIndex == XIN_INTERP_4x4_FILTER)
    {
        fltCoeff = g_lumaFilter4x4[frac];
    }
    else
    {
        fltCoeff = g_lumaFilter[frac];
    }

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col++)
        {
            sum  = src[col + 0 * srcStride] * fltCoeff[0];
            sum += src[col + 1 * srcStride] * fltCoeff[1];
            sum += src[col + 2 * srcStride] * fltCoeff[2];
            sum += src[col + 3 * srcStride] * fltCoeff[3];

            sum += src[col + 4 * srcStride] * fltCoeff[4];
            sum += src[col + 5 * srcStride] * fltCoeff[5];
            sum += src[col + 6 * srcStride] * fltCoeff[6];
            sum += src[col + 7 * srcStride] * fltCoeff[7];
            sum += offset;
            sum  = sum >> shift;

            dst[col] = (SINT16)sum;
        }

        src += srcStride;
        dst += dstStride;

    }

}

void  Xin266LumaInterpHorVetU8S16 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex)
{
    SINT16  firstPassDst[128*(128+8)];

    src -= (LUMA_INTERP_TAPS / 2 - 1) * srcStride;

    Xin266LumaInterpHorU8S16 (
        src,
        srcStride,
        firstPassDst,
        width,
        frac,
        width,
        height + LUMA_INTERP_TAPS - 1,
        filterIndex);

    Xin266LumaInterpVetS16S16 (
        firstPassDst + (LUMA_INTERP_TAPS / 2 - 1) * width,
        width,
        dst,
        dstStride,
        frac,
        width,
        height,
        filterIndex);

}

void  Xin266InterpAvgS16U8 (
    const SINT16 *src0,
    intptr_t     src0Stride,
    const SINT16 *src1,
    intptr_t     src1Stride,
    PIXEL        *dst,
    intptr_t     dstStride,
    UINT32       width,
    UINT32       height)
{
    UINT32 row, col;
    SINT32 shift;
    SINT32 offset;
    SINT32 value;

    shift  = XIN_MAX (2, INTERP_PRECISION - XIN_INTERNAL_BIT_DEPTH) + 1;
    offset = (1 << (shift - 1)) + 2*INTERP_PREC_OFFSET;

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col++)
        {
            value    = (src0[col] + src1[col] + offset) >> shift;
            value    = XIN_CLIP(value, MIN_PIXEL_VALUE, MAX_PIXEL_VALUE);
            dst[col] = (PIXEL)value;
        }

        src0 += src0Stride;
        src1 += src1Stride;
        dst  += dstStride;
    }

}

void  Xin266InterpWeightS16U8 (
    const SINT16 *src0,
    intptr_t     src0Stride,
    const SINT16 *src1,
    intptr_t     src1Stride,
    PIXEL        *dst,
    intptr_t     dstStride,
    UINT32       width,
    UINT32       height,
    SINT32       weightA,
    SINT32       weightB)
{
    UINT32 row, col;
    SINT32 shift;
    SINT32 offset;
    SINT32 value;

    shift  = XIN_MAX (2, INTERP_PRECISION - XIN_INTERNAL_BIT_DEPTH) + XIN_BCW_LOG_WGT_BASE;
    offset = (1 << (shift - 1)) + (INTERP_PREC_OFFSET << XIN_BCW_LOG_WGT_BASE);

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col++)
        {
            value    = (src0[col]*weightA + src1[col]*weightB + offset) >> shift;
            value    = XIN_CLIP(value, MIN_PIXEL_VALUE, MAX_PIXEL_VALUE);
            dst[col] = (PIXEL)value;
        }

        src0 += src0Stride;
        src1 += src1Stride;
        dst  += dstStride;
    }

}

