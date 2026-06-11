/***************************************************************************//**
 *
 * @file          h266_apply_pdpc_ssse3.c
 * @brief         h266 intra prediction subroutines (PDPC, SSSE3).
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
#include "string.h"
#include "basic_macro.h"
#include "h266_intra_pred_context.h"
#include "xin_video_common.h"
#include "h26x_trans_context.h"
#include "h26x_common_data.h"
#include "h26x_block_transpose.h"
#include "tmmintrin.h"
#include "assert.h"

static const SINT16 wxScale[3][16] =
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

void Xin266ApplyPDPC4xH_SSSE3 (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight)
{
    UINT32  width;
    UINT32  height;
    UINT32  height1;
    UINT32  rowIdx;
    UINT32  scale;
    PIXEL   *topBuf;
    PIXEL   *lftBuf;
    UINT32  top32;
    UINT32  pred32;
    __m128i wlx4;
    __m128i wtx4;
    __m128i lftx4;
    __m128i topx4;
    __m128i predx4;
    __m128i allZero;
    __m128i diftx4;
    __m128i diflx4;
    __m128i diffx4;
    __m128i c32x4;

    width   = 1 << lgWidth;
    height  = 1 << lgHeight;
    scale   = ((lgWidth - 2 + lgHeight) >> 2);
    topBuf  = nIntraBuf + 1;
    lftBuf  = nIntraBuf + 2 + width*4;
    allZero = _mm_setzero_si128 ();
    top32   = *((UINT32 *)topBuf);
    topx4   = _mm_cvtsi32_si128 (top32);
    topx4   = _mm_unpacklo_epi8 (topx4, allZero);
    wlx4    = _mm_loadu_si128 ((__m128i *)wxScale[scale]);
    c32x4   = _mm_set1_epi16 (32);
    height1 = XIN_MIN (12, height);

    for (rowIdx = 0; rowIdx < height1; rowIdx++)
    {
        wtx4   = _mm_set1_epi16 (wxScale[scale][rowIdx]);
        lftx4  = _mm_set1_epi16 (lftBuf[rowIdx]);
        
        pred32 = *((UINT32 *)pred);
        predx4 = _mm_cvtsi32_si128 (pred32);
        predx4 = _mm_unpacklo_epi8 (predx4, allZero);

        diftx4 = _mm_sub_epi16 (topx4, predx4);
        diftx4 = _mm_mullo_epi16 (diftx4, wtx4);

        diflx4 = _mm_sub_epi16 (lftx4, predx4);
        diflx4 = _mm_mullo_epi16 (diflx4, wlx4);

        diffx4 = _mm_add_epi16 (diftx4, diflx4);
        diffx4 = _mm_add_epi16 (diffx4, c32x4);
        diffx4 = _mm_srai_epi16 (diffx4, 6);

        predx4 = _mm_add_epi16 (predx4, diffx4);
        predx4 = _mm_packus_epi16 (predx4, predx4);
        pred32 = _mm_cvtsi128_si32 (predx4);

        *((UINT32 *)pred) = pred32;

        pred += predStride;

    }

    for (rowIdx = height1; rowIdx < height; rowIdx++)
    {
        lftx4  = _mm_set1_epi16 (lftBuf[rowIdx]);

        pred32 = *((UINT32 *)pred);
        predx4 = _mm_cvtsi32_si128 (pred32);
        predx4 = _mm_unpacklo_epi8 (predx4, allZero);

        diflx4 = _mm_sub_epi16 (lftx4, predx4);
        diflx4 = _mm_mullo_epi16 (diflx4, wlx4);

        diffx4 = _mm_add_epi16 (diflx4, c32x4);
        diffx4 = _mm_srai_epi16 (diffx4, 6);

        predx4 = _mm_add_epi16 (predx4, diffx4);
        predx4 = _mm_packus_epi16 (predx4, predx4);
        pred32 = _mm_cvtsi128_si32 (predx4);

        *((UINT32 *)pred) = pred32;

        pred += predStride;

    }

}

void Xin266ApplyPDPC8xH_SSSE3 (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight)
{
    UINT32  width;
    UINT32  height;
    UINT32  height1;
    UINT32  rowIdx;
    UINT32  scale;
    PIXEL   *topBuf;
    PIXEL   *lftBuf;
    __m128i wlx8;
    __m128i wtx8;
    __m128i lftx8;
    __m128i topx8;
    __m128i predx8;
    __m128i allZero;
    __m128i diftx8;
    __m128i diflx8;
    __m128i diffx8;
    __m128i c32x8;

    width   = 1 << lgWidth;
    height  = 1 << lgHeight;
    scale   = ((lgWidth - 2 + lgHeight) >> 2);
    topBuf  = nIntraBuf + 1;
    lftBuf  = nIntraBuf + 2 + width*4;
    allZero = _mm_setzero_si128 ();
    topx8   = _mm_loadl_epi64 ((__m128i *)topBuf);
    topx8   = _mm_unpacklo_epi8 (topx8, allZero);
    wlx8    = _mm_loadu_si128 ((__m128i *)wxScale[scale]);
    c32x8   = _mm_set1_epi16 (32);
    height1 = XIN_MIN (12, height);

    for (rowIdx = 0; rowIdx < height1; rowIdx++)
    {
        wtx8   = _mm_set1_epi16 (wxScale[scale][rowIdx]);
        lftx8  = _mm_set1_epi16 (lftBuf[rowIdx]);

        predx8 = _mm_loadl_epi64 ((__m128i *)pred);
        predx8 = _mm_unpacklo_epi8 (predx8, allZero);

        diftx8 = _mm_sub_epi16 (topx8, predx8);
        diftx8 = _mm_mullo_epi16 (diftx8, wtx8);

        diflx8 = _mm_sub_epi16 (lftx8, predx8);
        diflx8 = _mm_mullo_epi16 (diflx8, wlx8);

        diffx8 = _mm_add_epi16 (diftx8, diflx8);
        diffx8 = _mm_add_epi16 (diffx8, c32x8);
        diffx8 = _mm_srai_epi16 (diffx8, 6);

        predx8 = _mm_add_epi16 (predx8, diffx8);

        predx8 = _mm_packus_epi16 (predx8, predx8);

        _mm_storel_epi64 ((__m128i *)(pred), predx8);

        pred += predStride;

    }

    for (rowIdx = height1; rowIdx < height; rowIdx++)
    {
        lftx8  = _mm_set1_epi16 (lftBuf[rowIdx]);

        predx8 = _mm_loadl_epi64 ((__m128i *)pred);
        predx8 = _mm_unpacklo_epi8 (predx8, allZero);

        diflx8 = _mm_sub_epi16 (lftx8, predx8);
        diflx8 = _mm_mullo_epi16 (diflx8, wlx8);

        diffx8 = _mm_add_epi16 (diflx8, c32x8);
        diffx8 = _mm_srai_epi16 (diffx8, 6);

        predx8 = _mm_add_epi16 (predx8, diffx8);

        predx8 = _mm_packus_epi16 (predx8, predx8);

        _mm_storel_epi64 ((__m128i *)(pred), predx8);

        pred += predStride;

    }

}

void Xin266ApplyPDPC16xH_SSSE3 (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight)
{
    UINT32  width;
    UINT32  height;
    UINT32  height1;
    UINT32  rowIdx;
    UINT32  scale;
    PIXEL   *topBuf;
    PIXEL   *lftBuf;
    __m128i wl0x8, wl1x8;
    __m128i wtx8;
    __m128i lftx8;
    __m128i topx16;
    __m128i top0x8, top1x8;
    __m128i predx16;
    __m128i pred0x8, pred1x8;
    __m128i allZero;
    __m128i dift0x8, dift1x8;
    __m128i difl0x8, difl1x8;
    __m128i diff0x8, diff1x8;
    __m128i c32x8;

    width   = 1 << lgWidth;
    height  = 1 << lgHeight;
    scale   = ((lgWidth - 2 + lgHeight) >> 2);
    topBuf  = nIntraBuf + 1;
    lftBuf  = nIntraBuf + 2 + width*4;
    allZero = _mm_setzero_si128 ();
    topx16  = _mm_loadu_si128 ((__m128i *)topBuf);
    top0x8  = _mm_unpacklo_epi8 (topx16, allZero);
    top1x8  = _mm_unpackhi_epi8 (topx16, allZero);
    wl0x8   = _mm_loadu_si128 ((__m128i *)wxScale[scale]);
    wl1x8   = _mm_loadu_si128 ((__m128i *)(wxScale[scale] + 8));
    c32x8   = _mm_set1_epi16 (32);
    height1 = XIN_MIN (12, height);
    
    for (rowIdx = 0; rowIdx < height1; rowIdx++)
    {
        wtx8   = _mm_set1_epi16 (wxScale[scale][rowIdx]);
        lftx8  = _mm_set1_epi16 (lftBuf[rowIdx]);

        predx16 = _mm_loadu_si128 ((__m128i *)pred);
        pred0x8 = _mm_unpacklo_epi8 (predx16, allZero);
        pred1x8 = _mm_unpackhi_epi8 (predx16, allZero);

        dift0x8 = _mm_sub_epi16 (top0x8, pred0x8);
        dift1x8 = _mm_sub_epi16 (top1x8, pred1x8);

        dift0x8 = _mm_mullo_epi16 (dift0x8, wtx8);
        dift1x8 = _mm_mullo_epi16 (dift1x8, wtx8);

        difl0x8 = _mm_sub_epi16 (lftx8, pred0x8);
        difl1x8 = _mm_sub_epi16 (lftx8, pred1x8);

        difl0x8 = _mm_mullo_epi16 (difl0x8, wl0x8);
        difl1x8 = _mm_mullo_epi16 (difl1x8, wl1x8);

        diff0x8 = _mm_add_epi16 (dift0x8, difl0x8);
        diff0x8 = _mm_add_epi16 (diff0x8, c32x8);
        diff0x8 = _mm_srai_epi16 (diff0x8, 6);

        diff1x8 = _mm_add_epi16 (dift1x8, difl1x8);
        diff1x8 = _mm_add_epi16 (diff1x8, c32x8);
        diff1x8 = _mm_srai_epi16 (diff1x8, 6);

        pred0x8 = _mm_add_epi16 (pred0x8, diff0x8);
        pred1x8 = _mm_add_epi16 (pred1x8, diff1x8);

        predx16 = _mm_packus_epi16 (pred0x8, pred1x8);

        _mm_storeu_si128 ((__m128i *)(pred), predx16);

        pred += predStride;

    }

    for (rowIdx = height1; rowIdx < height; rowIdx++)
    {
        lftx8  = _mm_set1_epi16 (lftBuf[rowIdx]);

        predx16 = _mm_loadu_si128 ((__m128i *)pred);
        pred0x8 = _mm_unpacklo_epi8 (predx16, allZero);
        pred1x8 = _mm_unpackhi_epi8 (predx16, allZero);

        difl0x8 = _mm_sub_epi16 (lftx8, pred0x8);
        difl1x8 = _mm_sub_epi16 (lftx8, pred1x8);

        difl0x8 = _mm_mullo_epi16 (difl0x8, wl0x8);
        difl1x8 = _mm_mullo_epi16 (difl1x8, wl1x8);

        diff0x8 = _mm_add_epi16 (difl0x8, c32x8);
        diff0x8 = _mm_srai_epi16 (diff0x8, 6);

        diff1x8 = _mm_add_epi16 (difl1x8, c32x8);
        diff1x8 = _mm_srai_epi16 (diff1x8, 6);

        pred0x8 = _mm_add_epi16 (pred0x8, diff0x8);
        pred1x8 = _mm_add_epi16 (pred1x8, diff1x8);

        predx16 = _mm_packus_epi16 (pred0x8, pred1x8);

        _mm_storeu_si128 ((__m128i *)(pred), predx16);

        pred += predStride;

    }

}

void Xin266ApplyPDPC32xH_SSSE3 (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight)
{
    UINT32  width;
    UINT32  height;
    UINT32  height1;
    UINT32  rowIdx;
    UINT32  scale;
    PIXEL   *topBuf;
    PIXEL   *lftBuf;
    __m128i wl0x8, wl1x8;
    __m128i wtx8;
    __m128i lftx8;
    __m128i topx16;
    __m128i top0x8, top1x8, top2x8, top3x8;
    __m128i predx16;
    __m128i pred0x8, pred1x8;
    __m128i allZero;
    __m128i dift0x8, dift1x8;
    __m128i difl0x8, difl1x8;
    __m128i diff0x8, diff1x8;
    __m128i c32x8;

    width   = 1 << lgWidth;
    height  = 1 << lgHeight;
    scale   = ((lgWidth - 2 + lgHeight) >> 2);
    topBuf  = nIntraBuf + 1;
    lftBuf  = nIntraBuf + 2 + width*4;
    allZero = _mm_setzero_si128 ();
    topx16  = _mm_loadu_si128 ((__m128i *)topBuf);
    top0x8  = _mm_unpacklo_epi8 (topx16, allZero);
    top1x8  = _mm_unpackhi_epi8 (topx16, allZero);
    topx16  = _mm_loadu_si128 ((__m128i *)(topBuf + 16));
    top2x8  = _mm_unpacklo_epi8 (topx16, allZero);
    top3x8  = _mm_unpackhi_epi8 (topx16, allZero);
    wl0x8   = _mm_loadu_si128 ((__m128i *)wxScale[scale]);
    wl1x8   = _mm_loadu_si128 ((__m128i *)(wxScale[scale] + 8));
    c32x8   = _mm_set1_epi16 (32);
    height1 = XIN_MIN (12, height);

    for (rowIdx = 0; rowIdx < height1; rowIdx++)
    {
        wtx8   = _mm_set1_epi16 (rowIdx < 16 ? wxScale[scale][rowIdx] : 0);
        lftx8  = _mm_set1_epi16 (lftBuf[rowIdx]);

        predx16 = _mm_loadu_si128 ((__m128i *)pred);
        pred0x8 = _mm_unpacklo_epi8 (predx16, allZero);
        pred1x8 = _mm_unpackhi_epi8 (predx16, allZero);

        dift0x8 = _mm_sub_epi16 (top0x8, pred0x8);
        dift1x8 = _mm_sub_epi16 (top1x8, pred1x8);

        dift0x8 = _mm_mullo_epi16 (dift0x8, wtx8);
        dift1x8 = _mm_mullo_epi16 (dift1x8, wtx8);

        difl0x8 = _mm_sub_epi16 (lftx8, pred0x8);
        difl1x8 = _mm_sub_epi16 (lftx8, pred1x8);

        difl0x8 = _mm_mullo_epi16 (difl0x8, wl0x8);
        difl1x8 = _mm_mullo_epi16 (difl1x8, wl1x8);

        diff0x8 = _mm_add_epi16 (dift0x8, difl0x8);
        diff0x8 = _mm_add_epi16 (diff0x8, c32x8);
        diff0x8 = _mm_srai_epi16 (diff0x8, 6);

        diff1x8 = _mm_add_epi16 (dift1x8, difl1x8);
        diff1x8 = _mm_add_epi16 (diff1x8, c32x8);
        diff1x8 = _mm_srai_epi16 (diff1x8, 6);

        pred0x8 = _mm_add_epi16 (pred0x8, diff0x8);
        pred1x8 = _mm_add_epi16 (pred1x8, diff1x8);

        predx16 = _mm_packus_epi16 (pred0x8, pred1x8);

        _mm_storeu_si128 ((__m128i *)(pred), predx16);

        predx16 = _mm_loadu_si128 ((__m128i *)(pred + 16));
        pred0x8 = _mm_unpacklo_epi8 (predx16, allZero);
        pred1x8 = _mm_unpackhi_epi8 (predx16, allZero);

        dift0x8 = _mm_sub_epi16 (top2x8, pred0x8);
        dift1x8 = _mm_sub_epi16 (top3x8, pred1x8);

        dift0x8 = _mm_mullo_epi16 (dift0x8, wtx8);
        dift1x8 = _mm_mullo_epi16 (dift1x8, wtx8);

        diff0x8 = _mm_add_epi16 (dift0x8, c32x8);
        diff0x8 = _mm_srai_epi16 (diff0x8, 6);

        diff1x8 = _mm_add_epi16 (dift1x8, c32x8);
        diff1x8 = _mm_srai_epi16 (diff1x8, 6);

        pred0x8 = _mm_add_epi16 (pred0x8, diff0x8);
        pred1x8 = _mm_add_epi16 (pred1x8, diff1x8);

        predx16 = _mm_packus_epi16 (pred0x8, pred1x8);

        _mm_storeu_si128 ((__m128i *)(pred + 16), predx16);

        pred += predStride;

    }

    for (rowIdx = height1; rowIdx < height; rowIdx++)
    {
        lftx8  = _mm_set1_epi16 (lftBuf[rowIdx]);

        predx16 = _mm_loadu_si128 ((__m128i *)pred);
        pred0x8 = _mm_unpacklo_epi8 (predx16, allZero);
        pred1x8 = _mm_unpackhi_epi8 (predx16, allZero);

        difl0x8 = _mm_sub_epi16 (lftx8, pred0x8);
        difl1x8 = _mm_sub_epi16 (lftx8, pred1x8);

        difl0x8 = _mm_mullo_epi16 (difl0x8, wl0x8);
        difl1x8 = _mm_mullo_epi16 (difl1x8, wl1x8);

        diff0x8 = _mm_add_epi16 (difl0x8, c32x8);
        diff0x8 = _mm_srai_epi16 (diff0x8, 6);

        diff1x8 = _mm_add_epi16 (difl1x8, c32x8);
        diff1x8 = _mm_srai_epi16 (diff1x8, 6);

        pred0x8 = _mm_add_epi16 (pred0x8, diff0x8);
        pred1x8 = _mm_add_epi16 (pred1x8, diff1x8);

        predx16 = _mm_packus_epi16 (pred0x8, pred1x8);

        _mm_storeu_si128 ((__m128i *)(pred), predx16);

        pred += predStride;

    }

}

void Xin266ApplyPDPC64xH_SSSE3 (
    PIXEL       *pred,
    intptr_t    predStride,
    PIXEL       *nIntraBuf,
    UINT32      lgWidth,
    UINT32      lgHeight)
{
    UINT32  width;
    UINT32  height;
    UINT32  height1;
    UINT32  rowIdx;
    UINT32  scale;
    PIXEL   *topBuf;
    PIXEL   *lftBuf;
    __m128i wl0x8, wl1x8;
    __m128i wtx8;
    __m128i lftx8;
    __m128i topx16;
    __m128i top0x8, top1x8, top2x8, top3x8;
    __m128i top4x8, top5x8, top6x8, top7x8;
    __m128i predx16;
    __m128i pred0x8, pred1x8;
    __m128i allZero;
    __m128i dift0x8, dift1x8;
    __m128i difl0x8, difl1x8;
    __m128i diff0x8, diff1x8;
    __m128i c32x8;

    width   = 1 << lgWidth;
    height  = 1 << lgHeight;
    scale   = ((lgWidth - 2 + lgHeight) >> 2);
    topBuf  = nIntraBuf + 1;
    lftBuf  = nIntraBuf + 2 + width*4;
    allZero = _mm_setzero_si128 ();
    topx16  = _mm_loadu_si128 ((__m128i *)topBuf);
    top0x8  = _mm_unpacklo_epi8 (topx16, allZero);
    top1x8  = _mm_unpackhi_epi8 (topx16, allZero);
    topx16  = _mm_loadu_si128 ((__m128i *)(topBuf + 16));
    top2x8  = _mm_unpacklo_epi8 (topx16, allZero);
    top3x8  = _mm_unpackhi_epi8 (topx16, allZero);
    topx16  = _mm_loadu_si128 ((__m128i *)(topBuf + 32));
    top4x8  = _mm_unpacklo_epi8 (topx16, allZero);
    top5x8  = _mm_unpackhi_epi8 (topx16, allZero);
    topx16  = _mm_loadu_si128 ((__m128i *)(topBuf + 48));
    top6x8  = _mm_unpacklo_epi8 (topx16, allZero);
    top7x8  = _mm_unpackhi_epi8 (topx16, allZero);
    wl0x8   = _mm_loadu_si128 ((__m128i *)wxScale[scale]);
    wl1x8   = _mm_loadu_si128 ((__m128i *)(wxScale[scale] + 8));
    c32x8   = _mm_set1_epi16 (32);
    height1 = XIN_MIN (12, height);

    for (rowIdx = 0; rowIdx < height1; rowIdx++)
    {
        wtx8   = _mm_set1_epi16 (rowIdx < 16 ? wxScale[scale][rowIdx] : 0);
        lftx8  = _mm_set1_epi16 (lftBuf[rowIdx]);

        predx16 = _mm_loadu_si128 ((__m128i *)pred);
        pred0x8 = _mm_unpacklo_epi8 (predx16, allZero);
        pred1x8 = _mm_unpackhi_epi8 (predx16, allZero);

        dift0x8 = _mm_sub_epi16 (top0x8, pred0x8);
        dift1x8 = _mm_sub_epi16 (top1x8, pred1x8);

        dift0x8 = _mm_mullo_epi16 (dift0x8, wtx8);
        dift1x8 = _mm_mullo_epi16 (dift1x8, wtx8);

        difl0x8 = _mm_sub_epi16 (lftx8, pred0x8);
        difl1x8 = _mm_sub_epi16 (lftx8, pred1x8);

        difl0x8 = _mm_mullo_epi16 (difl0x8, wl0x8);
        difl1x8 = _mm_mullo_epi16 (difl1x8, wl1x8);

        diff0x8 = _mm_add_epi16 (dift0x8, difl0x8);
        diff0x8 = _mm_add_epi16 (diff0x8, c32x8);
        diff0x8 = _mm_srai_epi16 (diff0x8, 6);

        diff1x8 = _mm_add_epi16 (dift1x8, difl1x8);
        diff1x8 = _mm_add_epi16 (diff1x8, c32x8);
        diff1x8 = _mm_srai_epi16 (diff1x8, 6);

        pred0x8 = _mm_add_epi16 (pred0x8, diff0x8);
        pred1x8 = _mm_add_epi16 (pred1x8, diff1x8);

        predx16 = _mm_packus_epi16 (pred0x8, pred1x8);

        _mm_storeu_si128 ((__m128i *)(pred), predx16);

        predx16 = _mm_loadu_si128 ((__m128i *)(pred + 16));
        pred0x8 = _mm_unpacklo_epi8 (predx16, allZero);
        pred1x8 = _mm_unpackhi_epi8 (predx16, allZero);

        dift0x8 = _mm_sub_epi16 (top2x8, pred0x8);
        dift1x8 = _mm_sub_epi16 (top3x8, pred1x8);

        dift0x8 = _mm_mullo_epi16 (dift0x8, wtx8);
        dift1x8 = _mm_mullo_epi16 (dift1x8, wtx8);

        diff0x8 = _mm_add_epi16 (dift0x8, c32x8);
        diff0x8 = _mm_srai_epi16 (diff0x8, 6);

        diff1x8 = _mm_add_epi16 (dift1x8, c32x8);
        diff1x8 = _mm_srai_epi16 (diff1x8, 6);

        pred0x8 = _mm_add_epi16 (pred0x8, diff0x8);
        pred1x8 = _mm_add_epi16 (pred1x8, diff1x8);

        predx16 = _mm_packus_epi16 (pred0x8, pred1x8);

        _mm_storeu_si128 ((__m128i *)(pred + 16), predx16);

        predx16 = _mm_loadu_si128 ((__m128i *)(pred + 32));
        pred0x8 = _mm_unpacklo_epi8 (predx16, allZero);
        pred1x8 = _mm_unpackhi_epi8 (predx16, allZero);

        dift0x8 = _mm_sub_epi16 (top4x8, pred0x8);
        dift1x8 = _mm_sub_epi16 (top5x8, pred1x8);

        dift0x8 = _mm_mullo_epi16 (dift0x8, wtx8);
        dift1x8 = _mm_mullo_epi16 (dift1x8, wtx8);

        diff0x8 = _mm_add_epi16 (dift0x8, c32x8);
        diff0x8 = _mm_srai_epi16 (diff0x8, 6);

        diff1x8 = _mm_add_epi16 (dift1x8, c32x8);
        diff1x8 = _mm_srai_epi16 (diff1x8, 6);

        pred0x8 = _mm_add_epi16 (pred0x8, diff0x8);
        pred1x8 = _mm_add_epi16 (pred1x8, diff1x8);

        predx16 = _mm_packus_epi16 (pred0x8, pred1x8);

        _mm_storeu_si128 ((__m128i *)(pred + 32), predx16);

        predx16 = _mm_loadu_si128 ((__m128i *)(pred + 48));
        pred0x8 = _mm_unpacklo_epi8 (predx16, allZero);
        pred1x8 = _mm_unpackhi_epi8 (predx16, allZero);

        dift0x8 = _mm_sub_epi16 (top6x8, pred0x8);
        dift1x8 = _mm_sub_epi16 (top7x8, pred1x8);

        dift0x8 = _mm_mullo_epi16 (dift0x8, wtx8);
        dift1x8 = _mm_mullo_epi16 (dift1x8, wtx8);

        diff0x8 = _mm_add_epi16 (dift0x8, c32x8);
        diff0x8 = _mm_srai_epi16 (diff0x8, 6);

        diff1x8 = _mm_add_epi16 (dift1x8, c32x8);
        diff1x8 = _mm_srai_epi16 (diff1x8, 6);

        pred0x8 = _mm_add_epi16 (pred0x8, diff0x8);
        pred1x8 = _mm_add_epi16 (pred1x8, diff1x8);

        predx16 = _mm_packus_epi16 (pred0x8, pred1x8);

        _mm_storeu_si128 ((__m128i *)(pred + 48), predx16);

        pred += predStride;

    }

    for (rowIdx = height1; rowIdx < height; rowIdx++)
    {
        lftx8  = _mm_set1_epi16 (lftBuf[rowIdx]);

        predx16 = _mm_loadu_si128 ((__m128i *)pred);
        pred0x8 = _mm_unpacklo_epi8 (predx16, allZero);
        pred1x8 = _mm_unpackhi_epi8 (predx16, allZero);

        difl0x8 = _mm_sub_epi16 (lftx8, pred0x8);
        difl1x8 = _mm_sub_epi16 (lftx8, pred1x8);

        difl0x8 = _mm_mullo_epi16 (difl0x8, wl0x8);
        difl1x8 = _mm_mullo_epi16 (difl1x8, wl1x8);

        diff0x8 = _mm_add_epi16 (difl0x8, c32x8);
        diff0x8 = _mm_srai_epi16 (diff0x8, 6);

        diff1x8 = _mm_add_epi16 (difl1x8, c32x8);
        diff1x8 = _mm_srai_epi16 (diff1x8, 6);

        pred0x8 = _mm_add_epi16 (pred0x8, diff0x8);
        pred1x8 = _mm_add_epi16 (pred1x8, diff1x8);

        predx16 = _mm_packus_epi16 (pred0x8, pred1x8);

        _mm_storeu_si128 ((__m128i *)(pred), predx16);

        pred += predStride;

    }

}



