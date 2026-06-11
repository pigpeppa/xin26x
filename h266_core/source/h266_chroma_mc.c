/***************************************************************************//**
 *
 * @file          h266_chroma_mc.c
 * @brief         h.266 chroma motion compensation subroutines.
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

static const SINT16 g_chromaFilter[XIN_INTERP_UV_SUB_POS][CHROMA_INTERP_TAPS] =
{
    {  0, 64,  0,  0 },
    { -1, 63,  2,  0 },
    { -2, 62,  4,  0 },
    { -2, 60,  7, -1 },
    { -2, 58, 10, -2 },
    { -3, 57, 12, -2 },
    { -4, 56, 14, -2 },
    { -4, 55, 15, -2 },
    { -4, 54, 16, -2 },
    { -5, 53, 18, -2 },
    { -6, 52, 20, -2 },
    { -6, 49, 24, -3 },
    { -6, 46, 28, -4 },
    { -5, 44, 29, -4 },
    { -4, 42, 30, -4 },
    { -4, 39, 33, -4 },
    { -4, 36, 36, -4 },
    { -4, 33, 39, -4 },
    { -4, 30, 42, -4 },
    { -4, 29, 44, -5 },
    { -4, 28, 46, -6 },
    { -3, 24, 49, -6 },
    { -2, 20, 52, -6 },
    { -2, 18, 53, -5 },
    { -2, 16, 54, -4 },
    { -2, 15, 55, -4 },
    { -2, 14, 56, -4 },
    { -2, 12, 57, -3 },
    { -2, 10, 58, -2 },
    { -1,  7, 60, -2 },
    {  0,  4, 62, -2 },
    {  0,  2, 63, -1 },
};

void Xin266ChromaInterpHor (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex)
{
    const SINT16 *fltCoeff = g_chromaFilter[frac & XIN_MV_UV_FRAC_MASK];
    SINT32 value;
    UINT32 row, col;
    SINT32 sum;

    (void)filterIndex;
    
    src -= (CHROMA_INTERP_TAPS / 2 - 1);

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col++)
        {
            sum  = src[col + 0] * fltCoeff[0];
            sum += src[col + 1] * fltCoeff[1];
            sum += src[col + 2] * fltCoeff[2];
            sum += src[col + 3] * fltCoeff[3];

            value    = (sum + INTERP_OFFSET) >> INTERP_SHIFT;
            value    = XIN_CLIP(value, MIN_PIXEL_VALUE, MAX_PIXEL_VALUE);
            dst[col] = (PIXEL)value;
        }

        src += srcStride;
        dst += dstStride;

    }

}

void Xin266ChromaInterpVet (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex)
{
    const SINT16 *fltCoeff = g_chromaFilter[frac >> XIN_MV_UV_FRAC_BITS];
    SINT32 value;
    UINT32 row, col;
    SINT32 sum;

    (void)filterIndex;
    
    src -= (CHROMA_INTERP_TAPS / 2 - 1) * srcStride;

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col++)
        {
            sum  = src[col + 0 * srcStride] * fltCoeff[0];
            sum += src[col + 1 * srcStride] * fltCoeff[1];
            sum += src[col + 2 * srcStride] * fltCoeff[2];
            sum += src[col + 3 * srcStride] * fltCoeff[3];

            value    = (sum + INTERP_OFFSET) >> INTERP_SHIFT;
            value    = XIN_CLIP(value, MIN_PIXEL_VALUE, MAX_PIXEL_VALUE);
            dst[col] = (PIXEL)value;
        }

        src += srcStride;
        dst += dstStride;

    }

}

void Xin266ChromaInterpHorU8S16 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex)
{
    const SINT16  *fltCoeff = g_chromaFilter[frac & XIN_MV_UV_FRAC_MASK];
    UINT32 row, col;
    SINT32 sum;
    SINT32 offset;
    SINT32 shift;

    (void)filterIndex;
    
    src   -= CHROMA_INTERP_TAPS / 2 - 1;
    shift  = INTERP_SHIFT - XIN_MAX (2, INTERP_PRECISION - XIN_INTERNAL_BIT_DEPTH);
    offset = -INTERP_PREC_OFFSET << shift;

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col++)
        {
            sum  = src[col + 0] * fltCoeff[0];
            sum += src[col + 1] * fltCoeff[1];
            sum += src[col + 2] * fltCoeff[2];
            sum += src[col + 3] * fltCoeff[3];
            sum += offset;
            sum  = sum >> shift;

            dst[col] = (SINT16)sum;
        }

        src += srcStride;
        dst += dstStride;

    }

}

void Xin266ChromaInterpVetS16U8 (
    const SINT16 *src,
    intptr_t     srcStride,
    PIXEL        *dst,
    intptr_t     dstStride,
    SINT32       frac,
    UINT32       width,
    UINT32       height,
    UINT32      filterIndex)
{
    const SINT16* fltCoeff = g_chromaFilter[frac >> XIN_MV_UV_FRAC_BITS];
    UINT32 row, col;
    SINT32 value;
    SINT32 sum;
    SINT32 shift;
    SINT32 offset;

    (void)filterIndex;
    
    src    -= (CHROMA_INTERP_TAPS / 2 - 1) * srcStride;
    shift   = INTERP_SHIFT + XIN_MAX (2, (INTERP_PRECISION - XIN_INTERNAL_BIT_DEPTH));
    offset  = 1 << (shift - 1);
    offset += INTERP_PREC_OFFSET << INTERP_SHIFT;

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col++)
        {
            sum  = src[col + 0 * srcStride] * fltCoeff[0];
            sum += src[col + 1 * srcStride] * fltCoeff[1];
            sum += src[col + 2 * srcStride] * fltCoeff[2];
            sum += src[col + 3 * srcStride] * fltCoeff[3];

            value    = (sum + offset) >> shift;
            value    = XIN_CLIP(value, MIN_PIXEL_VALUE, MAX_PIXEL_VALUE);
            dst[col] = (PIXEL)value;
        }

        src += srcStride;
        dst += dstStride;

    }

}

void Xin266ChromaInterpVetS16S16 (
    const SINT16 *src,
    intptr_t     srcStride,
    SINT16       *dst,
    intptr_t     dstStride,
    SINT32       frac,
    UINT32       width,
    UINT32       height,
    UINT32      filterIndex)
{
    const SINT16* fltCoeff = g_chromaFilter[frac >> XIN_MV_UV_FRAC_BITS];
    UINT32 row, col;
    SINT32 value;
    SINT32 sum;

    (void)filterIndex;
    
    src -= (CHROMA_INTERP_TAPS / 2 - 1) * srcStride;

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col++)
        {
            sum  = src[col + 0 * srcStride] * fltCoeff[0];
            sum += src[col + 1 * srcStride] * fltCoeff[1];
            sum += src[col + 2 * srcStride] * fltCoeff[2];
            sum += src[col + 3 * srcStride] * fltCoeff[3];

            value    = sum >> INTERP_SHIFT;
            dst[col] = (SINT16)value;
        }

        src += srcStride;
        dst += dstStride;

    }

}

void Xin266ChromaInterpHorVet (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex)
{
    SINT16  firstPassDst[(128 / 2) * (128 / 2 + 8)];
    
    src -= (CHROMA_INTERP_TAPS / 2 - 1) * srcStride;

    Xin266ChromaInterpHorU8S16 (
        src,
        srcStride,
        firstPassDst,
        width,
        frac,
        width,
        height + CHROMA_INTERP_TAPS - 1,
        filterIndex);

    Xin266ChromaInterpVetS16U8 (
        firstPassDst + (CHROMA_INTERP_TAPS / 2 - 1) * width,
        width,
        dst,
        dstStride,
        frac,
        width,
        height,
        filterIndex);

}

void Xin266ChromaInterpVetU8S16 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex)
{
    const SINT16 *fltCoeff = g_chromaFilter[frac >> XIN_MV_UV_FRAC_BITS];
    UINT32 row, col;
    SINT32 sum;
    SINT32 shift;
    SINT32 offset;

    (void)filterIndex;
    
    src   -= (CHROMA_INTERP_TAPS / 2 - 1) * srcStride;
    shift  = INTERP_SHIFT - XIN_MAX (2, INTERP_PRECISION - XIN_INTERNAL_BIT_DEPTH);
    offset = -INTERP_PREC_OFFSET << shift;

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col++)
        {
            sum  = src[col + 0 * srcStride] * fltCoeff[0];
            sum += src[col + 1 * srcStride] * fltCoeff[1];
            sum += src[col + 2 * srcStride] * fltCoeff[2];
            sum += src[col + 3 * srcStride] * fltCoeff[3];
            sum += offset;
            sum  = sum >> shift;

            dst[col] = (SINT16)sum;
        }

        src += srcStride;
        dst += dstStride;

    }

}

void Xin266ChromaInterpHorVetU8S16 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex)
{
    SINT16  firstPassDst[(128 / 2) * (128 / 2 + 8)];

    (void)filterIndex;
    
    src -= (CHROMA_INTERP_TAPS / 2 - 1) * srcStride;

    Xin266ChromaInterpHorU8S16 (
        src,
        srcStride,
        firstPassDst,
        width,
        frac,
        width,
        height + CHROMA_INTERP_TAPS - 1,
        filterIndex);

    Xin266ChromaInterpVetS16S16 (
        firstPassDst + (CHROMA_INTERP_TAPS / 2 - 1) * width,
        width,
        dst,
        dstStride,
        frac,
        width,
        height,
        filterIndex);

}


