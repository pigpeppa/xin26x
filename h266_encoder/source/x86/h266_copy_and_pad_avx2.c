/***************************************************************************//**
 *
 * @file          h266_copy_and_pad_avx2.c
 * @brief         h266 copy and padding optimized subroutines (AVX2).
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
#include "h266_constant.h"
#include "xin_video_common.h"
#include "h26x_definition.h"
#include "h266_definition.h"
#include "h266_inter_pred_context.h"
#include "basic_macro.h"
#include "h26x_block_utility.h"
#include "memory.h"
#include "h26x_compute_dist.h"
#include "h26x_common_data.h"
#include "immintrin.h"

#ifdef __linux__
#include "xin_avx2_linux_patch.h"
#endif

static UINT8 shuffleWidth8[32] =
{
    2, 2, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 16, 16, 
};

static UINT8 shuffleWidth16[32] =
{
    2, 2, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 24, 24  
};

void Xin266CopyAndPad_AVX2 (
    PIXEL      *ref,
    intptr_t   refStride,
    PIXEL      *pad,
    intptr_t   padStride,
    UINT32     width,
    UINT32     height)
{
    SINT32   offset;
    UINT32   rowIdx;
    __m256i  pixelx16;
    __m256i  shufIndex;
    PIXEL    *input; 
    PIXEL    *output;

    shufIndex = _mm256_loadu_si256 ((__m256i *)((width == 8) ? shuffleWidth8 : shuffleWidth16));
    width    += LUMA_INTERP_TAPS - 1;
    height   += LUMA_INTERP_TAPS - 1;
    offset    = (LUMA_INTERP_TAPS>>1) - 1;
    input     = ref - offset*refStride - offset - DMVR_NUM_ITERATION;
    output    = pad - offset*padStride - offset - DMVR_NUM_ITERATION;
    
    pixelx16 = _mm256_loadu_si256 ((__m256i *)input);
    pixelx16 = _mm256_shuffle_epi8 (pixelx16, shufIndex);

    _mm256_storeu_si256 ((__m256i *)output,                 pixelx16);
    _mm256_storeu_si256 ((__m256i *)(output - padStride),   pixelx16);
    _mm256_storeu_si256 ((__m256i *)(output - padStride*2), pixelx16);

    input  += refStride;
    output += padStride;

    for (rowIdx = 1; rowIdx < height; rowIdx++)
    {
        pixelx16 = _mm256_loadu_si256 ((__m256i *)input);
        pixelx16 = _mm256_shuffle_epi8 (pixelx16, shufIndex);

        _mm256_storeu_si256 ((__m256i *)output, pixelx16);

        input  += refStride;
        output += padStride;
    }

    _mm256_storeu_si256 ((__m256i *)output,                 pixelx16);
    _mm256_storeu_si256 ((__m256i *)(output + padStride),   pixelx16);
    
    
}
