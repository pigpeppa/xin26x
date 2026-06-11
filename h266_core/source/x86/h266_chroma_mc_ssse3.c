/***************************************************************************//**
 *
 * @file          h266_chroma_mc_ssse3.c
 * @brief         h.266 chroma motion compensation subroutines (SSSE3).
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
#include "tmmintrin.h"
#include "xin_typedef.h"
#include "h266_inter_pred_context.h"

static const SINT8 g_chromaFilterS8[XIN_INTERP_UV_SUB_POS][CHROMA_INTERP_TAPS] =
{
    {  0, 64,  0,  0 },
    { -1, 63,  2,  0 },
    { -2, 62,  4,  0 },
    { -2, 60,  7, -1 },
    { -2, 58, 10, -2 },
    { -3, 57, 12, -2 },
    { -4, 56, 14, -2 },
    { -4, 55, 15, -2 },
    { -4, 54, 16, -2 },
    { -5, 53, 18, -2 },
    { -6, 52, 20, -2 },
    { -6, 49, 24, -3 },
    { -6, 46, 28, -4 },
    { -5, 44, 29, -4 },
    { -4, 42, 30, -4 },
    { -4, 39, 33, -4 },
    { -4, 36, 36, -4 },
    { -4, 33, 39, -4 },
    { -4, 30, 42, -4 },
    { -4, 29, 44, -5 },
    { -4, 28, 46, -6 },
    { -3, 24, 49, -6 },
    { -2, 20, 52, -6 },
    { -2, 18, 53, -5 },
    { -2, 16, 54, -4 },
    { -2, 15, 55, -4 },
    { -2, 14, 56, -4 },
    { -2, 12, 57, -3 },
    { -2, 10, 58, -2 },
    { -1,  7, 60, -2 },
    {  0,  4, 62, -2 },
    {  0,  2, 63, -1 },
};


static const SINT16 g_chromaFilterS16[XIN_INTERP_UV_SUB_POS][CHROMA_INTERP_TAPS] =
{
    {  0, 64,  0,  0 },
    { -1, 63,  2,  0 },
    { -2, 62,  4,  0 },
    { -2, 60,  7, -1 },
    { -2, 58, 10, -2 },
    { -3, 57, 12, -2 },
    { -4, 56, 14, -2 },
    { -4, 55, 15, -2 },
    { -4, 54, 16, -2 },
    { -5, 53, 18, -2 },
    { -6, 52, 20, -2 },
    { -6, 49, 24, -3 },
    { -6, 46, 28, -4 },
    { -5, 44, 29, -4 },
    { -4, 42, 30, -4 },
    { -4, 39, 33, -4 },
    { -4, 36, 36, -4 },
    { -4, 33, 39, -4 },
    { -4, 30, 42, -4 },
    { -4, 29, 44, -5 },
    { -4, 28, 46, -6 },
    { -3, 24, 49, -6 },
    { -2, 20, 52, -6 },
    { -2, 18, 53, -5 },
    { -2, 16, 54, -4 },
    { -2, 15, 55, -4 },
    { -2, 14, 56, -4 },
    { -2, 12, 57, -3 },
    { -2, 10, 58, -2 },
    { -1,  7, 60, -2 },
    {  0,  4, 62, -2 },
    {  0,  2, 63, -1 },
};


static const UINT8 shuffleIndex[16] =
{
    0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8
};

static const UINT8 shuffleIndex2[16] =
{
    2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10
};

static const SINT16 roundOffset[8] =
{
    INTERP_OFFSET, INTERP_OFFSET, INTERP_OFFSET, INTERP_OFFSET,
    INTERP_OFFSET, INTERP_OFFSET, INTERP_OFFSET, INTERP_OFFSET
};

static const SINT32 roundOffset2[4] =
{
    INTERP_OFFSET2, INTERP_OFFSET2, INTERP_OFFSET2, INTERP_OFFSET2
};

static const SINT16 roundOffset3[8] =
{
    INTERP_PREC_OFFSET, INTERP_PREC_OFFSET, INTERP_PREC_OFFSET, INTERP_PREC_OFFSET,
    INTERP_PREC_OFFSET, INTERP_PREC_OFFSET, INTERP_PREC_OFFSET, INTERP_PREC_OFFSET,
};

void Xin266ChromaInterpHor8xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    BOOL        useAltHpelIf)
{
    const SINT8 *fltCoeff = g_chromaFilterS8[frac & XIN_MV_UV_FRAC_MASK];
    UINT32  row;
    UINT32  coefx4;
    __m128i coef8x16;
    __m128i coef10x8, coef32x8;
    __m128i shufIdx;
    __m128i shufIdx2;
    __m128i roundx8;
    __m128i src8x16;
    __m128i src10x8;
    __m128i src32x8;
    __m128i prod10, prod32;
    __m128i prodSum;
    __m128i outx8;

    (void)width;
    (void)useAltHpelIf;

    src     -= (CHROMA_INTERP_TAPS / 2 - 1);
    shufIdx  = _mm_loadu_si128 ((__m128i *)shuffleIndex);
    shufIdx2 = _mm_loadu_si128 ((__m128i *)shuffleIndex2);
    roundx8  = _mm_loadu_si128 ((__m128i *)roundOffset);

    coefx4   = *((UINT32 *)fltCoeff);
    coef8x16 = _mm_cvtsi32_si128 (coefx4);
    coef8x16 = _mm_unpacklo_epi16 (coef8x16, coef8x16);

    coef10x8 = _mm_shuffle_epi32 (coef8x16, 0x00);
    coef32x8 = _mm_shuffle_epi32 (coef8x16, 0x55);

    for (row = 0; row < height; row++)
    {
        src8x16 = _mm_lddqu_si128 ((__m128i *)src);
        src10x8 = _mm_shuffle_epi8 (src8x16, shufIdx);

        prod10  = _mm_maddubs_epi16 (src10x8, coef10x8);
        prodSum = _mm_add_epi16 (prod10, roundx8);

        src32x8 = _mm_shuffle_epi8 (src8x16, shufIdx2);
        prod32  = _mm_maddubs_epi16 (src32x8, coef32x8);
        prodSum = _mm_add_epi16 (prod32, prodSum);

        outx8  = _mm_srai_epi16 (prodSum, INTERP_SHIFT);
        outx8  = _mm_packus_epi16 (outx8, outx8);

        _mm_storel_epi64 ((__m128i *)dst, outx8);

        src += srcStride;
        dst += dstStride;

    }

}


void Xin266ChromaInterpHorGt8xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    BOOL        useAltHpelIf)
{
    const SINT8 *fltCoeff = g_chromaFilterS8[frac & XIN_MV_UV_FRAC_MASK];
    UINT32  row, col;
    UINT32  coefx4;
    __m128i coef8x16;
    __m128i coef10x8, coef32x8;
    __m128i shufIdx;
    __m128i shufIdx2;
    __m128i roundx8;
    __m128i src8x16;
    __m128i src10x8;
    __m128i src32x8;
    __m128i prod10, prod32;
    __m128i prodSum;
    __m128i outLx8, outRx8;
    __m128i outx16;

    (void)useAltHpelIf;
    
    src     -= (CHROMA_INTERP_TAPS / 2 - 1);
    shufIdx  = _mm_loadu_si128 ((__m128i *)shuffleIndex);
    shufIdx2 = _mm_loadu_si128 ((__m128i *)shuffleIndex2);
    roundx8  = _mm_loadu_si128 ((__m128i *)roundOffset);

    coefx4   = *((UINT32 *)fltCoeff);
    coef8x16 = _mm_cvtsi32_si128 (coefx4);
    coef8x16 = _mm_unpacklo_epi16 (coef8x16, coef8x16);

    coef10x8 = _mm_shuffle_epi32 (coef8x16, 0x00);
    coef32x8 = _mm_shuffle_epi32 (coef8x16, 0x55);

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col += 16)
        {
            src8x16 = _mm_lddqu_si128 ((__m128i *)(src + col));

            src10x8 = _mm_shuffle_epi8 (src8x16, shufIdx);
            prod10  = _mm_maddubs_epi16 (src10x8, coef10x8);
            prodSum = _mm_add_epi16 (prod10, roundx8);

            src32x8 = _mm_shuffle_epi8 (src8x16, shufIdx2);
            prod32  = _mm_maddubs_epi16 (src32x8, coef32x8);
            prodSum = _mm_add_epi16 (prod32, prodSum);

            outLx8  = _mm_srai_epi16 (prodSum, INTERP_SHIFT);

            src8x16 = _mm_lddqu_si128 ((__m128i *)(src + col + 8));

            src10x8 = _mm_shuffle_epi8 (src8x16, shufIdx);
            prod10  = _mm_maddubs_epi16 (src10x8, coef10x8);
            prodSum = _mm_add_epi16 (prod10, roundx8);

            src32x8 = _mm_shuffle_epi8 (src8x16, shufIdx2);
            prod32  = _mm_maddubs_epi16 (src32x8, coef32x8);
            prodSum = _mm_add_epi16 (prod32, prodSum);

            outRx8  = _mm_srai_epi16 (prodSum, INTERP_SHIFT);

            outx16  = _mm_packus_epi16 (outLx8, outRx8);

            _mm_storeu_si128 ((__m128i *)(dst + col), outx16);

        }

        src += srcStride;
        dst += dstStride;

    }

}

void Xin266ChromaInterpVet8xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    BOOL        useAltHpelIf)
{
    const SINT8 *fltCoeff = g_chromaFilterS8[frac >> XIN_MV_UV_FRAC_BITS];
    UINT32  row;
    UINT32  coefx4;
    __m128i roundx8;
    __m128i coef8x16;
    __m128i coef10x8;
    __m128i coef32x8;
    __m128i src0x8;
    __m128i src1x8;
    __m128i src2x8;
    __m128i src3x8;
    __m128i src4x8;
    __m128i srcEve10x8, srcOdd10x8;
    __m128i srcEve32x8, srcOdd32x8;
    __m128i prod10, prod32;
    __m128i prodSum;
    __m128i outx8;

    (void)width;
    (void)useAltHpelIf;
    
    src     -= (CHROMA_INTERP_TAPS / 2 - 1) * srcStride;
    roundx8  = _mm_loadu_si128 ((__m128i *)roundOffset);
    coefx4   = *((UINT32 *)fltCoeff);
    coef8x16 = _mm_cvtsi32_si128 (coefx4);
    coef8x16 = _mm_unpacklo_epi16 (coef8x16, coef8x16);

    coef10x8 = _mm_shuffle_epi32 (coef8x16, 0x00);
    coef32x8 = _mm_shuffle_epi32 (coef8x16, 0x55);

    src0x8  = _mm_loadl_epi64 ((__m128i *) src);
    src1x8  = _mm_loadl_epi64 ((__m128i *)(src + srcStride));
    src    += srcStride*2;
    src2x8  = _mm_loadl_epi64 ((__m128i *) src);
    src    += srcStride;

    srcEve10x8 = _mm_unpacklo_epi8 (src0x8, src1x8);
    srcOdd10x8 = _mm_unpacklo_epi8 (src1x8, src2x8);

    for (row = 0; row < height; row += 2)
    {
        src3x8     = _mm_loadl_epi64 ((__m128i *) src);
        src4x8     = _mm_loadl_epi64 ((__m128i *)(src + srcStride));
        src       += srcStride*2;

        srcEve32x8 = _mm_unpacklo_epi8 (src2x8, src3x8);
        srcOdd32x8 = _mm_unpacklo_epi8 (src3x8, src4x8);

        prod10  = _mm_maddubs_epi16 (srcEve10x8, coef10x8);
        prodSum = _mm_add_epi16 (prod10, roundx8);
        prod32  = _mm_maddubs_epi16 (srcEve32x8, coef32x8);
        prodSum = _mm_add_epi16 (prodSum, prod32);
        outx8   = _mm_srai_epi16 (prodSum, INTERP_SHIFT);
        outx8   = _mm_packus_epi16 (outx8, outx8);

        srcEve10x8 = srcEve32x8;

        _mm_storel_epi64 ((__m128i *)(dst), outx8);

        prod10  = _mm_maddubs_epi16 (srcOdd10x8, coef10x8);
        prodSum = _mm_add_epi16 (prod10, roundx8);
        prod32  = _mm_maddubs_epi16 (srcOdd32x8, coef32x8);
        prodSum = _mm_add_epi16 (prodSum, prod32);
        outx8   = _mm_srai_epi16 (prodSum, INTERP_SHIFT);
        outx8   = _mm_packus_epi16 (outx8, outx8);

        srcOdd10x8 = srcOdd32x8;

        _mm_storel_epi64 ((__m128i *)(dst + dstStride), outx8);

        dst       += dstStride*2;
        src2x8     = src4x8;

    }

}

void Xin266ChromaInterpVetGt8xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    BOOL        useAltHpelIf)
{
    const PIXEL *srcRow;
    const SINT8 *fltCoeff = g_chromaFilterS8[frac >> XIN_MV_UV_FRAC_BITS];
    UINT32  row, col;
    UINT32  coefx4;
    PIXEL   *dstRow;
    __m128i roundx8;
    __m128i coef8x16;
    __m128i coef10x8;
    __m128i coef32x8;
    __m128i src0x16;
    __m128i src1x16;
    __m128i src2x16;
    __m128i src3x16;
    __m128i src4x16;
    __m128i srcEveL10x8, srcOddL10x8;
    __m128i srcEveR10x8, srcOddR10x8;
    __m128i srcEveL32x8, srcOddL32x8;
    __m128i srcEveR32x8, srcOddR32x8;
    __m128i prod10, prod32;
    __m128i prodSum;
    __m128i outLx8, outRx8;
    __m128i outx16;

    (void)useAltHpelIf;
    
    src     -= (CHROMA_INTERP_TAPS / 2 - 1) * srcStride;
    roundx8  = _mm_loadu_si128 ((__m128i *)roundOffset);
    coefx4   = *((UINT32 *)fltCoeff);
    coef8x16 = _mm_cvtsi32_si128 (coefx4);
    coef8x16 = _mm_unpacklo_epi16 (coef8x16, coef8x16);

    coef10x8 = _mm_shuffle_epi32 (coef8x16, 0x00);
    coef32x8 = _mm_shuffle_epi32 (coef8x16, 0x55);

    for (col = 0; col < width; col += 16)
    {
        dstRow = dst + col;
        srcRow = src + col;

        src0x16 = _mm_lddqu_si128 ((__m128i *) srcRow);
        src1x16 = _mm_lddqu_si128 ((__m128i *)(srcRow + srcStride));
        srcRow += srcStride*2;
        src2x16 = _mm_lddqu_si128 ((__m128i *) srcRow);
        srcRow += srcStride;

        srcEveL10x8 = _mm_unpacklo_epi8 (src0x16, src1x16);
        srcOddL10x8 = _mm_unpacklo_epi8 (src1x16, src2x16);
        srcEveR10x8 = _mm_unpackhi_epi8 (src0x16, src1x16);
        srcOddR10x8 = _mm_unpackhi_epi8 (src1x16, src2x16);

        for (row = 0; row < height; row += 2)
        {
            src3x16 = _mm_lddqu_si128 ((__m128i *) srcRow);
            src4x16 = _mm_lddqu_si128 ((__m128i *)(srcRow + srcStride));
            srcRow += srcStride*2;

            srcEveL32x8 = _mm_unpacklo_epi8 (src2x16, src3x16);
            srcOddL32x8 = _mm_unpacklo_epi8 (src3x16, src4x16);

            srcEveR32x8 = _mm_unpackhi_epi8 (src2x16, src3x16);
            srcOddR32x8 = _mm_unpackhi_epi8 (src3x16, src4x16);

            prod10  = _mm_maddubs_epi16 (srcEveL10x8, coef10x8);
            prodSum = _mm_add_epi16 (prod10, roundx8);
            prod32  = _mm_maddubs_epi16 (srcEveL32x8, coef32x8);
            prodSum = _mm_add_epi16 (prodSum, prod32);
            outLx8  = _mm_srai_epi16 (prodSum, INTERP_SHIFT);

            prod10  = _mm_maddubs_epi16 (srcEveR10x8, coef10x8);
            prodSum = _mm_add_epi16 (prod10, roundx8);
            prod32  = _mm_maddubs_epi16 (srcEveR32x8, coef32x8);
            prodSum = _mm_add_epi16 (prodSum, prod32);
            outRx8  = _mm_srai_epi16 (prodSum, INTERP_SHIFT);

            outx16  = _mm_packus_epi16 (outLx8, outRx8);

            srcEveL10x8 = srcEveL32x8;
            srcEveR10x8 = srcEveR32x8;

            _mm_storeu_si128 ((__m128i *)(dstRow), outx16);

            prod10  = _mm_maddubs_epi16 (srcOddL10x8, coef10x8);
            prodSum = _mm_add_epi16 (prod10, roundx8);
            prod32  = _mm_maddubs_epi16 (srcOddL32x8, coef32x8);
            prodSum = _mm_add_epi16 (prodSum, prod32);
            outLx8  = _mm_srai_epi16 (prodSum, INTERP_SHIFT);

            prod10  = _mm_maddubs_epi16 (srcOddR10x8, coef10x8);
            prodSum = _mm_add_epi16 (prod10, roundx8);
            prod32  = _mm_maddubs_epi16 (srcOddR32x8, coef32x8);
            prodSum = _mm_add_epi16 (prodSum, prod32);
            outRx8  = _mm_srai_epi16 (prodSum, INTERP_SHIFT);

            outx16  = _mm_packus_epi16 (outLx8, outRx8);

            srcOddL10x8 = srcOddL32x8;
            srcOddR10x8 = srcOddR32x8;

            _mm_storeu_si128 ((__m128i *)(dstRow + dstStride), outx16);

            dstRow  += dstStride*2;
            src2x16  = src4x16;

        }

    }

}

void Xin266ChromaInterpHor4xHU8S16_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    BOOL        useAltHpelIf)
{

    const SINT8 *fltCoeff = g_chromaFilterS8[frac & XIN_MV_UV_FRAC_MASK];
    UINT32  row;
    UINT32  coefx4;
    __m128i coef8x16;
    __m128i coef10x8, coef32x8;
    __m128i shufIdx;
    __m128i shufIdx2;
    __m128i src8x8;
    __m128i src10x4;
    __m128i src32x4;
    __m128i prod10, prod32;
    __m128i prodSum;
    __m128i roundx8;

    (void)useAltHpelIf;
    (void)width;

    src     -= CHROMA_INTERP_TAPS / 2 - 1;
    shufIdx  = _mm_loadu_si128 ((__m128i *)shuffleIndex);
    shufIdx2 = _mm_loadu_si128 ((__m128i *)shuffleIndex2);
    roundx8  = _mm_loadu_si128 ((__m128i *)roundOffset3);

    coefx4   = *((UINT32 *)fltCoeff);
    coef8x16 = _mm_cvtsi32_si128 (coefx4);
    coef8x16 = _mm_unpacklo_epi16 (coef8x16, coef8x16);

    coef10x8 = _mm_shuffle_epi32 (coef8x16, 0x00);
    coef32x8 = _mm_shuffle_epi32 (coef8x16, 0x55);

    for (row = 0; row < height; row++)
    {
        src8x8  = _mm_loadl_epi64 ((__m128i *)(src));
        src10x4 = _mm_shuffle_epi8 (src8x8, shufIdx);
        prod10  = _mm_maddubs_epi16 (src10x4, coef10x8);

        src32x4 = _mm_shuffle_epi8 (src8x8, shufIdx2);
        prod32  = _mm_maddubs_epi16 (src32x4, coef32x8);

        prodSum = _mm_add_epi16 (prod32, prod10);
        prodSum = _mm_sub_epi16 (prodSum, roundx8);

        _mm_storel_epi64 ((__m128i *)dst, prodSum);

        src += srcStride;
        dst += dstStride;
    }

}

void Xin266ChromaInterpHor8xHU8S16_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    BOOL        useAltHpelIf)
{

    const SINT8 *fltCoeff = g_chromaFilterS8[frac & XIN_MV_UV_FRAC_MASK];
    UINT32  row;
    UINT32  coefx4;
    __m128i coef8x16;
    __m128i coef10x8, coef32x8;
    __m128i shufIdx;
    __m128i shufIdx2;
    __m128i src8x16;
    __m128i src10x8;
    __m128i src32x8;
    __m128i prod10, prod32;
    __m128i prodSum;
    __m128i roundx8;

    (void)useAltHpelIf;
    (void)width;

    src     -= CHROMA_INTERP_TAPS / 2 - 1;
    shufIdx  = _mm_loadu_si128 ((__m128i *)shuffleIndex);
    shufIdx2 = _mm_loadu_si128 ((__m128i *)shuffleIndex2);
    roundx8  = _mm_loadu_si128 ((__m128i *)roundOffset3);

    coefx4   = *((UINT32 *)fltCoeff);
    coef8x16 = _mm_cvtsi32_si128 (coefx4);
    coef8x16 = _mm_unpacklo_epi16 (coef8x16, coef8x16);

    coef10x8 = _mm_shuffle_epi32 (coef8x16, 0x00);
    coef32x8 = _mm_shuffle_epi32 (coef8x16, 0x55);

    for (row = 0; row < height; row++)
    {
        src8x16 = _mm_lddqu_si128 ((__m128i *)(src));

        src10x8 = _mm_shuffle_epi8 (src8x16, shufIdx);
        prod10  = _mm_maddubs_epi16 (src10x8, coef10x8);

        src32x8 = _mm_shuffle_epi8 (src8x16, shufIdx2);
        prod32  = _mm_maddubs_epi16 (src32x8, coef32x8);

        prodSum = _mm_add_epi16 (prod32, prod10);
        prodSum = _mm_sub_epi16 (prodSum, roundx8);

        _mm_storeu_si128 ((__m128i *)dst, prodSum);

        src += srcStride;
        dst += dstStride;
    }

}

void Xin266ChromaInterpHorGt8xHU8S16_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    BOOL        useAltHpelIf)
{

    const SINT8 *fltCoeff = g_chromaFilterS8[frac & XIN_MV_UV_FRAC_MASK];
    UINT32  row, col;
    UINT32  coefx4;
    __m128i coef8x16;
    __m128i coef10x8, coef32x8;
    __m128i shufIdx;
    __m128i shufIdx2;
    __m128i src8x16;
    __m128i src10x8;
    __m128i src32x8;
    __m128i prod10, prod32;
    __m128i prodSum;
    __m128i roundx8;

    (void)useAltHpelIf;
    
    src     -= CHROMA_INTERP_TAPS / 2 - 1;
    shufIdx  = _mm_loadu_si128 ((__m128i *)shuffleIndex);
    shufIdx2 = _mm_loadu_si128 ((__m128i *)shuffleIndex2);
    roundx8  = _mm_loadu_si128 ((__m128i *)roundOffset3);

    coefx4   = *((UINT32 *)fltCoeff);
    coef8x16 = _mm_cvtsi32_si128 (coefx4);
    coef8x16 = _mm_unpacklo_epi16 (coef8x16, coef8x16);

    coef10x8 = _mm_shuffle_epi32 (coef8x16, 0x00);
    coef32x8 = _mm_shuffle_epi32 (coef8x16, 0x55);

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col += 16)
        {
            src8x16 = _mm_lddqu_si128 ((__m128i *)(src + col));

            src10x8 = _mm_shuffle_epi8 (src8x16, shufIdx);
            prod10  = _mm_maddubs_epi16 (src10x8, coef10x8);

            src32x8 = _mm_shuffle_epi8 (src8x16, shufIdx2);
            prod32  = _mm_maddubs_epi16 (src32x8, coef32x8);

            prodSum = _mm_add_epi16 (prod32, prod10);
            prodSum = _mm_sub_epi16 (prodSum, roundx8);

            _mm_storeu_si128 ((__m128i *)(dst + col), prodSum);

            src8x16 = _mm_lddqu_si128 ((__m128i *)(src + col + 8));

            src10x8 = _mm_shuffle_epi8 (src8x16, shufIdx);
            prod10  = _mm_maddubs_epi16 (src10x8, coef10x8);

            src32x8 = _mm_shuffle_epi8 (src8x16, shufIdx2);
            prod32  = _mm_maddubs_epi16 (src32x8, coef32x8);

            prodSum = _mm_add_epi16 (prod32, prod10);
            prodSum = _mm_sub_epi16 (prodSum, roundx8);

            _mm_storeu_si128 ((__m128i *)(dst + col + 8), prodSum);

        }

        src += srcStride;
        dst += dstStride;
    }

}

void Xin266ChromaInterpVet4xHS16U8_SSSE3 (
    const SINT16 *src,
    intptr_t     srcStride,
    PIXEL        *dst,
    intptr_t     dstStride,
    SINT32       frac,
    UINT32       width,
    UINT32       height,
    BOOL         useAltHpelIf)
{
    const SINT16* fltCoeff = g_chromaFilterS16[frac >> XIN_MV_UV_FRAC_BITS];
    UINT32  row;
    __m128i roundx4;
    __m128i coef16x8;
    __m128i coef10x8;
    __m128i coef32x8;
    __m128i src0x8, src1x8, src2x8, src3x8, src4x8;
    __m128i srcEveL10x4, srcEveL32x4;
    __m128i srcOddL10x4, srcOddL32x4;
    __m128i prod10, prod32;
    __m128i prodSum;
    __m128i outLx4;
    __m128i outx8;
    SINT32  outx4;

    (void)width;
    (void)useAltHpelIf;
    
    src     -= (CHROMA_INTERP_TAPS / 2 - 1) * srcStride;
    roundx4  = _mm_lddqu_si128 ((__m128i *)roundOffset2);
    coef16x8 = _mm_loadl_epi64 ((__m128i *)fltCoeff);
    coef10x8 = _mm_shuffle_epi32 (coef16x8, 0x00);
    coef32x8 = _mm_shuffle_epi32 (coef16x8, 0x55);

    src0x8  = _mm_loadl_epi64 ((__m128i *)(src));
    src1x8  = _mm_loadl_epi64 ((__m128i *)(src + srcStride));
    src    += srcStride*2;
    src2x8  = _mm_loadl_epi64 ((__m128i *)(src));
    src    += srcStride;

    srcEveL10x4 = _mm_unpacklo_epi16 (src0x8, src1x8);
    srcOddL10x4 = _mm_unpacklo_epi16 (src1x8, src2x8);

    for (row = 0; row < height; row += 2)
    {
        src3x8   = _mm_loadl_epi64 ((__m128i *)(src));
        src4x8   = _mm_loadl_epi64 ((__m128i *)(src + srcStride));
        src     += srcStride*2;

        srcEveL32x4 = _mm_unpacklo_epi16 (src2x8, src3x8);
        srcOddL32x4 = _mm_unpacklo_epi16 (src3x8, src4x8);

        prod10  = _mm_madd_epi16 (srcEveL10x4, coef10x8);
        prodSum = _mm_add_epi32 (prod10, roundx4);
        prod32  = _mm_madd_epi16 (srcEveL32x4, coef32x8);
        prodSum = _mm_add_epi32 (prodSum, prod32);
        outLx4  = _mm_srai_epi32 (prodSum, INTERP_SHIFT2);

        outx8 = _mm_packs_epi32 (outLx4, outLx4);
        outx8 = _mm_packus_epi16 (outx8, outx8);
        outx4 = _mm_cvtsi128_si32 (outx8);
        *((UINT32 *)(dst)) = outx4;

        srcEveL10x4 = srcEveL32x4;

        prod10  = _mm_madd_epi16 (srcOddL10x4, coef10x8);
        prodSum = _mm_add_epi32 (prod10, roundx4);
        prod32  = _mm_madd_epi16 (srcOddL32x4, coef32x8);
        prodSum = _mm_add_epi32 (prodSum, prod32);
        outLx4  = _mm_srai_epi32 (prodSum, INTERP_SHIFT2);

        outx8 = _mm_packs_epi32 (outLx4, outLx4);
        outx8 = _mm_packus_epi16 (outx8, outx8);
        outx4 = _mm_cvtsi128_si32 (outx8);
        *((UINT32 *)(dst + dstStride)) = outx4;

        srcOddL10x4 = srcOddL32x4;

        dst    += dstStride*2;
        src2x8  = src4x8;

    }


}

void Xin266ChromaInterpVetGt4xHS16U8_SSSE3 (
    const SINT16 *src,
    intptr_t     srcStride,
    PIXEL        *dst,
    intptr_t     dstStride,
    SINT32       frac,
    UINT32       width,
    UINT32       height,
    BOOL         useAltHpelIf)
{
    const SINT16 *srcRow;
    const SINT16* fltCoeff = g_chromaFilterS16[frac >> XIN_MV_UV_FRAC_BITS];
    PIXEL   *dstRow;
    UINT32  row, col;
    __m128i roundx4;
    __m128i coef16x8;
    __m128i coef10x8;
    __m128i coef32x8;
    __m128i src0x8, src1x8, src2x8, src3x8, src4x8;
    __m128i srcEveL10x4, srcEveL32x4;
    __m128i srcOddL10x4, srcOddL32x4;
    __m128i srcEveR10x4, srcEveR32x4;
    __m128i srcOddR10x4, srcOddR32x4;
    __m128i prod10, prod32;
    __m128i prodSum;
    __m128i outLx4, outRx4;
    __m128i outx8;

    (void)useAltHpelIf;
    
    src     -= (CHROMA_INTERP_TAPS / 2 - 1) * srcStride;
    roundx4  = _mm_lddqu_si128 ((__m128i *)roundOffset2);
    coef16x8 = _mm_loadl_epi64 ((__m128i *)fltCoeff);
    coef10x8 = _mm_shuffle_epi32 (coef16x8, 0x00);
    coef32x8 = _mm_shuffle_epi32 (coef16x8, 0x55);

    for (col = 0; col < width; col += 8)
    {
        srcRow   = src + col;
        dstRow   = dst + col;

        src0x8  = _mm_lddqu_si128 ((__m128i *)(srcRow));
        src1x8  = _mm_lddqu_si128 ((__m128i *)(srcRow + srcStride));
        srcRow += srcStride*2;
        src2x8  = _mm_lddqu_si128 ((__m128i *)(srcRow));
        srcRow += srcStride;

        srcEveL10x4 = _mm_unpacklo_epi16 (src0x8, src1x8);
        srcOddL10x4 = _mm_unpacklo_epi16 (src1x8, src2x8);

        srcEveR10x4 = _mm_unpackhi_epi16 (src0x8, src1x8);
        srcOddR10x4 = _mm_unpackhi_epi16 (src1x8, src2x8);

        for (row = 0; row < height; row += 2)
        {
            src3x8   = _mm_lddqu_si128 ((__m128i *)(srcRow));
            src4x8   = _mm_lddqu_si128 ((__m128i *)(srcRow + srcStride));
            srcRow  += srcStride*2;

            srcEveL32x4 = _mm_unpacklo_epi16 (src2x8, src3x8);
            srcOddL32x4 = _mm_unpacklo_epi16 (src3x8, src4x8);

            srcEveR32x4 = _mm_unpackhi_epi16 (src2x8, src3x8);
            srcOddR32x4 = _mm_unpackhi_epi16 (src3x8, src4x8);

            prod10  = _mm_madd_epi16 (srcEveL10x4, coef10x8);
            prodSum = _mm_add_epi32 (prod10, roundx4);
            prod32  = _mm_madd_epi16 (srcEveL32x4, coef32x8);
            prodSum = _mm_add_epi32 (prodSum, prod32);
            outLx4  = _mm_srai_epi32 (prodSum, INTERP_SHIFT2);

            prod10  = _mm_madd_epi16 (srcEveR10x4, coef10x8);
            prodSum = _mm_add_epi32 (prod10, roundx4);
            prod32  = _mm_madd_epi16 (srcEveR32x4, coef32x8);
            prodSum = _mm_add_epi32 (prodSum, prod32);
            outRx4  = _mm_srai_epi32 (prodSum, INTERP_SHIFT2);

            outx8 = _mm_packs_epi32 (outLx4, outRx4);
            outx8 = _mm_packus_epi16 (outx8, outx8);
            _mm_storel_epi64 ((__m128i *)(dstRow), outx8);

            srcEveL10x4 = srcEveL32x4;
            srcEveR10x4 = srcEveR32x4;

            prod10  = _mm_madd_epi16 (srcOddL10x4, coef10x8);
            prodSum = _mm_add_epi32 (prod10, roundx4);
            prod32  = _mm_madd_epi16 (srcOddL32x4, coef32x8);
            prodSum = _mm_add_epi32 (prodSum, prod32);
            outLx4  = _mm_srai_epi32 (prodSum, INTERP_SHIFT2);

            prod10  = _mm_madd_epi16 (srcOddR10x4, coef10x8);
            prodSum = _mm_add_epi32 (prod10, roundx4);
            prod32  = _mm_madd_epi16 (srcOddR32x4, coef32x8);
            prodSum = _mm_add_epi32 (prodSum, prod32);
            outRx4  = _mm_srai_epi32 (prodSum, INTERP_SHIFT2);

            outx8 = _mm_packs_epi32 (outLx4, outRx4);
            outx8 = _mm_packus_epi16 (outx8, outx8);
            _mm_storel_epi64 ((__m128i *)(dstRow + dstStride), outx8);

            srcOddL10x4 = srcOddL32x4;
            srcOddR10x4 = srcOddR32x4;

            dstRow += dstStride*2;
            src2x8  = src4x8;

        }

    }

}

void Xin266ChromaInterpHorVet4xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    BOOL        useAltHpelIf)
{
    SINT16  firstPassDst[(128 / 2) * (128 / 2 + 8)];

    src -= (CHROMA_INTERP_TAPS / 2 - 1) * srcStride;

    Xin266ChromaInterpHor4xHU8S16_SSSE3 (
        src,
        srcStride,
        firstPassDst,
        width,
        frac,
        width,
        height + CHROMA_INTERP_TAPS - 1,
        useAltHpelIf);

    Xin266ChromaInterpVet4xHS16U8_SSSE3 (
        firstPassDst + (CHROMA_INTERP_TAPS / 2 - 1) * width,
        width,
        dst,
        dstStride,
        frac,
        width,
        height,
        useAltHpelIf);

}

void Xin266ChromaInterpHorVet8xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    BOOL        useAltHpelIf)
{
    SINT16  firstPassDst[(128 / 2) * (128 / 2 + 8)];

    src -= (CHROMA_INTERP_TAPS / 2 - 1) * srcStride;

    Xin266ChromaInterpHor8xHU8S16_SSSE3 (
        src,
        srcStride,
        firstPassDst,
        width,
        frac,
        width,
        height + CHROMA_INTERP_TAPS - 1,
        useAltHpelIf);

    Xin266ChromaInterpVetGt4xHS16U8_SSSE3 (
        firstPassDst + (CHROMA_INTERP_TAPS / 2 - 1) * width,
        width,
        dst,
        dstStride,
        frac,
        width,
        height,
        useAltHpelIf);

}

void Xin266ChromaInterpHorVetGt8xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    BOOL        useAltHpelIf)
{
    SINT16  firstPassDst[(128 / 2) * (128 / 2 + 8)];

    src -= (CHROMA_INTERP_TAPS / 2 - 1) * srcStride;

    Xin266ChromaInterpHorGt8xHU8S16_SSSE3 (
        src,
        srcStride,
        firstPassDst,
        width,
        frac,
        width,
        height + CHROMA_INTERP_TAPS - 1,
        useAltHpelIf);

    Xin266ChromaInterpVetGt4xHS16U8_SSSE3 (
        firstPassDst + (CHROMA_INTERP_TAPS / 2 - 1) * width,
        width,
        dst,
        dstStride,
        frac,
        width,
        height,
        useAltHpelIf);

}

void Xin266ChromaInterpVet4xHS16S16_SSSE3 (
    const SINT16 *src,
    intptr_t     srcStride,
    SINT16       *dst,
    intptr_t     dstStride,
    SINT32       frac,
    UINT32       width,
    UINT32       height,
    BOOL         useAltHpelIf)
{
    const SINT16* fltCoeff = g_chromaFilterS16[frac >> XIN_MV_UV_FRAC_BITS];
    UINT32  row;
    __m128i coef16x8;
    __m128i coef10x8;
    __m128i coef32x8;
    __m128i src0x8, src1x8, src2x8, src3x8, src4x8;
    __m128i srcEveL10x4, srcEveL32x4;
    __m128i srcOddL10x4, srcOddL32x4;
    __m128i prod10, prod32;
    __m128i prodSum;
    __m128i outLx4;
    __m128i outx8;

    (void)width;
    (void)useAltHpelIf;
    
    src     -= (CHROMA_INTERP_TAPS / 2 - 1) * srcStride;
    coef16x8 = _mm_loadl_epi64 ((__m128i *)fltCoeff);
    coef10x8 = _mm_shuffle_epi32 (coef16x8, 0x00);
    coef32x8 = _mm_shuffle_epi32 (coef16x8, 0x55);

    src0x8  = _mm_loadl_epi64 ((__m128i *)(src));
    src1x8  = _mm_loadl_epi64 ((__m128i *)(src + srcStride));
    src    += srcStride*2;
    src2x8  = _mm_loadl_epi64 ((__m128i *)(src));
    src    += srcStride;

    srcEveL10x4 = _mm_unpacklo_epi16 (src0x8, src1x8);
    srcOddL10x4 = _mm_unpacklo_epi16 (src1x8, src2x8);

    for (row = 0; row < height; row += 2)
    {
        src3x8   = _mm_loadl_epi64 ((__m128i *)(src));
        src4x8   = _mm_loadl_epi64 ((__m128i *)(src + srcStride));
        src     += srcStride*2;

        srcEveL32x4 = _mm_unpacklo_epi16 (src2x8, src3x8);
        srcOddL32x4 = _mm_unpacklo_epi16 (src3x8, src4x8);

        prod10  = _mm_madd_epi16 (srcEveL10x4, coef10x8);
        prod32  = _mm_madd_epi16 (srcEveL32x4, coef32x8);
        prodSum = _mm_add_epi32 (prod10, prod32);
        outLx4  = _mm_srai_epi32 (prodSum, INTERP_SHIFT);

        outx8 = _mm_packs_epi32 (outLx4, outLx4);
        _mm_storel_epi64 ((__m128i *)(dst), outx8);

        srcEveL10x4 = srcEveL32x4;

        prod10  = _mm_madd_epi16 (srcOddL10x4, coef10x8);
        prod32  = _mm_madd_epi16 (srcOddL32x4, coef32x8);
        prodSum = _mm_add_epi32 (prod10, prod32);
        outLx4  = _mm_srai_epi32 (prodSum, INTERP_SHIFT);

        outx8 = _mm_packs_epi32 (outLx4, outLx4);
        _mm_storel_epi64 ((__m128i *)(dst + dstStride), outx8);

        srcOddL10x4 = srcOddL32x4;

        dst    += dstStride*2;
        src2x8  = src4x8;

    }

}

void Xin266ChromaInterpVetGt4xHS16S16_SSSE3 (
    const SINT16 *src,
    intptr_t     srcStride,
    SINT16       *dst,
    intptr_t     dstStride,
    SINT32       frac,
    UINT32       width,
    UINT32       height,
    BOOL         useAltHpelIf)
{
    const SINT16 *srcRow;
    const SINT16* fltCoeff = g_chromaFilterS16[frac >> XIN_MV_UV_FRAC_BITS];
    SINT16  *dstRow;
    UINT32  row, col;
    __m128i coef16x8;
    __m128i coef10x8;
    __m128i coef32x8;
    __m128i src0x8, src1x8, src2x8, src3x8, src4x8;
    __m128i srcEveL10x4, srcEveL32x4;
    __m128i srcOddL10x4, srcOddL32x4;
    __m128i srcEveR10x4, srcEveR32x4;
    __m128i srcOddR10x4, srcOddR32x4;
    __m128i prod10, prod32;
    __m128i prodSum;
    __m128i outLx4, outRx4;
    __m128i outx8;

    (void)useAltHpelIf;
    
    src     -= (CHROMA_INTERP_TAPS / 2 - 1) * srcStride;
    coef16x8 = _mm_loadl_epi64 ((__m128i *)fltCoeff);
    coef10x8 = _mm_shuffle_epi32 (coef16x8, 0x00);
    coef32x8 = _mm_shuffle_epi32 (coef16x8, 0x55);

    for (col = 0; col < width; col += 8)
    {
        srcRow   = src + col;
        dstRow   = dst + col;

        src0x8  = _mm_lddqu_si128 ((__m128i *)(srcRow));
        src1x8  = _mm_lddqu_si128 ((__m128i *)(srcRow + srcStride));
        srcRow += srcStride*2;
        src2x8  = _mm_lddqu_si128 ((__m128i *)(srcRow));
        srcRow += srcStride;

        srcEveL10x4 = _mm_unpacklo_epi16 (src0x8, src1x8);
        srcOddL10x4 = _mm_unpacklo_epi16 (src1x8, src2x8);

        srcEveR10x4 = _mm_unpackhi_epi16 (src0x8, src1x8);
        srcOddR10x4 = _mm_unpackhi_epi16 (src1x8, src2x8);

        for (row = 0; row < height; row += 2)
        {
            src3x8   = _mm_lddqu_si128 ((__m128i *)(srcRow));
            src4x8   = _mm_lddqu_si128 ((__m128i *)(srcRow + srcStride));
            srcRow  += srcStride*2;

            srcEveL32x4 = _mm_unpacklo_epi16 (src2x8, src3x8);
            srcOddL32x4 = _mm_unpacklo_epi16 (src3x8, src4x8);

            srcEveR32x4 = _mm_unpackhi_epi16 (src2x8, src3x8);
            srcOddR32x4 = _mm_unpackhi_epi16 (src3x8, src4x8);

            prod10  = _mm_madd_epi16 (srcEveL10x4, coef10x8);
            prod32  = _mm_madd_epi16 (srcEveL32x4, coef32x8);
            prodSum = _mm_add_epi32 (prod10, prod32);
            outLx4  = _mm_srai_epi32 (prodSum, INTERP_SHIFT);

            prod10  = _mm_madd_epi16 (srcEveR10x4, coef10x8);
            prod32  = _mm_madd_epi16 (srcEveR32x4, coef32x8);
            prodSum = _mm_add_epi32 (prod10, prod32);
            outRx4  = _mm_srai_epi32 (prodSum, INTERP_SHIFT);

            outx8 = _mm_packs_epi32 (outLx4, outRx4);
            _mm_storeu_si128 ((__m128i *)(dstRow), outx8);

            srcEveL10x4 = srcEveL32x4;
            srcEveR10x4 = srcEveR32x4;

            prod10  = _mm_madd_epi16 (srcOddL10x4, coef10x8);
            prod32  = _mm_madd_epi16 (srcOddL32x4, coef32x8);
            prodSum = _mm_add_epi32 (prod10, prod32);
            outLx4  = _mm_srai_epi32 (prodSum, INTERP_SHIFT);

            prod10  = _mm_madd_epi16 (srcOddR10x4, coef10x8);
            prod32  = _mm_madd_epi16 (srcOddR32x4, coef32x8);
            prodSum = _mm_add_epi32 (prod10, prod32);
            outRx4  = _mm_srai_epi32 (prodSum, INTERP_SHIFT);

            outx8 = _mm_packs_epi32 (outLx4, outRx4);
            _mm_storeu_si128 ((__m128i *)(dstRow + dstStride), outx8);

            srcOddL10x4 = srcOddL32x4;
            srcOddR10x4 = srcOddR32x4;

            dstRow += dstStride*2;
            src2x8  = src4x8;

        }

    }

}

void Xin266ChromaInterpHorVet4xHU8S16_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    BOOL        useAltHpelIf)
{
    SINT16  firstPassDst[(128 / 2) * (128 / 2 + 8)];

    src -= (CHROMA_INTERP_TAPS / 2 - 1) * srcStride;

    Xin266ChromaInterpHor4xHU8S16_SSSE3 (
        src,
        srcStride,
        firstPassDst,
        width,
        frac,
        width,
        height + CHROMA_INTERP_TAPS - 1,
        useAltHpelIf);

    Xin266ChromaInterpVet4xHS16S16_SSSE3 (
        firstPassDst + (CHROMA_INTERP_TAPS / 2 - 1) * width,
        width,
        dst,
        dstStride,
        frac,
        width,
        height,
        useAltHpelIf);

}

void Xin266ChromaInterpHorVet8xHU8S16_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    BOOL        useAltHpelIf)
{
    SINT16  firstPassDst[(128 / 2) * (128 / 2 + 8)];

    src -= (CHROMA_INTERP_TAPS / 2 - 1) * srcStride;

    Xin266ChromaInterpHor8xHU8S16_SSSE3 (
        src,
        srcStride,
        firstPassDst,
        width,
        frac,
        width,
        height + CHROMA_INTERP_TAPS - 1,
        useAltHpelIf);

    Xin266ChromaInterpVetGt4xHS16S16_SSSE3 (
        firstPassDst + (CHROMA_INTERP_TAPS / 2 - 1) * width,
        width,
        dst,
        dstStride,
        frac,
        width,
        height,
        useAltHpelIf);

}

void Xin266ChromaInterpHorVetGt8xHU8S16_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    BOOL        useAltHpelIf)
{
    SINT16  firstPassDst[(128 / 2) * (128 / 2 + 8)];

    src -= (CHROMA_INTERP_TAPS / 2 - 1) * srcStride;

    Xin266ChromaInterpHorGt8xHU8S16_SSSE3 (
        src,
        srcStride,
        firstPassDst,
        width,
        frac,
        width,
        height + CHROMA_INTERP_TAPS - 1,
        useAltHpelIf);

    Xin266ChromaInterpVetGt4xHS16S16_SSSE3 (
        firstPassDst + (CHROMA_INTERP_TAPS / 2 - 1) * width,
        width,
        dst,
        dstStride,
        frac,
        width,
        height,
        useAltHpelIf);

}


