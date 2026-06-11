/***************************************************************************//**
 *
 * @file          h266_intra_pred_dc_ssse3.c
 * @brief         h266 intra prediction subroutines (DC, SSSE3).
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

void Xin266IntraPredDc4xH_SSSE3 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight)
{
    UINT32  height;
    UINT32  idx;
    UINT32  rowIdx;
    SINT32  divOffset;
    SINT32  divShift;
    UINT32  top32;
    UINT32  lft32;
    PIXEL   *topBuf;
    PIXEL   *lftBuf;
    __m128i allZero;
    __m128i lftx4;
    __m128i topx4;
    __m128i sumPel;
    UINT32  dcx4;
    UINT32  sum32;

    (void)lgWidth;
    allZero   = _mm_setzero_si128 ();
    sumPel    = _mm_setzero_si128 ();
    height    = 1 << lgHeight;
    divShift  = (4 == height) ? lgWidth + 1 : XIN_MAX(lgWidth, lgHeight);
    divOffset = 1 << (divShift - 1);
    topBuf    = nBuf;
    lftBuf    = nBuf + 1 + multiRefIdx + 4*4;

    if (4 >= height)
    {
        top32  = *((UINT32 *)(topBuf + 1 + multiRefIdx));
        topx4  = _mm_cvtsi32_si128 (top32);
        sumPel = _mm_sad_epu8 (topx4, allZero);
    }

    if (4 <= height)
    {
        for (idx = 0; idx < height; idx += 4)
        {
            lft32  = *((UINT32 *)(lftBuf + 1 + multiRefIdx));
            lftx4   = _mm_cvtsi32_si128 (lft32);
            sumPel  = _mm_add_epi32 (sumPel, _mm_sad_epu8 (lftx4, allZero));
            lftBuf += 4;
        }
    }
    
    sum32 = _mm_cvtsi128_si32 (sumPel);
    sum32 = sum32 + divOffset;
    dcx4  = sum32 >> divShift;
    dcx4  = dcx4*0x01010101;

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        *((UINT32 *)(dst + dstStride*0)) = dcx4;
        *((UINT32 *)(dst + dstStride*1)) = dcx4;

        dst += dstStride*2;
    }

}

void Xin266IntraPredDc8xH_SSSE3 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight)
{
    UINT32  height;
    UINT32  idx;
    UINT32  rowIdx;
    SINT32  divOffset;
    SINT32  divShift;
    PIXEL   *topBuf;
    PIXEL   *lftBuf;
    __m128i allZero;
    __m128i lftx8;
    __m128i topx8;
    __m128i sumPel;
    __m128i offset128;
    __m128i dcx8;

    (void)lgWidth;
    allZero   = _mm_setzero_si128 ();
    sumPel    = _mm_setzero_si128 ();
    height    = 1 << lgHeight;
    divShift  = (8 == height) ? lgWidth + 1 : XIN_MAX(lgWidth, lgHeight);
    divOffset = 1 << (divShift - 1);
    offset128 = _mm_cvtsi32_si128 (divOffset);
    topBuf    = nBuf;
    lftBuf    = nBuf + 1 + multiRefIdx + 8*4;

    if (8 >= height)
    {
        topx8  = _mm_loadl_epi64 ((__m128i *)(topBuf + 1 + multiRefIdx));
        sumPel = _mm_sad_epu8 (topx8, allZero);
    }

    if (8 <= height)
    {
        for (idx = 0; idx < height; idx += 8)
        {
            lftx8   = _mm_loadl_epi64 ((__m128i *)(lftBuf + 1 + multiRefIdx));
            sumPel  = _mm_add_epi32 (sumPel, _mm_sad_epu8 (lftx8, allZero));
            lftBuf += 8;
        }
    }

    sumPel = _mm_add_epi32 (sumPel,  offset128);
    dcx8   = _mm_srli_epi32 (sumPel, divShift);
    dcx8   = _mm_shuffle_epi8 (dcx8, allZero);

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        _mm_storel_epi64 ((__m128i *)(dst + dstStride*0),  dcx8);
        _mm_storel_epi64 ((__m128i *)(dst + dstStride*1),  dcx8);

        dst += dstStride*2;
    }

}

void Xin266IntraPredDc16xH_SSSE3 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight)
{
    UINT32  height;
    UINT32  idx;
    UINT32  rowIdx;
    SINT32  divOffset;
    SINT32  divShift;
    PIXEL   *topBuf;
    PIXEL   *lftBuf;
    __m128i allZero;
    __m128i lftx16;
    __m128i topx16;
    __m128i sumPel;
    __m128i offset128;
    __m128i dcx16;

    (void)lgWidth;
    allZero   = _mm_setzero_si128 ();
    sumPel    = _mm_setzero_si128 ();
    height    = 1 << lgHeight;
    divShift  = (16 == height) ? lgWidth + 1 : XIN_MAX(lgWidth, lgHeight);
    divOffset = 1 << (divShift - 1);
    offset128 = _mm_cvtsi32_si128 (divOffset);
    topBuf    = nBuf;
    lftBuf    = nBuf + 1 + multiRefIdx + 16*4;

    if (16 >= height)
    {
        topx16 = _mm_lddqu_si128 ((__m128i *)(topBuf + 1 + multiRefIdx));
        sumPel = _mm_sad_epu8 (topx16, allZero);
    }

    if (16 <= height)
    {
        for (idx = 0; idx < height; idx += 16)
        {
            lftx16  = _mm_lddqu_si128 ((__m128i *)(lftBuf + 1 + multiRefIdx));
            sumPel  = _mm_add_epi32 (sumPel, _mm_sad_epu8 (lftx16, allZero));
            lftBuf += 16;
        }
    }

    sumPel = _mm_add_epi32 (sumPel,   _mm_shuffle_epi32 (sumPel, 0x4E));
    sumPel = _mm_add_epi32 (sumPel,   offset128);
    dcx16  = _mm_srli_epi32 (sumPel,  divShift);
    dcx16  = _mm_shuffle_epi8 (dcx16, allZero);

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        _mm_storeu_si128 ((__m128i *)(dst + dstStride*0),  dcx16);
        _mm_storeu_si128 ((__m128i *)(dst + dstStride*1),  dcx16);

        dst += dstStride*2;
    }

}

void Xin266IntraPredDc32xH_SSSE3 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight)
{
    UINT32  height;
    UINT32  idx;
    UINT32  rowIdx;
    SINT32  divOffset;
    SINT32  divShift;
    PIXEL   *topBuf;
    PIXEL   *lftBuf;
    __m128i allZero;
    __m128i lftx16;
    __m128i topx16;
    __m128i sumPel;
    __m128i offset128;
    __m128i dcx16;

    (void)lgWidth;
    allZero   = _mm_setzero_si128 ();
    sumPel    = _mm_setzero_si128 ();
    height    = 1 << lgHeight;
    divShift  = (32 == height) ? lgWidth + 1 : XIN_MAX(lgWidth, lgHeight);
    divOffset = 1 << (divShift - 1);
    offset128 = _mm_cvtsi32_si128 (divOffset);
    topBuf    = nBuf;
    lftBuf    = nBuf + 1 + multiRefIdx + 32*4;

    if (32 >= height)
    {
        topx16 = _mm_lddqu_si128 ((__m128i *)(topBuf + 1 + multiRefIdx));
        sumPel = _mm_sad_epu8 (topx16, allZero);
        topx16 = _mm_lddqu_si128 ((__m128i *)(topBuf + 1 + multiRefIdx + 16));
        sumPel = _mm_add_epi32 (sumPel, _mm_sad_epu8 (topx16, allZero));
    }

    if (32 <= height)
    {
        for (idx = 0; idx < height; idx += 16)
        {
            lftx16  = _mm_lddqu_si128 ((__m128i *)(lftBuf + 1 + multiRefIdx));
            sumPel  = _mm_add_epi32 (sumPel, _mm_sad_epu8 (lftx16, allZero));
            lftBuf += 16;
        }
    }

    sumPel = _mm_add_epi32 (sumPel,   _mm_shuffle_epi32 (sumPel, 0x4E));
    sumPel = _mm_add_epi32 (sumPel,   offset128);
    dcx16  = _mm_srli_epi32 (sumPel,  divShift);
    dcx16  = _mm_shuffle_epi8 (dcx16, allZero);

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        _mm_storeu_si128 ((__m128i *)dst,        dcx16);
        _mm_storeu_si128 ((__m128i *)(dst + 16), dcx16);

        dst += dstStride;
    }

}

void Xin266IntraPredDc64xH_SSSE3 (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    SINT32   multiRefIdx,
    UINT32   lgWidth,
    UINT32   lgHeight)
{
    UINT32  height;
    UINT32  idx;
    UINT32  rowIdx;
    SINT32  divOffset;
    SINT32  divShift;
    PIXEL   *topBuf;
    PIXEL   *lftBuf;
    __m128i allZero;
    __m128i lftx16;
    __m128i topx16;
    __m128i sumPel;
    __m128i offset128;
    __m128i dcx16;

    (void)lgWidth;
    allZero   = _mm_setzero_si128 ();
    sumPel    = _mm_setzero_si128 ();
    height    = 1 << lgHeight;
    divShift  = (64 == height) ? lgWidth + 1 : XIN_MAX(lgWidth, lgHeight);
    divOffset = 1 << (divShift - 1);
    offset128 = _mm_cvtsi32_si128 (divOffset);
    topBuf    = nBuf;
    lftBuf    = nBuf + 1 + multiRefIdx + 64*4;

    if (64 >= height)
    {
        topx16 = _mm_lddqu_si128 ((__m128i *)(topBuf + 1 + multiRefIdx));
        sumPel = _mm_sad_epu8 (topx16, allZero);
        topx16 = _mm_lddqu_si128 ((__m128i *)(topBuf + 1 + multiRefIdx + 16));
        sumPel = _mm_add_epi32 (sumPel, _mm_sad_epu8 (topx16, allZero));
        topx16 = _mm_lddqu_si128 ((__m128i *)(topBuf + 1 + multiRefIdx + 32));
        sumPel = _mm_add_epi32 (sumPel, _mm_sad_epu8 (topx16, allZero));
        topx16 = _mm_lddqu_si128 ((__m128i *)(topBuf + 1 + multiRefIdx + 48));
        sumPel = _mm_add_epi32 (sumPel, _mm_sad_epu8 (topx16, allZero));
    }

    if (64 <= height)
    {
        for (idx = 0; idx < height; idx += 16)
        {
            lftx16  = _mm_lddqu_si128 ((__m128i *)(lftBuf + 1 + multiRefIdx));
            sumPel  = _mm_add_epi32 (sumPel, _mm_sad_epu8 (lftx16, allZero));
            lftBuf += 16;
        }
    }

    sumPel = _mm_add_epi32 (sumPel,   _mm_shuffle_epi32 (sumPel, 0x4E));
    sumPel = _mm_add_epi32 (sumPel,   offset128);
    dcx16  = _mm_srli_epi32 (sumPel,  divShift);
    dcx16  = _mm_shuffle_epi8 (dcx16, allZero);

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        _mm_storeu_si128 ((__m128i *)dst,        dcx16);
        _mm_storeu_si128 ((__m128i *)(dst + 16), dcx16);
        _mm_storeu_si128 ((__m128i *)(dst + 32), dcx16);
        _mm_storeu_si128 ((__m128i *)(dst + 48), dcx16);

        dst += dstStride;
    }

}

