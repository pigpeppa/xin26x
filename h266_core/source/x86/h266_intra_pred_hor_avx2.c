/***************************************************************************//**
 *
 * @file          h266_intra_pred_hor_avx2.c
 * @brief         h266 intra prediction subroutines (horizontal, AVX2).
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
#include "immintrin.h"
#include "basic_macro.h"

#ifdef __linux__
#include "xin_avx2_linux_patch.h"
#endif

void Xin266IntraPredHor16xH_AVX2 (
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
        _mm_storeu_si128 ((__m128i *)(dstRow),             _mm_set1_epi8 (refMain[1 + rowIdx]));
        _mm_storeu_si128 ((__m128i *)(dstRow + dstStride), _mm_set1_epi8 (refMain[2 + rowIdx]));

        dstRow += dstStride*2;
    }

}

void Xin266IntraPredHor32xH_AVX2 (
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
    __m256i  lftx32;
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
        lftx32 = _mm256_set1_epi8 (refMain[1 + rowIdx]);
        _mm256_storeu_si256 ((__m256i *)(dstRow),             lftx32);

        lftx32 = _mm256_set1_epi8 (refMain[2 + rowIdx]);
        _mm256_storeu_si256 ((__m256i *)(dstRow + dstStride), lftx32);

        dstRow += dstStride*2;
    }

}

void Xin266IntraPredHor64xH_AVX2 (
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
    __m256i  lftx32;
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
        lftx32 = _mm256_set1_epi8 (refMain[1 + rowIdx]);
        _mm256_storeu_si256 ((__m256i *)(dstRow),      lftx32);
        _mm256_storeu_si256 ((__m256i *)(dstRow + 32), lftx32);

        lftx32 = _mm256_set1_epi8 (refMain[2 + rowIdx]);
        _mm256_storeu_si256 ((__m256i *)(dstRow + dstStride),      lftx32);
        _mm256_storeu_si256 ((__m256i *)(dstRow + dstStride + 32), lftx32);

        dstRow += dstStride*2;
    }

}

void Xin266ApplyPDPCHor32xH_AVX2 (
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
    __m256i topx32;
    __m256i top0x16, top1x16;
    __m256i allZero;
    __m256i orgDif0x16, orgDif1x16;
    SINT32  shift;
    __m256i dif0x16, dif1x16;
    __m256i pred0x16, pred1x16;
    __m256i predx32;
    __m256i const32x16;
    __m256i tlx16;

    width      = 1 << lgWidth;
    height     = 1 << lgHeight;
    topBuf     = nIntraBuf;
    lftBuf     = nIntraBuf + 1 + width*4;
    allZero    = _mm256_setzero_si256 ();
    angular    = ((lgWidth + lgHeight) - 2) >> 2;
    height     = XIN_MIN (3 << angular, height);
    tlx16      = _mm256_set1_epi16 (lftBuf[0]);
    topx32     = _mm256_loadu_si256 ((__m256i *)(topBuf + 1));
    top0x16    = _mm256_unpacklo_epi8 (topx32, allZero);
    top1x16    = _mm256_unpackhi_epi8 (topx32, allZero);
    orgDif0x16 = _mm256_sub_epi16 (top0x16, tlx16);
    orgDif1x16 = _mm256_sub_epi16 (top1x16, tlx16);
    const32x16 = _mm256_set1_epi16 (32);

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        shift   = 5 - (2*rowIdx>>angular);
        dif0x16 = _mm256_slli_epi16 (orgDif0x16, shift);
        dif1x16 = _mm256_slli_epi16 (orgDif1x16, shift);
        dif0x16 = _mm256_add_epi16 (dif0x16, const32x16);
        dif1x16 = _mm256_add_epi16 (dif1x16, const32x16);
        dif0x16 = _mm256_srai_epi16 (dif0x16, 6);
        dif1x16 = _mm256_srai_epi16 (dif1x16, 6);

        predx32  = _mm256_loadu_si256 ((__m256i *)(pred));
        pred0x16 = _mm256_unpacklo_epi8 (predx32, allZero);
        pred1x16 = _mm256_unpackhi_epi8 (predx32, allZero);
        pred0x16 = _mm256_add_epi16 (dif0x16, pred0x16);
        pred1x16 = _mm256_add_epi16 (dif1x16, pred1x16);

        predx32 = _mm256_packus_epi16 (pred0x16, pred1x16);

        _mm256_storeu_si256 ((__m256i *)(pred), predx32);

        pred += predStride;

    }

}

void Xin266ApplyPDPCHor64xH_AVX2 (
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
    __m256i top0x32, top1x32;
    __m256i top0x16, top1x16, top2x16, top3x16;
    __m256i allZero;
    __m256i orgDif0x16, orgDif1x16, orgDif2x16, orgDif3x16;
    SINT32  shift;
    __m256i dif0x16, dif1x16, dif2x16, dif3x16;
    __m256i pred0x16, pred1x16, pred2x16, pred3x16;
    __m256i pred0x32, pred1x32;
    __m256i const32x16;
    __m256i tlx16;

    width      = 1 << lgWidth;
    height     = 1 << lgHeight;
    topBuf     = nIntraBuf;
    lftBuf     = nIntraBuf + 1 + width*4;
    allZero    = _mm256_setzero_si256 ();
    angular    = ((lgWidth + lgHeight) - 2) >> 2;
    height     = XIN_MIN (3 << angular, height);
    tlx16      = _mm256_set1_epi16 (lftBuf[0]);
    top0x32    = _mm256_loadu_si256 ((__m256i *)(topBuf + 1));
    top0x16    = _mm256_unpacklo_epi8 (top0x32, allZero);
    top1x16    = _mm256_unpackhi_epi8 (top0x32, allZero);
    top1x32    = _mm256_loadu_si256 ((__m256i *)(topBuf + 1 + 32));
    top2x16    = _mm256_unpacklo_epi8 (top1x32, allZero);
    top3x16    = _mm256_unpackhi_epi8 (top1x32, allZero);
    orgDif0x16 = _mm256_sub_epi16 (top0x16, tlx16);
    orgDif1x16 = _mm256_sub_epi16 (top1x16, tlx16);
    orgDif2x16 = _mm256_sub_epi16 (top2x16, tlx16);
    orgDif3x16 = _mm256_sub_epi16 (top3x16, tlx16);
    const32x16 = _mm256_set1_epi16 (32);

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        shift   = 5 - (2*rowIdx>>angular);
        dif0x16 = _mm256_slli_epi16 (orgDif0x16, shift);
        dif1x16 = _mm256_slli_epi16 (orgDif1x16, shift);
        dif2x16 = _mm256_slli_epi16 (orgDif2x16, shift);
        dif3x16 = _mm256_slli_epi16 (orgDif3x16, shift);
        
        dif0x16 = _mm256_add_epi16 (dif0x16, const32x16);
        dif1x16 = _mm256_add_epi16 (dif1x16, const32x16);
        dif2x16 = _mm256_add_epi16 (dif2x16, const32x16);
        dif3x16 = _mm256_add_epi16 (dif3x16, const32x16);
        
        dif0x16 = _mm256_srai_epi16 (dif0x16, 6);
        dif1x16 = _mm256_srai_epi16 (dif1x16, 6);
        dif2x16 = _mm256_srai_epi16 (dif2x16, 6);
        dif3x16 = _mm256_srai_epi16 (dif3x16, 6);

        pred0x32 = _mm256_loadu_si256 ((__m256i *)(pred));
        pred0x16 = _mm256_unpacklo_epi8 (pred0x32, allZero);
        pred1x16 = _mm256_unpackhi_epi8 (pred0x32, allZero);
        pred1x32 = _mm256_loadu_si256 ((__m256i *)(pred + 32));
        pred2x16 = _mm256_unpacklo_epi8 (pred1x32, allZero);
        pred3x16 = _mm256_unpackhi_epi8 (pred1x32, allZero);
        
        pred0x16 = _mm256_add_epi16 (dif0x16, pred0x16);
        pred1x16 = _mm256_add_epi16 (dif1x16, pred1x16);
        pred2x16 = _mm256_add_epi16 (dif2x16, pred2x16);
        pred3x16 = _mm256_add_epi16 (dif3x16, pred3x16);

        pred0x32 = _mm256_packus_epi16 (pred0x16, pred1x16);
        pred1x32 = _mm256_packus_epi16 (pred2x16, pred3x16);

        _mm256_storeu_si256 ((__m256i *)(pred),      pred0x32);
        _mm256_storeu_si256 ((__m256i *)(pred + 32), pred1x32);

        pred += predStride;

    }

}

