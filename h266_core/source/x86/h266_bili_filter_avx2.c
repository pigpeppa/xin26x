/***************************************************************************//**
 *
 * @file          h266_bili_filter_avx2.c
 * @brief         h.266 bilinear filter subroutines (AVX2).
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
#include "immintrin.h"

#ifdef __linux__
#include "xin_avx2_linux_patch.h"
#endif

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

static const SINT16 roundOffset[16] =
{
    BILI_OFFSET, BILI_OFFSET, BILI_OFFSET, BILI_OFFSET,
    BILI_OFFSET, BILI_OFFSET, BILI_OFFSET, BILI_OFFSET,
    BILI_OFFSET, BILI_OFFSET, BILI_OFFSET, BILI_OFFSET,
    BILI_OFFSET, BILI_OFFSET, BILI_OFFSET, BILI_OFFSET,
};

static const SINT16 roundOffset2[16] =
{
    BILI_OFFSET2, BILI_OFFSET2, BILI_OFFSET2, BILI_OFFSET2,
    BILI_OFFSET2, BILI_OFFSET2, BILI_OFFSET2, BILI_OFFSET2,
    BILI_OFFSET2, BILI_OFFSET2, BILI_OFFSET2, BILI_OFFSET2,
    BILI_OFFSET2, BILI_OFFSET2, BILI_OFFSET2, BILI_OFFSET2,
};

static const UINT8 shuffleIndex[32] =
{
    0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8,
};

void Xin266BiliInterpCopy32xH_AVX2 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height)
{
    UINT32  row;
    __m256i src0x32, src1x32;
    __m256i dst0x16, dst1x16;
    __m256i dst2x16, dst3x16;
    __m128i src0x4, src1x4;
    __m128i dst0x4, dst1x4;

    (void)frac;
    (void)width;

    for (row = 0; row < height; row++)
    {
        src0x32 = _mm256_loadu_si256 ((__m256i *)(src));
        src1x32 = _mm256_loadu_si256 ((__m256i *)(src + srcStride));

        src0x4 = _mm_loadl_epi64 ((__m128i *)(src + 32));
        src1x4 = _mm_loadl_epi64 ((__m128i *)(src + srcStride + 32));

        dst0x16 = _mm256_cvtepu8_epi16 (_mm256_extracti128_si256 (src0x32, 0));
        dst1x16 = _mm256_cvtepu8_epi16 (_mm256_extracti128_si256 (src0x32, 1));
        dst0x4  = _mm_cvtepu8_epi16 (src0x4);

        dst2x16 = _mm256_cvtepu8_epi16 (_mm256_extracti128_si256 (src1x32, 0));
        dst3x16 = _mm256_cvtepu8_epi16 (_mm256_extracti128_si256 (src1x32, 1));
        dst1x4  = _mm_cvtepu8_epi16 (src1x4);

        dst0x16 = _mm256_slli_epi16 (dst0x16, BILI_SHIFT);
        dst1x16 = _mm256_slli_epi16 (dst1x16, BILI_SHIFT);
        dst0x4  = _mm_slli_epi16 (dst0x4, BILI_SHIFT);
        dst2x16 = _mm256_slli_epi16 (dst2x16, BILI_SHIFT);
        dst3x16 = _mm256_slli_epi16 (dst3x16, BILI_SHIFT);
        dst1x4  = _mm_slli_epi16 (dst1x4, BILI_SHIFT);

        _mm256_storeu_si256 ((__m256i *)(dst),                  dst0x16);
        _mm256_storeu_si256 ((__m256i *)(dst + 16),             dst1x16);
        _mm_storel_epi64 ((__m128i *)(dst + 32),                dst0x4);
        
        _mm256_storeu_si256 ((__m256i *)(dst + dstStride),      dst2x16);
        _mm256_storeu_si256 ((__m256i *)(dst + dstStride + 16), dst3x16);
        _mm_storel_epi64 ((__m128i *)(dst + dstStride + 32),    dst1x4);

        src += srcStride*2;
        dst += dstStride*2;

    }

}

void Xin266BiliInterpCopy64xH_AVX2 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height)
{
    UINT32 row;
    __m256i src0x32, src1x32;
    __m128i src2x4;
    __m256i dst0x16, dst1x16;
    __m256i dst2x16, dst3x16;
    __m128i dst4x4;

    (void)frac;
    (void)width;

    for (row = 0; row < height; row++)
    {
        src0x32 = _mm256_loadu_si256 ((__m256i *)(src));
        src1x32 = _mm256_loadu_si256 ((__m256i *)(src + 32));
        src2x4  = _mm_loadl_epi64 ((__m128i *)(src + 64));

        dst0x16 = _mm256_cvtepu8_epi16 (_mm256_extracti128_si256 (src0x32, 0));
        dst1x16 = _mm256_cvtepu8_epi16 (_mm256_extracti128_si256 (src0x32, 1));

        dst2x16 = _mm256_cvtepu8_epi16 (_mm256_extracti128_si256 (src1x32, 0));
        dst3x16 = _mm256_cvtepu8_epi16 (_mm256_extracti128_si256 (src1x32, 1));
        
        dst4x4  = _mm_cvtepu8_epi16 (src2x4);

        dst0x16 = _mm256_slli_epi16 (dst0x16, BILI_SHIFT);
        dst1x16 = _mm256_slli_epi16 (dst1x16, BILI_SHIFT);
        dst2x16 = _mm256_slli_epi16 (dst2x16, BILI_SHIFT);
        dst3x16 = _mm256_slli_epi16 (dst3x16, BILI_SHIFT);
        dst4x4  = _mm_slli_epi16 (dst4x4,  BILI_SHIFT);

        _mm256_storeu_si256 ((__m256i *)(dst),      dst0x16);
        _mm256_storeu_si256 ((__m256i *)(dst + 16), dst1x16);
        _mm256_storeu_si256 ((__m256i *)(dst + 32), dst2x16);
        _mm256_storeu_si256 ((__m256i *)(dst + 48), dst3x16);
        _mm_storel_epi64 ((__m128i *)(dst + 64), dst4x4);

        src += srcStride;
        dst += dstStride;

    }

}

void Xin266BiliInterpCopy128xH_AVX2 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height)
{
    UINT32 row;
    __m256i src0x32, src1x32;
    __m256i src2x32, src3x32;
    __m128i src4x4;
    __m256i dst0x16, dst1x16;
    __m256i dst2x16, dst3x16;
    __m256i dst4x16, dst5x16;
    __m256i dst6x16, dst7x16;
    __m128i dst8x4;

    (void)frac;
    (void)width;

    for (row = 0; row < height; row++)
    {
        src0x32 = _mm256_loadu_si256 ((__m256i *)(src));
        src1x32 = _mm256_loadu_si256 ((__m256i *)(src + 32));
        src2x32 = _mm256_loadu_si256 ((__m256i *)(src + 64));
        src3x32 = _mm256_loadu_si256 ((__m256i *)(src + 96));
        
        src4x4  = _mm_loadl_epi64 ((__m128i *)(src + 128));

        dst0x16 = _mm256_cvtepu8_epi16 (_mm256_extracti128_si256 (src0x32, 0));
        dst1x16 = _mm256_cvtepu8_epi16 (_mm256_extracti128_si256 (src0x32, 1));

        dst2x16 = _mm256_cvtepu8_epi16 (_mm256_extracti128_si256 (src1x32, 0));
        dst3x16 = _mm256_cvtepu8_epi16 (_mm256_extracti128_si256 (src1x32, 1));

        dst4x16 = _mm256_cvtepu8_epi16 (_mm256_extracti128_si256 (src2x32, 0));
        dst5x16 = _mm256_cvtepu8_epi16 (_mm256_extracti128_si256 (src2x32, 1));

        dst6x16 = _mm256_cvtepu8_epi16 (_mm256_extracti128_si256 (src3x32, 0));
        dst7x16 = _mm256_cvtepu8_epi16 (_mm256_extracti128_si256 (src3x32, 1));

        dst8x4  = _mm_cvtepu8_epi16 (src4x4);

        dst0x16 = _mm256_slli_epi16 (dst0x16, BILI_SHIFT);
        dst1x16 = _mm256_slli_epi16 (dst1x16, BILI_SHIFT);
        dst2x16 = _mm256_slli_epi16 (dst2x16, BILI_SHIFT);
        dst3x16 = _mm256_slli_epi16 (dst3x16, BILI_SHIFT);
        dst4x16 = _mm256_slli_epi16 (dst4x16, BILI_SHIFT);
        dst5x16 = _mm256_slli_epi16 (dst5x16, BILI_SHIFT);
        dst6x16 = _mm256_slli_epi16 (dst6x16, BILI_SHIFT);
        dst7x16 = _mm256_slli_epi16 (dst7x16, BILI_SHIFT);
        
        dst8x4  = _mm_slli_epi16 (dst8x4, BILI_SHIFT);

        _mm256_storeu_si256 ((__m256i *)(dst),       dst0x16);
        _mm256_storeu_si256 ((__m256i *)(dst +  16), dst1x16);
        _mm256_storeu_si256 ((__m256i *)(dst +  32), dst2x16);
        _mm256_storeu_si256 ((__m256i *)(dst +  48), dst3x16);
        _mm256_storeu_si256 ((__m256i *)(dst +  64), dst4x16);
        _mm256_storeu_si256 ((__m256i *)(dst +  80), dst5x16);
        _mm256_storeu_si256 ((__m256i *)(dst +  96), dst6x16);
        _mm256_storeu_si256 ((__m256i *)(dst + 112), dst7x16);

        _mm_storel_epi64 ((__m128i *)(dst + 128), dst8x4);

        src += srcStride;
        dst += dstStride;

    }

}

void Xin266BiliInterpHorGt8xH_AVX2 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height)
{
    const SINT8 *fltCoeff = g_biliFilterS8[frac&XIN_MV_FRAC_MASK];
    UINT32  row, col;
    __m128i coef10x8;
    __m256i coef10x16;
    __m256i srcx32;
    __m256i srcLRx16;
    __m256i src10x16;
    __m256i shufIdx;
    __m256i prodSum;
    __m256i roundx16;
    SINT32  coef10x2;
    __m128i srcx8;
    __m128i src10x4;
    __m128i shufId128;
    __m128i prodSum128;
    __m128i roundx8;

    src     -= (BILI_INTERP_TAPS/2 - 1);
    coef10x2 = *((SINT16 *)fltCoeff);
    coef10x2 = (coef10x2 << 16) | coef10x2;

    coef10x8  = _mm_cvtsi32_si128 (coef10x2);
    coef10x8  = _mm_shuffle_epi32 (coef10x8, 0x00);
    coef10x16 = _mm256_broadcastsi128_si256 (coef10x8);
    shufIdx   = _mm256_loadu_si256 ((__m256i *)shuffleIndex);
    shufId128 = _mm_loadu_si128 ((__m128i *)shuffleIndex);
    roundx16  = _mm256_loadu_si256 ((__m256i *)roundOffset);
    roundx8   = _mm_loadu_si128 ((__m128i *)roundOffset);

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col += 16)
        {
            srcx32   = _mm256_lddqu_si256 ((__m256i *)(src + col));
            srcLRx16 = _mm256_permute4x64_epi64 (srcx32, 0x94);
            src10x16 = _mm256_shuffle_epi8 (srcLRx16, shufIdx);
            prodSum  = _mm256_maddubs_epi16 (src10x16, coef10x16);
            prodSum  = _mm256_add_epi16 (prodSum, roundx16);
            prodSum  = _mm256_srai_epi16 (prodSum, BILI_SHIFT);

            _mm256_storeu_si256 ((__m256i *)(dst + col),  prodSum);
        }

        srcx8      = _mm_loadl_epi64 ((__m128i *)(src + width));
        src10x4    = _mm_shuffle_epi8 (srcx8, shufId128);
        prodSum128 = _mm_maddubs_epi16 (src10x4, coef10x8);
        prodSum128 = _mm_add_epi16 (prodSum128, roundx8);
        prodSum128 = _mm_srai_epi16 (prodSum128, BILI_SHIFT);

        _mm_storel_epi64 ((__m128i *)(dst + width),  prodSum128);
        
        src += srcStride;
        dst += dstStride;
    }

}

void Xin266BiliInterpVetGt8xH_AVX2 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height)
{
    const SINT8 *fltCoeff = g_biliFilterS8[frac>>XIN_MV_FRAC_BITS];
    UINT32 row, col;
    __m256i coef0x16;
    __m256i coef1x16;
    __m256i src0x16;
    __m256i src1x16;
    __m256i mul0x16;
    __m256i mul1x16;
    __m256i prodSum;
    __m256i roundx16;
    __m128i src0x4, src1x4;
    __m128i mul0x4, mul1x4;
    __m128i coef0x4, coef1x4;
    __m128i prodSum128;
    __m128i roundx4;

    src -= (BILI_INTERP_TAPS / 2 - 1) * srcStride;

    roundx16 = _mm256_loadu_si256 ((__m256i *)roundOffset);
    roundx4  = _mm_loadu_si128 ((__m128i *)roundOffset);
    coef0x16 = _mm256_set1_epi16 (fltCoeff[0]);
    coef1x16 = _mm256_set1_epi16 (fltCoeff[1]);
    coef0x4  = _mm_set1_epi16 (fltCoeff[0]);
    coef1x4  = _mm_set1_epi16 (fltCoeff[1]);

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col += 16)
        {
            src0x16 = _mm256_cvtepu8_epi16 (_mm_loadu_si128 ((__m128i *)(src + col)));
            src1x16 = _mm256_cvtepu8_epi16 (_mm_loadu_si128 ((__m128i *)(src + col + srcStride)));

            mul0x16 = _mm256_mullo_epi16 (src0x16, coef0x16);
            mul1x16 = _mm256_mullo_epi16 (src1x16, coef1x16);

            prodSum = _mm256_add_epi16 (mul0x16, mul1x16);
            prodSum = _mm256_add_epi16 (prodSum, roundx16);
            prodSum  = _mm256_srai_epi16 (prodSum, BILI_SHIFT);

            _mm256_storeu_si256 ((__m256i *)(dst + col),  prodSum);
        }

        src0x4 = _mm_cvtepu8_epi16 (_mm_loadl_epi64 ((__m128i *)(src + width)));
        src1x4 = _mm_cvtepu8_epi16 (_mm_loadl_epi64 ((__m128i *)(src + width + srcStride)));

        mul0x4 = _mm_mullo_epi16 (src0x4, coef0x4);
        mul1x4 = _mm_mullo_epi16 (src1x4, coef1x4);

        prodSum128 = _mm_add_epi16 (mul0x4, mul1x4);
        prodSum128 = _mm_add_epi16 (prodSum128, roundx4);
        prodSum128 = _mm_srai_epi16 (prodSum128, BILI_SHIFT);

        _mm_storel_epi64 ((__m128i *)(dst + width),  prodSum128);

        src += srcStride;
        dst += dstStride;

    }

}

void Xin266BiliInterpVetS16U8Gt8xH_AVX2 (
    const SINT16 *src,
    intptr_t     srcStride,
    SINT16       *dst,
    intptr_t     dstStride,
    SINT32       frac,
    UINT32       width,
    UINT32       height)
{
    const SINT8 *fltCoeff = g_biliFilterS8[frac>>XIN_MV_FRAC_BITS];
    UINT32 row, col;
    __m256i coef0x16;
    __m256i coef1x16;
    __m256i src0x16;
    __m256i src1x16;
    __m256i mul0x16;
    __m256i mul1x16;
    __m256i prodSum;
    __m256i roundx16;
    __m128i src0x4, src1x4;
    __m128i mul0x4, mul1x4;
    __m128i coef0x4, coef1x4;
    __m128i roundx4;
    __m128i prodSum128;

    src -= (BILI_INTERP_TAPS / 2 - 1) * srcStride;

    roundx16 = _mm256_loadu_si256 ((__m256i *)roundOffset2);
    coef0x16 = _mm256_set1_epi16 (fltCoeff[0]);
    coef1x16 = _mm256_set1_epi16 (fltCoeff[1]);
    coef0x4  = _mm_set1_epi16 (fltCoeff[0]);
    coef1x4  = _mm_set1_epi16 (fltCoeff[1]);
    roundx4  = _mm_loadu_si128 ((__m128i *)roundOffset2);

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col += 16)
        {
            src0x16 = _mm256_loadu_si256 ((__m256i *)(src + col));
            src1x16 = _mm256_loadu_si256 ((__m256i *)(src + col + srcStride));

            mul0x16 = _mm256_mullo_epi16 (src0x16, coef0x16);
            mul1x16 = _mm256_mullo_epi16 (src1x16, coef1x16);

            prodSum = _mm256_add_epi16 (mul0x16, mul1x16);
            prodSum = _mm256_add_epi16 (prodSum, roundx16);
            prodSum  = _mm256_srai_epi16 (prodSum, BILI_SHIFT2);

            _mm256_storeu_si256 ((__m256i *)(dst + col),  prodSum);
        }

        src0x4 = _mm_loadl_epi64 ((__m128i *)(src + width));
        src1x4 = _mm_loadl_epi64 ((__m128i *)(src + width + srcStride));

        mul0x4 = _mm_mullo_epi16 (src0x4, coef0x4);
        mul1x4 = _mm_mullo_epi16 (src1x4, coef1x4);

        prodSum128 = _mm_add_epi16 (mul0x4, mul1x4);
        prodSum128 = _mm_add_epi16 (prodSum128, roundx4);
        prodSum128 = _mm_srai_epi16 (prodSum128, BILI_SHIFT2);

        _mm_storel_epi64 ((__m128i *)(dst + width),  prodSum128);

        src += srcStride;
        dst += dstStride;

    }

}

void  Xin266BiliInterpHorVetGt8xH_AVX2 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height)
{
    SINT16  firstPassDst[(128+16)*(128+12)];

    src -= (BILI_INTERP_TAPS / 2 - 1) * srcStride;

    Xin266BiliInterpHorGt8xH_AVX2 (
        src,
        srcStride,
        firstPassDst,
        width + DMVR_PADING_SIZE*2,
        frac,
        width,
        height + BILI_INTERP_TAPS - 1);

    Xin266BiliInterpVetS16U8Gt8xH_AVX2 (
        firstPassDst + (BILI_INTERP_TAPS / 2 - 1) * (width + DMVR_PADING_SIZE*2),
        width + DMVR_PADING_SIZE*2,
        dst,
        dstStride,
        frac,
        width,
        height);

}


