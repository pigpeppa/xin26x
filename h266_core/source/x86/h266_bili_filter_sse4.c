/***************************************************************************//**
 *
 * @file          h266_bili_filter_sse4.c
 * @brief         h.266 bilinear filter subroutines (SSE4).
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
#include "h266_motion_comp.h"
#include "h266_inter_pred_context.h"
#include "basic_macro.h"
#include "smmintrin.h"

static const SINT8 g_biliFilterS8[XIN_INTERP_SUB_POS][BILI_INTERP_TAPS] =
{
    {16,  0,},
    {15,  1,},
    {14,  2,},
    {13,  3,},
    {12,  4,},
    {11,  5,},
    {10,  6,},
    { 9,  7,},
    { 8,  8,},
    { 7,  9,},
    { 6, 10,},
    { 5, 11,},
    { 4, 12,},
    { 3, 13,},
    { 2, 14,},
    { 1, 15,}
};

static const SINT16 roundOffset[8] =
{
    BILI_OFFSET, BILI_OFFSET, BILI_OFFSET, BILI_OFFSET,
    BILI_OFFSET, BILI_OFFSET, BILI_OFFSET, BILI_OFFSET
};

static const SINT16 roundOffset2[8] =
{
    BILI_OFFSET2, BILI_OFFSET2, BILI_OFFSET2, BILI_OFFSET2,
    BILI_OFFSET2, BILI_OFFSET2, BILI_OFFSET2, BILI_OFFSET2
};

static const UINT8 shuffleIndex[16] =
{
    0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8
};


void Xin266BiliInterpCopy4xH_SSE4 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height)
{
    UINT32 row;
    __m128i src0x8, src1x8;
    __m128i dst0x8, dst1x8;

    (void)frac;
    (void)width;

    for (row = 0; row < height; row++)
    {
        src0x8 = _mm_loadl_epi64 ((__m128i *)(src));
        src1x8 = _mm_loadl_epi64 ((__m128i *)(src + srcStride));

        dst0x8 = _mm_cvtepu8_epi16 (src0x8);
        dst1x8 = _mm_cvtepu8_epi16 (src1x8);

        dst0x8 = _mm_slli_epi16 (dst0x8, BILI_SHIFT);
        dst1x8 = _mm_slli_epi16 (dst1x8, BILI_SHIFT);

        _mm_storel_epi64 ((__m128i *)(dst),             dst0x8);
        _mm_storeu_si128 ((__m128i *)(dst + dstStride), dst1x8);

        src += srcStride*2;
        dst += dstStride*2;

    }

}


void Xin266BiliInterpCopy8xH_SSE4 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height)
{
    UINT32  row;
    __m128i src0x16, src1x16;
    __m128i dst00x8, dst01x8, dst10x8, dst11x8;
    __m128i allZero;

    (void)frac;
    (void)width;

    allZero = _mm_setzero_si128 ();

    for (row = 0; row < height; row++)
    {
        src0x16 = _mm_loadu_si128 ((__m128i *)(src));
        src1x16 = _mm_loadu_si128 ((__m128i *)(src + srcStride));

        dst00x8 = _mm_cvtepu8_epi16 (src0x16);
        dst10x8 = _mm_cvtepu8_epi16 (src1x16);

        dst01x8 = _mm_unpackhi_epi8 (src0x16, allZero);
        dst11x8 = _mm_unpackhi_epi8 (src1x16, allZero);

        dst00x8 = _mm_slli_epi16 (dst00x8, BILI_SHIFT);
        dst01x8 = _mm_slli_epi16 (dst01x8, BILI_SHIFT);

        dst10x8 = _mm_slli_epi16 (dst10x8, BILI_SHIFT);
        dst11x8 = _mm_slli_epi16 (dst11x8, BILI_SHIFT);

        _mm_storeu_si128 ((__m128i *)(dst),             dst00x8);
        _mm_storeu_si128 ((__m128i *)(dst + dstStride), dst10x8);

        _mm_storel_epi64 ((__m128i *)(dst + 8),             dst01x8);
        _mm_storel_epi64 ((__m128i *)(dst + 8 + dstStride), dst11x8);

        src += srcStride*2;
        dst += dstStride*2;

    }

}

void Xin266BiliInterpCopy16xH_SSE4 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height)
{
    UINT32  row;
    __m128i src00x16, src01x8;
    __m128i src10x16, src11x8;
    __m128i dst00x8, dst01x8, dst02x4;
    __m128i dst10x8, dst11x8, dst12x4;
    __m128i allZero;

    (void)frac;
    (void)width;

    allZero = _mm_setzero_si128 ();

    for (row = 0; row < height; row++)
    {
        src00x16 = _mm_loadu_si128 ((__m128i *)(src));
        src10x16 = _mm_loadu_si128 ((__m128i *)(src + srcStride));

        src01x8 = _mm_loadl_epi64 ((__m128i *)(src + 16));
        src11x8 = _mm_loadl_epi64 ((__m128i *)(src + 16 + srcStride));

        dst00x8 = _mm_cvtepu8_epi16 (src00x16);
        dst10x8 = _mm_cvtepu8_epi16 (src10x16);

        dst01x8 = _mm_unpackhi_epi8 (src00x16, allZero);
        dst11x8 = _mm_unpackhi_epi8 (src10x16, allZero);

        dst02x4 = _mm_cvtepu8_epi16 (src01x8);
        dst12x4 = _mm_cvtepu8_epi16 (src11x8);

        dst00x8 = _mm_slli_epi16 (dst00x8, BILI_SHIFT);
        dst01x8 = _mm_slli_epi16 (dst01x8, BILI_SHIFT);
        dst02x4 = _mm_slli_epi16 (dst02x4, BILI_SHIFT);
        dst10x8 = _mm_slli_epi16 (dst10x8, BILI_SHIFT);
        dst11x8 = _mm_slli_epi16 (dst11x8, BILI_SHIFT);
        dst12x4 = _mm_slli_epi16 (dst12x4, BILI_SHIFT);

        _mm_storeu_si128 ((__m128i *)(dst),                  dst00x8);
        _mm_storeu_si128 ((__m128i *)(dst + 8),              dst01x8);
        _mm_storel_epi64 ((__m128i *)(dst + 16),             dst02x4);
        _mm_storeu_si128 ((__m128i *)(dst + dstStride),      dst10x8);
        _mm_storeu_si128 ((__m128i *)(dst + dstStride + 8),  dst11x8);
        _mm_storel_epi64 ((__m128i *)(dst + dstStride + 16), dst12x4);

        src += srcStride*2;
        dst += dstStride*2;

    }

}

void Xin266BiliInterpHor4xH_SSE4 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height)
{
    const SINT8 *fltCoeff = g_biliFilterS8[frac&XIN_MV_FRAC_MASK];
    UINT32  row;
    __m128i coef10x8;
    __m128i srcx8;
    __m128i src10x8;
    __m128i shufIdx;
    __m128i prodSum;
    __m128i roundx8;
    SINT32  coef10x2;

    (void)width;

    src     -= (BILI_INTERP_TAPS/2 - 1);
    coef10x2 = *((SINT16 *)fltCoeff);
    coef10x2 = (coef10x2 << 16) | coef10x2;

    coef10x8 = _mm_cvtsi32_si128 (coef10x2);
    coef10x8 = _mm_shuffle_epi32 (coef10x8, 0x00);
    shufIdx  = _mm_loadu_si128 ((__m128i *)shuffleIndex);
    roundx8  = _mm_loadu_si128 ((__m128i *)roundOffset);

    for (row = 0; row < height; row++)
    {
        srcx8   = _mm_loadl_epi64 ((__m128i *)(src));
        src10x8 = _mm_shuffle_epi8 (srcx8, shufIdx);
        prodSum = _mm_maddubs_epi16 (src10x8, coef10x8);
        prodSum = _mm_add_epi16 (prodSum, roundx8);
        prodSum = _mm_srai_epi16 (prodSum, BILI_SHIFT);

        _mm_storeu_si128 ((__m128i *)(dst),  prodSum);


        src += srcStride;
        dst += dstStride;
    }

}

void Xin266BiliInterpHor8xH_SSE4 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height)
{
    const SINT8 *fltCoeff = g_biliFilterS8[frac&XIN_MV_FRAC_MASK];
    UINT32  row;
    __m128i coef10x8;
    __m128i srcx16;
    __m128i srcx8;
    __m128i src10x4;
    __m128i src10x8;
    __m128i shufIdx;
    __m128i prodSumx8;
    __m128i prodSumx4;
    __m128i roundx8;
    SINT32  coef10x2;

    (void)width;

    src     -= (BILI_INTERP_TAPS/2 - 1);
    coef10x2 = *((SINT16 *)fltCoeff);
    coef10x2 = (coef10x2 << 16) | coef10x2;

    coef10x8 = _mm_cvtsi32_si128 (coef10x2);
    coef10x8 = _mm_shuffle_epi32 (coef10x8, 0x00);
    shufIdx  = _mm_loadu_si128 ((__m128i *)shuffleIndex);
    roundx8  = _mm_loadu_si128 ((__m128i *)roundOffset);

    for (row = 0; row < height; row++)
    {
        srcx16  = _mm_loadu_si128 ((__m128i *)(src));
        srcx8   = _mm_loadu_si128 ((__m128i *)(src + 8));
        
        src10x8 = _mm_shuffle_epi8 (srcx16, shufIdx);
        src10x4 = _mm_shuffle_epi8 (srcx8,  shufIdx);
        
        prodSumx8 = _mm_maddubs_epi16 (src10x8, coef10x8);
        prodSumx8 = _mm_add_epi16 (prodSumx8, roundx8);
        prodSumx8 = _mm_srai_epi16 (prodSumx8, BILI_SHIFT);

        prodSumx4 = _mm_maddubs_epi16 (src10x4, coef10x8);
        prodSumx4 = _mm_add_epi16 (prodSumx4, roundx8);
        prodSumx4 = _mm_srai_epi16 (prodSumx4, BILI_SHIFT);

        _mm_storeu_si128 ((__m128i *)(dst),     prodSumx8);
        _mm_storel_epi64 ((__m128i *)(dst + 8), prodSumx4);
        
        src += srcStride;
        dst += dstStride;
        
    }

}

void Xin266BiliInterpVet4xH_SSE4 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height)
{
    const SINT8 *fltCoeff = g_biliFilterS8[frac>>XIN_MV_FRAC_BITS];
    UINT32 row;
    __m128i coef0x8;
    __m128i coef1x8;
    __m128i src0x8;
    __m128i src1x8;
    __m128i mul0x8;
    __m128i mul1x8;
    __m128i prodSum;
    __m128i roundx8;

    (void)width;
    src -= (BILI_INTERP_TAPS / 2 - 1) * srcStride;

    roundx8 = _mm_loadu_si128 ((__m128i *)roundOffset);
    coef0x8 = _mm_set1_epi16 (fltCoeff[0]);
    coef1x8 = _mm_set1_epi16 (fltCoeff[1]);

    for (row = 0; row < height; row++)
    {
        src0x8 = _mm_cvtepu8_epi16 (_mm_loadl_epi64 ((__m128i *)src));
        src1x8 = _mm_cvtepu8_epi16 (_mm_loadl_epi64 ((__m128i *)src + srcStride));

        mul0x8 = _mm_mullo_epi16 (src0x8, coef0x8);
        mul1x8 = _mm_mullo_epi16 (src1x8, coef1x8);

        prodSum = _mm_add_epi16 (mul0x8, mul1x8);
        prodSum = _mm_add_epi16 (prodSum, roundx8);
        prodSum = _mm_srai_epi16 (prodSum, BILI_SHIFT);

        _mm_storeu_si128 ((__m128i *)(dst),  prodSum);

        src += srcStride;
        dst += dstStride;

    }

}

void Xin266BiliInterpVet8xH_SSE4 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height)
{
    const SINT8 *fltCoeff = g_biliFilterS8[frac>>XIN_MV_FRAC_BITS];
    UINT32 row;
    __m128i coef0x8;
    __m128i coef1x8;
    __m128i src0x16;
    __m128i src1x16;
    __m128i src00x8, src01x4;
    __m128i src10x8, src11x4;
    __m128i allZero;
    __m128i mul00x8, mul10x8;
    __m128i mul01x4, mul11x4;
    __m128i prodSum;
    __m128i roundx8;

    (void)width;
    src -= (BILI_INTERP_TAPS / 2 - 1) * srcStride;

    roundx8 = _mm_loadu_si128 ((__m128i *)roundOffset);
    coef0x8 = _mm_set1_epi16 (fltCoeff[0]);
    coef1x8 = _mm_set1_epi16 (fltCoeff[1]);
    allZero = _mm_setzero_si128 ();

    for (row = 0; row < height; row++)
    {
        src0x16 = _mm_loadu_si128 ((__m128i *)(src));
        src1x16 = _mm_loadu_si128 ((__m128i *)(src + srcStride));

        src00x8 = _mm_cvtepu8_epi16 (src0x16);
        src10x8 = _mm_cvtepu8_epi16 (src1x16);

        src01x4 = _mm_unpackhi_epi8 (src0x16, allZero);
        src11x4 = _mm_unpackhi_epi8 (src1x16, allZero);

        mul00x8 = _mm_mullo_epi16 (src00x8, coef0x8);
        mul10x8 = _mm_mullo_epi16 (src10x8, coef1x8);

        mul01x4 = _mm_mullo_epi16 (src01x4, coef0x8);
        mul11x4 = _mm_mullo_epi16 (src11x4, coef1x8);

        prodSum = _mm_add_epi16 (mul00x8, mul10x8);
        prodSum = _mm_add_epi16 (prodSum, roundx8);
        prodSum = _mm_srai_epi16 (prodSum, BILI_SHIFT);

        _mm_storeu_si128 ((__m128i *)(dst),  prodSum);

        prodSum = _mm_add_epi16 (mul01x4, mul11x4);
        prodSum = _mm_add_epi16 (prodSum, roundx8);
        prodSum = _mm_srai_epi16 (prodSum, BILI_SHIFT);

        _mm_storel_epi64 ((__m128i *)(dst + 8),  prodSum);

        src += srcStride;
        dst += dstStride;

    }

}

void Xin266BiliInterpVetS16U88xH_SSE4 (
    const SINT16 *src,
    intptr_t     srcStride,
    SINT16       *dst,
    intptr_t     dstStride,
    SINT32       frac,
    UINT32       width,
    UINT32       height)
{
    const SINT8 *fltCoeff = g_biliFilterS8[frac>>XIN_MV_FRAC_BITS];
    UINT32  row;
    __m128i coef0x8;
    __m128i coef1x8;
    __m128i src0x8;
    __m128i src1x8;
    __m128i mul0x8;
    __m128i mul1x8;
    __m128i prodSum;
    __m128i roundx8;

    (void)width;
    src -= (BILI_INTERP_TAPS / 2 - 1) * srcStride;

    roundx8 = _mm_loadu_si128 ((__m128i *)roundOffset2);
    coef0x8 = _mm_set1_epi16 (fltCoeff[0]);
    coef1x8 = _mm_set1_epi16 (fltCoeff[1]);

    for (row = 0; row < height; row++)
    {
        src0x8 = _mm_loadu_si128 ((__m128i *)(src));
        src1x8 = _mm_loadu_si128 ((__m128i *)(src + srcStride));

        mul0x8 = _mm_mullo_epi16 (src0x8, coef0x8);
        mul1x8 = _mm_mullo_epi16 (src1x8, coef1x8);

        prodSum = _mm_add_epi16 (mul0x8, mul1x8);
        prodSum = _mm_add_epi16 (prodSum, roundx8);
        prodSum = _mm_srai_epi16 (prodSum, BILI_SHIFT2);

        _mm_storeu_si128 ((__m128i *)(dst),  prodSum);

        src0x8 = _mm_loadu_si128 ((__m128i *)(src + 8));
        src1x8 = _mm_loadu_si128 ((__m128i *)(src + 8 + srcStride));

        mul0x8 = _mm_mullo_epi16 (src0x8, coef0x8);
        mul1x8 = _mm_mullo_epi16 (src1x8, coef1x8);

        prodSum = _mm_add_epi16 (mul0x8, mul1x8);
        prodSum = _mm_add_epi16 (prodSum, roundx8);
        prodSum = _mm_srai_epi16 (prodSum, BILI_SHIFT2);

        _mm_storel_epi64 ((__m128i *)(dst + 8),  prodSum);

        src += srcStride;
        dst += dstStride;

    }

}

void Xin266BiliInterpVetS16U84xH_SSE4 (
    const SINT16 *src,
    intptr_t     srcStride,
    SINT16       *dst,
    intptr_t     dstStride,
    SINT32       frac,
    UINT32       width,
    UINT32       height)
{
    const SINT8 *fltCoeff = g_biliFilterS8[frac>>XIN_MV_FRAC_BITS];
    UINT32 row;
    __m128i coef0x8;
    __m128i coef1x8;
    __m128i src0x8;
    __m128i src1x8;
    __m128i mul0x8;
    __m128i mul1x8;
    __m128i prodSum;
    __m128i roundx8;

    (void)width;
    src -= (BILI_INTERP_TAPS / 2 - 1) * srcStride;

    roundx8 = _mm_loadu_si128 ((__m128i *)roundOffset2);
    coef0x8 = _mm_set1_epi16 (fltCoeff[0]);
    coef1x8 = _mm_set1_epi16 (fltCoeff[1]);

    for (row = 0; row < height; row++)
    {
        src0x8 = _mm_loadu_si128 ((__m128i *)(src));
        src1x8 = _mm_loadu_si128 ((__m128i *)(src + srcStride));

        mul0x8 = _mm_mullo_epi16 (src0x8, coef0x8);
        mul1x8 = _mm_mullo_epi16 (src1x8, coef1x8);

        prodSum = _mm_add_epi16 (mul0x8, mul1x8);
        prodSum = _mm_add_epi16 (prodSum, roundx8);
        prodSum = _mm_srai_epi16 (prodSum, BILI_SHIFT2);

        _mm_storeu_si128 ((__m128i *)(dst),  prodSum);

        src += srcStride;
        dst += dstStride;

    }

}

void  Xin266BiliInterpHorVet4xH_SSE4 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height)
{
    SINT16  firstPassDst[(128+4)*(128+12)];

    src -= (BILI_INTERP_TAPS / 2 - 1) * srcStride;

    Xin266BiliInterpHor4xH_SSE4 (
        src,
        srcStride,
        firstPassDst,
        width + DMVR_PADING_SIZE*2,
        frac,
        width,
        height + BILI_INTERP_TAPS - 1);

    Xin266BiliInterpVetS16U84xH_SSE4 (
        firstPassDst + (BILI_INTERP_TAPS / 2 - 1) * (width +  + DMVR_PADING_SIZE*2),
        width + DMVR_PADING_SIZE*2,
        dst,
        dstStride,
        frac,
        width,
        height);

}

void  Xin266BiliInterpHorVet8xH_SSE4 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height)
{
    SINT16  firstPassDst[(128+4)*(128+12)];

    src -= (BILI_INTERP_TAPS / 2 - 1) * srcStride;

    Xin266BiliInterpHor8xH_SSE4 (
        src,
        srcStride,
        firstPassDst,
        width + DMVR_PADING_SIZE*2,
        frac,
        width,
        height + BILI_INTERP_TAPS - 1);

    Xin266BiliInterpVetS16U88xH_SSE4 (
        firstPassDst + (BILI_INTERP_TAPS / 2 - 1) * (width + DMVR_PADING_SIZE*2),
        width + DMVR_PADING_SIZE*2,
        dst,
        dstStride,
        frac,
        width,
        height);

}

