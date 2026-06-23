/***************************************************************************//**
 *
 * @file          h266_construct_weight_Input_avx2.c
 * @brief         Construct input for weighted bi-directional motion estimation (AVX2).
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
#include <immintrin.h>

#ifdef __linux__
#include "xin_avx2_linux_patch.h"
#endif

void Xin266ConstructWeightBiMeInputGt8xH_AVX2 (
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
    __m256i weight01x8;
    __m128i inputTx16, predTx16, inputBx16, predBx16;
    __m256i inputT16x16, predT16x16, inputB16x16, predB16x16;
    __m256i inPredTItl0x8, inPredTItl1x8, inPredBItl0x8, inPredBItl1x8;
    __m256i outputT32x8A, outputT32x8B, outputB32x8A, outputB32x8B;
    __m256i offsetx8;
    __m256i outputT16x16, outputB16x16;
    __m256i outputx32;

    normalizer = ((1 << 12) + (bcwWeight > 0 ? (bcwWeight >> 1) : -(bcwWeight >> 1))) / bcwWeight;
    weight0    = normalizer << XIN_BCW_LOG_WGT_BASE;
    weight1    = -(XIN_BCW_WGT_BASE - bcwWeight)*normalizer;
    weight01x8 = _mm256_set1_epi32 ((weight1 << 16) | (weight0 & 0xFFFF));
    offsetx8   = _mm256_set1_epi32 (1 << 11);

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        for (colIdx = 0; colIdx < width; colIdx += 16)
        {
            inputTx16 = _mm_lddqu_si128 ((__m128i *)(input + colIdx));
            inputBx16 = _mm_lddqu_si128 ((__m128i *)(input + colIdx + inputStride));
            predTx16  = _mm_lddqu_si128 ((__m128i *)(pred + colIdx));
            predBx16  = _mm_lddqu_si128 ((__m128i *)(pred + colIdx + predStride));

            inputT16x16 = _mm256_cvtepu8_epi16 (inputTx16);
            predT16x16  = _mm256_cvtepu8_epi16 (predTx16);
            inputB16x16 = _mm256_cvtepu8_epi16 (inputBx16);
            predB16x16  = _mm256_cvtepu8_epi16 (predBx16);

            inPredTItl0x8 = _mm256_unpacklo_epi16 (inputT16x16, predT16x16);
            inPredTItl1x8 = _mm256_unpackhi_epi16 (inputT16x16, predT16x16);
            inPredBItl0x8 = _mm256_unpacklo_epi16 (inputB16x16, predB16x16);
            inPredBItl1x8 = _mm256_unpackhi_epi16 (inputB16x16, predB16x16);

            outputT32x8A = _mm256_madd_epi16 (inPredTItl0x8, weight01x8);
            outputT32x8B = _mm256_madd_epi16 (inPredTItl1x8, weight01x8);
            outputB32x8A = _mm256_madd_epi16 (inPredBItl0x8, weight01x8);
            outputB32x8B = _mm256_madd_epi16 (inPredBItl1x8, weight01x8);

            outputT32x8A = _mm256_add_epi32 (outputT32x8A, offsetx8);
            outputT32x8B = _mm256_add_epi32 (outputT32x8B, offsetx8);
            outputB32x8A = _mm256_add_epi32 (outputB32x8A, offsetx8);
            outputB32x8B = _mm256_add_epi32 (outputB32x8B, offsetx8);

            outputT32x8A = _mm256_srai_epi32 (outputT32x8A, 12);
            outputT32x8B = _mm256_srai_epi32 (outputT32x8B, 12);
            outputB32x8A = _mm256_srai_epi32 (outputB32x8A, 12);
            outputB32x8B = _mm256_srai_epi32 (outputB32x8B, 12);

            outputT16x16 = _mm256_packs_epi32 (outputT32x8A, outputT32x8B);
            outputB16x16 = _mm256_packs_epi32 (outputB32x8A, outputB32x8B);

            outputx32 = _mm256_packus_epi16 (outputT16x16, outputB16x16);
            outputx32 = _mm256_permute4x64_epi64 (outputx32, 0xD8);
            
            _mm_storeu_si128 ((__m128i *)(output + colIdx),                _mm256_castsi256_si128 (outputx32));
            _mm_storeu_si128 ((__m128i *)(output + colIdx + outputStride), _mm256_extracti128_si256 (outputx32, 1));
            
        }

        input  += 2*inputStride;
        pred   += 2*predStride;
        output += 2*outputStride;

    }

}

