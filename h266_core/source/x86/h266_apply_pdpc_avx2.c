/***************************************************************************//**
 *
 * @file          h266_apply_pdpc_avx2.c
 * @brief         h266 intra prediction subroutines (PDPC, AVX2).
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
#include "immintrin.h"
#include "assert.h"

#ifdef __linux__
#include "xin_avx2_linux_patch.h"
#endif

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

void Xin266ApplyPDPC16xH_AVX2 (
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
    __m256i wlx16;
    __m256i wtx16;
    __m256i lftx16;
    __m128i top128;
    __m256i topx16;
    __m128i pred128;
    __m256i predx16;
    __m256i diftx16;
    __m256i diflx16;
    __m256i diffx16;
    __m256i c32x16;
    __m128i predLo, predHi;

    width   = 1 << lgWidth;
    height  = 1 << lgHeight;
    scale   = ((lgWidth - 2 + lgHeight) >> 2);
    topBuf  = nIntraBuf + 1;
    lftBuf  = nIntraBuf + 2 + width*4;
    top128  = _mm_loadu_si128 ((__m128i *)topBuf);
    topx16  = _mm256_cvtepu8_epi16 (top128);
    wlx16   = _mm256_loadu_si256 ((__m256i *)wxScale[scale]);
    c32x16  = _mm256_set1_epi16 (32);
    height1 = XIN_MIN (12, height);

    for (rowIdx = 0; rowIdx < height1; rowIdx++)
    {
        wtx16  = _mm256_set1_epi16 (wxScale[scale][rowIdx]);
        lftx16 = _mm256_set1_epi16 (lftBuf[rowIdx]);

        pred128 = _mm_loadu_si128 ((__m128i *)pred);
        predx16 = _mm256_cvtepu8_epi16 (pred128);
        diftx16 = _mm256_sub_epi16 (topx16, predx16);
        diftx16 = _mm256_mullo_epi16 (diftx16, wtx16);
        diflx16 = _mm256_sub_epi16 (lftx16, predx16);
        diflx16 = _mm256_mullo_epi16 (diflx16, wlx16);

        diffx16 = _mm256_add_epi16 (diftx16, diflx16);
        diffx16 = _mm256_add_epi16 (diffx16, c32x16);
        diffx16 = _mm256_srai_epi16 (diffx16, 6);
        predx16 = _mm256_add_epi16 (predx16, diffx16);

        predLo  = _mm256_castsi256_si128 (predx16);
        predHi  = _mm256_extracti128_si256 (predx16, 1);
        pred128 = _mm_packus_epi16 (predLo, predHi);

        _mm_storeu_si128 ((__m128i *)(pred), pred128);

        pred += predStride;

    }

    for (rowIdx = height1; rowIdx < height; rowIdx++)
    {
        lftx16  = _mm256_set1_epi16 (lftBuf[rowIdx]);

        pred128 = _mm_loadu_si128 ((__m128i *)pred);
        predx16 = _mm256_cvtepu8_epi16 (pred128);
        diflx16 = _mm256_sub_epi16 (lftx16, predx16);
        diflx16 = _mm256_mullo_epi16 (diflx16, wlx16);
        diffx16 = _mm256_add_epi16 (diflx16, c32x16);
        diffx16 = _mm256_srai_epi16 (diffx16, 6);
        predx16 = _mm256_add_epi16 (predx16, diffx16);

        predLo  = _mm256_castsi256_si128 (predx16);
        predHi  = _mm256_extracti128_si256 (predx16, 1);
        pred128 = _mm_packus_epi16 (predLo, predHi);

        _mm_storeu_si128 ((__m128i *)(pred), pred128);

        pred += predStride;

    }

}

void Xin266ApplyPDPC32xH_AVX2 (
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
    __m256i wlx16;
    __m256i wtx16;
    __m256i lftx16;
    __m256i topx32;
    __m256i top0x16, top1x16;
    __m256i predx32;
    __m256i pred0x16, pred1x16;
    __m256i allZero;
    __m256i dift0x16, dift1x16;
    __m256i difl0x16, difl1x16;
    __m256i diff0x16, diff1x16;
    __m256i c32x16;
    __m128i predx16;
    __m256i pred256;
    __m128i predLo, predHi;

    width   = 1 << lgWidth;
    height  = 1 << lgHeight;
    scale   = ((lgWidth - 2 + lgHeight) >> 2);
    topBuf  = nIntraBuf + 1;
    lftBuf  = nIntraBuf + 2 + width*3;
    allZero = _mm256_setzero_si256 ();
    topx32  = _mm256_loadu_si256 ((__m256i *)topBuf);
    top0x16 = _mm256_unpacklo_epi8 (topx32, allZero);
    top1x16 = _mm256_unpackhi_epi8 (topx32, allZero);
    wlx16   = _mm256_loadu_si256 ((__m256i *)wxScale[scale]);
    c32x16  = _mm256_set1_epi16 (32);
    height1 = XIN_MIN (12, height);

    for (rowIdx = 0; rowIdx < height1; rowIdx++)
    {
        wtx16  = _mm256_set1_epi16 (rowIdx < 16 ? wxScale[scale][rowIdx] : 0);
        lftx16 = _mm256_set1_epi16 (lftBuf[rowIdx]);

        predx32  = _mm256_loadu_si256 ((__m256i *)pred);
        pred0x16 = _mm256_unpacklo_epi8 (predx32, allZero);
        pred1x16 = _mm256_unpackhi_epi8 (predx32, allZero);

        dift0x16 = _mm256_sub_epi16 (top0x16, pred0x16);
        dift1x16 = _mm256_sub_epi16 (top1x16, pred1x16);

        dift0x16 = _mm256_mullo_epi16 (dift0x16, wtx16);
        dift1x16 = _mm256_mullo_epi16 (dift1x16, wtx16);

        difl0x16 = _mm256_sub_epi16 (lftx16, pred0x16);
        difl1x16 = _mm256_sub_epi16 (lftx16, pred1x16);

        difl0x16 = _mm256_mullo_epi16 (difl0x16, wlx16);
        difl1x16 = _mm256_mullo_epi16 (difl1x16, wlx16);

        diff0x16 = _mm256_add_epi16 (dift0x16, difl0x16);
        diff0x16 = _mm256_add_epi16 (diff0x16, c32x16);
        diff0x16 = _mm256_srai_epi16 (diff0x16, 6);

        diff1x16 = _mm256_add_epi16 (dift1x16, difl1x16);
        diff1x16 = _mm256_add_epi16 (diff1x16, c32x16);
        diff1x16 = _mm256_srai_epi16 (diff1x16, 6);

        pred0x16 = _mm256_add_epi16 (pred0x16, diff0x16);
        pred1x16 = _mm256_add_epi16 (pred1x16, diff1x16);

        predx32 = _mm256_packus_epi16 (pred0x16, pred1x16);

        _mm256_storeu_si256 ((__m256i *)(pred), predx32);

        pred += predStride;

    }

    for (rowIdx = height1; rowIdx < height; rowIdx++)
    {
        lftx16  = _mm256_set1_epi16 (lftBuf[rowIdx]);

        predx16 = _mm_loadu_si128 ((__m128i *)pred);
        pred256 = _mm256_cvtepu8_epi16 (predx16);
        
        difl0x16 = _mm256_sub_epi16 (lftx16, pred256);
        difl0x16 = _mm256_mullo_epi16 (difl0x16, wlx16);
        diff0x16 = _mm256_add_epi16 (difl0x16, c32x16);
        diff0x16 = _mm256_srai_epi16 (diff0x16, 6);
        pred256  = _mm256_add_epi16 (pred256, diff0x16);

        predLo  = _mm256_castsi256_si128 (pred256);
        predHi  = _mm256_extracti128_si256 (pred256, 1);
        predx16 = _mm_packus_epi16 (predLo, predHi);

        _mm_storeu_si128 ((__m128i *)(pred), predx16);

        pred += predStride;

    }

}

void Xin266ApplyPDPC64xH_AVX2 (
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
    lftBuf  = nIntraBuf + 2 + width*3;
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

