/***************************************************************************//**
 *
 * @file          h266_iskip_2d_avx2.c
 * @brief         h.266 inverse skip transform (AVX2).
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
#include "h26x_trans_context.h"
#include "basic_macro.h"
#include "memory.h"
#include "h26x_definition.h"
#include "h26x_common_data.h"
#ifdef __linux__
#include "xin_avx2_linux_patch.h"
#endif

#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif

void Xin266ISkip64xH_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height)
{
    UINT32  rowIdx;
    UINT32  shift;
    __m256i input0x16;
    __m256i input1x16;
    __m256i input2x16;
    __m256i input3x16;

    shift = XIN_TR_MAX_LG_RANGE - XIN_8_BIT_DEPTH - ((calcLog2[width] + calcLog2[height]) >> 1);

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        input0x16 = _mm256_lddqu_si256 ((__m256i *)(input +  0));
        input1x16 = _mm256_lddqu_si256 ((__m256i *)(input + 16));
        input2x16 = _mm256_lddqu_si256 ((__m256i *)(input + 32));
        input3x16 = _mm256_lddqu_si256 ((__m256i *)(input + 48));

        input0x16 = _mm256_srai_epi16 (input0x16, shift);
        input1x16 = _mm256_srai_epi16 (input1x16, shift);
        input2x16 = _mm256_srai_epi16 (input2x16, shift);
        input3x16 = _mm256_srai_epi16 (input3x16, shift);

        _mm256_storeu_si256 ((__m256i *)(output +  0), input0x16);
        _mm256_storeu_si256 ((__m256i *)(output + 16), input1x16);
        _mm256_storeu_si256 ((__m256i *)(output + 32), input2x16);
        _mm256_storeu_si256 ((__m256i *)(output + 48), input3x16);

        input  += inputStride;
        output += outputStride;

    }

}

void Xin266ISkip32xH_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height)
{
    UINT32  rowIdx;
    UINT32  shift;
    __m256i input0x16;
    __m256i input1x16;

    shift = XIN_TR_MAX_LG_RANGE - XIN_8_BIT_DEPTH - ((calcLog2[width] + calcLog2[height]) >> 1);

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        input0x16 = _mm256_lddqu_si256 ((__m256i *)(input +  0));
        input1x16 = _mm256_lddqu_si256 ((__m256i *)(input + 16));

        input0x16 = _mm256_srai_epi16 (input0x16, shift);
        input1x16 = _mm256_srai_epi16 (input1x16, shift);

        _mm256_storeu_si256 ((__m256i *)(output +  0), input0x16);
        _mm256_storeu_si256 ((__m256i *)(output + 16), input1x16);

        input  += inputStride;
        output += outputStride;

    }

}

void Xin266ISkip16xH_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height)
{
    UINT32  rowIdx;
    UINT32  shift;
    __m256i input0x16;
    __m256i input1x16;

    shift = XIN_TR_MAX_LG_RANGE - XIN_8_BIT_DEPTH - ((calcLog2[width] + calcLog2[height]) >> 1);

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        input0x16 = _mm256_lddqu_si256 ((__m256i *)(input + 0));
        input1x16 = _mm256_lddqu_si256 ((__m256i *)(input + inputStride));

        input0x16 = _mm256_srai_epi16 (input0x16, shift);
        input1x16 = _mm256_srai_epi16 (input1x16, shift);

        _mm256_storeu_si256 ((__m256i *)(output + 0),            input0x16);
        _mm256_storeu_si256 ((__m256i *)(output + outputStride), input1x16);

        input  += inputStride*2;
        output += outputStride*2;

    }

}

void Xin266ISkip8xH_SSE4 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height)
{
    UINT32  rowIdx;
    UINT32  shift;
    __m128i input0x8;
    __m128i input1x8;

    shift = XIN_TR_MAX_LG_RANGE - XIN_8_BIT_DEPTH - ((calcLog2[width] + calcLog2[height]) >> 1);

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        input0x8 = _mm_loadu_si128 ((__m128i *)(input + 0));
        input1x8 = _mm_loadu_si128 ((__m128i *)(input + inputStride));

        input0x8 = _mm_srai_epi16 (input0x8, shift);
        input1x8 = _mm_srai_epi16 (input1x8, shift);

        _mm_storeu_si128 ((__m128i *)(output + 0),            input0x8);
        _mm_storeu_si128 ((__m128i *)(output + outputStride), input1x8);

        input  += inputStride*2;
        output += outputStride*2;

    }

}

void Xin266ISkip4xH_SSE4 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height)
{
    UINT32  rowIdx;
    UINT32  shift;
    __m128i input0x4;
    __m128i input1x4;

    shift = XIN_TR_MAX_LG_RANGE - XIN_8_BIT_DEPTH - ((calcLog2[width] + calcLog2[height]) >> 1);

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        input0x4 = _mm_loadl_epi64 ((__m128i *)(input + 0));
        input1x4 = _mm_loadl_epi64 ((__m128i *)(input + inputStride));

        input0x4 = _mm_srai_epi16 (input0x4, shift);
        input1x4 = _mm_srai_epi16 (input1x4, shift);

        _mm_storel_epi64 ((__m128i *)(output + 0),            input0x4);
        _mm_storel_epi64 ((__m128i *)(output + outputStride), input1x4);

        input  += inputStride*2;
        output += outputStride*2;

    }

}

