/***************************************************************************//**
 *
 * @file          h266_intra_pred_ver_sse4.c
 * @brief         h266 intra prediction subroutines (vertical, SSE4).
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
#include <emmintrin.h>
#include <tmmintrin.h>
#include <smmintrin.h>
#include "basic_macro.h"

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

void Xin266IntraPredVer4xH_SSE4 (
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
    UINT32   top32;
    PIXEL    *dstRow;

    width   = 1 << lgWidth;
    height  = 1 << lgHeight;
    topBuf  = nBuf;
    lftBuf  = nBuf + 1 + multiRefIdx + width*4;
    refMain = topBuf;

    // compensate for line offset in reference line buffers
    refMain += multiRefIdx;
    dstRow   = dst;
    top32    = *((UINT32 *)(refMain + 1));

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        *((UINT32 *)(dstRow + dstStride*0)) = top32;

        *((UINT32 *)(dstRow + dstStride*1)) = top32;

        dstRow += dstStride*2;
    }

}

void Xin266IntraPredVer8xH_SSE4 (
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
    __m128i  topx8;
    PIXEL    *dstRow;

    width   = 1 << lgWidth;
    height  = 1 << lgHeight;
    topBuf  = nBuf;
    lftBuf  = nBuf + 1 + multiRefIdx + width*4;
    refMain = topBuf;

    // compensate for line offset in reference line buffers
    refMain += multiRefIdx;
    dstRow   = dst;
    topx8    = _mm_loadl_epi64 ((__m128i *)(refMain + 1));

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        _mm_storel_epi64 ((__m128i *)(dstRow), topx8);

        _mm_storel_epi64 ((__m128i *)(dstRow + dstStride), topx8);

        dstRow += dstStride*2;
    }

}

void Xin266IntraPredVer16xH_SSE4 (
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
    __m128i  topx16;
    PIXEL    *dstRow;

    width   = 1 << lgWidth;
    height  = 1 << lgHeight;
    topBuf  = nBuf;
    lftBuf  = nBuf + 1 + multiRefIdx + width*4;
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

void Xin266IntraPredVer32xH_SSE4 (
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
    __m128i  top0x16;
    __m128i  top1x16;
    PIXEL    *dstRow;

    width   = 1 << lgWidth;
    height  = 1 << lgHeight;
    topBuf  = nBuf;
    lftBuf  = nBuf + 1 + multiRefIdx + width*4;
    refMain = topBuf;

    // compensate for line offset in reference line buffers
    refMain += multiRefIdx;
    dstRow   = dst;
    top0x16  = _mm_loadu_si128 ((__m128i *)(refMain + 1));
    top1x16  = _mm_loadu_si128 ((__m128i *)(refMain + 17));

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        _mm_storeu_si128 ((__m128i *)(dstRow),      top0x16);
        _mm_storeu_si128 ((__m128i *)(dstRow + 16), top1x16);

        _mm_storeu_si128 ((__m128i *)(dstRow + dstStride),      top0x16);
        _mm_storeu_si128 ((__m128i *)(dstRow + dstStride + 16), top1x16);

        dstRow += dstStride*2;
    }

}

void Xin266IntraPredVer64xH_SSE4 (
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
    __m128i  top0x16;
    __m128i  top1x16;
    __m128i  top2x16;
    __m128i  top3x16;
    PIXEL    *dstRow;

    width   = 1 << lgWidth;
    height  = 1 << lgHeight;
    topBuf  = nBuf;
    refMain = topBuf;

    // compensate for line offset in reference line buffers
    refMain += multiRefIdx;
    dstRow   = dst;
    top0x16  = _mm_loadu_si128 ((__m128i *)(refMain + 1));
    top1x16  = _mm_loadu_si128 ((__m128i *)(refMain + 17));
    top2x16  = _mm_loadu_si128 ((__m128i *)(refMain + 33));
    top3x16  = _mm_loadu_si128 ((__m128i *)(refMain + 49));

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        _mm_storeu_si128 ((__m128i *)(dstRow),      top0x16);
        _mm_storeu_si128 ((__m128i *)(dstRow + 16), top1x16);
        _mm_storeu_si128 ((__m128i *)(dstRow + 32), top2x16);
        _mm_storeu_si128 ((__m128i *)(dstRow + 48), top3x16);

        _mm_storeu_si128 ((__m128i *)(dstRow + dstStride),      top0x16);
        _mm_storeu_si128 ((__m128i *)(dstRow + dstStride + 16), top1x16);
        _mm_storeu_si128 ((__m128i *)(dstRow + dstStride + 32), top2x16);
        _mm_storeu_si128 ((__m128i *)(dstRow + dstStride + 48), top3x16);

        dstRow += dstStride*2;
    }

}

void Xin266ApplyPDPCVer4xH_SSE4 (
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
    __m128i wlx4;
    __m128i deltax4;
    __m128i const32x4;
    __m128i allZero;
    SINT32  delta;
    UINT32  pred32;
    __m128i predx4;

    width     = 1 << lgWidth;
    height    = 1 << lgHeight;
    topBuf    = nIntraBuf;
    lftBuf    = nIntraBuf + 1 + width*4;
    angular   = ((lgWidth + lgHeight) - 2) >> 2;
    width     = XIN_MIN (3 << angular, width);
    wlx4      = _mm_loadl_epi64 ((__m128i *)wlScale[angular]);
    allZero   = _mm_setzero_si128 ();
    const32x4 = _mm_set1_epi16 (32);

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        delta = lftBuf[1 + rowIdx] - topBuf[0];

        if (delta)
        {
            deltax4 = _mm_set1_epi16 ((SINT16)delta);
            deltax4 = _mm_mullo_epi16 (deltax4, wlx4);
            deltax4 = _mm_add_epi16 (deltax4, const32x4);
            deltax4 = _mm_srai_epi16 (deltax4, 6);

            pred32  = *((UINT32 *)(pred));
            predx4  = _mm_cvtsi32_si128 (pred32);
            predx4 = _mm_unpacklo_epi8 (predx4, allZero);
            predx4 = _mm_add_epi16 (predx4, deltax4);

            predx4 = _mm_packus_epi16 (predx4, predx4);
            pred32 = _mm_cvtsi128_si32 (predx4);

            *((UINT32 *)(pred)) = pred32;
        }

        pred += predStride;

    }

}

void Xin266ApplyPDPCVer8xH_SSE4 (
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
    __m128i wlx8;
    __m128i deltax8;
    __m128i const32x8;
    __m128i allZero;
    SINT32  delta;
    __m128i predx8;

    width     = 1 << lgWidth;
    height    = 1 << lgHeight;
    topBuf    = nIntraBuf;
    lftBuf    = nIntraBuf + 1 + width*4;
    angular   = ((lgWidth + lgHeight) - 2) >> 2;
    width     = XIN_MIN (3 << angular, width);
    wlx8      = _mm_loadu_si128 ((__m128i *)wlScale[angular]);
    allZero   = _mm_setzero_si128 ();
    const32x8 = _mm_set1_epi16 (32);

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        delta = lftBuf[1 + rowIdx] - topBuf[0];

        if (delta)
        {
            deltax8 = _mm_set1_epi16 ((SINT16)delta);
            deltax8 = _mm_mullo_epi16 (deltax8, wlx8);
            deltax8 = _mm_add_epi16 (deltax8, const32x8);
            deltax8 = _mm_srai_epi16 (deltax8, 6);

            predx8 = _mm_loadl_epi64 ((__m128i *)(pred));
            predx8 = _mm_unpacklo_epi8 (predx8, allZero);
            predx8 = _mm_add_epi16 (predx8, deltax8);

            predx8 = _mm_packus_epi16 (predx8, predx8);

            _mm_storel_epi64 ((__m128i *)(pred), predx8);

        }

        pred += predStride;

    }

}

void Xin266ApplyPDPCVer16xH_SSE4 (
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
    __m128i wl0x8, wl1x8;
    __m128i deltax8;
    __m128i dif0x8, dif1x8;
    __m128i const32x8;
    __m128i allZero;
    SINT32  delta;
    __m128i predx16;
    __m128i pred0x8, pred1x8;

    width     = 1 << lgWidth;
    height    = 1 << lgHeight;
    topBuf    = nIntraBuf;
    lftBuf    = nIntraBuf + 1 + width*4;
    angular   = ((lgWidth + lgHeight) - 2) >> 2;
    width     = XIN_MIN (3 << angular, width);
    wl0x8     = _mm_loadu_si128 ((__m128i *)wlScale[angular]);
    wl1x8     = _mm_loadu_si128 ((__m128i *)(wlScale[angular] + 8));
    allZero   = _mm_setzero_si128 ();
    const32x8 = _mm_set1_epi16 (32);

    if (width <= 8)
    {
        for (rowIdx = 0; rowIdx < height; rowIdx++)
        {
            delta = lftBuf[1 + rowIdx] - topBuf[0];

            if (delta)
            {
                deltax8 = _mm_set1_epi16 ((SINT16)delta);
                dif0x8  = _mm_mullo_epi16 (deltax8, wl0x8);
                dif0x8  = _mm_add_epi16 (dif0x8, const32x8);
                dif0x8  = _mm_srai_epi16 (dif0x8, 6);

                pred0x8 = _mm_loadl_epi64 ((__m128i *)(pred));
                pred0x8 = _mm_unpacklo_epi8 (pred0x8, allZero);
                pred0x8 = _mm_add_epi16 (pred0x8, dif0x8);

                pred0x8 = _mm_packus_epi16 (pred0x8, pred0x8);

                _mm_storel_epi64 ((__m128i *)(pred), pred0x8);

            }

            pred += predStride;

        }
    }
    else
    {
        for (rowIdx = 0; rowIdx < height; rowIdx++)
        {
            delta = lftBuf[1 + rowIdx] - topBuf[0];

            if (delta)
            {
                deltax8 = _mm_set1_epi16 ((SINT16)delta);
                dif0x8  = _mm_mullo_epi16 (deltax8, wl0x8);
                dif1x8  = _mm_mullo_epi16 (deltax8, wl1x8);
                dif0x8  = _mm_add_epi16 (dif0x8, const32x8);
                dif1x8  = _mm_add_epi16 (dif1x8, const32x8);
                dif0x8  = _mm_srai_epi16 (dif0x8, 6);
                dif1x8  = _mm_srai_epi16 (dif1x8, 6);

                predx16 = _mm_loadu_si128 ((__m128i *)(pred));
                pred0x8 = _mm_unpacklo_epi8 (predx16, allZero);
                pred1x8 = _mm_unpackhi_epi8 (predx16, allZero);

                pred0x8 = _mm_add_epi16 (pred0x8, dif0x8);
                pred1x8 = _mm_add_epi16 (pred1x8, dif1x8);
                predx16 = _mm_packus_epi16 (pred0x8, pred1x8);

                _mm_storeu_si128 ((__m128i *)(pred), predx16);

            }

            pred += predStride;

        }
        
    }

}

