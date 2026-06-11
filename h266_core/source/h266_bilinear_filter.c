/***************************************************************************//**
 *
 * @file          h266_bilinear_filter.c
 * @brief         h.266 bilinear filter subroutines.
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

static const SINT16 g_biliFilter[XIN_INTERP_SUB_POS][BILI_INTERP_TAPS] =
{
    {16,  0,},
    {15,  1,},
    {14,  2,},
    {13,  3,},
    {12,  4,},
    {11,  5,},
    {10,  6,},
    { 9,  7,},
    { 8,  8,},
    { 7,  9,},
    { 6, 10,},
    { 5, 11,},
    { 4, 12,},
    { 3, 13,},
    { 2, 14,},
    { 1, 15,}
};

void Xin266BiliInterpCopy (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height)
{
    UINT32 row, col;
    SINT32 shift;

    (void)frac;
    shift = BILINEAR_PREC - XIN_INTERNAL_BIT_DEPTH;

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width + DMVR_PADING_SIZE; col++)
        {
            dst[col] = src[col] << shift;
        }

        src += srcStride;
        dst += dstStride;

    }

}

void Xin266BiliInterpHor (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height)
{
    const SINT16 *fltCoeff = g_biliFilter[frac&XIN_MV_FRAC_MASK];
    SINT32 value;
    UINT32 row, col;
    SINT32 sum;
    SINT32 shift;
    SINT32 offset;

    src   -= (BILI_INTERP_TAPS/2 - 1);
    shift  = BILINEAR_FILTER_PREC - (BILINEAR_PREC - XIN_INTERNAL_BIT_DEPTH);
    offset = 1 << (shift - 1);

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width + DMVR_PADING_SIZE; col++)
        {
            sum  = src[col + 0] * fltCoeff[0];
            sum += src[col + 1] * fltCoeff[1];

            value    = (sum + offset) >> shift;
            dst[col] = (SINT16)value;
        }

        src += srcStride;
        dst += dstStride;

    }

}

void Xin266BiliInterpVet (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height)
{
    const SINT16 *fltCoeff = g_biliFilter[frac>>XIN_MV_FRAC_BITS];
    SINT32 value;
    UINT32 row, col;
    SINT32 sum;
    SINT32 shift;
    SINT32 offset;

    src   -= (BILI_INTERP_TAPS / 2 - 1) * srcStride;
    shift  = BILINEAR_FILTER_PREC - (BILINEAR_PREC - XIN_INTERNAL_BIT_DEPTH);
    offset = 1 << (shift - 1);

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width + DMVR_PADING_SIZE; col++)
        {
            sum  = src[col + 0 * srcStride] * fltCoeff[0];
            sum += src[col + 1 * srcStride] * fltCoeff[1];


            value    = (sum + offset) >> shift;
            dst[col] = (SINT16)value;
        }

        src += srcStride;
        dst += dstStride;

    }

}

void Xin266BiliInterpHorU8S16 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height)
{
    const SINT16  *fltCoeff = g_biliFilter[frac&XIN_MV_FRAC_MASK];
    UINT32 row, col;
    SINT32 sum;
    SINT32 value;
    SINT32 shift;
    SINT32 offset;

    src   -= BILI_INTERP_TAPS / 2 - 1;
    shift  = BILINEAR_FILTER_PREC - (BILINEAR_PREC - XIN_INTERNAL_BIT_DEPTH);
    offset = 1 << (shift - 1);

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width + DMVR_PADING_SIZE; col++)
        {
            sum  = src[col + 0] * fltCoeff[0];
            sum += src[col + 1] * fltCoeff[1];

            value    = (sum + offset) >> shift;
            dst[col] = (SINT16)value;
        }

        src += srcStride;
        dst += dstStride;

    }

}

void Xin266BiliInterpVetS16U8 (
    const SINT16 *src,
    intptr_t     srcStride,
    SINT16       *dst,
    intptr_t     dstStride,
    SINT32       frac,
    UINT32       width,
    UINT32       height)
{
    const SINT16* fltCoeff = g_biliFilter[frac>>XIN_MV_FRAC_BITS];
    UINT32 row, col;
    SINT32 value;
    SINT32 sum;
    SINT32 shift;
    SINT32 offset;

    src   -= (BILI_INTERP_TAPS/2 - 1) * srcStride;
    shift  = 4;
    offset = 8;

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width + DMVR_PADING_SIZE; col++)
        {
            sum  = src[col + 0 * srcStride] * fltCoeff[0];
            sum += src[col + 1 * srcStride] * fltCoeff[1];

            value    = (sum + offset) >> shift;
            dst[col] = (SINT16)value;
        }

        src += srcStride;
        dst += dstStride;

    }

}

void  Xin266BiliInterpHorVet (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height)
{
    SINT16  firstPassDst[(128+4)*(128+12)];

    src -= (BILI_INTERP_TAPS / 2 - 1) * srcStride;

    Xin266BiliInterpHorU8S16 (
        src,
        srcStride,
        firstPassDst,
        width + DMVR_PADING_SIZE,
        frac,
        width,
        height + BILI_INTERP_TAPS - 1);

    Xin266BiliInterpVetS16U8 (
        firstPassDst + (BILI_INTERP_TAPS / 2 - 1) * (width + DMVR_PADING_SIZE),
        width + DMVR_PADING_SIZE,
        dst,
        dstStride,
        frac,
        width,
        height);

}

