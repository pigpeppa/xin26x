/***************************************************************************//**
 *
 * @file          h266_intra_pred_ang_ssse3.c
 * @brief         h266 intra prediction subroutines (angular, SSSE3).
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
#include "tmmintrin.h"
#include "assert.h"

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

void Xin266LumaIntraFilterGt8xH_SSSE3 (
    PIXEL    *pred,
    intptr_t predStride,
    BOOL     interpFlag,
    SINT32   intraPredAngle,
    UINT32   multiRefIdx,
    PIXEL    *refMain,
    SINT32   width,
    SINT32   height)
{
    SINT32  rowIdx;
    SINT32  colIdx;
    SINT32  deltaPos;
    SINT32  deltaInt;
    SINT32  deltaFrac;
    __m128i ref0x16;
    __m128i ref1x16;
    __m128i ref2x16;
    __m128i ref3x16;
    __m128i flt01x8;
    __m128i flt23x8;
    __m128i ref01x8Lo;
    __m128i ref01x8Hi;
    __m128i ref23x8Lo;
    __m128i ref23x8Hi;
    __m128i sum01x8Lo;
    __m128i sum01x8Hi;
    __m128i sum23x8Lo;
    __m128i sum23x8Hi;
    __m128i sum0123Lo;
    __m128i sum0123Hi;
    __m128i const32x8;
    __m128i sum0123;
    SINT8   (*fltCoeff)[4];

    deltaPos  = intraPredAngle*(1 + multiRefIdx);
    const32x8 = _mm_set1_epi16(32);
    fltCoeff  = interpFlag ? linearFilter : cubicFilter;

    if (XIN_ABS(intraPredAngle) & 0x1F)
    {
        for (rowIdx = 0; rowIdx < height; rowIdx++)
        {
            deltaInt  = deltaPos >> 5;
            deltaFrac = deltaPos & 31;
            flt01x8   = _mm_set1_epi16 (((SINT16 *)fltCoeff[deltaFrac])[0]);
            flt23x8   = _mm_set1_epi16 (((SINT16 *)fltCoeff[deltaFrac])[1]);

            for (colIdx = 0; colIdx < width; colIdx += 16)
            {
                ref0x16 = _mm_loadu_si128 ((__m128i *)(refMain + colIdx + deltaInt + 0));
                ref1x16 = _mm_loadu_si128 ((__m128i *)(refMain + colIdx + deltaInt + 1));
                ref2x16 = _mm_loadu_si128 ((__m128i *)(refMain + colIdx + deltaInt + 2));
                ref3x16 = _mm_loadu_si128 ((__m128i *)(refMain + colIdx + deltaInt + 3));

                ref01x8Lo = _mm_unpacklo_epi8 (ref0x16, ref1x16);
                ref01x8Hi = _mm_unpackhi_epi8 (ref0x16, ref1x16);
                ref23x8Lo = _mm_unpacklo_epi8 (ref2x16, ref3x16);
                ref23x8Hi = _mm_unpackhi_epi8 (ref2x16, ref3x16);

                sum01x8Lo = _mm_maddubs_epi16 (ref01x8Lo, flt01x8);
                sum01x8Hi = _mm_maddubs_epi16 (ref01x8Hi, flt01x8);
                sum23x8Lo = _mm_maddubs_epi16 (ref23x8Lo, flt23x8);
                sum23x8Hi = _mm_maddubs_epi16 (ref23x8Hi, flt23x8);

                sum0123Lo = _mm_add_epi16 (sum01x8Lo, sum23x8Lo);
                sum0123Hi = _mm_add_epi16 (sum01x8Hi, sum23x8Hi);

                sum0123Lo = _mm_add_epi16 (sum0123Lo, const32x8);
                sum0123Hi = _mm_add_epi16 (sum0123Hi, const32x8);

                sum0123Lo = _mm_srai_epi16 (sum0123Lo, 6);
                sum0123Hi = _mm_srai_epi16 (sum0123Hi, 6);

                sum0123 = _mm_packus_epi16 (sum0123Lo, sum0123Hi);

                _mm_storeu_si128 ((__m128i *)(pred + colIdx), sum0123);

            }

            pred     += predStride;
            deltaPos += intraPredAngle;

        }

    }
    else
    {
        for (rowIdx = 0; rowIdx < height; rowIdx++)
        {
            deltaInt = deltaPos >> 5;

            // Just copy the integer samples
            for (colIdx = 0; colIdx < width; colIdx += 16)
            {
                ref0x16 = _mm_loadu_si128 ((__m128i *)(refMain + colIdx + deltaInt + 1));

                _mm_storeu_si128 ((__m128i *)(pred + colIdx), ref0x16);
            }

            pred     += predStride;
            deltaPos += intraPredAngle;
        }
    }

}

void Xin266LumaIntraFilter8xH_SSSE3 (
    PIXEL    *pred,
    intptr_t predStride,
    BOOL     interpFlag,
    SINT32   intraPredAngle,
    UINT32   multiRefIdx,
    PIXEL    *refMain,
    SINT32   width,
    SINT32   height)
{
    SINT32  rowIdx;
    SINT32  colIdx;
    SINT32  deltaPos;
    SINT32  deltaInt;
    SINT32  deltaFrac;
    __m128i ref0x16;
    __m128i ref1x16;
    __m128i ref2x16;
    __m128i ref3x16;
    __m128i flt01x8;
    __m128i flt23x8;
    __m128i ref01x8;
    __m128i ref23x8;
    __m128i sum01x8;
    __m128i sum23x8;
    __m128i sum0123;
    __m128i const32x8;
    SINT8   (*fltCoeff)[4];

    deltaPos  = intraPredAngle*(1 + multiRefIdx);
    const32x8 = _mm_set1_epi16(32);
    fltCoeff  = interpFlag ? linearFilter : cubicFilter;

    if (XIN_ABS(intraPredAngle) & 0x1F)
    {
        for (rowIdx = 0; rowIdx < height; rowIdx++)
        {
            deltaInt  = deltaPos >> 5;
            deltaFrac = deltaPos & 31;
            flt01x8   = _mm_set1_epi16 (((SINT16 *)fltCoeff[deltaFrac])[0]);
            flt23x8   = _mm_set1_epi16 (((SINT16 *)fltCoeff[deltaFrac])[1]);

            for (colIdx = 0; colIdx < width; colIdx += 8)
            {
                ref0x16 = _mm_loadl_epi64 ((__m128i *)(refMain + colIdx + deltaInt + 0));
                ref1x16 = _mm_loadl_epi64 ((__m128i *)(refMain + colIdx + deltaInt + 1));
                ref2x16 = _mm_loadl_epi64 ((__m128i *)(refMain + colIdx + deltaInt + 2));
                ref3x16 = _mm_loadl_epi64 ((__m128i *)(refMain + colIdx + deltaInt + 3));

                ref01x8 = _mm_unpacklo_epi8 (ref0x16, ref1x16);
                ref23x8 = _mm_unpacklo_epi8 (ref2x16, ref3x16);

                sum01x8 = _mm_maddubs_epi16 (ref01x8, flt01x8);
                sum23x8 = _mm_maddubs_epi16 (ref23x8, flt23x8);

                sum0123 = _mm_add_epi16 (sum01x8, sum23x8);
                sum0123 = _mm_add_epi16 (sum0123, const32x8);
                sum0123 = _mm_srai_epi16 (sum0123, 6);

                sum0123 = _mm_packus_epi16 (sum0123, sum0123);

                _mm_storel_epi64 ((__m128i *)(pred + colIdx), sum0123);

            }

            pred     += predStride;
            deltaPos += intraPredAngle;

        }

    }
    else
    {
        for (rowIdx = 0; rowIdx < height; rowIdx++)
        {
            deltaInt = deltaPos >> 5;

            // Just copy the integer samples
            for (colIdx = 0; colIdx < width; colIdx += 8)
            {
                ref0x16 = _mm_loadl_epi64 ((__m128i *)(refMain + colIdx + deltaInt + 1));

                _mm_storel_epi64 ((__m128i *)(pred + colIdx), ref0x16);
            }

            pred     += predStride;
            deltaPos += intraPredAngle;
        }
    }

}

void Xin266ChromaIntraFilter4xH_SSSE3 (
    PIXEL    *pred,
    intptr_t predStride,
    SINT32   intraPredAngle,
    PIXEL    *refMain,
    SINT32   width,
    SINT32   height)
{
    SINT32  rowIdx;
    SINT32  deltaPos;
    SINT32  deltaInt;
    SINT32  deltaFrac;
    UINT32  ref32;
    UINT32  refA32, refB32;
    __m128i ref0x4;
    __m128i ref1x4;
    __m128i dif10;
    __m128i allZero;
    __m128i const16x4;
    __m128i fracx4;
    __m128i sum;
    __m128i sumx8;
    UINT32  sum32;

    (void)width;

    deltaPos  = intraPredAngle;
    allZero   = _mm_setzero_si128 ();
    const16x4 = _mm_set1_epi16 (16);

    if (XIN_ABS(intraPredAngle) & 0x1F)
    {
        for (rowIdx = 0; rowIdx < height; rowIdx++)
        {
            deltaInt  = deltaPos >> 5;
            deltaFrac = deltaPos & 31;
            fracx4    = _mm_set1_epi16 ((SINT16)deltaFrac);

            // Do linear filtering
            refA32 = *((UINT32 *)(refMain + deltaInt + 1));
            refB32 = *((UINT32 *)(refMain + deltaInt + 2));

            ref0x4 = _mm_cvtsi32_si128 (refA32);
            ref1x4 = _mm_cvtsi32_si128 (refB32);

            ref0x4 = _mm_unpacklo_epi8 (ref0x4, allZero);
            ref1x4 = _mm_unpacklo_epi8 (ref1x4, allZero);

            dif10 = _mm_sub_epi16 (ref1x4, ref0x4);
            dif10 = _mm_mullo_epi16 (dif10, fracx4);
            dif10 = _mm_add_epi16 (dif10, const16x4);
            dif10 = _mm_srai_epi16 (dif10, 5);

            sum   = _mm_add_epi16 (ref0x4, dif10);
            sumx8 = _mm_packus_epi16 (sum, sum);
            sum32 = _mm_cvtsi128_si32 (sumx8);

            *((UINT32 *)(pred)) = sum32;

            pred     += predStride;
            deltaPos += intraPredAngle;
            
        }
        
    }
    else
    {
        for (rowIdx = 0; rowIdx < height; rowIdx++)
        {
            deltaInt  = deltaPos >> 5;

            // Just copy the integer samples
            ref32 = *((UINT32 *)(refMain + deltaInt + 1));
            
            *((UINT32 *)pred) = ref32;

            pred     += predStride;
            deltaPos += intraPredAngle;

        }
    }

}

void Xin266ChromaIntraFilterGt4xH_SSSE3 (
    PIXEL    *pred,
    intptr_t predStride,
    SINT32   intraPredAngle,
    PIXEL    *refMain,
    SINT32   width,
    SINT32   height)
{
    SINT32  rowIdx;
    SINT32  colIdx;
    SINT32  deltaPos;
    SINT32  deltaInt;
    SINT32  deltaFrac;
    __m128i ref0x16;
    __m128i ref1x16;
    __m128i ref0x8;
    __m128i ref1x8;
    __m128i dif10;
    __m128i allZero;
    __m128i const16x8;
    __m128i fracx8;
    __m128i sum;
    __m128i sumx8;

    deltaPos  = intraPredAngle;
    allZero   = _mm_setzero_si128 ();
    const16x8 = _mm_set1_epi16 (16);

    if (XIN_ABS(intraPredAngle) & 0x1F)
    {
        for (rowIdx = 0; rowIdx < height; rowIdx++)
        {
            deltaInt  = deltaPos >> 5;
            deltaFrac = deltaPos & 31;
            fracx8    = _mm_set1_epi16 ((SINT16)deltaFrac);

            // Do linear filtering
            for (colIdx = 0; colIdx < width; colIdx += 8)
            {
                ref0x16 = _mm_loadl_epi64 ((__m128i *)(refMain + colIdx + deltaInt + 1));
                ref1x16 = _mm_loadl_epi64 ((__m128i *)(refMain + colIdx + deltaInt + 2));

                ref0x8 = _mm_unpacklo_epi8 (ref0x16, allZero);
                ref1x8 = _mm_unpacklo_epi8 (ref1x16, allZero);

                dif10 = _mm_sub_epi16 (ref1x8, ref0x8);
                dif10 = _mm_mullo_epi16 (dif10, fracx8);
                dif10 = _mm_add_epi16 (dif10, const16x8);
                dif10 = _mm_srai_epi16 (dif10, 5);

                sum   = _mm_add_epi16 (ref0x8, dif10);
                sumx8 = _mm_packus_epi16 (sum, sum);

                _mm_storel_epi64 ((__m128i *)(pred + colIdx), sumx8);

            }

            pred     += predStride;
            deltaPos += intraPredAngle;

        }
    }
    else
    {
        for (rowIdx = 0; rowIdx < height; rowIdx++)
        {
            deltaInt  = deltaPos >> 5;

            // Just copy the integer samples
            for (colIdx = 0; colIdx < width; colIdx += 8)
            {
                ref0x16 = _mm_loadl_epi64 ((__m128i *)(refMain + colIdx + deltaInt + 1));

                _mm_storel_epi64 ((__m128i *)(pred + colIdx), ref0x16);
            }

            pred     += predStride;
            deltaPos += intraPredAngle;

        }
    }

}

static void Xin266ApplyPDPCHor8xH_SSSE3 (
    PIXEL    *pred,
    intptr_t predStride,
    SINT32   invAngle,
    UINT32   angularScale,
    PIXEL    *refSide,
    UINT32   lgWidth,
    UINT32   lgHeight)
{
    SINT32  height;
    SINT32  rowIdx;
    SINT32  shift;
    SINT32  invSum;
    __m128i lftx8;
    __m128i predx8;
    __m128i allZero;
    __m128i difx8;
    __m128i c32x8;

    (void)lgWidth;
    height  = 1 << lgHeight;
    invSum  = 256 + invAngle;
    allZero = _mm_setzero_si128 ();
    c32x8   = _mm_set1_epi16 (32);
    height  = XIN_MIN (3 << angularScale, height);

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        shift = 5 - (2 * rowIdx >> angularScale);

        lftx8 = _mm_loadl_epi64 ((__m128i *)(refSide + (invSum >> 9) + 1));
        lftx8 = _mm_unpacklo_epi8 (lftx8, allZero);

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

static void Xin266ApplyPDPCHorGt8xH_SSSE3 (
    PIXEL    *pred,
    intptr_t predStride,
    SINT32   invAngle,
    UINT32   angularScale,
    PIXEL    *refSide,
    UINT32   lgWidth,
    UINT32   lgHeight)
{
    SINT32  width;
    SINT32  height;
    SINT32  rowIdx;
    SINT32  colIdx;
    SINT32  shift;
    SINT32  invSum;
    __m128i lftx16;
    __m128i lft0x8;
    __m128i lft1x8;
    __m128i predx16;
    __m128i pred0x8;
    __m128i pred1x8;
    __m128i allZero;
    __m128i dif0x8;
    __m128i dif1x8;
    __m128i c32x8;

    width   = 1 << lgWidth;
    height  = 1 << lgHeight;
    invSum  = 256 + invAngle;
    allZero = _mm_setzero_si128 ();
    c32x8   = _mm_set1_epi16 (32);
    height  = XIN_MIN (3 << angularScale, height);

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        shift = 5 - (2 * rowIdx >> angularScale);

        for (colIdx = 0; colIdx < width; colIdx += 16)
        {
            lftx16 = _mm_loadu_si128 ((__m128i *)(refSide + colIdx + (invSum >> 9) + 1));
            lft0x8 = _mm_unpacklo_epi8 (lftx16, allZero);
            lft1x8 = _mm_unpackhi_epi8 (lftx16, allZero);

            predx16 = _mm_loadu_si128 ((__m128i *)(pred + colIdx));
            pred0x8 = _mm_unpacklo_epi8 (predx16, allZero);
            pred1x8 = _mm_unpackhi_epi8 (predx16, allZero);

            dif0x8 = _mm_sub_epi16 (lft0x8, pred0x8);
            dif1x8 = _mm_sub_epi16 (lft1x8, pred1x8);
            dif0x8 = _mm_slli_epi16 (dif0x8, shift);
            dif1x8 = _mm_slli_epi16 (dif1x8, shift);
            dif0x8 = _mm_add_epi16 (dif0x8, c32x8);
            dif1x8 = _mm_add_epi16 (dif1x8, c32x8);
            dif0x8 = _mm_srai_epi16 (dif0x8, 6);
            dif1x8 = _mm_srai_epi16 (dif1x8, 6);

            pred0x8 = _mm_add_epi16 (dif0x8, pred0x8);
            pred1x8 = _mm_add_epi16 (dif1x8, pred1x8);

            predx16 = _mm_packus_epi16 (pred0x8, pred1x8);

            _mm_storeu_si128 ((__m128i *)(pred + colIdx), predx16);

        }

        pred   += predStride;
        invSum += invAngle;

    }

}

