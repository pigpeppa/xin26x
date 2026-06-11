/***************************************************************************//**
 *
 * @file          h266_intra_pred_planar_avx2.c
 * @brief         h266 intra prediction subroutines (planar, AVX2).
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
#include "immintrin.h"
#include "basic_macro.h"

#ifdef __linux__
#include "xin_avx2_linux_patch.h"
#endif

static const UINT16 rgtMult[64] =
{
    1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,
};

// TL T T T T TR TR TR TR L L L L BL BL BL BL
void Xin266IntraPredPlanarGt8xH_AVX2 (
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
    __m256i allZero;
    __m256i topx16;
    __m256i botx16;
    __m256i lftx16;
    __m256i rgtx16;
    __m256i accRgtx16;
    __m256i botLftx16;
    __m256i topRgtx16;
    __m256i rgtMulx16;
    __m256i verPredx16;
    __m256i horPredx16;
    __m256i sumx16;
    __m256i offsetx16;

    width      = 1 << lgWidth;
    height     = 1 << lgHeight;
    topBuf     = nBuf;
    lftBuf     = nBuf + 1 + multiRefIdx + width*4;
    minLgSize  = XIN_MIN (lgWidth, lgHeight);
    finalShift = 1 + lgWidth + lgHeight - minLgSize;
    offset     = 1 << (lgWidth + lgHeight - minLgSize);

    allZero   = _mm256_setzero_si256 ();
    botLftx16 = _mm256_set1_epi16 (lftBuf[height + 1]);
    topRgtx16 = _mm256_set1_epi16 (topBuf[width + 1]);
    offsetx16 = _mm256_set1_epi16 (offset);

    // Get left and above reference column and row
    for (idx = 0; idx < width; idx += 16)
    {
        topx16 = _mm256_cvtepu8_epi16 ( _mm_loadu_si128 ((__m128i *)(topBuf + idx + 1)));
        botx16 = _mm256_sub_epi16 (botLftx16, topx16);
        topx16 = _mm256_slli_epi16 (topx16, lgHeight);

        _mm256_storeu_si256 ((__m256i *)(topRow + idx), topx16);
        _mm256_storeu_si256 ((__m256i *)(botRow + idx), botx16);
    }

    for (idx = 0; idx < height; idx += 16)
    {
        lftx16 = _mm256_cvtepu8_epi16 (_mm_loadu_si128 ((__m128i *)(lftBuf + idx + 1)));
        rgtx16 = _mm256_sub_epi16 (topRgtx16, lftx16);
        lftx16 = _mm256_slli_epi16 (lftx16, lgWidth);

        _mm256_storeu_si256 ((__m256i *)(lftCol + idx), lftx16);
        _mm256_storeu_si256 ((__m256i *)(rgtCol + idx), rgtx16);
    }

    for (row = 0; row < height; row++)
    {
        lftx16 = _mm256_set1_epi16 (lftCol[row]);
        rgtx16 = _mm256_set1_epi16 (rgtCol[row]);

        for (col = 0; col < width; col += 16)
        {
            topx16 = _mm256_loadu_si256 ((__m256i *)(topRow + col));
            botx16 = _mm256_loadu_si256 ((__m256i *)(botRow + col));
            topx16 = _mm256_add_epi16 (topx16, botx16);

            rgtMulx16  = _mm256_loadu_si256 ((__m256i *)(rgtMult + col));
            accRgtx16  = _mm256_mullo_epi16 (rgtx16, rgtMulx16);
            horPredx16 = _mm256_add_epi16 (lftx16, accRgtx16);

            verPredx16 = _mm256_slli_epi16 (topx16,     lgWidth - minLgSize);
            horPredx16 = _mm256_slli_epi16 (horPredx16, lgHeight - minLgSize);

            sumx16 = _mm256_add_epi16 (verPredx16, horPredx16);
            sumx16 = _mm256_add_epi16 (sumx16, offsetx16);
            sumx16 = _mm256_srai_epi16 (sumx16, finalShift);
            sumx16 = _mm256_packus_epi16 (sumx16, sumx16);
            sumx16 = _mm256_permute4x64_epi64 (sumx16, 0xD8);

            _mm256_storeu_si256 ((__m256i *)(topRow + col), topx16);
            _mm_storeu_si128 ((__m128i *)(dst + col),  _mm256_castsi256_si128 (sumx16));

        }

        dst += dstStride;

    }

}


