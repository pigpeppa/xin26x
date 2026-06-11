/***************************************************************************//**
 *
 * @file          h266_intra_pred_ver_avx2.c
 * @brief         h266 intra prediction subroutines (vertical, AVX2).
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
#include <immintrin.h>
#include "basic_macro.h"

#ifdef __linux__
#include "xin_avx2_linux_patch.h"
#endif

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

void Xin266IntraPredVer16xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight)
{
    SINT32   width;
    SINT32   height;
    SINT32   rowIdx;
    PIXEL    *topBuf;
    PIXEL    *refMain;
    __m128i  topx16;
    PIXEL    *dstRow;

    width   = 1 << lgWidth;
    height  = 1 << lgHeight;
    topBuf  = nBuf;
    refMain = topBuf;

    // compensate for line offset in reference line buffers
    refMain += multiRefIdx;
    dstRow   = dst;
    topx16   = _mm_loadu_si128 ((__m128i *)(refMain + 1));

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        _mm_storeu_si128 ((__m128i *)(dstRow), topx16);

        _mm_storeu_si128 ((__m128i *)(dstRow + dstStride), topx16);

        dstRow += dstStride*2;
    }

}

void Xin266IntraPredVer32xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight)
{
    SINT32   width;
    SINT32   height;
    SINT32   rowIdx;
    PIXEL    *topBuf;
    PIXEL    *refMain;
    __m256i  topx32;
    PIXEL    *dstRow;

    width   = 1 << lgWidth;
    height  = 1 << lgHeight;
    topBuf  = nBuf;
    refMain = topBuf;

    // compensate for line offset in reference line buffers
    refMain += multiRefIdx;
    dstRow   = dst;
    topx32   = _mm256_loadu_si256 ((__m256i *)(refMain + 1));

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        _mm256_storeu_si256 ((__m256i *)(dstRow),             topx32);
        _mm256_storeu_si256 ((__m256i *)(dstRow + dstStride), topx32);

        dstRow += dstStride*2;
    }

}

void Xin266IntraPredVer64xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight)
{
    SINT32   width;
    SINT32   height;
    SINT32   rowIdx;
    PIXEL    *topBuf;
    PIXEL    *refMain;
    __m256i  top0x32;
    __m256i  top1x32;
    PIXEL    *dstRow;

    width   = 1 << lgWidth;
    height  = 1 << lgHeight;
    topBuf  = nBuf;
    refMain = topBuf;

    // compensate for line offset in reference line buffers
    refMain += multiRefIdx;
    dstRow   = dst;
    top0x32  = _mm256_loadu_si256 ((__m256i *)(refMain + 1));
    top1x32  = _mm256_loadu_si256 ((__m256i *)(refMain + 33));

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        _mm256_storeu_si256 ((__m256i *)(dstRow),             top0x32);
        _mm256_storeu_si256 ((__m256i *)(dstRow + 32),        top1x32);

        _mm256_storeu_si256 ((__m256i *)(dstRow + dstStride),      top0x32);
        _mm256_storeu_si256 ((__m256i *)(dstRow + dstStride + 32), top1x32);

        dstRow += dstStride*2;
    }

}


