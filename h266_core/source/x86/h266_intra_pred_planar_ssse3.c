/***************************************************************************//**
 *
 * @file          h266_intra_pred_planar_ssse3.c
 * @brief         h266 intra prediction subroutines (planar, SSSE3).
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
#include "tmmintrin.h"
#include "basic_macro.h"

static const UINT16 rgtMult[64] =
{
    1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,
};

// TL T T T T TR TR TR TR L L L L BL BL BL BL
void Xin266IntraPredPlanar4xH_SSSE3 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight)
{
    UINT32  width;
    UINT32  height;
    UINT32  minLgSize;
    UINT32  idx;
    PIXEL   *lftBuf;
    PIXEL   *topBuf;
    UINT32  row;
    SINT32  finalShift;
    SINT16  offset;
    SINT16  lftCol[64];
    SINT16  rgtCol[64];
    UINT32  top32;
    UINT32  lft32;
    UINT32  ref32;
    __m128i allZero;
    __m128i topx4;
    __m128i botx4;
    __m128i lftx4;
    __m128i rgtx4;
    __m128i accRgtx4;
    __m128i botLftx4;
    __m128i topRgtx4;
    __m128i rgtMulx4;
    __m128i verPredx4;
    __m128i horPredx4;
    __m128i sumx4;
    __m128i offsetx4;

    width      = 1 << lgWidth;
    height     = 1 << lgHeight;
    topBuf     = nBuf;
    lftBuf     = nBuf + 1 + multiRefIdx + width*4;
    minLgSize  = XIN_MIN (lgWidth, lgHeight);
    finalShift = 1 + lgWidth + lgHeight - minLgSize;
    offset     = 1 << (lgWidth + lgHeight - minLgSize);

    allZero  = _mm_setzero_si128 ();
    botLftx4 = _mm_set1_epi16 (lftBuf[height + 1]);
    topRgtx4 = _mm_set1_epi16 (topBuf[width + 1]);
    offsetx4 = _mm_set1_epi16 (offset);

    // Get left and above reference column and row
    top32 = *((UINT32 *)(topBuf + 1));
    topx4 = _mm_cvtsi32_si128 (top32);
    topx4 = _mm_unpacklo_epi8 (topx4, allZero);
    botx4 = _mm_sub_epi16 (botLftx4, topx4);
    topx4 = _mm_slli_epi16 (topx4, lgHeight);

    rgtMulx4 = _mm_loadu_si128 ((__m128i *)(rgtMult));

    for (idx = 0; idx < height; idx += 4)
    {
        lft32 = *((UINT32 *)(lftBuf + idx + 1));
        lftx4 = _mm_cvtsi32_si128 (lft32);
        lftx4 = _mm_unpacklo_epi8 (lftx4, allZero);
        rgtx4 = _mm_sub_epi16 (topRgtx4, lftx4);
        lftx4 = _mm_slli_epi16 (lftx4, lgWidth);

        _mm_storel_epi64 ((__m128i *)(lftCol + idx), lftx4);
        _mm_storel_epi64 ((__m128i *)(rgtCol + idx), rgtx4);
    }

    for (row = 0; row < height; row++)
    {
        lftx4 = _mm_set1_epi16 (lftCol[row]);
        rgtx4 = _mm_set1_epi16 (rgtCol[row]);
        topx4 = _mm_add_epi16 (topx4, botx4);

        accRgtx4  = _mm_mullo_epi16 (rgtx4, rgtMulx4);
        horPredx4 = _mm_add_epi16 (lftx4, accRgtx4);

        verPredx4 = _mm_slli_epi16 (topx4,     lgWidth - minLgSize);
        horPredx4 = _mm_slli_epi16 (horPredx4, lgHeight - minLgSize);

        sumx4 = _mm_add_epi16 (verPredx4, horPredx4);
        sumx4 = _mm_add_epi16 (sumx4, offsetx4);
        sumx4 = _mm_srai_epi16 (sumx4, finalShift);
        sumx4 = _mm_packus_epi16 (sumx4, sumx4);
        ref32 = _mm_cvtsi128_si32 (sumx4);

        *((UINT32 *)(dst)) = ref32;

        dst += dstStride;

    }

}

// TL T T T T TR TR TR TR L L L L BL BL BL BL
void Xin266IntraPredPlanar_SSSE3 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight)
{
    UINT32  width;
    UINT32  height;
    UINT32  minLgSize;
    UINT32  idx;
    PIXEL   *lftBuf;
    PIXEL   *topBuf;
    UINT32  col;
    UINT32  row;
    SINT32  finalShift;
    SINT16  offset;
    SINT16  topRow[64];
    SINT16  lftCol[64];
    SINT16  botRow[64];
    SINT16  rgtCol[64];
    __m128i allZero;
    __m128i topx8;
    __m128i botx8;
    __m128i lftx8;
    __m128i rgtx8;
    __m128i accRgtx8;
    __m128i botLftx8;
    __m128i topRgtx8;
    __m128i rgtMulx8;
    __m128i verPredx8;
    __m128i horPredx8;
    __m128i sumx8;
    __m128i offsetx8;

    width      = 1 << lgWidth;
    height     = 1 << lgHeight;
    topBuf     = nBuf;
    lftBuf     = nBuf + 1 + multiRefIdx + width*4;
    minLgSize  = XIN_MIN (lgWidth, lgHeight);
    finalShift = 1 + lgWidth + lgHeight - minLgSize;
    offset     = 1 << (lgWidth + lgHeight - minLgSize);

    allZero  = _mm_setzero_si128 ();
    botLftx8 = _mm_set1_epi16 (lftBuf[height + 1]);
    topRgtx8 = _mm_set1_epi16 (topBuf[width + 1]);
    offsetx8 = _mm_set1_epi16 (offset);

    // Get left and above reference column and row
    for (idx = 0; idx < width; idx += 8)
    {
        topx8 = _mm_loadl_epi64 ((__m128i *)(topBuf + idx + 1));
        topx8 = _mm_unpacklo_epi8 (topx8, allZero);
        botx8 = _mm_sub_epi16 (botLftx8, topx8);
        topx8 = _mm_slli_epi16 (topx8, lgHeight);

        _mm_storeu_si128 ((__m128i *)(topRow + idx), topx8);
        _mm_storeu_si128 ((__m128i *)(botRow + idx), botx8);
    }

    for (idx = 0; idx < height; idx += 8)
    {
        lftx8 = _mm_loadl_epi64 ((__m128i *)(lftBuf + idx + 1));
        lftx8 = _mm_unpacklo_epi8 (lftx8, allZero);
        rgtx8 = _mm_sub_epi16 (topRgtx8, lftx8);
        lftx8 = _mm_slli_epi16 (lftx8, lgWidth);

        _mm_storeu_si128 ((__m128i *)(lftCol + idx), lftx8);
        _mm_storeu_si128 ((__m128i *)(rgtCol + idx), rgtx8);
    }

    for (row = 0; row < height; row++)
    {
        lftx8 = _mm_set1_epi16 (lftCol[row]);
        rgtx8 = _mm_set1_epi16 (rgtCol[row]);

        for (col = 0; col < width; col += 8)
        {
            topx8 = _mm_loadu_si128 ((__m128i *)(topRow + col));
            botx8 = _mm_loadu_si128 ((__m128i *)(botRow + col));
            topx8 = _mm_add_epi16 (topx8, botx8);

            rgtMulx8  = _mm_loadu_si128 ((__m128i *)(rgtMult + col));
            accRgtx8  = _mm_mullo_epi16 (rgtx8, rgtMulx8);
            horPredx8 = _mm_add_epi16 (lftx8, accRgtx8);

            verPredx8 = _mm_slli_epi16 (topx8,     lgWidth - minLgSize);
            horPredx8 = _mm_slli_epi16 (horPredx8, lgHeight - minLgSize);

            sumx8 = _mm_add_epi16 (verPredx8, horPredx8);
            sumx8 = _mm_add_epi16 (sumx8, offsetx8);
            sumx8 = _mm_srai_epi16 (sumx8, finalShift);
            sumx8 = _mm_packus_epi16 (sumx8, sumx8);

            _mm_storeu_si128 ((__m128i *)(topRow + col), topx8);
            _mm_storel_epi64 ((__m128i *)(dst + col),    sumx8);

        }

        dst += dstStride;

    }

}
