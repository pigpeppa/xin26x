/***************************************************************************//**
 *
 * @file          h266_luma_mc_avx2.c
 * @brief         h.266 luma motion compensation subroutines (AVX2).
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
#include "immintrin.h"
#include "xin_typedef.h"
#include "h266_motion_comp.h"
#include "h266_inter_pred_context.h"
#include "h26x_definition.h"
#include "h266_constant.h"
#include "basic_macro.h"
#ifdef __linux__
#include "xin_avx2_linux_patch.h"
#endif

static const UINT8 shuffleIndex[32] =
{
    0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8,
};

static const UINT8 shuffleIndex32[32] =
{
    2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10
};

static const UINT8 shuffleIndex54[32] =
{
    4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12,
};

static const UINT8 shuffleIndex76[32] =
{
    6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14
};

static const SINT32 roundOffset2[8] =
{
    INTERP_OFFSET2, INTERP_OFFSET2, INTERP_OFFSET2, INTERP_OFFSET2,
    INTERP_OFFSET2, INTERP_OFFSET2, INTERP_OFFSET2, INTERP_OFFSET2
};

static const SINT16 roundOffset[16] =
{
    INTERP_OFFSET, INTERP_OFFSET, INTERP_OFFSET, INTERP_OFFSET,
    INTERP_OFFSET, INTERP_OFFSET, INTERP_OFFSET, INTERP_OFFSET,
    INTERP_OFFSET, INTERP_OFFSET, INTERP_OFFSET, INTERP_OFFSET,
    INTERP_OFFSET, INTERP_OFFSET, INTERP_OFFSET, INTERP_OFFSET,
};

static const SINT16 roundOffset3[16] =
{
    INTERP_PREC_OFFSET, INTERP_PREC_OFFSET, INTERP_PREC_OFFSET, INTERP_PREC_OFFSET,
    INTERP_PREC_OFFSET, INTERP_PREC_OFFSET, INTERP_PREC_OFFSET, INTERP_PREC_OFFSET,
    INTERP_PREC_OFFSET, INTERP_PREC_OFFSET, INTERP_PREC_OFFSET, INTERP_PREC_OFFSET,
    INTERP_PREC_OFFSET, INTERP_PREC_OFFSET, INTERP_PREC_OFFSET, INTERP_PREC_OFFSET,
};

static const SINT8 g_lumaFilterS8[XIN_INTERP_SUB_POS][LUMA_INTERP_TAPS] =
{
    {  0, 0,   0, 64,  0,   0,  0,  0 },
    {  0, 1,  -3, 63,  4,  -2,  1,  0 },
    { -1, 2,  -5, 62,  8,  -3,  1,  0 },
    { -1, 3,  -8, 60, 13,  -4,  1,  0 },
    { -1, 4, -10, 58, 17,  -5,  1,  0 },
    { -1, 4, -11, 52, 26,  -8,  3, -1 },
    { -1, 3,  -9, 47, 31, -10,  4, -1 },
    { -1, 4, -11, 45, 34, -10,  4, -1 },
    { -1, 4, -11, 40, 40, -11,  4, -1 },
    { -1, 4, -10, 34, 45, -11,  4, -1 },
    { -1, 4, -10, 31, 47,  -9,  3, -1 },
    { -1, 3,  -8, 26, 52, -11,  4, -1 },
    {  0, 1,  -5, 17, 58, -10,  4, -1 },
    {  0, 1,  -4, 13, 60,  -8,  3, -1 },
    {  0, 1,  -3,  8, 62,  -5,  2, -1 },
    {  0, 1,  -2,  4, 63,  -3,  1,  0 }
};

static const SINT16 g_lumaFilterS16[XIN_INTERP_SUB_POS][LUMA_INTERP_TAPS] =
{
    {  0, 0,   0, 64,  0,   0,  0,  0 },
    {  0, 1,  -3, 63,  4,  -2,  1,  0 },
    { -1, 2,  -5, 62,  8,  -3,  1,  0 },
    { -1, 3,  -8, 60, 13,  -4,  1,  0 },
    { -1, 4, -10, 58, 17,  -5,  1,  0 },
    { -1, 4, -11, 52, 26,  -8,  3, -1 },
    { -1, 3,  -9, 47, 31, -10,  4, -1 },
    { -1, 4, -11, 45, 34, -10,  4, -1 },
    { -1, 4, -11, 40, 40, -11,  4, -1 },
    { -1, 4, -10, 34, 45, -11,  4, -1 },
    { -1, 4, -10, 31, 47,  -9,  3, -1 },
    { -1, 3,  -8, 26, 52, -11,  4, -1 },
    {  0, 1,  -5, 17, 58, -10,  4, -1 },
    {  0, 1,  -4, 13, 60,  -8,  3, -1 },
    {  0, 1,  -3,  8, 62,  -5,  2, -1 },
    {  0, 1,  -2,  4, 63,  -3,  1,  0 }
};

static const SINT8 g_lumaFilter4x4S8[XIN_INTERP_SUB_POS][LUMA_INTERP_TAPS] =
{
    {  0, 0,   0, 64,  0,   0,  0,  0 },
    {  0, 1,  -3, 63,  4,  -2,  1,  0 },
    {  0, 1,  -5, 62,  8,  -3,  1,  0 },
    {  0, 2,  -8, 60, 13,  -4,  1,  0 },
    {  0, 3, -10, 58, 17,  -5,  1,  0 }, //1/4
    {  0, 3, -11, 52, 26,  -8,  2,  0 },
    {  0, 2,  -9, 47, 31, -10,  3,  0 },
    {  0, 3, -11, 45, 34, -10,  3,  0 },
    {  0, 3, -11, 40, 40, -11,  3,  0 }, //1/2
    {  0, 3, -10, 34, 45, -11,  3,  0 },
    {  0, 3, -10, 31, 47,  -9,  2,  0 },
    {  0, 2,  -8, 26, 52, -11,  3,  0 },
    {  0, 1,  -5, 17, 58, -10,  3,  0 }, //3/4
    {  0, 1,  -4, 13, 60,  -8,  2,  0 },
    {  0, 1,  -3,  8, 62,  -5,  1,  0 },
    {  0, 1,  -2,  4, 63,  -3,  1,  0 }
};

static const SINT16 g_lumaFilter4x4S16[XIN_INTERP_SUB_POS][LUMA_INTERP_TAPS] =
{
    {  0, 0,   0, 64,  0,   0,  0,  0 },
    {  0, 1,  -3, 63,  4,  -2,  1,  0 },
    {  0, 1,  -5, 62,  8,  -3,  1,  0 },
    {  0, 2,  -8, 60, 13,  -4,  1,  0 },
    {  0, 3, -10, 58, 17,  -5,  1,  0 }, //1/4
    {  0, 3, -11, 52, 26,  -8,  2,  0 },
    {  0, 2,  -9, 47, 31, -10,  3,  0 },
    {  0, 3, -11, 45, 34, -10,  3,  0 },
    {  0, 3, -11, 40, 40, -11,  3,  0 }, //1/2
    {  0, 3, -10, 34, 45, -11,  3,  0 },
    {  0, 3, -10, 31, 47,  -9,  2,  0 },
    {  0, 2,  -8, 26, 52, -11,  3,  0 },
    {  0, 1,  -5, 17, 58, -10,  3,  0 }, //3/4
    {  0, 1,  -4, 13, 60,  -8,  2,  0 },
    {  0, 1,  -3,  8, 62,  -5,  1,  0 },
    {  0, 1,  -2,  4, 63,  -3,  1,  0 }
};

static const SINT8 g_lumaAltHpelIFilterS8[LUMA_INTERP_TAPS] =
{
    0, 3, 9, 20, 20, 9, 3, 0
};

static const SINT16 g_lumaAltHpelIFilterS16[LUMA_INTERP_TAPS] =
{
    0, 3, 9, 20, 20, 9, 3, 0
};

void Xin266InterpCopy32xH_AVX2 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex)
{
    UINT32   rowIdx;
    __m256i  src0x32;
    __m256i  src1x32;
    intptr_t srcStridex2;
    intptr_t dstStridex2;

    (void)width;
    (void)frac;
    (void)filterIndex;

    srcStridex2 = srcStride*2;
    dstStridex2 = dstStride*2;

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        src0x32 = _mm256_lddqu_si256 ((__m256i *)(src));
        src1x32 = _mm256_lddqu_si256 ((__m256i *)(src + srcStride));

        _mm256_storeu_si256 ((__m256i *)(dst),             src0x32);
        _mm256_storeu_si256 ((__m256i *)(dst + dstStride), src1x32);

        src += srcStridex2;
        dst += dstStridex2;
    }

}

void Xin266InterpCopy64xH_AVX2 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex)
{
    UINT32   rowIdx;
    __m256i  src00x32;
    __m256i  src01x32;
    __m256i  src10x32;
    __m256i  src11x32;
    intptr_t srcStridex2;
    intptr_t dstStridex2;

    (void)width;
    (void)frac;
    (void)filterIndex;

    srcStridex2 = srcStride*2;
    dstStridex2 = dstStride*2;

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        src00x32 = _mm256_lddqu_si256 ((__m256i *)(src));
        src01x32 = _mm256_lddqu_si256 ((__m256i *)(src + 32));
        src10x32 = _mm256_lddqu_si256 ((__m256i *)(src + srcStride));
        src11x32 = _mm256_lddqu_si256 ((__m256i *)(src + srcStride + 32));

        _mm256_storeu_si256 ((__m256i *)(dst),                  src00x32);
        _mm256_storeu_si256 ((__m256i *)(dst + 32),             src01x32);
        _mm256_storeu_si256 ((__m256i *)(dst + dstStride),      src10x32);
        _mm256_storeu_si256 ((__m256i *)(dst + dstStride + 32), src11x32);

        src += srcStridex2;
        dst += dstStridex2;
    }

}

void Xin266InterpCopy128xH_AVX2 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex)
{
    UINT32   rowIdx;
    __m256i  src00x32;
    __m256i  src01x32;
    __m256i  src02x32;
    __m256i  src03x32;
    __m256i  src10x32;
    __m256i  src11x32;
    __m256i  src12x32;
    __m256i  src13x32;
    intptr_t srcStridex2;
    intptr_t dstStridex2;

    (void)width;
    (void)frac;
    (void)filterIndex;

    srcStridex2 = srcStride*2;
    dstStridex2 = dstStride*2;

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        src00x32 = _mm256_lddqu_si256 ((__m256i *)(src));
        src01x32 = _mm256_lddqu_si256 ((__m256i *)(src + 32));
        src02x32 = _mm256_lddqu_si256 ((__m256i *)(src + 64));
        src03x32 = _mm256_lddqu_si256 ((__m256i *)(src + 96));
        src10x32 = _mm256_lddqu_si256 ((__m256i *)(src + srcStride));
        src11x32 = _mm256_lddqu_si256 ((__m256i *)(src + srcStride + 32));
        src12x32 = _mm256_lddqu_si256 ((__m256i *)(src + srcStride + 64));
        src13x32 = _mm256_lddqu_si256 ((__m256i *)(src + srcStride + 96));

        _mm256_storeu_si256 ((__m256i *)(dst),                  src00x32);
        _mm256_storeu_si256 ((__m256i *)(dst + 32),             src01x32);
        _mm256_storeu_si256 ((__m256i *)(dst + 64),             src02x32);
        _mm256_storeu_si256 ((__m256i *)(dst + 96),             src03x32);
        _mm256_storeu_si256 ((__m256i *)(dst + dstStride),      src10x32);
        _mm256_storeu_si256 ((__m256i *)(dst + dstStride + 32), src11x32);
        _mm256_storeu_si256 ((__m256i *)(dst + dstStride + 64), src12x32);
        _mm256_storeu_si256 ((__m256i *)(dst + dstStride + 96), src13x32);

        src += srcStridex2;
        dst += dstStridex2;
    }

}

void Xin266LumaInterpVetGt8xHS16S16_AVX2 (
    const SINT16 *src,
    intptr_t     srcStride,
    SINT16       *dst,
    intptr_t     dstStride,
    SINT32       frac,
    UINT32       width,
    UINT32       height,
    UINT32       filterIndex)
{
    const SINT16 *srcRow;
    const SINT16 *fltCoeff;
    SINT16   *dstRow;
    UINT32   row, col;
    __m256i  src0x16, src1x16, src2x16;
    __m256i  src3x16, src4x16, src5x16;
    __m256i  src6x16, src7x16, src8x16;
    __m256i  srcEveLo10x8, srcEveLo32x8, srcEveLo54x8, srcEveLo76x8;
    __m256i  srcEveHi10x8, srcEveHi32x8, srcEveHi54x8, srcEveHi76x8;
    __m256i  srcOddLo10x8, srcOddLo32x8, srcOddLo54x8, srcOddLo76x8;
    __m256i  srcOddHi10x8, srcOddHi32x8, srcOddHi54x8, srcOddHi76x8;
    __m256i  coef10x16, coef32x16, coef54x16, coef76x16;
    __m256i  prod10, prod32, prod54, prod76;
    __m256i  outLox8, outHix8;
    __m256i  prodSum, outx16;
    __m128i  coef8x16;
    __m256i  coef8x162;

    src -= (LUMA_INTERP_TAPS / 2 - 1) * srcStride;
    frac = frac >> XIN_MV_FRAC_BITS;

    if ((filterIndex == XIN_INTERP_ALT_FILTER) && frac == 8)
    {
        fltCoeff = g_lumaAltHpelIFilterS16;
    }
    else if (filterIndex == XIN_INTERP_4x4_FILTER)
    {
        fltCoeff = g_lumaFilter4x4S16[frac];
    }
    else
    {
        fltCoeff = g_lumaFilterS16[frac];
    }

    coef8x16  = _mm_lddqu_si128 ((__m128i *)fltCoeff);
    coef8x162 = _mm256_set_m128i (coef8x16, coef8x16);

    coef10x16 = _mm256_shuffle_epi32 (coef8x162, 0x00);
    coef32x16 = _mm256_shuffle_epi32 (coef8x162, 0x55);
    coef54x16 = _mm256_shuffle_epi32 (coef8x162, 0xAA);
    coef76x16 = _mm256_shuffle_epi32 (coef8x162, 0xFF);

    for (col = 0; col < width; col += 16)
    {
        srcRow   = src + col;
        dstRow   = dst + col;

        src0x16 = _mm256_loadu_si256 ((__m256i *)(srcRow));
        src1x16 = _mm256_loadu_si256 ((__m256i *)(srcRow + srcStride));
        srcRow += srcStride*2;
        src2x16 = _mm256_loadu_si256 ((__m256i *)(srcRow));
        src3x16 = _mm256_loadu_si256 ((__m256i *)(srcRow + srcStride));
        srcRow += srcStride*2;
        src4x16 = _mm256_loadu_si256 ((__m256i *)(srcRow));
        src5x16 = _mm256_loadu_si256 ((__m256i *)(srcRow + srcStride));
        src6x16 = _mm256_loadu_si256 ((__m256i *)(srcRow + srcStride*2));
        srcRow += srcStride*3;

        srcEveLo10x8 = _mm256_unpacklo_epi16 (src0x16, src1x16);
        srcEveLo32x8 = _mm256_unpacklo_epi16 (src2x16, src3x16);
        srcEveLo54x8 = _mm256_unpacklo_epi16 (src4x16, src5x16);

        srcEveHi10x8 = _mm256_unpackhi_epi16 (src0x16, src1x16);
        srcEveHi32x8 = _mm256_unpackhi_epi16 (src2x16, src3x16);
        srcEveHi54x8 = _mm256_unpackhi_epi16 (src4x16, src5x16);

        srcOddLo10x8 = _mm256_unpacklo_epi16 (src1x16, src2x16);
        srcOddLo32x8 = _mm256_unpacklo_epi16 (src3x16, src4x16);
        srcOddLo54x8 = _mm256_unpacklo_epi16 (src5x16, src6x16);

        srcOddHi10x8 = _mm256_unpackhi_epi16 (src1x16, src2x16);
        srcOddHi32x8 = _mm256_unpackhi_epi16 (src3x16, src4x16);
        srcOddHi54x8 = _mm256_unpackhi_epi16 (src5x16, src6x16);

        for (row = 0; row < height; row += 2)
        {
            src7x16  = _mm256_loadu_si256 ((__m256i *)(srcRow));
            src8x16  = _mm256_loadu_si256 ((__m256i *)(srcRow + srcStride));
            srcRow  += srcStride*2;

            prod10  = _mm256_madd_epi16 (srcEveLo10x8, coef10x16);
            prod32  = _mm256_madd_epi16 (srcEveLo32x8, coef32x16);
            prodSum = _mm256_add_epi32 (prod10, prod32);
            prod54  = _mm256_madd_epi16 (srcEveLo54x8, coef54x16);
            prodSum = _mm256_add_epi32 (prodSum, prod54);

            srcEveLo76x8 = _mm256_unpacklo_epi16 (src6x16, src7x16);

            prod76  = _mm256_madd_epi16 (srcEveLo76x8, coef76x16);
            prodSum = _mm256_add_epi32 (prodSum, prod76);
            outLox8 = _mm256_srai_epi32 (prodSum, INTERP_SHIFT);

            srcEveLo10x8 = srcEveLo32x8;
            srcEveLo32x8 = srcEveLo54x8;
            srcEveLo54x8 = srcEveLo76x8;

            prod10  = _mm256_madd_epi16 (srcEveHi10x8, coef10x16);
            prod32  = _mm256_madd_epi16 (srcEveHi32x8, coef32x16);
            prodSum = _mm256_add_epi32 (prod10, prod32);
            prod54  = _mm256_madd_epi16 (srcEveHi54x8, coef54x16);
            prodSum = _mm256_add_epi32 (prodSum, prod54);

            srcEveHi76x8 = _mm256_unpackhi_epi16 (src6x16, src7x16);

            prod76  = _mm256_madd_epi16 (srcEveHi76x8, coef76x16);
            prodSum = _mm256_add_epi32 (prodSum, prod76);
            outHix8 = _mm256_srai_epi32 (prodSum, INTERP_SHIFT);

            srcEveHi10x8 = srcEveHi32x8;
            srcEveHi32x8 = srcEveHi54x8;
            srcEveHi54x8 = srcEveHi76x8;

            outx16 = _mm256_packs_epi32 (outLox8, outHix8);
            _mm256_storeu_si256 ((__m256i *)(dstRow), outx16);

            prod10  = _mm256_madd_epi16 (srcOddLo10x8, coef10x16);
            prod32  = _mm256_madd_epi16 (srcOddLo32x8, coef32x16);
            prodSum = _mm256_add_epi32 (prod10, prod32);
            prod54  = _mm256_madd_epi16 (srcOddLo54x8, coef54x16);
            prodSum = _mm256_add_epi32 (prodSum, prod54);

            srcOddLo76x8 = _mm256_unpacklo_epi16 (src7x16, src8x16);

            prod76  = _mm256_madd_epi16 (srcOddLo76x8, coef76x16);
            prodSum = _mm256_add_epi32 (prodSum, prod76);
            outLox8 = _mm256_srai_epi32 (prodSum, INTERP_SHIFT);

            srcOddLo10x8 = srcOddLo32x8;
            srcOddLo32x8 = srcOddLo54x8;
            srcOddLo54x8 = srcOddLo76x8;

            prod10  = _mm256_madd_epi16 (srcOddHi10x8, coef10x16);
            prod32  = _mm256_madd_epi16 (srcOddHi32x8, coef32x16);
            prodSum = _mm256_add_epi32 (prod10, prod32);
            prod54  = _mm256_madd_epi16 (srcOddHi54x8, coef54x16);
            prodSum = _mm256_add_epi32 (prodSum, prod54);

            srcOddHi76x8 = _mm256_unpackhi_epi16 (src7x16, src8x16);

            prod76  = _mm256_madd_epi16 (srcOddHi76x8, coef76x16);
            prodSum = _mm256_add_epi32 (prodSum, prod76);
            outHix8 = _mm256_srai_epi32 (prodSum, INTERP_SHIFT);

            srcOddHi10x8 = srcOddHi32x8;
            srcOddHi32x8 = srcOddHi54x8;
            srcOddHi54x8 = srcOddHi76x8;

            outx16 = _mm256_packs_epi32 (outLox8, outHix8);
            _mm256_storeu_si256 ((__m256i *)(dstRow + dstStride), outx16);

            dstRow += dstStride*2;
            src6x16 = src8x16;

        }
    }

}

void Xin266LumaInterpHorGt16xH_AVX2 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex)
{
    const SINT8 *fltCoeff;
    UINT32  row;
    UINT32  col;
    __m128i coef8x16;
    __m256i coef8x32;
    __m256i coef10x16, coef32x16;
    __m256i coef54x16, coef76x16;
    __m256i shufIdx;
    __m256i roundx16;
    __m256i srcx32;
    __m256i srcLRx16;
    __m256i src10x16;
    __m256i src32x16;
    __m256i src54x16;
    __m256i src76x16;
    __m256i prod10, prod32;
    __m256i prod54, prod76;
    __m256i prodSum;
    __m128i outLx8, outRx8;
    __m128i out8x16;
    __m256i outx16;

    src -= (LUMA_INTERP_TAPS/2 - 1);
    frac = frac & XIN_MV_FRAC_MASK;

    if ((filterIndex == XIN_INTERP_ALT_FILTER) && frac == 8)
    {
        fltCoeff = g_lumaAltHpelIFilterS8;
    }
    else if (filterIndex == XIN_INTERP_4x4_FILTER)
    {
        fltCoeff = g_lumaFilter4x4S8[frac];
    }
    else
    {
        fltCoeff = g_lumaFilterS8[frac];
    }

    shufIdx  = _mm256_loadu_si256 ((__m256i *)shuffleIndex);
    roundx16 = _mm256_loadu_si256 ((__m256i *)roundOffset);

    coef8x16 = _mm_loadl_epi64 ((__m128i *)fltCoeff);
    coef8x16 = _mm_unpacklo_epi16 (coef8x16, coef8x16);
    coef8x32 = _mm256_setr_m128i (coef8x16, coef8x16);

    coef10x16 = _mm256_shuffle_epi32 (coef8x32, 0x00);
    coef32x16 = _mm256_shuffle_epi32 (coef8x32, 0x55);
    coef54x16 = _mm256_shuffle_epi32 (coef8x32, 0xAA);
    coef76x16 = _mm256_shuffle_epi32 (coef8x32, 0xFF);

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col += 32)
        {
            srcx32   = _mm256_lddqu_si256 ((__m256i *)(src + col));
            srcLRx16 = _mm256_permute4x64_epi64 (srcx32, 0x94);

            src10x16 = _mm256_shuffle_epi8 (srcLRx16, shufIdx);
            srcLRx16 = _mm256_shuffle_epi32 (srcLRx16, 0x39);
            src54x16 = _mm256_shuffle_epi8 (srcLRx16, shufIdx);

            prod10  = _mm256_maddubs_epi16 (src10x16, coef10x16);
            prod54  = _mm256_maddubs_epi16 (src54x16, coef54x16);
            prodSum = _mm256_add_epi16 (prod10, roundx16);
            prodSum = _mm256_add_epi16 (prodSum, prod54);

            srcx32   = _mm256_lddqu_si256 ((__m256i *)(src + col + 2));
            srcLRx16 = _mm256_permute4x64_epi64 (srcx32, 0x94);

            src32x16 = _mm256_shuffle_epi8 (srcLRx16, shufIdx);
            srcLRx16 = _mm256_shuffle_epi32 (srcLRx16, 0x39);
            src76x16 = _mm256_shuffle_epi8 (srcLRx16, shufIdx);

            prod32  = _mm256_maddubs_epi16 (src32x16, coef32x16);
            prod76  = _mm256_maddubs_epi16 (src76x16, coef76x16);

            prodSum = _mm256_add_epi16 (prod32, prodSum);
            prodSum = _mm256_add_epi16 (prodSum, prod76);

            outx16  = _mm256_srai_epi16 (prodSum, INTERP_SHIFT);
            outRx8  = _mm256_extracti128_si256 (outx16, 1);
            outLx8  = _mm256_castsi256_si128  (outx16);
            out8x16 = _mm_packus_epi16 (outLx8, outRx8);

            _mm_storeu_si128 ((__m128i *)(dst + col), out8x16);

            srcx32   = _mm256_lddqu_si256 ((__m256i *)(src + col + 16));
            srcLRx16 = _mm256_permute4x64_epi64 (srcx32, 0x94);

            src10x16 = _mm256_shuffle_epi8 (srcLRx16, shufIdx);
            srcLRx16 = _mm256_shuffle_epi32 (srcLRx16, 0x39);
            src54x16 = _mm256_shuffle_epi8 (srcLRx16, shufIdx);

            prod10  = _mm256_maddubs_epi16 (src10x16, coef10x16);
            prod54  = _mm256_maddubs_epi16 (src54x16, coef54x16);
            prodSum = _mm256_add_epi16 (prod10, roundx16);
            prodSum = _mm256_add_epi16 (prodSum, prod54);

            srcx32   = _mm256_lddqu_si256 ((__m256i *)(src + col + 18));
            srcLRx16 = _mm256_permute4x64_epi64 (srcx32, 0x94);

            src32x16 = _mm256_shuffle_epi8 (srcLRx16, shufIdx);
            srcLRx16 = _mm256_shuffle_epi32 (srcLRx16, 0x39);
            src76x16 = _mm256_shuffle_epi8 (srcLRx16, shufIdx);

            prod32  = _mm256_maddubs_epi16 (src32x16, coef32x16);
            prod76  = _mm256_maddubs_epi16 (src76x16, coef76x16);

            prodSum = _mm256_add_epi16 (prod32, prodSum);
            prodSum = _mm256_add_epi16 (prodSum, prod76);

            outx16  = _mm256_srai_epi16 (prodSum, INTERP_SHIFT);
            outRx8  = _mm256_extracti128_si256 (outx16, 1);
            outLx8  = _mm256_castsi256_si128  (outx16);
            out8x16 = _mm_packus_epi16 (outLx8, outRx8);

            _mm_storeu_si128 ((__m128i *)(dst + col + 16), out8x16);

        }

        src += srcStride;
        dst += dstStride;

    }

}

void Xin266LumaInterpVetGt16xH_AVX2 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex)
{
    const PIXEL *srcRow;
    const SINT8 *fltCoeff;
    UINT32   row;
    UINT32   col;
    PIXEL    *dstRow;
    __m256i  src0x32, src1x32, src2x32;
    __m256i  src3x32, src4x32, src5x32;
    __m256i  src6x32, src7x32, src8x32;
    __m256i  srcEveLo10x16, srcEveLo32x16, srcEveLo54x16, srcEveLo76x16;
    __m256i  srcEveHi10x16, srcEveHi32x16, srcEveHi54x16, srcEveHi76x16;
    __m256i  srcOddLo10x16, srcOddLo32x16, srcOddLo54x16, srcOddLo76x16;
    __m256i  srcOddHi10x16, srcOddHi32x16, srcOddHi54x16, srcOddHi76x16;
    __m256i  coef10x16, coef32x16, coef54x16, coef76x16;
    __m256i  prod10, prod32, prod54, prod76;
    __m256i  prodSum, outLox16, outHix16, outx32;
    __m256i  roundx16;
    __m128i coef8x16;
    __m256i coef8x32;

    src -= (LUMA_INTERP_TAPS / 2 - 1) * srcStride;
    frac = frac >> XIN_MV_FRAC_BITS;

    if ((filterIndex == XIN_INTERP_ALT_FILTER) && frac == 8)
    {
        fltCoeff = g_lumaAltHpelIFilterS8;
    }
    else if (filterIndex == XIN_INTERP_4x4_FILTER)
    {
        fltCoeff = g_lumaFilter4x4S8[frac];
    }
    else
    {
        fltCoeff = g_lumaFilterS8[frac];
    }

    roundx16  = _mm256_set1_epi16 (INTERP_OFFSET);
    coef8x16 = _mm_loadl_epi64 ((__m128i *)fltCoeff);
    coef8x16 = _mm_unpacklo_epi16 (coef8x16, coef8x16);
    coef8x32 = _mm256_setr_m128i (coef8x16, coef8x16);

    coef10x16 = _mm256_shuffle_epi32 (coef8x32, 0x00);
    coef32x16 = _mm256_shuffle_epi32 (coef8x32, 0x55);
    coef54x16 = _mm256_shuffle_epi32 (coef8x32, 0xAA);
    coef76x16 = _mm256_shuffle_epi32 (coef8x32, 0xFF);

    for (col = 0; col < width; col += 32)
    {
        srcRow   = src + col;
        dstRow   = dst + col;

        src0x32  = _mm256_loadu_si256 ((__m256i *) srcRow);
        src1x32  = _mm256_loadu_si256 ((__m256i *)(srcRow + srcStride));
        srcRow  += srcStride*2;
        src2x32  = _mm256_loadu_si256 ((__m256i *) srcRow);
        src3x32  = _mm256_loadu_si256 ((__m256i *)(srcRow + srcStride));
        srcRow  += srcStride*2;
        src4x32  = _mm256_loadu_si256 ((__m256i *) srcRow);
        src5x32  = _mm256_loadu_si256 ((__m256i *)(srcRow + srcStride));
        src6x32  = _mm256_loadu_si256 ((__m256i *)(srcRow + srcStride*2));
        srcRow  += srcStride*3;

        srcEveLo10x16 = _mm256_unpacklo_epi8 (src0x32, src1x32);
        srcEveLo32x16 = _mm256_unpacklo_epi8 (src2x32, src3x32);
        srcEveLo54x16 = _mm256_unpacklo_epi8 (src4x32, src5x32);

        srcOddLo10x16 = _mm256_unpacklo_epi8 (src1x32, src2x32);
        srcOddLo32x16 = _mm256_unpacklo_epi8 (src3x32, src4x32);
        srcOddLo54x16 = _mm256_unpacklo_epi8 (src5x32, src6x32);

        srcEveHi10x16 = _mm256_unpackhi_epi8 (src0x32, src1x32);
        srcEveHi32x16 = _mm256_unpackhi_epi8 (src2x32, src3x32);
        srcEveHi54x16 = _mm256_unpackhi_epi8 (src4x32, src5x32);

        srcOddHi10x16 = _mm256_unpackhi_epi8 (src1x32, src2x32);
        srcOddHi32x16 = _mm256_unpackhi_epi8 (src3x32, src4x32);
        srcOddHi54x16 = _mm256_unpackhi_epi8 (src5x32, src6x32);

        for (row = 0; row < height; row += 2)
        {
            src7x32  = _mm256_loadu_si256 ((__m256i *) srcRow);
            src8x32  = _mm256_loadu_si256 ((__m256i *)(srcRow + srcStride));
            srcRow  += srcStride*2;

            prod10  = _mm256_maddubs_epi16 (srcEveLo10x16, coef10x16);
            prodSum = _mm256_add_epi16 (prod10, roundx16);
            prod32  = _mm256_maddubs_epi16 (srcEveLo32x16, coef32x16);
            prodSum = _mm256_add_epi16 (prodSum, prod32);
            prod54  = _mm256_maddubs_epi16 (srcEveLo54x16, coef54x16);
            prodSum = _mm256_add_epi16 (prodSum, prod54);

            srcEveLo76x16 = _mm256_unpacklo_epi8 (src6x32, src7x32);

            prod76   = _mm256_maddubs_epi16 (srcEveLo76x16, coef76x16);
            prodSum  = _mm256_add_epi16 (prodSum, prod76);
            outLox16 = _mm256_srai_epi16 (prodSum, INTERP_SHIFT);
            outLox16 = _mm256_packus_epi16 (outLox16, outLox16);

            srcEveLo10x16 = srcEveLo32x16;
            srcEveLo32x16 = srcEveLo54x16;
            srcEveLo54x16 = srcEveLo76x16;

            prod10  = _mm256_maddubs_epi16 (srcEveHi10x16, coef10x16);
            prodSum = _mm256_add_epi16 (prod10, roundx16);
            prod32  = _mm256_maddubs_epi16 (srcEveHi32x16, coef32x16);
            prodSum = _mm256_add_epi16 (prodSum, prod32);
            prod54  = _mm256_maddubs_epi16 (srcEveHi54x16, coef54x16);
            prodSum = _mm256_add_epi16 (prodSum, prod54);

            srcEveHi76x16 = _mm256_unpackhi_epi8 (src6x32, src7x32);

            prod76  = _mm256_maddubs_epi16 (srcEveHi76x16, coef76x16);
            prodSum = _mm256_add_epi16 (prodSum, prod76);

            outHix16 = _mm256_srai_epi16 (prodSum, INTERP_SHIFT);
            outHix16 = _mm256_packus_epi16 (outHix16, outHix16);

            srcEveHi10x16 = srcEveHi32x16;
            srcEveHi32x16 = srcEveHi54x16;
            srcEveHi54x16 = srcEveHi76x16;

            outx32  = _mm256_unpacklo_epi64 (outLox16, outHix16);
             _mm256_storeu_si256 ((__m256i *)(dstRow), outx32);

            prod10  = _mm256_maddubs_epi16 (srcOddLo10x16, coef10x16);
            prodSum = _mm256_add_epi16 (prod10, roundx16);
            prod32  = _mm256_maddubs_epi16 (srcOddLo32x16, coef32x16);
            prodSum = _mm256_add_epi16 (prodSum, prod32);
            prod54  = _mm256_maddubs_epi16 (srcOddLo54x16, coef54x16);
            prodSum = _mm256_add_epi16 (prodSum, prod54);

            srcOddLo76x16 = _mm256_unpacklo_epi8 (src7x32, src8x32);

            prod76   = _mm256_maddubs_epi16 (srcOddLo76x16, coef76x16);
            prodSum  = _mm256_add_epi16 (prodSum, prod76);
            outLox16 = _mm256_srai_epi16 (prodSum, INTERP_SHIFT);
            outLox16 = _mm256_packus_epi16 (outLox16, outLox16);

            srcOddLo10x16 = srcOddLo32x16;
            srcOddLo32x16 = srcOddLo54x16;
            srcOddLo54x16 = srcOddLo76x16;

            prod10  = _mm256_maddubs_epi16 (srcOddHi10x16, coef10x16);
            prodSum = _mm256_add_epi16 (prod10, roundx16);
            prod32  = _mm256_maddubs_epi16 (srcOddHi32x16, coef32x16);
            prodSum = _mm256_add_epi16 (prodSum, prod32);
            prod54  = _mm256_maddubs_epi16 (srcOddHi54x16, coef54x16);
            prodSum = _mm256_add_epi16 (prodSum, prod54);

            srcOddHi76x16 = _mm256_unpackhi_epi8 (src7x32, src8x32);

            prod76   = _mm256_maddubs_epi16 (srcOddHi76x16, coef76x16);
            prodSum  = _mm256_add_epi16 (prodSum, prod76);
            outHix16 = _mm256_srai_epi16 (prodSum, INTERP_SHIFT);
            outHix16 = _mm256_packus_epi16 (outHix16, outHix16);

            srcOddHi10x16 = srcOddHi32x16;
            srcOddHi32x16 = srcOddHi54x16;
            srcOddHi54x16 = srcOddHi76x16;

            outx32  = _mm256_unpacklo_epi64 (outLox16, outHix16);
            _mm256_storeu_si256 ((__m256i *)(dstRow + dstStride), outx32);

            dstRow  += dstStride*2;
            src6x32  = src8x32;

        }
    }

}

void Xin266LumaInterpHorGt8xHU8S16_AVX2 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex)
{
    const SINT8 *fltCoeff;
    UINT32   row, col;
    __m128i  coef8x16;
    __m256i  coef8x32;
    __m256i  coef10x16, coef32x16;
    __m256i  coef54x16, coef76x16;
    __m256i  shufIdx10, shufIdx32;
    __m256i  shufIdx54, shufIdx76;
    __m256i  src0x32, src1x32;
    __m256i  srcLR0x16, srcLR1x16;
    __m256i  src010x16, src110x16;
    __m256i  src032x16, src132x16;
    __m256i  src054x16, src154x16;
    __m256i  src076x16, src176x16;
    __m256i  prod010, prod032;
    __m256i  prod054, prod076;
    __m256i  prod110, prod132;
    __m256i  prod154, prod176;
    __m256i  prodSum03210, prodSum07654;
    __m256i  prodSum13210, prodSum17654;
    __m256i  prodSum0, prodSum1;
    __m256i  roundx16;
    intptr_t srcStride4;
    intptr_t dstStride4;

    src -= LUMA_INTERP_TAPS / 2 - 1;
    frac = frac & XIN_MV_FRAC_MASK;

    if ((filterIndex == XIN_INTERP_ALT_FILTER) && frac == 8)
    {
        fltCoeff = g_lumaAltHpelIFilterS8;
    }
    else if (filterIndex == XIN_INTERP_4x4_FILTER)
    {
        fltCoeff = g_lumaFilter4x4S8[frac];
    }
    else
    {
        fltCoeff = g_lumaFilterS8[frac];
    }

    shufIdx10 = _mm256_lddqu_si256 ((__m256i *)shuffleIndex);
    shufIdx32 = _mm256_lddqu_si256 ((__m256i *)shuffleIndex32);
    shufIdx54 = _mm256_lddqu_si256 ((__m256i *)shuffleIndex54);
    shufIdx76 = _mm256_lddqu_si256 ((__m256i *)shuffleIndex76);
    roundx16  = _mm256_lddqu_si256 ((__m256i *)roundOffset3);

    coef8x16 = _mm_loadl_epi64 ((__m128i *)fltCoeff);
    coef8x16 = _mm_unpacklo_epi16 (coef8x16, coef8x16);
    coef8x32 = _mm256_setr_m128i (coef8x16, coef8x16);

    coef10x16 = _mm256_shuffle_epi32 (coef8x32, 0x00);
    coef32x16 = _mm256_shuffle_epi32 (coef8x32, 0x55);
    coef54x16 = _mm256_shuffle_epi32 (coef8x32, 0xAA);
    coef76x16 = _mm256_shuffle_epi32 (coef8x32, 0xFF);

    srcStride4 = srcStride*4;
    dstStride4 = dstStride*4;

    for (row = 0; row < height; row += 4)
    {
        for (col = 0; col < width; col += 16)
        {
            src0x32 = _mm256_lddqu_si256 ((__m256i *)(src + col));
            src1x32 = _mm256_lddqu_si256 ((__m256i *)(src + col + srcStride));

            srcLR0x16 = _mm256_permute4x64_epi64 (src0x32, 0x94);
            srcLR1x16 = _mm256_permute4x64_epi64 (src1x32, 0x94);

            src010x16 = _mm256_shuffle_epi8 (srcLR0x16, shufIdx10);
            src032x16 = _mm256_shuffle_epi8 (srcLR0x16, shufIdx32);
            src054x16 = _mm256_shuffle_epi8 (srcLR0x16, shufIdx54);
            src076x16 = _mm256_shuffle_epi8 (srcLR0x16, shufIdx76);

            src110x16 = _mm256_shuffle_epi8 (srcLR1x16, shufIdx10);
            src132x16 = _mm256_shuffle_epi8 (srcLR1x16, shufIdx32);
            src154x16 = _mm256_shuffle_epi8 (srcLR1x16, shufIdx54);
            src176x16 = _mm256_shuffle_epi8 (srcLR1x16, shufIdx76);

            prod010 = _mm256_maddubs_epi16 (src010x16, coef10x16);
            prod032 = _mm256_maddubs_epi16 (src032x16, coef32x16);
            prod054 = _mm256_maddubs_epi16 (src054x16, coef54x16);
            prod076 = _mm256_maddubs_epi16 (src076x16, coef76x16);

            prod110 = _mm256_maddubs_epi16 (src110x16, coef10x16);
            prod132 = _mm256_maddubs_epi16 (src132x16, coef32x16);
            prod154 = _mm256_maddubs_epi16 (src154x16, coef54x16);
            prod176 = _mm256_maddubs_epi16 (src176x16, coef76x16);

            prodSum03210 = _mm256_add_epi16 (prod032,      prod010);
            prodSum07654 = _mm256_add_epi16 (prod076,      prod054);
            prodSum0     = _mm256_add_epi16 (prodSum07654, prodSum03210);
            prodSum0     = _mm256_sub_epi16 (prodSum0,     roundx16);

            prodSum13210 = _mm256_add_epi16 (prod132,      prod110);
            prodSum17654 = _mm256_add_epi16 (prod176,      prod154);
            prodSum1     = _mm256_add_epi16 (prodSum17654, prodSum13210);
            prodSum1     = _mm256_sub_epi16 (prodSum1,     roundx16);

            _mm256_storeu_si256 ((__m256i *)(dst + col),             prodSum0);
            _mm256_storeu_si256 ((__m256i *)(dst + col + dstStride), prodSum1);

            src0x32 = _mm256_lddqu_si256 ((__m256i *)(src + col + srcStride*2));
            src1x32 = _mm256_lddqu_si256 ((__m256i *)(src + col + srcStride*3));

            srcLR0x16 = _mm256_permute4x64_epi64 (src0x32, 0x94);
            srcLR1x16 = _mm256_permute4x64_epi64 (src1x32, 0x94);

            src010x16 = _mm256_shuffle_epi8 (srcLR0x16, shufIdx10);
            src032x16 = _mm256_shuffle_epi8 (srcLR0x16, shufIdx32);
            src054x16 = _mm256_shuffle_epi8 (srcLR0x16, shufIdx54);
            src076x16 = _mm256_shuffle_epi8 (srcLR0x16, shufIdx76);

            src110x16 = _mm256_shuffle_epi8 (srcLR1x16, shufIdx10);
            src132x16 = _mm256_shuffle_epi8 (srcLR1x16, shufIdx32);
            src154x16 = _mm256_shuffle_epi8 (srcLR1x16, shufIdx54);
            src176x16 = _mm256_shuffle_epi8 (srcLR1x16, shufIdx76);

            prod010 = _mm256_maddubs_epi16 (src010x16, coef10x16);
            prod032 = _mm256_maddubs_epi16 (src032x16, coef32x16);
            prod054 = _mm256_maddubs_epi16 (src054x16, coef54x16);
            prod076 = _mm256_maddubs_epi16 (src076x16, coef76x16);

            prod110 = _mm256_maddubs_epi16 (src110x16, coef10x16);
            prod132 = _mm256_maddubs_epi16 (src132x16, coef32x16);
            prod154 = _mm256_maddubs_epi16 (src154x16, coef54x16);
            prod176 = _mm256_maddubs_epi16 (src176x16, coef76x16);

            prodSum03210 = _mm256_add_epi16 (prod032,      prod010);
            prodSum07654 = _mm256_add_epi16 (prod076,      prod054);
            prodSum0     = _mm256_add_epi16 (prodSum07654, prodSum03210);
            prodSum0     = _mm256_sub_epi16 (prodSum0,     roundx16);

            prodSum13210 = _mm256_add_epi16 (prod132,      prod110);
            prodSum17654 = _mm256_add_epi16 (prod176,      prod154);
            prodSum1     = _mm256_add_epi16 (prodSum17654, prodSum13210);
            prodSum1     = _mm256_sub_epi16 (prodSum1,     roundx16);

            _mm256_storeu_si256 ((__m256i *)(dst + col + dstStride*2), prodSum0);
            _mm256_storeu_si256 ((__m256i *)(dst + col + dstStride*3), prodSum1);

        }

        src += srcStride4;
        dst += dstStride4;

    }

}

void Xin266LumaInterpVetGt8xHS16U8_AVX2 (
    const SINT16 *src,
    intptr_t     srcStride,
    PIXEL        *dst,
    intptr_t     dstStride,
    SINT32       frac,
    UINT32       width,
    UINT32       height,
    UINT32       filterIndex)
{
    const SINT16 *srcRow;
    const SINT16 *fltCoeff;
    PIXEL    *dstRow;
    UINT32   row, col;
    __m256i  src0x16, src1x16, src2x16;
    __m256i  src3x16, src4x16, src5x16;
    __m256i  src6x16, src7x16, src8x16;
    __m256i  srcEveLo10x8, srcEveLo32x8, srcEveLo54x8, srcEveLo76x8;
    __m256i  srcEveHi10x8, srcEveHi32x8, srcEveHi54x8, srcEveHi76x8;
    __m256i  srcOddLo10x8, srcOddLo32x8, srcOddLo54x8, srcOddLo76x8;
    __m256i  srcOddHi10x8, srcOddHi32x8, srcOddHi54x8, srcOddHi76x8;
    __m256i  coef10x8, coef32x8, coef54x8, coef76x8;
    __m256i  prod10, prod32, prod54, prod76;
    __m256i  outLox8, outHix8;
    __m256i  prodSum, outx16;
    __m128i  out0x8, out1x8;
    __m128i  out8x16;
    __m256i  roundx8;
    __m256i  coefx16;
    __m128i  coefx8;

    src -= (LUMA_INTERP_TAPS / 2 - 1) * srcStride;
    frac = frac >> XIN_MV_FRAC_BITS;

    if ((filterIndex == XIN_INTERP_ALT_FILTER) && frac == 8)
    {
        fltCoeff = g_lumaAltHpelIFilterS16;
    }
    else if (filterIndex == XIN_INTERP_4x4_FILTER)
    {
        fltCoeff = g_lumaFilter4x4S16[frac];
    }
    else
    {
        fltCoeff = g_lumaFilterS16[frac];
    }

    roundx8  = _mm256_lddqu_si256 ((__m256i *)roundOffset2);
    coefx8   = _mm_lddqu_si128 ((__m128i *)fltCoeff);
    coefx16  = _mm256_setr_m128i (coefx8, coefx8);

    coef10x8 = _mm256_shuffle_epi32 (coefx16, 0x00);
    coef32x8 = _mm256_shuffle_epi32 (coefx16, 0x55);
    coef54x8 = _mm256_shuffle_epi32 (coefx16, 0xAA);
    coef76x8 = _mm256_shuffle_epi32 (coefx16, 0xFF);

    for (col = 0; col < width; col += 16)
    {
        srcRow   = src + col;
        dstRow   = dst + col;

        src0x16  = _mm256_lddqu_si256 ((__m256i *)(srcRow));
        src1x16  = _mm256_lddqu_si256 ((__m256i *)(srcRow + srcStride));
        srcRow += srcStride*2;
        src2x16  = _mm256_lddqu_si256 ((__m256i *)(srcRow));
        src3x16  = _mm256_lddqu_si256 ((__m256i *)(srcRow + srcStride));
        srcRow += srcStride*2;
        src4x16  = _mm256_lddqu_si256 ((__m256i *)(srcRow));
        src5x16  = _mm256_lddqu_si256 ((__m256i *)(srcRow + srcStride));
        src6x16  = _mm256_lddqu_si256 ((__m256i *)(srcRow + srcStride*2));
        srcRow += srcStride*3;

        srcEveLo10x8 = _mm256_unpacklo_epi16 (src0x16, src1x16);
        srcEveLo32x8 = _mm256_unpacklo_epi16 (src2x16, src3x16);
        srcEveLo54x8 = _mm256_unpacklo_epi16 (src4x16, src5x16);

        srcEveHi10x8 = _mm256_unpackhi_epi16 (src0x16, src1x16);
        srcEveHi32x8 = _mm256_unpackhi_epi16 (src2x16, src3x16);
        srcEveHi54x8 = _mm256_unpackhi_epi16 (src4x16, src5x16);

        srcOddLo10x8 = _mm256_unpacklo_epi16 (src1x16, src2x16);
        srcOddLo32x8 = _mm256_unpacklo_epi16 (src3x16, src4x16);
        srcOddLo54x8 = _mm256_unpacklo_epi16 (src5x16, src6x16);

        srcOddHi10x8 = _mm256_unpackhi_epi16 (src1x16, src2x16);
        srcOddHi32x8 = _mm256_unpackhi_epi16 (src3x16, src4x16);
        srcOddHi54x8 = _mm256_unpackhi_epi16 (src5x16, src6x16);

        for (row = 0; row < height; row += 2)
        {
            src7x16  = _mm256_lddqu_si256 ((__m256i *)(srcRow));
            src8x16  = _mm256_lddqu_si256 ((__m256i *)(srcRow + srcStride));
            srcRow  += srcStride*2;

            prod10  = _mm256_madd_epi16 (srcEveLo10x8, coef10x8);
            prodSum = _mm256_add_epi32 (prod10, roundx8);
            prod32  = _mm256_madd_epi16 (srcEveLo32x8, coef32x8);
            prodSum = _mm256_add_epi32 (prodSum, prod32);
            prod54  = _mm256_madd_epi16 (srcEveLo54x8, coef54x8);
            prodSum = _mm256_add_epi32 (prodSum, prod54);

            srcEveLo76x8 = _mm256_unpacklo_epi16 (src6x16, src7x16);

            prod76  = _mm256_madd_epi16 (srcEveLo76x8, coef76x8);
            prodSum = _mm256_add_epi32 (prodSum, prod76);
            outLox8 = _mm256_srai_epi32 (prodSum, INTERP_SHIFT2);

            srcEveLo10x8 = srcEveLo32x8;
            srcEveLo32x8 = srcEveLo54x8;
            srcEveLo54x8 = srcEveLo76x8;

            prod10  = _mm256_madd_epi16 (srcEveHi10x8, coef10x8);
            prodSum = _mm256_add_epi32 (prod10, roundx8);
            prod32  = _mm256_madd_epi16 (srcEveHi32x8, coef32x8);
            prodSum = _mm256_add_epi32 (prodSum, prod32);
            prod54  = _mm256_madd_epi16 (srcEveHi54x8, coef54x8);
            prodSum = _mm256_add_epi32 (prodSum, prod54);

            srcEveHi76x8 = _mm256_unpackhi_epi16 (src6x16, src7x16);

            prod76  = _mm256_madd_epi16 (srcEveHi76x8, coef76x8);
            prodSum = _mm256_add_epi32 (prodSum, prod76);
            outHix8 = _mm256_srai_epi32 (prodSum, INTERP_SHIFT2);

            srcEveHi10x8 = srcEveHi32x8;
            srcEveHi32x8 = srcEveHi54x8;
            srcEveHi54x8 = srcEveHi76x8;

            outx16  = _mm256_packs_epi32 (outLox8, outHix8);
            out1x8  = _mm256_extracti128_si256 (outx16, 1);
            out0x8  = _mm256_castsi256_si128  (outx16);
            out8x16 = _mm_packus_epi16 (out0x8, out1x8);

            _mm_storeu_si128 ((__m128i *)(dstRow), out8x16);

            prod10  = _mm256_madd_epi16 (srcOddLo10x8, coef10x8);
            prodSum = _mm256_add_epi32 (prod10, roundx8);
            prod32  = _mm256_madd_epi16 (srcOddLo32x8, coef32x8);
            prodSum = _mm256_add_epi32 (prodSum, prod32);
            prod54  = _mm256_madd_epi16 (srcOddLo54x8, coef54x8);
            prodSum = _mm256_add_epi32 (prodSum, prod54);

            srcOddLo76x8 = _mm256_unpacklo_epi16 (src7x16, src8x16);

            prod76  = _mm256_madd_epi16 (srcOddLo76x8, coef76x8);
            prodSum = _mm256_add_epi32 (prodSum, prod76);
            outLox8 = _mm256_srai_epi32 (prodSum, INTERP_SHIFT2);

            srcOddLo10x8 = srcOddLo32x8;
            srcOddLo32x8 = srcOddLo54x8;
            srcOddLo54x8 = srcOddLo76x8;

            prod10  = _mm256_madd_epi16 (srcOddHi10x8, coef10x8);
            prodSum = _mm256_add_epi32 (prod10, roundx8);
            prod32  = _mm256_madd_epi16 (srcOddHi32x8, coef32x8);
            prodSum = _mm256_add_epi32 (prodSum, prod32);
            prod54  = _mm256_madd_epi16 (srcOddHi54x8, coef54x8);
            prodSum = _mm256_add_epi32 (prodSum, prod54);

            srcOddHi76x8 = _mm256_unpackhi_epi16 (src7x16, src8x16);

            prod76  = _mm256_madd_epi16 (srcOddHi76x8, coef76x8);
            prodSum = _mm256_add_epi32 (prodSum, prod76);
            outHix8 = _mm256_srai_epi32 (prodSum, INTERP_SHIFT2);

            srcOddHi10x8 = srcOddHi32x8;
            srcOddHi32x8 = srcOddHi54x8;
            srcOddHi54x8 = srcOddHi76x8;

            outx16  = _mm256_packs_epi32 (outLox8, outHix8);
            out1x8  = _mm256_extracti128_si256 (outx16, 1);
            out0x8  = _mm256_castsi256_si128  (outx16);
            out8x16 = _mm_packus_epi16 (out0x8, out1x8);

            _mm_storeu_si128 ((__m128i *)(dstRow + dstStride), out8x16);

            dstRow += dstStride*2;
            src6x16 = src8x16;

        }

    }

}

void Xin266LumaInterpHorVetGt8xH_AVX2 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex)
{
    SINT16  firstPassDst[128*(128+8)];

    src -= (LUMA_INTERP_TAPS / 2 - 1) * srcStride;

    Xin266LumaInterpHorGt8xHU8S16_AVX2 (
        src,
        srcStride,
        firstPassDst,
        width,
        frac,
        width,
        height + LUMA_INTERP_TAPS - 1,
        filterIndex);

    Xin266LumaInterpVetGt8xHS16U8_AVX2 (
        firstPassDst + (LUMA_INTERP_TAPS / 2 - 1) * width,
        width,
        dst,
        dstStride,
        frac,
        width,
        height,
        filterIndex);

}

void Xin266LumaInterpHorVetGt16xHU8S16_AVX2 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex)
{
    SINT16  firstPassDst[128*(128+8)];

    src -= (LUMA_INTERP_TAPS / 2 - 1) * srcStride;

    Xin266LumaInterpHorGt8xHU8S16_AVX2 (
        src,
        srcStride,
        firstPassDst,
        width,
        frac,
        width,
        height + LUMA_INTERP_TAPS - 1,
        filterIndex);

    Xin266LumaInterpVetGt8xHS16S16_AVX2 (
        firstPassDst + (LUMA_INTERP_TAPS / 2 - 1) * width,
        width,
        dst,
        dstStride,
        frac,
        width,
        height,
        filterIndex);

}

void Xin266InterpAvgGt8xHS16U8_AVX2 (
    const SINT16 *src0,
    intptr_t     src0Stride,
    const SINT16 *src1,
    intptr_t     src1Stride,
    PIXEL        *dst,
    intptr_t     dstStride,
    UINT32       width,
    UINT32       height)
{
    UINT32 rowIdx, colIdx;
    __m256i c256x16;
    __m256i c128x16;
    __m256i src0Tx16, src0Bx16;
    __m256i src1Tx16, src1Bx16;
    __m256i sumTx16, sumBx16;
    __m256i sumx32;
    __m128i dstTx16, dstBx16;
    UINT8   *dstRow;
    const SINT16  *src0Row;
    const SINT16  *src1Row;

    c256x16 = _mm256_set1_epi16 (256);
    c128x16 = _mm256_srli_epi16 (c256x16, 1);

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        for (colIdx = 0; colIdx < width; colIdx += 16)
        {
            src0Row = src0 + colIdx;
            src1Row = src1 + colIdx;
            dstRow  = dst + colIdx;

            src0Tx16 = _mm256_loadu_si256 ((__m256i *)src0Row);
            src0Bx16 = _mm256_loadu_si256 ((__m256i *)(src0Row + src0Stride));

            src1Tx16 = _mm256_loadu_si256 ((__m256i *)src1Row);
            src1Bx16 = _mm256_loadu_si256 ((__m256i *)(src1Row + src1Stride));

            sumTx16 = _mm256_add_epi16 (src0Tx16, src1Tx16);
            sumBx16 = _mm256_add_epi16 (src0Bx16, src1Bx16);

            sumTx16 = _mm256_mulhrs_epi16 (sumTx16, c256x16);
            sumBx16 = _mm256_mulhrs_epi16 (sumBx16, c256x16);

            sumTx16 = _mm256_add_epi16 (sumTx16, c128x16);
            sumBx16 = _mm256_add_epi16 (sumBx16, c128x16);

            sumx32  = _mm256_packus_epi16 (sumTx16, sumBx16);
            sumx32  = _mm256_permute4x64_epi64 (sumx32, 0xD8);

            dstTx16 = _mm256_castsi256_si128 (sumx32);
            dstBx16 = _mm256_extracti128_si256 (sumx32, 1);

            _mm_storeu_si128 ((__m128i *)(dstRow),             dstTx16);
            _mm_storeu_si128 ((__m128i *)(dstRow + dstStride), dstBx16);

        }

        src0 += src0Stride*2;
        src1 += src1Stride*2;
        dst  += dstStride*2;

    }

}

void  Xin266InterpWeightGt8xHS16U8_AVX2 (
    const SINT16 *src0,
    intptr_t     src0Stride,
    const SINT16 *src1,
    intptr_t     src1Stride,
    PIXEL        *dst,
    intptr_t     dstStride,
    UINT32       width,
    UINT32       height,
    SINT32       weightA,
    SINT32       weightB)
{
    __m256i weightABx8;
    __m256i src0Tx16, src0Bx16, src1Tx16, src1Bx16;
    __m256i srcItlT0x8, srcItlT1x8;
    __m256i srcItlB0x8, srcItlB1x8;
    __m256i dstT0x8, dstT1x8, dstB0x8, dstB1x8;
    __m256i offsetx8;
    __m256i dstTx16, dstBx16;
    __m256i outputx32;
    SINT32  shift;
    SINT32  offset;
    UINT32  rowIdx, colIdx;

    shift      = XIN_MAX (2, INTERP_PRECISION - XIN_INTERNAL_BIT_DEPTH) + XIN_BCW_LOG_WGT_BASE;
    offset     = (1 << (shift - 1)) + (INTERP_PREC_OFFSET << XIN_BCW_LOG_WGT_BASE);
    weightABx8 = _mm256_set1_epi32 ((weightB << 16) | (weightA & 0xFFFF));
    offsetx8   = _mm256_set1_epi32 (offset);

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        for (colIdx = 0; colIdx < width; colIdx += 16)
        {
            src0Tx16 = _mm256_loadu_si256 ((__m256i *)(src0 + colIdx));
            src0Bx16 = _mm256_loadu_si256 ((__m256i *)(src0 + colIdx + src0Stride));

            src1Tx16 = _mm256_loadu_si256 ((__m256i *)(src1 + colIdx));
            src1Bx16 = _mm256_loadu_si256 ((__m256i *)(src1 + colIdx + src1Stride));

            srcItlT0x8 = _mm256_unpacklo_epi16 (src0Tx16, src1Tx16);
            srcItlT1x8 = _mm256_unpackhi_epi16 (src0Tx16, src1Tx16);

            srcItlB0x8 = _mm256_unpacklo_epi16 (src0Bx16, src1Bx16);
            srcItlB1x8 = _mm256_unpackhi_epi16 (src0Bx16, src1Bx16);

            dstT0x8 = _mm256_madd_epi16 (srcItlT0x8, weightABx8);
            dstT1x8 = _mm256_madd_epi16 (srcItlT1x8, weightABx8);

            dstB0x8 = _mm256_madd_epi16 (srcItlB0x8, weightABx8);
            dstB1x8 = _mm256_madd_epi16 (srcItlB1x8, weightABx8);

            dstT0x8 = _mm256_add_epi32 (dstT0x8, offsetx8);
            dstT1x8 = _mm256_add_epi32 (dstT1x8, offsetx8);

            dstB0x8 = _mm256_add_epi32 (dstB0x8, offsetx8);
            dstB1x8 = _mm256_add_epi32 (dstB1x8, offsetx8);

            dstT0x8 = _mm256_srai_epi32 (dstT0x8, shift);
            dstT1x8 = _mm256_srai_epi32 (dstT1x8, shift);

            dstB0x8 = _mm256_srai_epi32 (dstB0x8, shift);
            dstB1x8 = _mm256_srai_epi32 (dstB1x8, shift);

            dstTx16 = _mm256_packs_epi32 (dstT0x8, dstT1x8);
            dstBx16 = _mm256_packs_epi32 (dstB0x8, dstB1x8);

            outputx32 = _mm256_packus_epi16 (dstTx16, dstBx16);
            outputx32 = _mm256_permute4x64_epi64 (outputx32, 0xD8);

            _mm_storeu_si128 ((__m128i *)(dst + colIdx),             _mm256_castsi256_si128 (outputx32));
            _mm_storeu_si128 ((__m128i *)(dst + colIdx + dstStride), _mm256_extracti128_si256 (outputx32, 1));
            
        }

        src0 += 2*src0Stride;
        src1 += 2*src1Stride;
        dst  += 2*dstStride;

    }
    
}


