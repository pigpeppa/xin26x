/***************************************************************************//**
 *
 * @file          h266_copy_and_pad_sse4.c
 * @brief         h266 copy and padding optimized subroutines (SSE4).
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
#include <emmintrin.h>
#include <tmmintrin.h>
#include <smmintrin.h>

static UINT8 shuffleWidth8Uv[16] =
{
    1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 11, 
};

static UINT8 shuffleWidth4Uv[16] =
{
    1, 1, 2, 3, 4, 5, 6, 7, 7, 
};

void Xin266CopyAndPadUv_SSE4 (
    PIXEL      *refU,
    PIXEL      *refV,
    intptr_t   refStride,
    PIXEL      *padU,
    PIXEL      *padV,
    intptr_t   padStride,
    UINT32     width,
    UINT32     height)
{
    SINT32   offset;
    UINT32   rowIdx;
    __m128i  pelUx16;
    __m128i  pelVx16;
    __m128i  shufIndex;
    PIXEL    *inputU; 
    PIXEL    *outputU;
    PIXEL    *inputV; 
    PIXEL    *outputV;

    shufIndex = _mm_loadu_si128 ((__m128i *)((width == 8) ? shuffleWidth8Uv : shuffleWidth4Uv));
    width    += CHROMA_INTERP_TAPS - 1;
    height   += CHROMA_INTERP_TAPS - 1;
    offset    = (CHROMA_INTERP_TAPS>>1) - 1;
    inputU    = refU - offset*refStride - offset - (DMVR_NUM_ITERATION >> 1);
    outputU   = padU - offset*padStride - offset - (DMVR_NUM_ITERATION >> 1);
    inputV    = refV - offset*refStride - offset - (DMVR_NUM_ITERATION >> 1);
    outputV   = padV - offset*padStride - offset - (DMVR_NUM_ITERATION >> 1);
    
    pelUx16 = _mm_loadu_si128 ((__m128i *)inputU);
    pelVx16 = _mm_loadu_si128 ((__m128i *)inputV);
    
    pelUx16 = _mm_shuffle_epi8 (pelUx16, shufIndex);
    pelVx16 = _mm_shuffle_epi8 (pelVx16, shufIndex);

    _mm_storeu_si128 ((__m128i *)outputU,                 pelUx16);
    _mm_storeu_si128 ((__m128i *)(outputU - padStride),   pelUx16);

    _mm_storeu_si128 ((__m128i *)outputV,                 pelVx16);
    _mm_storeu_si128 ((__m128i *)(outputV - padStride),   pelVx16);

    inputU  += refStride;
    outputU += padStride;
    inputV  += refStride;
    outputV += padStride;

    for (rowIdx = 1; rowIdx < height; rowIdx++)
    {
        pelUx16 = _mm_loadu_si128 ((__m128i *)inputU);
        pelVx16 = _mm_loadu_si128 ((__m128i *)inputV);
    
        pelUx16 = _mm_shuffle_epi8 (pelUx16, shufIndex);
        pelVx16 = _mm_shuffle_epi8 (pelVx16, shufIndex);

        _mm_storeu_si128 ((__m128i *)outputU, pelUx16);
        _mm_storeu_si128 ((__m128i *)outputV, pelVx16);

        inputU  += refStride;
        outputU += padStride;

        inputV  += refStride;
        outputV += padStride;
    }

    _mm_storeu_si128 ((__m128i *)outputU,   pelUx16);
    _mm_storeu_si128 ((__m128i *)outputV,   pelVx16);

}

