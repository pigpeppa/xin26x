/***************************************************************************//**
 *
 * @file          h266_intra_pred_hor_ssse3.c
 * @brief         h266 intra prediction subroutines (horizontal, SSSE3).
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

void Xin266IntraPredHor4xH_SSSE3 (
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
    PIXEL    *lftBuf;
    PIXEL    *refMain;
    UINT32   lft32;
    PIXEL    *dstRow;

    width   = 1 << lgWidth;
    height  = 1 << lgHeight;
    lftBuf  = nBuf + 1 + multiRefIdx + width*4;
    refMain = lftBuf;

    // compensate for line offset in reference line buffers
    refMain += multiRefIdx;
    dstRow   = dst;

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        lft32 = refMain[1 + rowIdx]*0x01010101;
        *((UINT32 *)(dstRow + dstStride*0)) = lft32;

        lft32 = refMain[2 + rowIdx]*0x01010101;
        *((UINT32 *)(dstRow + dstStride*1)) = lft32;

        dstRow += dstStride*2;
    }

}

void Xin266IntraPredHor8xH_SSSE3 (
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
    PIXEL    *lftBuf;
    PIXEL    *topBuf;
    PIXEL    *refMain;
    PIXEL    *refSide;
    __m128i  lftx8;
    PIXEL    *dstRow;

    width   = 1 << lgWidth;
    height  = 1 << lgHeight;
    topBuf  = nBuf;
    lftBuf  = nBuf + 1 + multiRefIdx + width*4;
    refMain = lftBuf;
    refSide = topBuf;

    // compensate for line offset in reference line buffers
    refMain += multiRefIdx;
    dstRow   = dst;

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        lftx8 = _mm_set1_epi8 (refMain[1 + rowIdx]);

        _mm_storel_epi64 ((__m128i *)(dstRow), lftx8);

        lftx8 = _mm_set1_epi8(refMain[2 + rowIdx]);

        _mm_storel_epi64 ((__m128i *)(dstRow + dstStride), lftx8);

        dstRow += dstStride*2;
    }

}

void Xin266IntraPredHor16xH_SSSE3 (
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
    PIXEL    *lftBuf;
    PIXEL    *topBuf;
    PIXEL    *refMain;
    __m128i  lftx16;
    PIXEL    *dstRow;

    width   = 1 << lgWidth;
    height  = 1 << lgHeight;
    topBuf  = nBuf;
    lftBuf  = nBuf + 1 + multiRefIdx + width*4;
    refMain = lftBuf;

    // compensate for line offset in reference line buffers
    refMain += multiRefIdx;
    dstRow   = dst;

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        lftx16 = _mm_set1_epi8 (refMain[1 + rowIdx]);

        _mm_storeu_si128 ((__m128i *)(dstRow), lftx16);

        lftx16 = _mm_set1_epi8 (refMain[2 + rowIdx]);

        _mm_storeu_si128 ((__m128i *)(dstRow + dstStride), lftx16);

        dstRow += dstStride*2;
    }

}

void Xin266IntraPredHor32xH_SSSE3 (
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
    PIXEL    *lftBuf;
    PIXEL    *topBuf;
    PIXEL    *refMain;
    __m128i  lft0x16, lft1x16;
    PIXEL    *dstRow;

    width   = 1 << lgWidth;
    height  = 1 << lgHeight;
    topBuf  = nBuf;
    lftBuf  = nBuf + 1 + multiRefIdx + width*4;
    refMain = lftBuf;

    // compensate for line offset in reference line buffers
    refMain += multiRefIdx;
    dstRow   = dst;

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {

        lft0x16 = _mm_set1_epi8 (refMain[1 + rowIdx]);

        _mm_storeu_si128 ((__m128i *)(dstRow),      lft0x16);
        _mm_storeu_si128 ((__m128i *)(dstRow + 16), lft0x16);

        lft1x16 = _mm_set1_epi8 (refMain[2 + rowIdx]);

        _mm_storeu_si128 ((__m128i *)(dstRow + dstStride),      lft1x16);
        _mm_storeu_si128 ((__m128i *)(dstRow + dstStride + 16), lft1x16);

        dstRow += dstStride*2;
    }

}

void Xin266IntraPredHor64xH_SSSE3 (
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
    PIXEL    *lftBuf;
    PIXEL    *topBuf;
    PIXEL    *refMain;
    __m128i  lftx16;
    PIXEL    *dstRow;

    width   = 1 << lgWidth;
    height  = 1 << lgHeight;
    topBuf  = nBuf;
    lftBuf  = nBuf + 1 + multiRefIdx + width*4;
    refMain = lftBuf;

    // compensate for line offset in reference line buffers
    refMain += multiRefIdx;
    dstRow   = dst;

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        lftx16 = _mm_set1_epi8 (refMain[1 + rowIdx]);

        _mm_storeu_si128 ((__m128i *)(dstRow),      lftx16);
        _mm_storeu_si128 ((__m128i *)(dstRow + 16), lftx16);
        _mm_storeu_si128 ((__m128i *)(dstRow + 32), lftx16);
        _mm_storeu_si128 ((__m128i *)(dstRow + 48), lftx16);

        lftx16 = _mm_set1_epi8 (refMain[2 + rowIdx]);

        _mm_storeu_si128 ((__m128i *)(dstRow + dstStride),      lftx16);
        _mm_storeu_si128 ((__m128i *)(dstRow + dstStride + 16), lftx16);
        _mm_storeu_si128 ((__m128i *)(dstRow + dstStride + 32), lftx16);
        _mm_storeu_si128 ((__m128i *)(dstRow + dstStride + 48), lftx16);

        dstRow += dstStride*2;
    }

}

void Xin266ApplyPDPCHor4xH_SSSE3 (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight)
{
    SINT32  angular;
    SINT32  width;
    SINT32  height;
    SINT32  rowIdx;
    PIXEL   *topBuf;
    PIXEL   *lftBuf;
    UINT32  top32;
    __m128i tlx4;
    __m128i topx4;
    __m128i allZero;
    __m128i orgDifx4;
    SINT32  shift;
    __m128i difx4;
    UINT32  pred32;
    __m128i predx4;
    __m128i const32x4;

    width     = 1 << lgWidth;
    height    = 1 << lgHeight;
    topBuf    = nIntraBuf;
    lftBuf    = nIntraBuf + 1 + width*4;
    allZero   = _mm_setzero_si128 ();
    angular   = ((lgWidth + lgHeight) - 2) >> 2;
    height    = XIN_MIN (3 << angular, height);
    tlx4      = _mm_set1_epi16 (lftBuf[0]);
    top32     = *((UINT32 *)(topBuf + 1));
    topx4     = _mm_cvtsi32_si128 (top32);
    topx4     = _mm_unpacklo_epi8 (topx4, allZero);
    orgDifx4  = _mm_sub_epi16 (topx4, tlx4);
    const32x4 = _mm_set1_epi16 (32);

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        shift = 5 - (2*rowIdx>>angular);
        difx4 = _mm_slli_epi16 (orgDifx4, shift);
        difx4 = _mm_add_epi16 (difx4, const32x4);
        difx4 = _mm_srai_epi16 (difx4, 6);

        pred32 = *((UINT32 *)pred);
        predx4 = _mm_cvtsi32_si128 (pred32);
        predx4 = _mm_unpacklo_epi8 (predx4, allZero);
        predx4 = _mm_add_epi16 (difx4, predx4);

        predx4 = _mm_packus_epi16 (predx4, predx4);
        pred32 = _mm_cvtsi128_si32 (predx4);

        *((UINT32 *)pred) = pred32;

        pred += predStride;

    }

}

void Xin266ApplyPDPCHor8xH_SSSE3 (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight)
{
    SINT32  angular;
    SINT32  width;
    SINT32  height;
    SINT32  rowIdx;
    PIXEL   *topBuf;
    PIXEL   *lftBuf;
    __m128i tlx8;
    __m128i topx8;
    __m128i allZero;
    __m128i orgDifx8;
    SINT32  shift;
    __m128i difx8;
    __m128i predx8;
    __m128i const32x8;

    width     = 1 << lgWidth;
    height    = 1 << lgHeight;
    topBuf    = nIntraBuf;
    lftBuf    = nIntraBuf + 1 + width*4;
    allZero   = _mm_setzero_si128 ();
    angular   = ((lgWidth + lgHeight) - 2) >> 2;
    height    = XIN_MIN (3 << angular, height);
    tlx8      = _mm_set1_epi16 (lftBuf[0]);
    topx8     = _mm_loadl_epi64 ((__m128i *)(topBuf + 1));
    topx8     = _mm_unpacklo_epi8 (topx8, allZero);
    orgDifx8  = _mm_sub_epi16 (topx8, tlx8);
    const32x8 = _mm_set1_epi16 (32);

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        shift = 5 - (2*rowIdx>>angular);
        difx8 = _mm_slli_epi16 (orgDifx8, shift);
        difx8 = _mm_add_epi16 (difx8, const32x8);
        difx8 = _mm_srai_epi16 (difx8, 6);

        predx8 = _mm_loadl_epi64 ((__m128i *)pred);
        predx8 = _mm_unpacklo_epi8 (predx8, allZero);
        predx8 = _mm_add_epi16 (difx8, predx8);

        predx8 = _mm_packus_epi16 (predx8, predx8);

        _mm_storel_epi64 ((__m128i *)(pred), predx8);

        pred += predStride;
    }

}

void Xin266ApplyPDPCHor16xH_SSSE3 (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight)
{
    SINT32  angular;
    SINT32  width;
    SINT32  height;
    SINT32  rowIdx;
    PIXEL   *topBuf;
    PIXEL   *lftBuf;
    __m128i tlx8;
    __m128i topx16;
    __m128i top0x8, top1x8;
    __m128i allZero;
    __m128i orgDif0x8, orgDif1x8;
    SINT32  shift;
    __m128i dif0x8, dif1x8;
    __m128i pred0x8, pred1x8;
    __m128i predx16;
    __m128i const32x8;

    width     = 1 << lgWidth;
    height    = 1 << lgHeight;
    topBuf    = nIntraBuf;
    lftBuf    = nIntraBuf + 1 + width*4;
    allZero   = _mm_setzero_si128 ();
    angular   = ((lgWidth + lgHeight) - 2) >> 2;
    height    = XIN_MIN (3 << angular, height);
    tlx8      = _mm_set1_epi16 (lftBuf[0]);
    topx16    = _mm_loadu_si128 ((__m128i *)(topBuf + 1));
    top0x8    = _mm_unpacklo_epi8 (topx16, allZero);
    top1x8    = _mm_unpackhi_epi8 (topx16, allZero);
    orgDif0x8 = _mm_sub_epi16 (top0x8, tlx8);
    orgDif1x8 = _mm_sub_epi16 (top1x8, tlx8);
    const32x8 = _mm_set1_epi16 (32);

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        shift   = 5 - (2*rowIdx>>angular);
        dif0x8  = _mm_slli_epi16 (orgDif0x8, shift);
        dif1x8  = _mm_slli_epi16 (orgDif1x8, shift);
        dif0x8  = _mm_add_epi16 (dif0x8, const32x8);
        dif1x8  = _mm_add_epi16 (dif1x8, const32x8);
        dif0x8  = _mm_srai_epi16 (dif0x8, 6);
        dif1x8  = _mm_srai_epi16 (dif1x8, 6);

        predx16 = _mm_loadu_si128 ((__m128i *)pred);
        pred0x8 = _mm_unpacklo_epi8 (predx16, allZero);
        pred1x8 = _mm_unpackhi_epi8 (predx16, allZero);
        pred0x8 = _mm_add_epi16 (dif0x8, pred0x8);
        pred1x8 = _mm_add_epi16 (dif1x8, pred1x8);

        predx16 = _mm_packus_epi16 (pred0x8, pred1x8);

        _mm_storeu_si128 ((__m128i *)(pred), predx16);

        pred += predStride;
        
    }

}

