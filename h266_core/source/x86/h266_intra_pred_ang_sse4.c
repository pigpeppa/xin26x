/***************************************************************************//**
 *
 * @file          h266_intra_pred_ang_sse4.c
 * @brief         h266 intra prediction subroutines (angular, SSE4).
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
#include "string.h"
#include "basic_macro.h"
#include "h266_intra_pred_context.h"
#include "xin_video_common.h"
#include "h26x_trans_context.h"
#include "h26x_common_data.h"
#include "h26x_block_transpose.h"
#include "smmintrin.h"

static const SINT16 wlScale[3][16] =
{
    {
        32,  8, 2,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    },
    {
        32, 16, 8,  4,  2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    },
    {
        32, 32, 16, 16, 8, 8, 4, 4, 2, 2, 1, 1, 0, 0, 0, 0,
    }
};

static const UINT8 blendMask[3][16] =
{
    {
        0xFF, 0xFF, 0xFF,    0,    0,    0,    0,    0,    0,    0,    0,    0,   0,   0,   0,   0,
    },
    {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,    0,    0,    0,    0,    0,    0,   0,   0,   0,   0,
    },
    {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   0,   0,   0,   0,
    },
};


static SINT8 cubicFilter[32][4] =
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

static SINT8 linearFilter[32][4] =
{
    { 16, 32, 16, 0 },
    { 16, 32, 16, 0 },
    { 15, 31, 17, 1 },
    { 15, 31, 17, 1 },
    { 14, 30, 18, 2 },
    { 14, 30, 18, 2 },
    { 13, 29, 19, 3 },
    { 13, 29, 19, 3 },
    { 12, 28, 20, 4 },
    { 12, 28, 20, 4 },
    { 11, 27, 21, 5 },
    { 11, 27, 21, 5 },
    { 10, 26, 22, 6 },
    { 10, 26, 22, 6 },
    { 9, 25, 23, 7 },
    { 9, 25, 23, 7 },
    { 8, 24, 24, 8 },
    { 8, 24, 24, 8 },
    { 7, 23, 25, 9 },
    { 7, 23, 25, 9 },
    { 6, 22, 26, 10 },
    { 6, 22, 26, 10 },
    { 5, 21, 27, 11 },
    { 5, 21, 27, 11 },
    { 4, 20, 28, 12 },
    { 4, 20, 28, 12 },
    { 3, 19, 29, 13 },
    { 3, 19, 29, 13 },
    { 2, 18, 30, 14 },
    { 2, 18, 30, 14 },
    { 1, 17, 31, 15 },
    { 1, 17, 31, 15 },
};

static const SINT32 invIntraAngTable[32] =
{
    0,   16384, 8192, 5461, 4096, 2731, 2048, 1638, 1365, 1170, 1024, 910, 819, 712, 630, 565,
    512, 468,   420,  364,  321,  287,  256,  224,  191,  161,  128,  96,  64,  48,  32,  16
};

void Xin266ApplyAngPDPCHor4xH_SSE4 (
    PIXEL    *pred,
    intptr_t predStride,
    SINT32   absAngleMode,
    UINT32   angularScale,
    PIXEL    *refSide,
    SINT32   width,
    SINT32   height)
{
    SINT32  rowIdx;
    SINT32  invSum;
    UINT32  shift;
    __m128i lftx4;
    __m128i allZero;
    __m128i predx4;
    __m128i difx4;
    __m128i c32x4;
    UINT32  lft32;
    UINT32  pred32;
    SINT32  invAngle;

    (void)width;

    invAngle = invIntraAngTable[absAngleMode];
    height   = XIN_MIN (3 << angularScale, height);
    invSum   = 256 + invAngle;
    allZero  = _mm_setzero_si128 ();
    c32x4    = _mm_set1_epi16 (32);

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        shift = 5 - (2 * rowIdx >> angularScale);
        lft32  = *((UINT32 *)(refSide + (invSum >> 9) + 1));
        lftx4  = _mm_cvtsi32_si128 (lft32);
        lftx4  = _mm_unpacklo_epi8 (lftx4, allZero);
        pred32 = *((UINT32 *)(pred));
        predx4 = _mm_cvtsi32_si128 (pred32);
        predx4 = _mm_unpacklo_epi8 (predx4, allZero);

        difx4 = _mm_sub_epi16 (lftx4, predx4);
        difx4 = _mm_slli_epi16 (difx4, shift);
        difx4 = _mm_add_epi16 (difx4, c32x4);
        difx4 = _mm_srai_epi16 (difx4, 6);

        predx4 = _mm_add_epi16 (difx4, predx4);
        predx4 = _mm_packus_epi16 (predx4, predx4);

        pred32 = _mm_cvtsi128_si32 (predx4);
        *((UINT32 *)pred) = pred32;

        pred   += predStride;
        invSum += invAngle;

    }
}

void Xin266ApplyAngPDPCHor8xH_SSE4 (
    PIXEL    *pred,
    intptr_t predStride,
    SINT32   absAngleMode,
    UINT32   angularScale,
    PIXEL    *refSide,
    SINT32   width,
    SINT32   height)
{
    SINT32  rowIdx;
    SINT32  invSum;
    UINT32  shift;
    __m128i lftx8;
    __m128i allZero;
    __m128i predx8;
    __m128i difx8;
    __m128i c32x8;
    SINT32  invAngle;

    (void)width;
    invAngle = invIntraAngTable[absAngleMode];
    height   = XIN_MIN (3 << angularScale, height);
    invSum   = 256 + invAngle;
    allZero  = _mm_setzero_si128 ();
    c32x8    = _mm_set1_epi16 (32);

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        shift = 5 - (2 * rowIdx >> angularScale);

        lftx8  = _mm_loadl_epi64 ((__m128i *)(refSide + (invSum >> 9) + 1));
        lftx8  = _mm_unpacklo_epi8 (lftx8, allZero);
        predx8 = _mm_loadl_epi64 ((__m128i *)(pred));
        predx8 = _mm_unpacklo_epi8 (predx8, allZero);

        difx8 = _mm_sub_epi16 (lftx8, predx8);
        difx8 = _mm_slli_epi16 (difx8, shift);
        difx8 = _mm_add_epi16 (difx8, c32x8);
        difx8 = _mm_srai_epi16 (difx8, 6);

        predx8 = _mm_add_epi16 (difx8, predx8);
        predx8 = _mm_packus_epi16 (predx8, predx8);

        _mm_storel_epi64 ((__m128i *)(pred), predx8);

        pred   += predStride;
        invSum += invAngle;

    }

}

void Xin266LinearTransform8xH_SSE4 (
    PIXEL    *src,
    intptr_t srcStride,
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   scale,
    SINT32   offset,
    SINT32   shift,
    UINT32   width,
    UINT32   height)
{
    UINT32  rowIdx;
    __m128i src0x8, src1x8;
    __m128i scalex8;
    __m128i offsetx8;

    (void)width;

    scalex8  = _mm_set1_epi16 ((short)scale);
    offsetx8 = _mm_set1_epi16 ((short)offset);

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        src0x8 = _mm_loadl_epi64 ((__m128i *)(src + (rowIdx + 0)*srcStride));
        src1x8 = _mm_loadl_epi64 ((__m128i *)(src + (rowIdx + 1)*srcStride));

        src0x8 = _mm_cvtepu8_epi16 (src0x8);
        src1x8 = _mm_cvtepu8_epi16 (src1x8);

        src0x8 = _mm_mullo_epi16 (src0x8, scalex8);
        src1x8 = _mm_mullo_epi16 (src1x8, scalex8);

        src0x8 = _mm_srai_epi16 (src0x8, shift);
        src1x8 = _mm_srai_epi16 (src1x8, shift);

        src0x8 = _mm_add_epi16 (src0x8, offsetx8);
        src1x8 = _mm_add_epi16 (src1x8, offsetx8);

        src0x8 = _mm_packus_epi16 (src0x8, src0x8);
        src1x8 = _mm_packus_epi16 (src1x8, src1x8);

        _mm_storel_epi64 ((__m128i *)(dst + (rowIdx + 0)*dstStride), src0x8);
        _mm_storel_epi64 ((__m128i *)(dst + (rowIdx + 1)*dstStride), src1x8);

    }

}

#if 0
void Xin266ApplyAngPDPCVert_SSE4 (
    PIXEL    *pred,
    intptr_t predStride,
    SINT32   invAngle,
    UINT32   angularScale,
    PIXEL    *refSide,
    SINT32   width,
    SINT32   height)
{
    SINT32  rowIdx;
    SINT32  colIdx;
    SINT32  invAngleSum;
    PIXEL   lft;
    SINT32  wl;
    __m128i index128;
    __m128i count128;
    __m128i blendMask;
    __m128i lft0x8, lft1x8;
    __m128i wl0x8, wl1x8;
    __m128i pred0x8, pred1x8;
    __m128i allZero;
    __m128i difx8;
    __m128i c32x8;

    width   = XIN_MIN (3 << angularScale, width);
    allZero = _mm_setzero_si128 ();
    c32x8   = _mm_set1_epi16 (32);
    
    
    if (invAngle == 512)
    {
        index128  = _mm_loadu_si128 ((__m128i *)(one2fifteen));
        count128  = _mm_set1_epi8 (width);
        blendMask = _mm_cmpgt_epi8 (count128, index128);
        wl0x8     = _mm_loadu_si128 ((__m128i *)(wlScale[angularScale]));
        wl1x8     = _mm_loadu_si128 ((__m128i *)(wlScale[angularScale] + 8));

        for (rowIdx = 0; rowIdx < height; rowIdx++)
        {
            lft0x8  = _mm_loadl_epi64 ((__m128i *)(refSide + rowIdx + 1));
            lft0x8  = _mm_unpacklo_epi8 (lft0x8, allZero);
            pred0x8 = _mm_loadl_epi64 ((__m128i *)(pred));
            difx8   = _mm_sub_epi16 (lft0x8, pred0x8);
            difx8   = _mm_mullo_epi16 (difx8, wl0x8);
            difx8   = _mm_add_epi16 (difx8, c32x8);
            difx8   = _mm_srai_epi16 (difx8, 6);
            pred0x8 = _mm_add_epi16 (difx8, pred0x8);
            pred0x8 = _mm_packus_epi16 (pred0x8, pred0x8);
        }

        
        
    }
    else
    {
        for (rowIdx = 0; rowIdx < height; rowIdx++)
        {
            invAngleSum = 256;

            for (colIdx = 0; colIdx < width; colIdx++)
            {
                invAngleSum += invAngle;

                wl  = 32 >> (2 * colIdx >> angularScale);
                lft = refSide[rowIdx + (invAngleSum >> 9) + 1];

                pred[colIdx] = (PIXEL)XIN_CLIP(pred[colIdx] + ((wl * (lft - pred[colIdx]) + 32) >> 6), MIN_PIXEL_VALUE, MAX_PIXEL_VALUE);
            }

            pred += predStride;
        }
    }
    
}
#endif
