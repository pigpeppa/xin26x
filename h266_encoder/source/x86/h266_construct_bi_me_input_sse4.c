/***************************************************************************//**
 *
 * @file          h266_construct_bi_me_input_sse4.c
 * @brief         Construct input for bi-directional motion estimation (SSE4).
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
#include <xin_typedef.h>
#include <h266_constant.h>
#include "basic_macro.h"
#include <emmintrin.h>
#include <tmmintrin.h>
#include <smmintrin.h>

void Xin266ConstructWeightBiMeInputGt4xH_SSE4 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    PIXEL    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height,
    SINT32   bcwWeight)
{
    SINT32  normalizer;
    SINT32  weight0;
    SINT32  weight1;
    UINT32  rowIdx;
    UINT32  colIdx;
    __m128i weight01x4;
    __m128i inputTx8, predTx8, inputBx8, predBx8;
    __m128i inPredTItl0x4, inPredTItl1x4, inPredBItl0x4, inPredBItl1x4;
    __m128i outputT32x4A, outputT32x4B, outputB32x4A, outputB32x4B;
    __m128i outputT16x8, outputB16x8;
    __m128i offsetx4;

    normalizer = ((1 << 12) + (bcwWeight > 0 ? (bcwWeight >> 1) : -(bcwWeight >> 1))) / bcwWeight;
    weight0    = normalizer << XIN_BCW_LOG_WGT_BASE;
    weight1    = -(XIN_BCW_WGT_BASE - bcwWeight)*normalizer;
    weight01x4 = _mm_set1_epi32 ((weight1 << 16) | (weight0 & 0xFFFF));
    offsetx4   = _mm_set1_epi32 (1 << 11);

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        for (colIdx = 0; colIdx < width; colIdx += 8)
        {
            inputTx8 = _mm_loadl_epi64 ((__m128i *)(input + colIdx));
            inputBx8 = _mm_loadl_epi64 ((__m128i *)(input + colIdx + inputStride));
            predTx8  = _mm_loadl_epi64 ((__m128i *)(pred + colIdx));
            predBx8  = _mm_loadl_epi64 ((__m128i *)(pred + colIdx + predStride));

            inputTx8 = _mm_cvtepu8_epi16 (inputTx8);
            predTx8  = _mm_cvtepu8_epi16 (predTx8);
            inputBx8 = _mm_cvtepu8_epi16 (inputBx8);
            predBx8  = _mm_cvtepu8_epi16 (predBx8);

            inPredTItl0x4 = _mm_unpacklo_epi16 (inputTx8, predTx8);
            inPredTItl1x4 = _mm_unpackhi_epi16 (inputTx8, predTx8);
            inPredBItl0x4 = _mm_unpacklo_epi16 (inputBx8, predBx8);
            inPredBItl1x4 = _mm_unpackhi_epi16 (inputBx8, predBx8);

            outputT32x4A = _mm_madd_epi16 (inPredTItl0x4, weight01x4);
            outputT32x4B = _mm_madd_epi16 (inPredTItl1x4, weight01x4);
            outputB32x4A = _mm_madd_epi16 (inPredBItl0x4, weight01x4);
            outputB32x4B = _mm_madd_epi16 (inPredBItl1x4, weight01x4);

            outputT32x4A = _mm_add_epi32 (outputT32x4A, offsetx4);
            outputT32x4B = _mm_add_epi32 (outputT32x4B, offsetx4);
            outputB32x4A = _mm_add_epi32 (outputB32x4A, offsetx4);
            outputB32x4B = _mm_add_epi32 (outputB32x4B, offsetx4);

            outputT32x4A = _mm_srai_epi32 (outputT32x4A, 12);
            outputT32x4B = _mm_srai_epi32 (outputT32x4B, 12);
            outputB32x4A = _mm_srai_epi32 (outputB32x4A, 12);
            outputB32x4B = _mm_srai_epi32 (outputB32x4B, 12);

            outputT16x8 = _mm_packs_epi32 (outputT32x4A, outputT32x4B);
            outputB16x8 = _mm_packs_epi32 (outputB32x4A, outputB32x4B);
            
            _mm_storel_epi64 ((__m128i *)(output + colIdx),                _mm_packus_epi16 (outputT16x8, outputT16x8));
            _mm_storel_epi64 ((__m128i *)(output + colIdx + outputStride), _mm_packus_epi16 (outputB16x8, outputB16x8));
            
        }

        input  += 2*inputStride;
        pred   += 2*predStride;
        output += 2*outputStride;

    }

}
