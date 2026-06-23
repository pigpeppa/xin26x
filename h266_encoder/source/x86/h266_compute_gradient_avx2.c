/***************************************************************************//**
 *
 * @file          h266_compute_gradient_avx2.c
 * @brief         This file computes horizontal, vertical, 45-degree and 135-degree gradients for a block (AVX2).
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
#include "immintrin.h"
#include "xin_typedef.h"
#include "basic_macro.h"
#ifdef __linux__
#include "xin_avx2_linux_patch.h"
#endif

static const UINT8 pixelMask31[32] =
{
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
};

void Xin266ComputeGradient32xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    SINT32   *cuGrad,
    SINT32   width,
    SINT32   height)
{
    __m256i curx32;
    __m256i rgtx32;
    __m256i botx32;
    __m256i botRgtx32;
    __m256i sad0;
    __m256i sad1;
    __m256i sad2;
    __m256i sad3;
    __m256i sad10, sad32;
    __m256i sadLo, sadHi;
    __m256i sadx2;
    __m128i sad3210;
    __m256i pixelMask;
    SINT32  rowIdx;

    (void)width;

    pixelMask = _mm256_loadu_si256 ((__m256i *)(pixelMask31));
    sad0      = _mm256_setzero_si256 ();
    sad1      = _mm256_setzero_si256 ();
    sad2      = _mm256_setzero_si256 ();
    sad3      = _mm256_setzero_si256 ();
    
    for (rowIdx = 0; rowIdx < height - 1; rowIdx++)
    {
        curx32    = _mm256_loadu_si256 ((__m256i *)(input + inputStride*0 + 0));
        rgtx32    = _mm256_loadu_si256 ((__m256i *)(input + inputStride*0 + 1));
        botx32    = _mm256_loadu_si256 ((__m256i *)(input + inputStride*1 + 0));
        botRgtx32 = _mm256_loadu_si256 ((__m256i *)(input + inputStride*1 + 1));

        curx32    = _mm256_and_si256 (curx32,    pixelMask);
        rgtx32    = _mm256_and_si256 (rgtx32,    pixelMask);
        botx32    = _mm256_and_si256 (botx32,    pixelMask);
        botRgtx32 = _mm256_and_si256 (botRgtx32, pixelMask);

        sad0 = _mm256_add_epi32 (sad0, _mm256_sad_epu8 (rgtx32,    curx32));
        sad1 = _mm256_add_epi32 (sad1, _mm256_sad_epu8 (botx32,    curx32));
        sad2 = _mm256_add_epi32 (sad2, _mm256_sad_epu8 (rgtx32,    botx32));
        sad3 = _mm256_add_epi32 (sad3, _mm256_sad_epu8 (botRgtx32, curx32));

        input += inputStride;
    }

    sad10   = _mm256_or_si256 (_mm256_slli_epi64 (sad1, 32), sad0);
    sad32   = _mm256_or_si256 (_mm256_slli_epi64 (sad3, 32), sad2);
    sadLo   = _mm256_unpacklo_epi64 (sad10, sad32);
    sadHi   = _mm256_unpackhi_epi64 (sad10, sad32);
    sadx2   = _mm256_add_epi32 (sadLo, sadHi);
    sad3210 = _mm_add_epi32 (_mm256_castsi256_si128(sadx2), _mm256_extracti128_si256(sadx2, 1));

    _mm_storeu_si128 ((__m128i *)cuGrad, sad3210);
    
}

void Xin266ComputeGradient64xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    SINT32   *cuGrad,
    SINT32   width,
    SINT32   height)
{
    __m256i curx32;
    __m256i rgtx32;
    __m256i botx32;
    __m256i botRgtx32;
    __m256i sad0;
    __m256i sad1;
    __m256i sad2;
    __m256i sad3;
    __m256i sad10, sad32;
    __m256i sadLo, sadHi;
    __m256i sadx2;
    __m128i sad3210;
    __m256i pixelMask;
    SINT32  rowIdx;

    (void)width;

    pixelMask = _mm256_loadu_si256 ((__m256i *)(pixelMask31));
    sad0      = _mm256_setzero_si256 ();
    sad1      = _mm256_setzero_si256 ();
    sad2      = _mm256_setzero_si256 ();
    sad3      = _mm256_setzero_si256 ();
    
    for (rowIdx = 0; rowIdx < height - 1; rowIdx++)
    {
        curx32    = _mm256_loadu_si256 ((__m256i *)(input + inputStride*0 + 0));
        rgtx32    = _mm256_loadu_si256 ((__m256i *)(input + inputStride*0 + 1));
        botx32    = _mm256_loadu_si256 ((__m256i *)(input + inputStride*1 + 0));
        botRgtx32 = _mm256_loadu_si256 ((__m256i *)(input + inputStride*1 + 1));

        sad0 = _mm256_add_epi32 (sad0, _mm256_sad_epu8 (rgtx32,    curx32));
        sad1 = _mm256_add_epi32 (sad1, _mm256_sad_epu8 (botx32,    curx32));
        sad2 = _mm256_add_epi32 (sad2, _mm256_sad_epu8 (rgtx32,    botx32));
        sad3 = _mm256_add_epi32 (sad3, _mm256_sad_epu8 (botRgtx32, curx32));

        curx32    = _mm256_loadu_si256 ((__m256i *)(input + inputStride*0 + 32));
        rgtx32    = _mm256_loadu_si256 ((__m256i *)(input + inputStride*0 + 33));
        botx32    = _mm256_loadu_si256 ((__m256i *)(input + inputStride*1 + 32));
        botRgtx32 = _mm256_loadu_si256 ((__m256i *)(input + inputStride*1 + 33));
        
        curx32    = _mm256_and_si256 (curx32,    pixelMask);
        rgtx32    = _mm256_and_si256 (rgtx32,    pixelMask);
        botx32    = _mm256_and_si256 (botx32,    pixelMask);
        botRgtx32 = _mm256_and_si256 (botRgtx32, pixelMask);

        sad0 = _mm256_add_epi32 (sad0, _mm256_sad_epu8 (rgtx32,    curx32));
        sad1 = _mm256_add_epi32 (sad1, _mm256_sad_epu8 (botx32,    curx32));
        sad2 = _mm256_add_epi32 (sad2, _mm256_sad_epu8 (rgtx32,    botx32));
        sad3 = _mm256_add_epi32 (sad3, _mm256_sad_epu8 (botRgtx32, curx32));

        input += inputStride;
        
    }

    sad10   = _mm256_or_si256 (_mm256_slli_epi64 (sad1, 32), sad0);
    sad32   = _mm256_or_si256 (_mm256_slli_epi64 (sad3, 32), sad2);
    sadLo   = _mm256_unpacklo_epi64 (sad10, sad32);
    sadHi   = _mm256_unpackhi_epi64 (sad10, sad32);
    sadx2   = _mm256_add_epi32 (sadLo, sadHi);
    sad3210 = _mm_add_epi32 (_mm256_castsi256_si128(sadx2), _mm256_extracti128_si256(sadx2, 1));

    _mm_storeu_si128 ((__m128i *)cuGrad, sad3210);
    
}


