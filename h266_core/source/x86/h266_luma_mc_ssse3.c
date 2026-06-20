/***************************************************************************//**
 *
 * @file          h266_luma_mc_ssse3.c
 * @brief         h.266 luma motion compensation subroutines (SSSE3).
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
#include "h266_motion_comp.h"
#include "h26x_definition.h"
#include "h266_constant.h"
#include "basic_macro.h"

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

static const UINT8 shuffleIndex[16] =
{
    0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8
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

void Xin266InterpCopy4xH_SSSE3 (
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
    UINT32   src0x8;
    UINT32   src1x8;
    intptr_t srcStridex2;
    intptr_t dstStridex2;

    (void)width;
    (void)frac;
    (void)filterIndex;

    srcStridex2 = srcStride*2;
    dstStridex2 = dstStride*2;

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        src0x8 = *((UINT32 *)(src));
        src1x8 = *((UINT32 *)(src + srcStride));

        *((UINT32 *)(dst))             = src0x8;
        *((UINT32 *)(dst + dstStride)) = src1x8;

        src += srcStridex2;
        dst += dstStridex2;
    }

}

void Xin266InterpCopy8xH_SSSE3 (
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
    __m128i  src0x8;
    __m128i  src1x8;
    intptr_t srcStridex2;
    intptr_t dstStridex2;

    (void)width;
    (void)frac;
    (void)filterIndex;

    srcStridex2 = srcStride*2;
    dstStridex2 = dstStride*2;

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        src0x8 = _mm_loadl_epi64 ((__m128i *)(src));
        src1x8 = _mm_loadl_epi64 ((__m128i *)(src + srcStride));

        _mm_storel_epi64 ((__m128i *)(dst),             src0x8);
        _mm_storel_epi64 ((__m128i *)(dst + dstStride), src1x8);

        src += srcStridex2;
        dst += dstStridex2;
    }

}

void Xin266InterpCopy16xH_SSSE3 (
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
    __m128i  src0x16;
    __m128i  src1x16;
    intptr_t srcStridex2;
    intptr_t dstStridex2;

    (void)width;
    (void)frac;
    (void)filterIndex;

    srcStridex2 = srcStride*2;
    dstStridex2 = dstStride*2;

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        src0x16 = _mm_loadu_si128 ((__m128i *)(src));
        src1x16 = _mm_loadu_si128 ((__m128i *)(src + srcStride));

        _mm_storeu_si128 ((__m128i *)(dst),             src0x16);
        _mm_storeu_si128 ((__m128i *)(dst + dstStride), src1x16);

        src += srcStridex2;
        dst += dstStridex2;
    }

}

void Xin266InterpCopy32xH_SSSE3 (
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
    __m128i  src00x16;
    __m128i  src01x16;
    __m128i  src10x16;
    __m128i  src11x16;
    intptr_t srcStridex2;
    intptr_t dstStridex2;

    (void)width;
    (void)frac;
    (void)filterIndex;

    srcStridex2 = srcStride*2;
    dstStridex2 = dstStride*2;

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        src00x16 = _mm_loadu_si128 ((__m128i *)(src));
        src01x16 = _mm_loadu_si128 ((__m128i *)(src + 16));
        src10x16 = _mm_loadu_si128 ((__m128i *)(src + srcStride));
        src11x16 = _mm_loadu_si128 ((__m128i *)(src + 16 + srcStride));

        _mm_storeu_si128 ((__m128i *)(dst),                  src00x16);
        _mm_storeu_si128 ((__m128i *)(dst + 16),             src01x16);
        _mm_storeu_si128 ((__m128i *)(dst + dstStride),      src10x16);
        _mm_storeu_si128 ((__m128i *)(dst + 16 + dstStride), src11x16);

        src += srcStridex2;
        dst += dstStridex2;
    }

}

void Xin266InterpCopy64xH_SSSE3 (
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
    __m128i  src00x16;
    __m128i  src01x16;
    __m128i  src02x16;
    __m128i  src03x16;

    (void)width;
    (void)frac;
    (void)filterIndex;

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        src00x16 = _mm_loadu_si128 ((__m128i *)(src));
        src01x16 = _mm_loadu_si128 ((__m128i *)(src + 16));
        src02x16 = _mm_loadu_si128 ((__m128i *)(src + 32));
        src03x16 = _mm_loadu_si128 ((__m128i *)(src + 48));

        _mm_storeu_si128 ((__m128i *)(dst),      src00x16);
        _mm_storeu_si128 ((__m128i *)(dst + 16), src01x16);
        _mm_storeu_si128 ((__m128i *)(dst + 32), src02x16);
        _mm_storeu_si128 ((__m128i *)(dst + 48), src03x16);

        src += srcStride;
        dst += dstStride;
    }

}

void Xin266LumaInterpHor4xH_SSSE3 (
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
    __m128i coef8x16;
    __m128i coef10x8, coef32x8;
    __m128i coef54x8, coef76x8;
    __m128i shufIdx;
    __m128i roundx8;
    __m128i src8x16;
    __m128i src10x4;
    __m128i src32x4;
    __m128i src54x4;
    __m128i src76x4;
    __m128i prod10, prod32;
    __m128i prod54, prod76;
    __m128i prodSum;
    __m128i outx4;

    (void)width;

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

    shufIdx  = _mm_loadu_si128 ((__m128i *)shuffleIndex);
    roundx8  = _mm_loadu_si128 ((__m128i *)roundOffset);
    coef8x16 = _mm_loadl_epi64 ((__m128i *)fltCoeff);
    coef8x16 = _mm_unpacklo_epi16 (coef8x16, coef8x16);

    coef10x8 = _mm_shuffle_epi32 (coef8x16, 0x00);
    coef32x8 = _mm_shuffle_epi32 (coef8x16, 0x55);
    coef54x8 = _mm_shuffle_epi32 (coef8x16, 0xAA);
    coef76x8 = _mm_shuffle_epi32 (coef8x16, 0xFF);

    for (row = 0; row < height; row++)
    {
        src8x16 = _mm_lddqu_si128 ((__m128i *)src);

        src10x4 = _mm_shuffle_epi8 (src8x16, shufIdx);
        src8x16 = _mm_srli_si128 (src8x16, 2);
        src32x4 = _mm_shuffle_epi8 (src8x16, shufIdx);
        src8x16 = _mm_srli_si128 (src8x16, 2);
        src54x4 = _mm_shuffle_epi8 (src8x16, shufIdx);
        src8x16 = _mm_srli_si128 (src8x16, 2);
        src76x4 = _mm_shuffle_epi8 (src8x16, shufIdx);

        prod10  = _mm_maddubs_epi16 (src10x4, coef10x8);
        prod54  = _mm_maddubs_epi16 (src54x4, coef54x8);

        prodSum = _mm_add_epi16 (prod10, roundx8);
        prodSum = _mm_add_epi16 (prodSum, prod54);

        prod32  = _mm_maddubs_epi16 (src32x4, coef32x8);
        prod76  = _mm_maddubs_epi16 (src76x4, coef76x8);

        prodSum = _mm_add_epi16 (prod32, prodSum);
        prodSum = _mm_add_epi16 (prodSum, prod76);

        outx4  = _mm_srai_epi16 (prodSum, INTERP_SHIFT);
        outx4  = _mm_packus_epi16 (outx4, outx4);

        *((UINT32 *)dst) = _mm_cvtsi128_si32 (outx4);

        src += srcStride;
        dst += dstStride;

    }

}


void Xin266LumaInterpHor8xH_SSSE3 (
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
    __m128i coef8x16;
    __m128i coef10x8, coef32x8;
    __m128i coef54x8, coef76x8;
    __m128i shufIdx;
    __m128i roundx8;
    __m128i src8x16;
    __m128i src10x8;
    __m128i src32x8;
    __m128i src54x8;
    __m128i src76x8;
    __m128i prod10, prod32;
    __m128i prod54, prod76;
    __m128i prodSum;
    __m128i outx8;

    (void)width;

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

    shufIdx  = _mm_loadu_si128 ((__m128i *)shuffleIndex);
    roundx8  = _mm_loadu_si128 ((__m128i *)roundOffset);

    coef8x16 = _mm_loadl_epi64 ((__m128i *)fltCoeff);
    coef8x16 = _mm_unpacklo_epi16 (coef8x16, coef8x16);

    coef10x8 = _mm_shuffle_epi32 (coef8x16, 0x00);
    coef32x8 = _mm_shuffle_epi32 (coef8x16, 0x55);
    coef54x8 = _mm_shuffle_epi32 (coef8x16, 0xAA);
    coef76x8 = _mm_shuffle_epi32 (coef8x16, 0xFF);

    for (row = 0; row < height; row++)
    {
        src8x16 = _mm_lddqu_si128 ((__m128i *)src);
        src10x8 = _mm_shuffle_epi8 (src8x16, shufIdx);
        src8x16 = _mm_shuffle_epi32 (src8x16, 0x39);
        src54x8 = _mm_shuffle_epi8 (src8x16, shufIdx);

        prod10  = _mm_maddubs_epi16 (src10x8, coef10x8);
        prod54  = _mm_maddubs_epi16 (src54x8, coef54x8);

        prodSum = _mm_add_epi16 (prod10, roundx8);
        prodSum = _mm_add_epi16 (prodSum, prod54);

        src8x16 = _mm_lddqu_si128 ((__m128i *)(src + 2));
        src32x8 = _mm_shuffle_epi8 (src8x16, shufIdx);
        src8x16 = _mm_shuffle_epi32 (src8x16, 0x39);
        src76x8 = _mm_shuffle_epi8 (src8x16, shufIdx);

        prod32  = _mm_maddubs_epi16 (src32x8, coef32x8);
        prod76  = _mm_maddubs_epi16 (src76x8, coef76x8);

        prodSum = _mm_add_epi16 (prod32, prodSum);
        prodSum = _mm_add_epi16 (prodSum, prod76);

        outx8  = _mm_srai_epi16 (prodSum, INTERP_SHIFT);
        outx8  = _mm_packus_epi16 (outx8, outx8);

        _mm_storel_epi64 ((__m128i *)dst, outx8);

        src += srcStride;
        dst += dstStride;

    }

}

void Xin266LumaInterpHor16xH_SSSE3 (
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
    __m128i coef8x16;
    __m128i coef10x8, coef32x8;
    __m128i coef54x8, coef76x8;
    __m128i shufIdx;
    __m128i roundx8;
    __m128i src8x16;
    __m128i src10x8;
    __m128i src32x8;
    __m128i src54x8;
    __m128i src76x8;
    __m128i prod10, prod32;
    __m128i prod54, prod76;
    __m128i prodSum;
    __m128i outLx8, outRx8;
    __m128i outx16;

    (void)width;

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

    shufIdx  = _mm_loadu_si128 ((__m128i *)shuffleIndex);
    roundx8  = _mm_loadu_si128 ((__m128i *)roundOffset);

    coef8x16 = _mm_loadl_epi64 ((__m128i *)fltCoeff);
    coef8x16 = _mm_unpacklo_epi16 (coef8x16, coef8x16);

    coef10x8 = _mm_shuffle_epi32 (coef8x16, 0x00);
    coef32x8 = _mm_shuffle_epi32 (coef8x16, 0x55);
    coef54x8 = _mm_shuffle_epi32 (coef8x16, 0xAA);
    coef76x8 = _mm_shuffle_epi32 (coef8x16, 0xFF);

    for (row = 0; row < height; row++)
    {
        src8x16 = _mm_lddqu_si128 ((__m128i *)src);
        src10x8 = _mm_shuffle_epi8 (src8x16, shufIdx);
        src8x16 = _mm_shuffle_epi32 (src8x16, 0x39);
        src54x8 = _mm_shuffle_epi8 (src8x16, shufIdx);

        prod10  = _mm_maddubs_epi16 (src10x8, coef10x8);
        prod54  = _mm_maddubs_epi16 (src54x8, coef54x8);

        prodSum = _mm_add_epi16 (prod10, roundx8);
        prodSum = _mm_add_epi16 (prodSum, prod54);

        src8x16 = _mm_lddqu_si128 ((__m128i *)(src + 2));
        src32x8 = _mm_shuffle_epi8 (src8x16, shufIdx);
        src8x16 = _mm_shuffle_epi32 (src8x16, 0x39);
        src76x8 = _mm_shuffle_epi8 (src8x16, shufIdx);

        prod32  = _mm_maddubs_epi16 (src32x8, coef32x8);
        prod76  = _mm_maddubs_epi16 (src76x8, coef76x8);

        prodSum = _mm_add_epi16 (prod32, prodSum);
        prodSum = _mm_add_epi16 (prodSum, prod76);

        outLx8  = _mm_srai_epi16 (prodSum, INTERP_SHIFT);

        src8x16 = _mm_lddqu_si128 ((__m128i *)(src + 8));
        src10x8 = _mm_shuffle_epi8 (src8x16, shufIdx);
        src8x16 = _mm_shuffle_epi32 (src8x16, 0x39);
        src54x8 = _mm_shuffle_epi8 (src8x16, shufIdx);

        prod10  = _mm_maddubs_epi16 (src10x8, coef10x8);
        prod54  = _mm_maddubs_epi16 (src54x8, coef54x8);

        prodSum = _mm_add_epi16 (prod10, roundx8);
        prodSum = _mm_add_epi16 (prodSum, prod54);

        src8x16 = _mm_lddqu_si128 ((__m128i *)(src + 10));
        src32x8 = _mm_shuffle_epi8 (src8x16, shufIdx);
        src8x16 = _mm_shuffle_epi32 (src8x16, 0x39);
        src76x8 = _mm_shuffle_epi8 (src8x16, shufIdx);

        prod32  = _mm_maddubs_epi16 (src32x8, coef32x8);
        prod76  = _mm_maddubs_epi16 (src76x8, coef76x8);

        prodSum = _mm_add_epi16 (prod32, prodSum);
        prodSum = _mm_add_epi16 (prodSum, prod76);

        outRx8  = _mm_srai_epi16 (prodSum, INTERP_SHIFT);
        outx16  = _mm_packus_epi16 (outLx8, outRx8);

        _mm_storeu_si128 ((__m128i *)(dst), outx16);

        src += srcStride;
        dst += dstStride;

    }

}

void Xin266LumaInterpHorGt16xH_SSSE3 (
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
    __m128i coef10x8, coef32x8;
    __m128i coef54x8, coef76x8;
    __m128i shufIdx;
    __m128i roundx8;
    __m128i src8x16;
    __m128i src10x8;
    __m128i src32x8;
    __m128i src54x8;
    __m128i src76x8;
    __m128i prod10, prod32;
    __m128i prod54, prod76;
    __m128i prodSum;
    __m128i outLx8, outRx8;
    __m128i outx16;

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

    shufIdx  = _mm_loadu_si128 ((__m128i *)shuffleIndex);
    roundx8  = _mm_loadu_si128 ((__m128i *)roundOffset);

    coef8x16 = _mm_loadl_epi64 ((__m128i *)fltCoeff);
    coef8x16 = _mm_unpacklo_epi16 (coef8x16, coef8x16);

    coef10x8 = _mm_shuffle_epi32 (coef8x16, 0x00);
    coef32x8 = _mm_shuffle_epi32 (coef8x16, 0x55);
    coef54x8 = _mm_shuffle_epi32 (coef8x16, 0xAA);
    coef76x8 = _mm_shuffle_epi32 (coef8x16, 0xFF);

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col += 16)
        {
            src8x16 = _mm_lddqu_si128 ((__m128i *)(src + col));
            src10x8 = _mm_shuffle_epi8 (src8x16, shufIdx);
            src8x16 = _mm_shuffle_epi32 (src8x16, 0x39);
            src54x8 = _mm_shuffle_epi8 (src8x16, shufIdx);

            prod10  = _mm_maddubs_epi16 (src10x8, coef10x8);
            prod54  = _mm_maddubs_epi16 (src54x8, coef54x8);

            prodSum = _mm_add_epi16 (prod10, roundx8);
            prodSum = _mm_add_epi16 (prodSum, prod54);

            src8x16 = _mm_lddqu_si128 ((__m128i *)(src + col + 2));
            src32x8 = _mm_shuffle_epi8 (src8x16, shufIdx);
            src8x16 = _mm_shuffle_epi32 (src8x16, 0x39);
            src76x8 = _mm_shuffle_epi8 (src8x16, shufIdx);

            prod32  = _mm_maddubs_epi16 (src32x8, coef32x8);
            prod76  = _mm_maddubs_epi16 (src76x8, coef76x8);

            prodSum = _mm_add_epi16 (prod32, prodSum);
            prodSum = _mm_add_epi16 (prodSum, prod76);

            outLx8  = _mm_srai_epi16 (prodSum, INTERP_SHIFT);

            src8x16 = _mm_lddqu_si128 ((__m128i *)(src + col + 8));
            src10x8 = _mm_shuffle_epi8 (src8x16, shufIdx);
            src8x16 = _mm_shuffle_epi32 (src8x16, 0x39);
            src54x8 = _mm_shuffle_epi8 (src8x16, shufIdx);

            prod10  = _mm_maddubs_epi16 (src10x8, coef10x8);
            prod54  = _mm_maddubs_epi16 (src54x8, coef54x8);

            prodSum = _mm_add_epi16 (prod10, roundx8);
            prodSum = _mm_add_epi16 (prodSum, prod54);

            src8x16 = _mm_lddqu_si128 ((__m128i *)(src + col + 10));
            src32x8 = _mm_shuffle_epi8 (src8x16, shufIdx);
            src8x16 = _mm_shuffle_epi32 (src8x16, 0x39);
            src76x8 = _mm_shuffle_epi8 (src8x16, shufIdx);

            prod32  = _mm_maddubs_epi16 (src32x8, coef32x8);
            prod76  = _mm_maddubs_epi16 (src76x8, coef76x8);

            prodSum = _mm_add_epi16 (prod32, prodSum);
            prodSum = _mm_add_epi16 (prodSum, prod76);

            outRx8  = _mm_srai_epi16 (prodSum, INTERP_SHIFT);
            outx16  = _mm_packus_epi16 (outLx8, outRx8);

            _mm_storeu_si128 ((__m128i *)(dst + col), outx16);

        }

        src += srcStride;
        dst += dstStride;

    }

}

void Xin266LumaInterpVet4xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex)
{
    const SINT16 *fltCoeff;
    UINT32   row;
    __m128i  src0x4, src1x4, src2x4;
    __m128i  src3x4, src4x4, src5x4;
    __m128i  src6x4, src7x4;
    __m128i  coef0x8, coef1x8, coef2x8, coef3x8;
    __m128i  coef4x8, coef5x8, coef6x8, coef7x8;
    __m128i  prod0, prod1, prod2, prod3;
    __m128i  prod4, prod5, prod6, prod7;
    __m128i  prodSum, outx8;
    __m128i  roundx8;
    __m128i  allZero;

    (void)width;

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

    roundx8  = _mm_loadu_si128 ((__m128i *)roundOffset);
    allZero  = _mm_setzero_si128 ();

    coef0x8  = _mm_set1_epi16 (fltCoeff[0]);
    coef1x8  = _mm_set1_epi16 (fltCoeff[1]);
    coef2x8  = _mm_set1_epi16 (fltCoeff[2]);
    coef3x8  = _mm_set1_epi16 (fltCoeff[3]);
    coef4x8  = _mm_set1_epi16 (fltCoeff[4]);
    coef5x8  = _mm_set1_epi16 (fltCoeff[5]);
    coef6x8  = _mm_set1_epi16 (fltCoeff[6]);
    coef7x8  = _mm_set1_epi16 (fltCoeff[7]);

    src0x4  = _mm_cvtsi32_si128 (*((UINT32 *)(src)));
    src1x4  = _mm_cvtsi32_si128 (*((UINT32 *)(src + srcStride)));
    src    += srcStride*2;
    src2x4  = _mm_cvtsi32_si128 (*((UINT32 *)(src)));
    src3x4  = _mm_cvtsi32_si128 (*((UINT32 *)(src + srcStride)));
    src    += srcStride*2;
    src4x4  = _mm_cvtsi32_si128 (*((UINT32 *)(src)));
    src5x4  = _mm_cvtsi32_si128 (*((UINT32 *)(src + srcStride)));
    src6x4  = _mm_cvtsi32_si128 (*((UINT32 *)(src + srcStride*2)));
    src    += srcStride*3;

    src0x4 = _mm_unpacklo_epi8 (src0x4, allZero);
    src1x4 = _mm_unpacklo_epi8 (src1x4, allZero);
    src2x4 = _mm_unpacklo_epi8 (src2x4, allZero);
    src3x4 = _mm_unpacklo_epi8 (src3x4, allZero);
    src4x4 = _mm_unpacklo_epi8 (src4x4, allZero);
    src5x4 = _mm_unpacklo_epi8 (src5x4, allZero);
    src6x4 = _mm_unpacklo_epi8 (src6x4, allZero);

    for (row = 0; row < height; row++)
    {
        src7x4 = _mm_cvtsi32_si128 (*((UINT32 *)(src)));
        src7x4 = _mm_unpacklo_epi8 (src7x4, allZero);

        prod0 = _mm_mullo_epi16 (src0x4, coef0x8);
        prod1 = _mm_mullo_epi16 (src1x4, coef1x8);
        prod2 = _mm_mullo_epi16 (src2x4, coef2x8);
        prod3 = _mm_mullo_epi16 (src3x4, coef3x8);
        prod4 = _mm_mullo_epi16 (src4x4, coef4x8);
        prod5 = _mm_mullo_epi16 (src5x4, coef5x8);
        prod6 = _mm_mullo_epi16 (src6x4, coef6x8);
        prod7 = _mm_mullo_epi16 (src7x4, coef7x8);

        prodSum = _mm_add_epi16 (prod0, roundx8);
        prodSum = _mm_add_epi16 (prod1, prodSum);
        prodSum = _mm_add_epi16 (prod2, prodSum);
        prodSum = _mm_add_epi16 (prod3, prodSum);
        prodSum = _mm_add_epi16 (prod4, prodSum);
        prodSum = _mm_add_epi16 (prod5, prodSum);
        prodSum = _mm_add_epi16 (prod6, prodSum);
        prodSum = _mm_add_epi16 (prod7, prodSum);

        outx8   = _mm_srai_epi16 (prodSum, INTERP_SHIFT);
        outx8   = _mm_packus_epi16 (outx8, outx8);

        *((UINT32 *)dst) = _mm_cvtsi128_si32 (outx8);

        src += srcStride;
        dst += dstStride;

        src0x4 = src1x4;
        src1x4 = src2x4;
        src2x4 = src3x4;
        src3x4 = src4x4;
        src4x4 = src5x4;
        src5x4 = src6x4;
        src6x4 = src7x4;

    }

}

void Xin266LumaInterpVet8xH_SSSE3 (
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
    UINT32   row;
    __m128i  src0x8, src1x8, src2x8;
    __m128i  src3x8, src4x8, src5x8;
    __m128i  src6x8, src7x8, src8x8;
    __m128i  srcEve10x8, srcEve32x8, srcEve54x8, srcEve76x8;
    __m128i  srcOdd10x8, srcOdd32x8, srcOdd54x8, srcOdd76x8;
    __m128i  coef10x8, coef32x8, coef54x8, coef76x8;
    __m128i  prod10, prod32, prod54, prod76;
    __m128i  prodSum, outx8;
    __m128i  roundx8, coef8x16;

    (void)width;

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

    roundx8  = _mm_loadu_si128 ((__m128i *)roundOffset);
    coef8x16 = _mm_loadl_epi64 ((__m128i *)fltCoeff);
    coef8x16 = _mm_unpacklo_epi16 (coef8x16, coef8x16);

    coef10x8 = _mm_shuffle_epi32 (coef8x16, 0x00);
    coef32x8 = _mm_shuffle_epi32 (coef8x16, 0x55);
    coef54x8 = _mm_shuffle_epi32 (coef8x16, 0xAA);
    coef76x8 = _mm_shuffle_epi32 (coef8x16, 0xFF);

    src0x8  = _mm_loadl_epi64 ((__m128i *) src);
    src1x8  = _mm_loadl_epi64 ((__m128i *)(src + srcStride));
    src    += srcStride*2;
    src2x8  = _mm_loadl_epi64 ((__m128i *) src);
    src3x8  = _mm_loadl_epi64 ((__m128i *)(src + srcStride));
    src    += srcStride*2;
    src4x8  = _mm_loadl_epi64 ((__m128i *) src);
    src5x8  = _mm_loadl_epi64 ((__m128i *)(src + srcStride));
    src6x8  = _mm_loadl_epi64 ((__m128i *)(src + srcStride*2));
    src    += srcStride*3;

    srcEve10x8 = _mm_unpacklo_epi8 (src0x8, src1x8);
    srcEve32x8 = _mm_unpacklo_epi8 (src2x8, src3x8);
    srcEve54x8 = _mm_unpacklo_epi8 (src4x8, src5x8);

    srcOdd10x8 = _mm_unpacklo_epi8 (src1x8, src2x8);
    srcOdd32x8 = _mm_unpacklo_epi8 (src3x8, src4x8);
    srcOdd54x8 = _mm_unpacklo_epi8 (src5x8, src6x8);

    for (row = 0; row < height; row += 2)
    {
        src7x8     = _mm_loadl_epi64 ((__m128i *) src);
        src8x8     = _mm_loadl_epi64 ((__m128i *)(src + srcStride));
        src       += srcStride*2;

        prod10  = _mm_maddubs_epi16 (srcEve10x8, coef10x8);
        prodSum = _mm_add_epi16 (prod10, roundx8);
        prod32  = _mm_maddubs_epi16 (srcEve32x8, coef32x8);
        prodSum = _mm_add_epi16 (prodSum, prod32);
        prod54  = _mm_maddubs_epi16 (srcEve54x8, coef54x8);
        prodSum = _mm_add_epi16 (prodSum, prod54);

        srcEve76x8 = _mm_unpacklo_epi8 (src6x8, src7x8);

        prod76  = _mm_maddubs_epi16 (srcEve76x8, coef76x8);
        prodSum = _mm_add_epi16 (prodSum, prod76);
        outx8   = _mm_srai_epi16 (prodSum, INTERP_SHIFT);
        outx8   = _mm_packus_epi16 (outx8, outx8);

        _mm_storel_epi64 ((__m128i *)(dst), outx8);

        srcEve10x8 = srcEve32x8;
        srcEve32x8 = srcEve54x8;
        srcEve54x8 = srcEve76x8;

        prod10  = _mm_maddubs_epi16 (srcOdd10x8, coef10x8);
        prodSum = _mm_add_epi16 (prod10, roundx8);
        prod32  = _mm_maddubs_epi16 (srcOdd32x8, coef32x8);
        prodSum = _mm_add_epi16 (prodSum, prod32);
        prod54  = _mm_maddubs_epi16 (srcOdd54x8, coef54x8);
        prodSum = _mm_add_epi16 (prodSum, prod54);

        srcOdd76x8 = _mm_unpacklo_epi8 (src7x8, src8x8);

        prod76  = _mm_maddubs_epi16 (srcOdd76x8, coef76x8);
        prodSum = _mm_add_epi16 (prodSum, prod76);
        outx8   = _mm_srai_epi16 (prodSum, INTERP_SHIFT);
        outx8   = _mm_packus_epi16 (outx8, outx8);

        _mm_storel_epi64 ((__m128i *)(dst + dstStride), outx8);

        srcOdd10x8 = srcOdd32x8;
        srcOdd32x8 = srcOdd54x8;
        srcOdd54x8 = srcOdd76x8;

        dst       += dstStride*2;

        src6x8     = src8x8;

    }

}

void Xin266LumaInterpVet16xH_SSSE3 (
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
    UINT32   row;
    __m128i  src0x16, src1x16, src2x16;
    __m128i  src3x16, src4x16, src5x16;
    __m128i  src6x16, src7x16, src8x16;
    __m128i  srcEveLo10x8, srcEveLo32x8, srcEveLo54x8, srcEveLo76x8;
    __m128i  srcEveHi10x8, srcEveHi32x8, srcEveHi54x8, srcEveHi76x8;
    __m128i  srcOddLo10x8, srcOddLo32x8, srcOddLo54x8, srcOddLo76x8;
    __m128i  srcOddHi10x8, srcOddHi32x8, srcOddHi54x8, srcOddHi76x8;
    __m128i  coef10x8, coef32x8, coef54x8, coef76x8;
    __m128i  prod10, prod32, prod54, prod76;
    __m128i  prodSum, outHix8, outLox8, outx16;
    __m128i  roundx8, coef8x16;

    (void)width;

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

    roundx8  = _mm_loadu_si128 ((__m128i *)roundOffset);
    coef8x16 = _mm_loadl_epi64 ((__m128i *)fltCoeff);
    coef8x16 = _mm_unpacklo_epi16 (coef8x16, coef8x16);

    coef10x8 = _mm_shuffle_epi32 (coef8x16, 0x00);
    coef32x8 = _mm_shuffle_epi32 (coef8x16, 0x55);
    coef54x8 = _mm_shuffle_epi32 (coef8x16, 0xAA);
    coef76x8 = _mm_shuffle_epi32 (coef8x16, 0xFF);

    src0x16  = _mm_lddqu_si128 ((__m128i *) src);
    src1x16  = _mm_lddqu_si128 ((__m128i *)(src + srcStride));
    src     += srcStride*2;
    src2x16  = _mm_lddqu_si128 ((__m128i *) src);
    src3x16  = _mm_lddqu_si128 ((__m128i *)(src + srcStride));
    src     += srcStride*2;
    src4x16  = _mm_lddqu_si128 ((__m128i *) src);
    src5x16  = _mm_lddqu_si128 ((__m128i *)(src + srcStride));
    src6x16  = _mm_lddqu_si128 ((__m128i *)(src + srcStride*2));
    src     += srcStride*3;

    srcEveLo10x8 = _mm_unpacklo_epi8 (src0x16, src1x16);
    srcEveLo32x8 = _mm_unpacklo_epi8 (src2x16, src3x16);
    srcEveLo54x8 = _mm_unpacklo_epi8 (src4x16, src5x16);

    srcOddLo10x8 = _mm_unpacklo_epi8 (src1x16, src2x16);
    srcOddLo32x8 = _mm_unpacklo_epi8 (src3x16, src4x16);
    srcOddLo54x8 = _mm_unpacklo_epi8 (src5x16, src6x16);

    srcEveHi10x8 = _mm_unpackhi_epi8 (src0x16, src1x16);
    srcEveHi32x8 = _mm_unpackhi_epi8 (src2x16, src3x16);
    srcEveHi54x8 = _mm_unpackhi_epi8 (src4x16, src5x16);

    srcOddHi10x8 = _mm_unpackhi_epi8 (src1x16, src2x16);
    srcOddHi32x8 = _mm_unpackhi_epi8 (src3x16, src4x16);
    srcOddHi54x8 = _mm_unpackhi_epi8 (src5x16, src6x16);

    for (row = 0; row < height; row += 2)
    {
        src7x16  = _mm_lddqu_si128 ((__m128i *) src);
        src8x16  = _mm_lddqu_si128 ((__m128i *)(src + srcStride));
        src     += srcStride*2;

        prod10  = _mm_maddubs_epi16 (srcEveLo10x8, coef10x8);
        prodSum = _mm_add_epi16 (prod10, roundx8);
        prod32  = _mm_maddubs_epi16 (srcEveLo32x8, coef32x8);
        prodSum = _mm_add_epi16 (prodSum, prod32);
        prod54  = _mm_maddubs_epi16 (srcEveLo54x8, coef54x8);
        prodSum = _mm_add_epi16 (prodSum, prod54);

        srcEveLo76x8 = _mm_unpacklo_epi8 (src6x16, src7x16);

        prod76  = _mm_maddubs_epi16 (srcEveLo76x8, coef76x8);
        prodSum = _mm_add_epi16 (prodSum, prod76);
        outLox8 = _mm_srai_epi16 (prodSum, INTERP_SHIFT);
        outLox8 = _mm_packus_epi16 (outLox8, outLox8);

        srcEveLo10x8 = srcEveLo32x8;
        srcEveLo32x8 = srcEveLo54x8;
        srcEveLo54x8 = srcEveLo76x8;

        prod10  = _mm_maddubs_epi16 (srcEveHi10x8, coef10x8);
        prodSum = _mm_add_epi16 (prod10, roundx8);
        prod32  = _mm_maddubs_epi16 (srcEveHi32x8, coef32x8);
        prodSum = _mm_add_epi16 (prodSum, prod32);
        prod54  = _mm_maddubs_epi16 (srcEveHi54x8, coef54x8);
        prodSum = _mm_add_epi16 (prodSum, prod54);

        srcEveHi76x8 = _mm_unpackhi_epi8 (src6x16, src7x16);

        prod76  = _mm_maddubs_epi16 (srcEveHi76x8, coef76x8);
        prodSum = _mm_add_epi16 (prodSum, prod76);
        outHix8 = _mm_srai_epi16 (prodSum, INTERP_SHIFT);
        outHix8 = _mm_packus_epi16 (outHix8, outHix8);

        srcEveHi10x8 = srcEveHi32x8;
        srcEveHi32x8 = srcEveHi54x8;
        srcEveHi54x8 = srcEveHi76x8;

        outx16  = _mm_unpacklo_epi64 (outLox8, outHix8);
        _mm_storeu_si128 ((__m128i *)(dst), outx16);

        prod10  = _mm_maddubs_epi16 (srcOddLo10x8, coef10x8);
        prodSum = _mm_add_epi16 (prod10, roundx8);
        prod32  = _mm_maddubs_epi16 (srcOddLo32x8, coef32x8);
        prodSum = _mm_add_epi16 (prodSum, prod32);
        prod54  = _mm_maddubs_epi16 (srcOddLo54x8, coef54x8);
        prodSum = _mm_add_epi16 (prodSum, prod54);

        srcOddLo76x8 = _mm_unpacklo_epi8 (src7x16, src8x16);

        prod76  = _mm_maddubs_epi16 (srcOddLo76x8, coef76x8);
        prodSum = _mm_add_epi16 (prodSum, prod76);
        outLox8 = _mm_srai_epi16 (prodSum, INTERP_SHIFT);
        outLox8 = _mm_packus_epi16 (outLox8, outLox8);

        srcOddLo10x8 = srcOddLo32x8;
        srcOddLo32x8 = srcOddLo54x8;
        srcOddLo54x8 = srcOddLo76x8;

        prod10  = _mm_maddubs_epi16 (srcOddHi10x8, coef10x8);
        prodSum = _mm_add_epi16 (prod10, roundx8);
        prod32  = _mm_maddubs_epi16 (srcOddHi32x8, coef32x8);
        prodSum = _mm_add_epi16 (prodSum, prod32);
        prod54  = _mm_maddubs_epi16 (srcOddHi54x8, coef54x8);
        prodSum = _mm_add_epi16 (prodSum, prod54);

        srcOddHi76x8 = _mm_unpackhi_epi8 (src7x16, src8x16);

        prod76  = _mm_maddubs_epi16 (srcOddHi76x8, coef76x8);
        prodSum = _mm_add_epi16 (prodSum, prod76);
        outHix8 = _mm_srai_epi16 (prodSum, INTERP_SHIFT);
        outHix8 = _mm_packus_epi16 (outHix8, outHix8);

        srcOddHi10x8 = srcOddHi32x8;
        srcOddHi32x8 = srcOddHi54x8;
        srcOddHi54x8 = srcOddHi76x8;

        outx16  = _mm_unpacklo_epi64 (outLox8, outHix8);
        _mm_storeu_si128 ((__m128i *)(dst + dstStride), outx16);

        dst     += dstStride*2;
        src6x16 = src8x16;

    }

}

void Xin266LumaInterpVetGt16xH_SSSE3 (
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
    __m128i  src0x16, src1x16, src2x16;
    __m128i  src3x16, src4x16, src5x16;
    __m128i  src6x16, src7x16, src8x16;
    __m128i  srcEveLo10x8, srcEveLo32x8, srcEveLo54x8, srcEveLo76x8;
    __m128i  srcEveHi10x8, srcEveHi32x8, srcEveHi54x8, srcEveHi76x8;
    __m128i  srcOddLo10x8, srcOddLo32x8, srcOddLo54x8, srcOddLo76x8;
    __m128i  srcOddHi10x8, srcOddHi32x8, srcOddHi54x8, srcOddHi76x8;
    __m128i  coef10x8, coef32x8, coef54x8, coef76x8;
    __m128i  prod10, prod32, prod54, prod76;
    __m128i  prodSum, outLox8, outHix8, outx16;
    __m128i  roundx8, coef8x16;

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

    roundx8  = _mm_loadu_si128 ((__m128i *)roundOffset);
    coef8x16 = _mm_loadl_epi64 ((__m128i *)fltCoeff);
    coef8x16 = _mm_unpacklo_epi16 (coef8x16, coef8x16);

    coef10x8 = _mm_shuffle_epi32 (coef8x16, 0x00);
    coef32x8 = _mm_shuffle_epi32 (coef8x16, 0x55);
    coef54x8 = _mm_shuffle_epi32 (coef8x16, 0xAA);
    coef76x8 = _mm_shuffle_epi32 (coef8x16, 0xFF);

    for (col = 0; col < width; col += 16)
    {
        srcRow   = src + col;
        dstRow   = dst + col;

        src0x16  = _mm_lddqu_si128 ((__m128i *) srcRow);
        src1x16  = _mm_lddqu_si128 ((__m128i *)(srcRow + srcStride));
        srcRow  += srcStride*2;
        src2x16  = _mm_lddqu_si128 ((__m128i *) srcRow);
        src3x16  = _mm_lddqu_si128 ((__m128i *)(srcRow + srcStride));
        srcRow  += srcStride*2;
        src4x16  = _mm_lddqu_si128 ((__m128i *) srcRow);
        src5x16  = _mm_lddqu_si128 ((__m128i *)(srcRow + srcStride));
        src6x16  = _mm_lddqu_si128 ((__m128i *)(srcRow + srcStride*2));
        srcRow  += srcStride*3;

        srcEveLo10x8 = _mm_unpacklo_epi8 (src0x16, src1x16);
        srcEveLo32x8 = _mm_unpacklo_epi8 (src2x16, src3x16);
        srcEveLo54x8 = _mm_unpacklo_epi8 (src4x16, src5x16);

        srcOddLo10x8 = _mm_unpacklo_epi8 (src1x16, src2x16);
        srcOddLo32x8 = _mm_unpacklo_epi8 (src3x16, src4x16);
        srcOddLo54x8 = _mm_unpacklo_epi8 (src5x16, src6x16);

        srcEveHi10x8 = _mm_unpackhi_epi8 (src0x16, src1x16);
        srcEveHi32x8 = _mm_unpackhi_epi8 (src2x16, src3x16);
        srcEveHi54x8 = _mm_unpackhi_epi8 (src4x16, src5x16);

        srcOddHi10x8 = _mm_unpackhi_epi8 (src1x16, src2x16);
        srcOddHi32x8 = _mm_unpackhi_epi8 (src3x16, src4x16);
        srcOddHi54x8 = _mm_unpackhi_epi8 (src5x16, src6x16);

        for (row = 0; row < height; row += 2)
        {
            src7x16  = _mm_lddqu_si128 ((__m128i *) srcRow);
            src8x16  = _mm_lddqu_si128 ((__m128i *)(srcRow + srcStride));
            srcRow  += srcStride*2;

            prod10  = _mm_maddubs_epi16 (srcEveLo10x8, coef10x8);
            prodSum = _mm_add_epi16 (prod10, roundx8);
            prod32  = _mm_maddubs_epi16 (srcEveLo32x8, coef32x8);
            prodSum = _mm_add_epi16 (prodSum, prod32);
            prod54  = _mm_maddubs_epi16 (srcEveLo54x8, coef54x8);
            prodSum = _mm_add_epi16 (prodSum, prod54);

            srcEveLo76x8 = _mm_unpacklo_epi8 (src6x16, src7x16);

            prod76  = _mm_maddubs_epi16 (srcEveLo76x8, coef76x8);
            prodSum = _mm_add_epi16 (prodSum, prod76);
            outLox8 = _mm_srai_epi16 (prodSum, INTERP_SHIFT);
            outLox8 = _mm_packus_epi16 (outLox8, outLox8);

            srcEveLo10x8 = srcEveLo32x8;
            srcEveLo32x8 = srcEveLo54x8;
            srcEveLo54x8 = srcEveLo76x8;

            prod10  = _mm_maddubs_epi16 (srcEveHi10x8, coef10x8);
            prodSum = _mm_add_epi16 (prod10, roundx8);
            prod32  = _mm_maddubs_epi16 (srcEveHi32x8, coef32x8);
            prodSum = _mm_add_epi16 (prodSum, prod32);
            prod54  = _mm_maddubs_epi16 (srcEveHi54x8, coef54x8);
            prodSum = _mm_add_epi16 (prodSum, prod54);

            srcEveHi76x8 = _mm_unpackhi_epi8 (src6x16, src7x16);

            prod76  = _mm_maddubs_epi16 (srcEveHi76x8, coef76x8);
            prodSum = _mm_add_epi16 (prodSum, prod76);

            outHix8 = _mm_srai_epi16 (prodSum, INTERP_SHIFT);
            outHix8 = _mm_packus_epi16 (outHix8, outHix8);

            srcEveHi10x8 = srcEveHi32x8;
            srcEveHi32x8 = srcEveHi54x8;
            srcEveHi54x8 = srcEveHi76x8;

            outx16  = _mm_unpacklo_epi64 (outLox8, outHix8);
            _mm_storeu_si128 ((__m128i *)(dstRow), outx16);

            prod10  = _mm_maddubs_epi16 (srcOddLo10x8, coef10x8);
            prodSum = _mm_add_epi16 (prod10, roundx8);
            prod32  = _mm_maddubs_epi16 (srcOddLo32x8, coef32x8);
            prodSum = _mm_add_epi16 (prodSum, prod32);
            prod54  = _mm_maddubs_epi16 (srcOddLo54x8, coef54x8);
            prodSum = _mm_add_epi16 (prodSum, prod54);

            srcOddLo76x8 = _mm_unpacklo_epi8 (src7x16, src8x16);

            prod76  = _mm_maddubs_epi16 (srcOddLo76x8, coef76x8);
            prodSum = _mm_add_epi16 (prodSum, prod76);
            outLox8 = _mm_srai_epi16 (prodSum, INTERP_SHIFT);
            outLox8 = _mm_packus_epi16 (outLox8, outLox8);

            srcOddLo10x8 = srcOddLo32x8;
            srcOddLo32x8 = srcOddLo54x8;
            srcOddLo54x8 = srcOddLo76x8;

            prod10  = _mm_maddubs_epi16 (srcOddHi10x8, coef10x8);
            prodSum = _mm_add_epi16 (prod10, roundx8);
            prod32  = _mm_maddubs_epi16 (srcOddHi32x8, coef32x8);
            prodSum = _mm_add_epi16 (prodSum, prod32);
            prod54  = _mm_maddubs_epi16 (srcOddHi54x8, coef54x8);
            prodSum = _mm_add_epi16 (prodSum, prod54);

            srcOddHi76x8 = _mm_unpackhi_epi8 (src7x16, src8x16);

            prod76  = _mm_maddubs_epi16 (srcOddHi76x8, coef76x8);
            prodSum = _mm_add_epi16 (prodSum, prod76);
            outHix8 = _mm_srai_epi16 (prodSum, INTERP_SHIFT);
            outHix8 = _mm_packus_epi16 (outHix8, outHix8);

            srcOddHi10x8 = srcOddHi32x8;
            srcOddHi32x8 = srcOddHi54x8;
            srcOddHi54x8 = srcOddHi76x8;

            outx16  = _mm_unpacklo_epi64 (outLox8, outHix8);
            _mm_storeu_si128 ((__m128i *)(dstRow + dstStride), outx16);

            dstRow  += dstStride*2;
            src6x16  = src8x16;

        }
    }

}

void Xin266LumaInterpHor4xHU8S16_SSSE3 (
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
    UINT32  row;
    __m128i coef8x16;
    __m128i coef10x8, coef32x8;
    __m128i coef54x8, coef76x8;
    __m128i shufIdx;
    __m128i src8x16;
    __m128i src10x4;
    __m128i src32x4;
    __m128i src54x4;
    __m128i src76x4;
    __m128i prod10, prod32;
    __m128i prod54, prod76;
    __m128i prodSum;
    __m128i roundx8;

    (void)width;

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

    shufIdx  = _mm_loadu_si128 ((__m128i *)shuffleIndex);
    roundx8  = _mm_loadu_si128 ((__m128i *)roundOffset3);

    coef8x16 = _mm_loadl_epi64 ((__m128i *)fltCoeff);
    coef8x16 = _mm_unpacklo_epi16 (coef8x16, coef8x16);

    coef10x8 = _mm_shuffle_epi32 (coef8x16, 0x00);
    coef32x8 = _mm_shuffle_epi32 (coef8x16, 0x55);
    coef54x8 = _mm_shuffle_epi32 (coef8x16, 0xAA);
    coef76x8 = _mm_shuffle_epi32 (coef8x16, 0xFF);

    for (row = 0; row < height; row++)
    {
        src8x16 = _mm_lddqu_si128 ((__m128i *)src);

        src10x4 = _mm_shuffle_epi8 (src8x16, shufIdx);
        src8x16 = _mm_srli_si128 (src8x16, 2);
        src32x4 = _mm_shuffle_epi8 (src8x16, shufIdx);
        src8x16 = _mm_srli_si128 (src8x16, 2);
        src54x4 = _mm_shuffle_epi8 (src8x16, shufIdx);
        src8x16 = _mm_srli_si128 (src8x16, 2);
        src76x4 = _mm_shuffle_epi8 (src8x16, shufIdx);

        prod10  = _mm_maddubs_epi16 (src10x4, coef10x8);
        prod54  = _mm_maddubs_epi16 (src54x4, coef54x8);

        prodSum = _mm_sub_epi16 (prod10, roundx8);
        prodSum = _mm_add_epi16 (prodSum, prod54);

        prod32  = _mm_maddubs_epi16 (src32x4, coef32x8);
        prod76  = _mm_maddubs_epi16 (src76x4, coef76x8);

        prodSum = _mm_add_epi16 (prod32, prodSum);
        prodSum = _mm_add_epi16 (prodSum, prod76);

        _mm_storel_epi64 ((__m128i *)dst, prodSum);

        src += srcStride;
        dst += dstStride;

    }

}

void Xin266LumaInterpHor8xHU8S16_SSSE3 (
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
    UINT32  row;
    __m128i coef8x16;
    __m128i coef10x8, coef32x8;
    __m128i coef54x8, coef76x8;
    __m128i shufIdx;
    __m128i src8x16;
    __m128i src10x8;
    __m128i src32x8;
    __m128i src54x8;
    __m128i src76x8;
    __m128i prod10, prod32;
    __m128i prod54, prod76;
    __m128i prodSum;
    __m128i roundx8;

    (void)width;

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

    shufIdx  = _mm_loadu_si128 ((__m128i *)shuffleIndex);
    roundx8  = _mm_loadu_si128 ((__m128i *)roundOffset3);

    coef8x16 = _mm_loadl_epi64 ((__m128i *)fltCoeff);
    coef8x16 = _mm_unpacklo_epi16 (coef8x16, coef8x16);

    coef10x8 = _mm_shuffle_epi32 (coef8x16, 0x00);
    coef32x8 = _mm_shuffle_epi32 (coef8x16, 0x55);
    coef54x8 = _mm_shuffle_epi32 (coef8x16, 0xAA);
    coef76x8 = _mm_shuffle_epi32 (coef8x16, 0xFF);

    for (row = 0; row < height; row++)
    {
        src8x16 = _mm_lddqu_si128 ((__m128i *)src);
        src10x8 = _mm_shuffle_epi8 (src8x16, shufIdx);
        src8x16 = _mm_shuffle_epi32 (src8x16, 0x39);
        src54x8 = _mm_shuffle_epi8 (src8x16, shufIdx);

        prod10  = _mm_maddubs_epi16 (src10x8, coef10x8);
        prod54  = _mm_maddubs_epi16 (src54x8, coef54x8);
        prodSum = _mm_add_epi16 (prod10, prod54);

        src8x16 = _mm_lddqu_si128 ((__m128i *)(src + 2));
        src32x8 = _mm_shuffle_epi8 (src8x16, shufIdx);
        src8x16 = _mm_shuffle_epi32 (src8x16, 0x39);
        src76x8 = _mm_shuffle_epi8 (src8x16, shufIdx);

        prod32  = _mm_maddubs_epi16 (src32x8, coef32x8);
        prod76  = _mm_maddubs_epi16 (src76x8, coef76x8);

        prodSum = _mm_add_epi16 (prod32, prodSum);
        prodSum = _mm_add_epi16 (prodSum, prod76);
        prodSum = _mm_sub_epi16 (prodSum, roundx8);

        _mm_storeu_si128 ((__m128i *)dst, prodSum);

        src += srcStride;
        dst += dstStride;

    }

}

void Xin266LumaInterpHorGt8xHU8S16_SSSE3 (
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
    UINT32  row, col;
    __m128i coef8x16;
    __m128i coef10x8, coef32x8;
    __m128i coef54x8, coef76x8;
    __m128i shufIdx;
    __m128i src8x16;
    __m128i src10x8;
    __m128i src32x8;
    __m128i src54x8;
    __m128i src76x8;
    __m128i prod10, prod32;
    __m128i prod54, prod76;
    __m128i prodSum;
    __m128i roundx8;

    (void)width;

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

    shufIdx  = _mm_loadu_si128 ((__m128i *)shuffleIndex);
    roundx8  = _mm_loadu_si128 ((__m128i *)roundOffset3);

    coef8x16 = _mm_loadl_epi64 ((__m128i *)fltCoeff);
    coef8x16 = _mm_unpacklo_epi16 (coef8x16, coef8x16);

    coef10x8 = _mm_shuffle_epi32 (coef8x16, 0x00);
    coef32x8 = _mm_shuffle_epi32 (coef8x16, 0x55);
    coef54x8 = _mm_shuffle_epi32 (coef8x16, 0xAA);
    coef76x8 = _mm_shuffle_epi32 (coef8x16, 0xFF);

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col += 16)
        {
            src8x16 = _mm_lddqu_si128 ((__m128i *)(src + col));
            src10x8 = _mm_shuffle_epi8 (src8x16, shufIdx);
            src8x16 = _mm_shuffle_epi32 (src8x16, 0x39);
            src54x8 = _mm_shuffle_epi8 (src8x16, shufIdx);

            prod10  = _mm_maddubs_epi16 (src10x8, coef10x8);
            prod54  = _mm_maddubs_epi16 (src54x8, coef54x8);
            prodSum = _mm_add_epi16 (prod10, prod54);

            src8x16 = _mm_lddqu_si128 ((__m128i *)(src + col + 2));
            src32x8 = _mm_shuffle_epi8 (src8x16, shufIdx);
            src8x16 = _mm_shuffle_epi32 (src8x16, 0x39);
            src76x8 = _mm_shuffle_epi8 (src8x16, shufIdx);

            prod32  = _mm_maddubs_epi16 (src32x8, coef32x8);
            prod76  = _mm_maddubs_epi16 (src76x8, coef76x8);

            prodSum = _mm_add_epi16 (prod32, prodSum);
            prodSum = _mm_add_epi16 (prodSum, prod76);
            prodSum = _mm_sub_epi16 (prodSum, roundx8);

            _mm_storeu_si128 ((__m128i *)(dst + col), prodSum);

            src8x16 = _mm_lddqu_si128 ((__m128i *)(src + col + 8));
            src10x8 = _mm_shuffle_epi8 (src8x16, shufIdx);
            src8x16 = _mm_shuffle_epi32 (src8x16, 0x39);
            src54x8 = _mm_shuffle_epi8 (src8x16, shufIdx);

            prod10  = _mm_maddubs_epi16 (src10x8, coef10x8);
            prod54  = _mm_maddubs_epi16 (src54x8, coef54x8);

            prodSum = _mm_add_epi16 (prod10, prod54);

            src8x16 = _mm_lddqu_si128 ((__m128i *)(src + col + 10));
            src32x8 = _mm_shuffle_epi8 (src8x16, shufIdx);
            src8x16 = _mm_shuffle_epi32 (src8x16, 0x39);
            src76x8 = _mm_shuffle_epi8 (src8x16, shufIdx);

            prod32  = _mm_maddubs_epi16 (src32x8, coef32x8);
            prod76  = _mm_maddubs_epi16 (src76x8, coef76x8);

            prodSum = _mm_add_epi16 (prod32, prodSum);
            prodSum = _mm_add_epi16 (prodSum, prod76);
            prodSum = _mm_sub_epi16 (prodSum, roundx8);

            _mm_storeu_si128 ((__m128i *)(dst + col + 8), prodSum);

        }

        src += srcStride;
        dst += dstStride;

    }

}

void Xin266LumaInterpVet4xHS16U8_SSSE3 (
    const SINT16 *src,
    intptr_t     srcStride,
    PIXEL        *dst,
    intptr_t     dstStride,
    SINT32       frac,
    UINT32       width,
    UINT32       height,
    UINT32      filterIndex)
{
    const SINT16 *srcRow;
    const SINT16 *fltCoeff;
    PIXEL    *dstRow;
    UINT32   row;
    __m128i  src0x8, src1x8, src2x8;
    __m128i  src3x8, src4x8, src5x8;
    __m128i  src6x8, src7x8, src8x8;
    __m128i  srcEveLo10x4, srcEveLo32x4, srcEveLo54x4, srcEveLo76x4;
    __m128i  srcEveHi10x4, srcEveHi32x4, srcEveHi54x4, srcEveHi76x4;
    __m128i  srcOddLo10x4, srcOddLo32x4, srcOddLo54x4, srcOddLo76x4;
    __m128i  srcOddHi10x4, srcOddHi32x4, srcOddHi54x4, srcOddHi76x4;
    __m128i  coef10x8, coef32x8, coef54x8, coef76x8;
    __m128i  prod10, prod32, prod54, prod76;
    __m128i  outLox4, outHix4;
    __m128i  prodSum, outx8;
    __m128i  roundx4, coef8x16;

    (void)(width);

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

    roundx4  = _mm_lddqu_si128 ((__m128i *)roundOffset2);
    coef8x16 = _mm_lddqu_si128 ((__m128i *)fltCoeff);

    coef10x8 = _mm_shuffle_epi32 (coef8x16, 0x00);
    coef32x8 = _mm_shuffle_epi32 (coef8x16, 0x55);
    coef54x8 = _mm_shuffle_epi32 (coef8x16, 0xAA);
    coef76x8 = _mm_shuffle_epi32 (coef8x16, 0xFF);

    srcRow   = src;
    dstRow   = dst;

    src0x8  = _mm_loadl_epi64 ((__m128i *)(srcRow));
    src1x8  = _mm_loadl_epi64 ((__m128i *)(srcRow + srcStride));
    srcRow += srcStride*2;
    src2x8  = _mm_loadl_epi64 ((__m128i *)(srcRow));
    src3x8  = _mm_loadl_epi64 ((__m128i *)(srcRow + srcStride));
    srcRow += srcStride*2;
    src4x8  = _mm_loadl_epi64 ((__m128i *)(srcRow));
    src5x8  = _mm_loadl_epi64 ((__m128i *)(srcRow + srcStride));
    src6x8  = _mm_loadl_epi64 ((__m128i *)(srcRow + srcStride*2));
    srcRow += srcStride*3;

    srcEveLo10x4 = _mm_unpacklo_epi16 (src0x8, src1x8);
    srcEveLo32x4 = _mm_unpacklo_epi16 (src2x8, src3x8);
    srcEveLo54x4 = _mm_unpacklo_epi16 (src4x8, src5x8);

    srcEveHi10x4 = _mm_unpackhi_epi16 (src0x8, src1x8);
    srcEveHi32x4 = _mm_unpackhi_epi16 (src2x8, src3x8);
    srcEveHi54x4 = _mm_unpackhi_epi16 (src4x8, src5x8);

    srcOddLo10x4 = _mm_unpacklo_epi16 (src1x8, src2x8);
    srcOddLo32x4 = _mm_unpacklo_epi16 (src3x8, src4x8);
    srcOddLo54x4 = _mm_unpacklo_epi16 (src5x8, src6x8);

    srcOddHi10x4 = _mm_unpackhi_epi16 (src1x8, src2x8);
    srcOddHi32x4 = _mm_unpackhi_epi16 (src3x8, src4x8);
    srcOddHi54x4 = _mm_unpackhi_epi16 (src5x8, src6x8);

    for (row = 0; row < height; row += 2)
    {
        src7x8   = _mm_loadl_epi64 ((__m128i *)(srcRow));
        src8x8   = _mm_loadl_epi64 ((__m128i *)(srcRow + srcStride));
        srcRow  += srcStride*2;

        prod10  = _mm_madd_epi16 (srcEveLo10x4, coef10x8);
        prodSum = _mm_add_epi32 (prod10, roundx4);
        prod32  = _mm_madd_epi16 (srcEveLo32x4, coef32x8);
        prodSum = _mm_add_epi32 (prodSum, prod32);
        prod54  = _mm_madd_epi16 (srcEveLo54x4, coef54x8);
        prodSum = _mm_add_epi32 (prodSum, prod54);

        srcEveLo76x4 = _mm_unpacklo_epi16 (src6x8, src7x8);

        prod76  = _mm_madd_epi16 (srcEveLo76x4, coef76x8);
        prodSum = _mm_add_epi32 (prodSum, prod76);
        outLox4 = _mm_srai_epi32 (prodSum, INTERP_SHIFT2);

        srcEveLo10x4 = srcEveLo32x4;
        srcEveLo32x4 = srcEveLo54x4;
        srcEveLo54x4 = srcEveLo76x4;

        prod10  = _mm_madd_epi16 (srcEveHi10x4, coef10x8);
        prodSum = _mm_add_epi32 (prod10, roundx4);
        prod32  = _mm_madd_epi16 (srcEveHi32x4, coef32x8);
        prodSum = _mm_add_epi32 (prodSum, prod32);
        prod54  = _mm_madd_epi16 (srcEveHi54x4, coef54x8);
        prodSum = _mm_add_epi32 (prodSum, prod54);

        srcEveHi76x4 = _mm_unpackhi_epi16 (src6x8, src7x8);

        prod76  = _mm_madd_epi16 (srcEveHi76x4, coef76x8);
        prodSum = _mm_add_epi32 (prodSum, prod76);
        outHix4 = _mm_srai_epi32 (prodSum, INTERP_SHIFT2);

        srcEveHi10x4 = srcEveHi32x4;
        srcEveHi32x4 = srcEveHi54x4;
        srcEveHi54x4 = srcEveHi76x4;

        outx8 = _mm_packs_epi32 (outLox4, outHix4);
        outx8 = _mm_packus_epi16 (outx8, outx8);

        *((UINT32 *)(dstRow)) = _mm_cvtsi128_si32 (outx8);

        prod10  = _mm_madd_epi16 (srcOddLo10x4, coef10x8);
        prodSum = _mm_add_epi32 (prod10, roundx4);
        prod32  = _mm_madd_epi16 (srcOddLo32x4, coef32x8);
        prodSum = _mm_add_epi32 (prodSum, prod32);
        prod54  = _mm_madd_epi16 (srcOddLo54x4, coef54x8);
        prodSum = _mm_add_epi32 (prodSum, prod54);

        srcOddLo76x4 = _mm_unpacklo_epi16 (src7x8, src8x8);

        prod76  = _mm_madd_epi16 (srcOddLo76x4, coef76x8);
        prodSum = _mm_add_epi32 (prodSum, prod76);
        outLox4 = _mm_srai_epi32 (prodSum, INTERP_SHIFT2);

        srcOddLo10x4 = srcOddLo32x4;
        srcOddLo32x4 = srcOddLo54x4;
        srcOddLo54x4 = srcOddLo76x4;

        prod10  = _mm_madd_epi16 (srcOddHi10x4, coef10x8);
        prodSum = _mm_add_epi32 (prod10, roundx4);
        prod32  = _mm_madd_epi16 (srcOddHi32x4, coef32x8);
        prodSum = _mm_add_epi32 (prodSum, prod32);
        prod54  = _mm_madd_epi16 (srcOddHi54x4, coef54x8);
        prodSum = _mm_add_epi32 (prodSum, prod54);

        srcOddHi76x4 = _mm_unpackhi_epi16 (src7x8, src8x8);

        prod76  = _mm_madd_epi16 (srcOddHi76x4, coef76x8);
        prodSum = _mm_add_epi32 (prodSum, prod76);
        outHix4 = _mm_srai_epi32 (prodSum, INTERP_SHIFT2);

        srcOddHi10x4 = srcOddHi32x4;
        srcOddHi32x4 = srcOddHi54x4;
        srcOddHi54x4 = srcOddHi76x4;

        outx8 = _mm_packs_epi32 (outLox4, outHix4);
        outx8 = _mm_packus_epi16 (outx8, outx8);
        *((UINT32 *)(dstRow + dstStride)) = _mm_cvtsi128_si32 (outx8);

        dstRow += dstStride*2;
        src6x8  = src8x8;

    }

}

void Xin266LumaInterpVetS16U8_SSSE3 (
    const SINT16 *src,
    intptr_t     srcStride,
    PIXEL        *dst,
    intptr_t     dstStride,
    SINT32       frac,
    UINT32       width,
    UINT32       height,
    UINT32      filterIndex)
{
    const SINT16 *srcRow;
    const SINT16 *fltCoeff;
    PIXEL    *dstRow;
    UINT32   row, col;
    __m128i  src0x8, src1x8, src2x8;
    __m128i  src3x8, src4x8, src5x8;
    __m128i  src6x8, src7x8, src8x8;
    __m128i  srcEveLo10x4, srcEveLo32x4, srcEveLo54x4, srcEveLo76x4;
    __m128i  srcEveHi10x4, srcEveHi32x4, srcEveHi54x4, srcEveHi76x4;
    __m128i  srcOddLo10x4, srcOddLo32x4, srcOddLo54x4, srcOddLo76x4;
    __m128i  srcOddHi10x4, srcOddHi32x4, srcOddHi54x4, srcOddHi76x4;
    __m128i  coef10x8, coef32x8, coef54x8, coef76x8;
    __m128i  prod10, prod32, prod54, prod76;
    __m128i  outLox4, outHix4;
    __m128i  prodSum, outx8;
    __m128i  roundx4, coef8x16;

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

    roundx4  = _mm_lddqu_si128 ((__m128i *)roundOffset2);
    coef8x16 = _mm_lddqu_si128 ((__m128i *)fltCoeff);

    coef10x8 = _mm_shuffle_epi32 (coef8x16, 0x00);
    coef32x8 = _mm_shuffle_epi32 (coef8x16, 0x55);
    coef54x8 = _mm_shuffle_epi32 (coef8x16, 0xAA);
    coef76x8 = _mm_shuffle_epi32 (coef8x16, 0xFF);

    for (col = 0; col < width; col += 8)
    {
        srcRow   = src + col;
        dstRow   = dst + col;

        src0x8  = _mm_lddqu_si128 ((__m128i *)(srcRow));
        src1x8  = _mm_lddqu_si128 ((__m128i *)(srcRow + srcStride));
        srcRow += srcStride*2;
        src2x8  = _mm_lddqu_si128 ((__m128i *)(srcRow));
        src3x8  = _mm_lddqu_si128 ((__m128i *)(srcRow + srcStride));
        srcRow += srcStride*2;
        src4x8  = _mm_lddqu_si128 ((__m128i *)(srcRow));
        src5x8  = _mm_lddqu_si128 ((__m128i *)(srcRow + srcStride));
        src6x8  = _mm_lddqu_si128 ((__m128i *)(srcRow + srcStride*2));
        srcRow += srcStride*3;

        srcEveLo10x4 = _mm_unpacklo_epi16 (src0x8, src1x8);
        srcEveLo32x4 = _mm_unpacklo_epi16 (src2x8, src3x8);
        srcEveLo54x4 = _mm_unpacklo_epi16 (src4x8, src5x8);

        srcEveHi10x4 = _mm_unpackhi_epi16 (src0x8, src1x8);
        srcEveHi32x4 = _mm_unpackhi_epi16 (src2x8, src3x8);
        srcEveHi54x4 = _mm_unpackhi_epi16 (src4x8, src5x8);

        srcOddLo10x4 = _mm_unpacklo_epi16 (src1x8, src2x8);
        srcOddLo32x4 = _mm_unpacklo_epi16 (src3x8, src4x8);
        srcOddLo54x4 = _mm_unpacklo_epi16 (src5x8, src6x8);

        srcOddHi10x4 = _mm_unpackhi_epi16 (src1x8, src2x8);
        srcOddHi32x4 = _mm_unpackhi_epi16 (src3x8, src4x8);
        srcOddHi54x4 = _mm_unpackhi_epi16 (src5x8, src6x8);

        for (row = 0; row < height; row += 2)
        {
            src7x8   = _mm_lddqu_si128 ((__m128i *)(srcRow));
            src8x8   = _mm_lddqu_si128 ((__m128i *)(srcRow + srcStride));
            srcRow  += srcStride*2;

            prod10  = _mm_madd_epi16 (srcEveLo10x4, coef10x8);
            prodSum = _mm_add_epi32 (prod10, roundx4);
            prod32  = _mm_madd_epi16 (srcEveLo32x4, coef32x8);
            prodSum = _mm_add_epi32 (prodSum, prod32);
            prod54  = _mm_madd_epi16 (srcEveLo54x4, coef54x8);
            prodSum = _mm_add_epi32 (prodSum, prod54);

            srcEveLo76x4 = _mm_unpacklo_epi16 (src6x8, src7x8);

            prod76  = _mm_madd_epi16 (srcEveLo76x4, coef76x8);
            prodSum = _mm_add_epi32 (prodSum, prod76);
            outLox4 = _mm_srai_epi32 (prodSum, INTERP_SHIFT2);

            srcEveLo10x4 = srcEveLo32x4;
            srcEveLo32x4 = srcEveLo54x4;
            srcEveLo54x4 = srcEveLo76x4;

            prod10  = _mm_madd_epi16 (srcEveHi10x4, coef10x8);
            prodSum = _mm_add_epi32 (prod10, roundx4);
            prod32  = _mm_madd_epi16 (srcEveHi32x4, coef32x8);
            prodSum = _mm_add_epi32 (prodSum, prod32);
            prod54  = _mm_madd_epi16 (srcEveHi54x4, coef54x8);
            prodSum = _mm_add_epi32 (prodSum, prod54);

            srcEveHi76x4 = _mm_unpackhi_epi16 (src6x8, src7x8);

            prod76  = _mm_madd_epi16 (srcEveHi76x4, coef76x8);
            prodSum = _mm_add_epi32 (prodSum, prod76);
            outHix4 = _mm_srai_epi32 (prodSum, INTERP_SHIFT2);

            srcEveHi10x4 = srcEveHi32x4;
            srcEveHi32x4 = srcEveHi54x4;
            srcEveHi54x4 = srcEveHi76x4;

            outx8 = _mm_packs_epi32 (outLox4, outHix4);
            outx8 = _mm_packus_epi16 (outx8, outx8);
            _mm_storel_epi64 ((__m128i *)(dstRow), outx8);

            prod10  = _mm_madd_epi16 (srcOddLo10x4, coef10x8);
            prodSum = _mm_add_epi32 (prod10, roundx4);
            prod32  = _mm_madd_epi16 (srcOddLo32x4, coef32x8);
            prodSum = _mm_add_epi32 (prodSum, prod32);
            prod54  = _mm_madd_epi16 (srcOddLo54x4, coef54x8);
            prodSum = _mm_add_epi32 (prodSum, prod54);

            srcOddLo76x4 = _mm_unpacklo_epi16 (src7x8, src8x8);

            prod76  = _mm_madd_epi16 (srcOddLo76x4, coef76x8);
            prodSum = _mm_add_epi32 (prodSum, prod76);
            outLox4 = _mm_srai_epi32 (prodSum, INTERP_SHIFT2);

            srcOddLo10x4 = srcOddLo32x4;
            srcOddLo32x4 = srcOddLo54x4;
            srcOddLo54x4 = srcOddLo76x4;

            prod10  = _mm_madd_epi16 (srcOddHi10x4, coef10x8);
            prodSum = _mm_add_epi32 (prod10, roundx4);
            prod32  = _mm_madd_epi16 (srcOddHi32x4, coef32x8);
            prodSum = _mm_add_epi32 (prodSum, prod32);
            prod54  = _mm_madd_epi16 (srcOddHi54x4, coef54x8);
            prodSum = _mm_add_epi32 (prodSum, prod54);

            srcOddHi76x4 = _mm_unpackhi_epi16 (src7x8, src8x8);

            prod76  = _mm_madd_epi16 (srcOddHi76x4, coef76x8);
            prodSum = _mm_add_epi32 (prodSum, prod76);
            outHix4 = _mm_srai_epi32 (prodSum, INTERP_SHIFT2);

            srcOddHi10x4 = srcOddHi32x4;
            srcOddHi32x4 = srcOddHi54x4;
            srcOddHi54x4 = srcOddHi76x4;

            outx8 = _mm_packs_epi32 (outLox4, outHix4);
            outx8 = _mm_packus_epi16 (outx8, outx8);
            _mm_storel_epi64((__m128i *)(dstRow + dstStride), outx8);

            dstRow += dstStride*2;
            src6x8  = src8x8;

        }
    }

}

void Xin266LumaInterpHorVet4xH_SSSE3 (
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

    Xin266LumaInterpHor4xHU8S16_SSSE3 (
        src,
        srcStride,
        firstPassDst,
        width,
        frac,
        width,
        height + LUMA_INTERP_TAPS - 1,
        filterIndex);

    Xin266LumaInterpVet4xHS16U8_SSSE3 (
        firstPassDst + (LUMA_INTERP_TAPS / 2 - 1) * width,
        width,
        dst,
        dstStride,
        frac,
        width,
        height,
        filterIndex);

}

void Xin266LumaInterpHorVet8xH_SSSE3 (
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

    Xin266LumaInterpHor8xHU8S16_SSSE3 (
        src,
        srcStride,
        firstPassDst,
        width,
        frac,
        width,
        height + LUMA_INTERP_TAPS - 1,
        filterIndex);

    Xin266LumaInterpVetS16U8_SSSE3 (
        firstPassDst + (LUMA_INTERP_TAPS / 2 - 1) * width,
        width,
        dst,
        dstStride,
        frac,
        width,
        height,
        filterIndex);

}

void Xin266LumaInterpHorVetGt8xH_SSSE3 (
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

    Xin266LumaInterpHorGt8xHU8S16_SSSE3 (
        src,
        srcStride,
        firstPassDst,
        width,
        frac,
        width,
        height + LUMA_INTERP_TAPS - 1,
        filterIndex);

    Xin266LumaInterpVetS16U8_SSSE3 (
        firstPassDst + (LUMA_INTERP_TAPS / 2 - 1) * width,
        width,
        dst,
        dstStride,
        frac,
        width,
        height,
        filterIndex);

}

void Xin266LumaInterpVet4xHU8S16_SSSE3 (
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
    UINT32   row;
    __m128i  src0x4, src1x4, src2x4;
    __m128i  src3x4, src4x4, src5x4;
    __m128i  src6x4, src7x4, src8x4;
    __m128i  srcEve10x8, srcEve32x8, srcEve54x8, srcEve76x8;
    __m128i  srcOdd10x8, srcOdd32x8, srcOdd54x8, srcOdd76x8;
    __m128i  coef10x8, coef32x8, coef54x8, coef76x8;
    __m128i  prod10, prod32, prod54, prod76;
    __m128i  prodSum;
    __m128i  roundx8, coef8x16;

    (void)width;

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

    roundx8  = _mm_loadu_si128 ((__m128i *)roundOffset3);
    coef8x16 = _mm_loadl_epi64 ((__m128i *)fltCoeff);
    coef8x16 = _mm_unpacklo_epi16 (coef8x16, coef8x16);

    coef10x8 = _mm_shuffle_epi32 (coef8x16, 0x00);
    coef32x8 = _mm_shuffle_epi32 (coef8x16, 0x55);
    coef54x8 = _mm_shuffle_epi32 (coef8x16, 0xAA);
    coef76x8 = _mm_shuffle_epi32 (coef8x16, 0xFF);

    src0x4  = _mm_cvtsi32_si128 (*((UINT32 *)(src)));
    src1x4  = _mm_cvtsi32_si128 (*((UINT32 *)(src + srcStride)));
    src    += srcStride*2;

    src2x4  = _mm_cvtsi32_si128 (*((UINT32 *)(src)));
    src3x4  = _mm_cvtsi32_si128 (*((UINT32 *)(src + srcStride)));
    src    += srcStride*2;

    src4x4  = _mm_cvtsi32_si128 (*((UINT32 *)(src)));
    src5x4  = _mm_cvtsi32_si128 (*((UINT32 *)(src + srcStride)));
    src6x4  = _mm_cvtsi32_si128 (*((UINT32 *)(src + srcStride*2)));
    src    += srcStride*3;

    srcEve10x8 = _mm_unpacklo_epi8 (src0x4, src1x4);
    srcEve32x8 = _mm_unpacklo_epi8 (src2x4, src3x4);
    srcEve54x8 = _mm_unpacklo_epi8 (src4x4, src5x4);

    srcOdd10x8 = _mm_unpacklo_epi8 (src1x4, src2x4);
    srcOdd32x8 = _mm_unpacklo_epi8 (src3x4, src4x4);
    srcOdd54x8 = _mm_unpacklo_epi8 (src5x4, src6x4);

    for (row = 0; row < height; row += 2)
    {
        src7x4     = _mm_cvtsi32_si128 (*((UINT32 *)(src)));
        src8x4     = _mm_cvtsi32_si128 (*((UINT32 *)(src + srcStride)));
        src       += srcStride*2;

        prod10  = _mm_maddubs_epi16 (srcEve10x8, coef10x8);
        prodSum = _mm_sub_epi16 (prod10, roundx8);
        prod32  = _mm_maddubs_epi16 (srcEve32x8, coef32x8);
        prodSum = _mm_add_epi16 (prodSum, prod32);
        prod54  = _mm_maddubs_epi16 (srcEve54x8, coef54x8);
        prodSum = _mm_add_epi16 (prodSum, prod54);

        srcEve76x8 = _mm_unpacklo_epi8 (src6x4, src7x4);

        prod76  = _mm_maddubs_epi16 (srcEve76x8, coef76x8);
        prodSum = _mm_add_epi16 (prodSum, prod76);

        _mm_storel_epi64 ((__m128i *)(dst), prodSum);

        srcEve10x8 = srcEve32x8;
        srcEve32x8 = srcEve54x8;
        srcEve54x8 = srcEve76x8;

        prod10  = _mm_maddubs_epi16 (srcOdd10x8, coef10x8);
        prodSum = _mm_sub_epi16 (prod10, roundx8);
        prod32  = _mm_maddubs_epi16 (srcOdd32x8, coef32x8);
        prodSum = _mm_add_epi16 (prodSum, prod32);
        prod54  = _mm_maddubs_epi16 (srcOdd54x8, coef54x8);
        prodSum = _mm_add_epi16 (prodSum, prod54);

        srcOdd76x8 = _mm_unpacklo_epi8 (src7x4, src8x4);

        prod76  = _mm_maddubs_epi16 (srcOdd76x8, coef76x8);
        prodSum = _mm_add_epi16 (prodSum, prod76);

        _mm_storel_epi64 ((__m128i *)(dst + dstStride), prodSum);

        srcOdd10x8 = srcOdd32x8;
        srcOdd32x8 = srcOdd54x8;
        srcOdd54x8 = srcOdd76x8;

        dst       += dstStride*2;

        src6x4     = src8x4;

    }

}

void Xin266LumaInterpVet8xHU8S16_SSSE3 (
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
    UINT32   row;
    __m128i  src0x8, src1x8, src2x8;
    __m128i  src3x8, src4x8, src5x8;
    __m128i  src6x8, src7x8, src8x8;
    __m128i  srcEve10x8, srcEve32x8, srcEve54x8, srcEve76x8;
    __m128i  srcOdd10x8, srcOdd32x8, srcOdd54x8, srcOdd76x8;
    __m128i  coef10x8, coef32x8, coef54x8, coef76x8;
    __m128i  prod10, prod32, prod54, prod76;
    __m128i  prodSum;
    __m128i  roundx8, coef8x16;

    (void)width;

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

    roundx8  = _mm_loadu_si128 ((__m128i *)roundOffset3);
    coef8x16 = _mm_loadl_epi64 ((__m128i *)fltCoeff);
    coef8x16 = _mm_unpacklo_epi16 (coef8x16, coef8x16);

    coef10x8 = _mm_shuffle_epi32 (coef8x16, 0x00);
    coef32x8 = _mm_shuffle_epi32 (coef8x16, 0x55);
    coef54x8 = _mm_shuffle_epi32 (coef8x16, 0xAA);
    coef76x8 = _mm_shuffle_epi32 (coef8x16, 0xFF);

    src0x8  = _mm_loadl_epi64 ((__m128i *) src);
    src1x8  = _mm_loadl_epi64 ((__m128i *)(src + srcStride));
    src    += srcStride*2;
    src2x8  = _mm_loadl_epi64 ((__m128i *) src);
    src3x8  = _mm_loadl_epi64 ((__m128i *)(src + srcStride));
    src    += srcStride*2;
    src4x8  = _mm_loadl_epi64 ((__m128i *) src);
    src5x8  = _mm_loadl_epi64 ((__m128i *)(src + srcStride));
    src6x8  = _mm_loadl_epi64 ((__m128i *)(src + srcStride*2));
    src    += srcStride*3;

    srcEve10x8 = _mm_unpacklo_epi8 (src0x8, src1x8);
    srcEve32x8 = _mm_unpacklo_epi8 (src2x8, src3x8);
    srcEve54x8 = _mm_unpacklo_epi8 (src4x8, src5x8);

    srcOdd10x8 = _mm_unpacklo_epi8 (src1x8, src2x8);
    srcOdd32x8 = _mm_unpacklo_epi8 (src3x8, src4x8);
    srcOdd54x8 = _mm_unpacklo_epi8 (src5x8, src6x8);

    for (row = 0; row < height; row += 2)
    {
        src7x8     = _mm_loadl_epi64 ((__m128i *) src);
        src8x8     = _mm_loadl_epi64 ((__m128i *)(src + srcStride));
        src       += srcStride*2;

        prod10  = _mm_maddubs_epi16 (srcEve10x8, coef10x8);
        prodSum = _mm_sub_epi16 (prod10, roundx8);
        prod32  = _mm_maddubs_epi16 (srcEve32x8, coef32x8);
        prodSum = _mm_add_epi16 (prodSum, prod32);
        prod54  = _mm_maddubs_epi16 (srcEve54x8, coef54x8);
        prodSum = _mm_add_epi16 (prodSum, prod54);

        srcEve76x8 = _mm_unpacklo_epi8 (src6x8, src7x8);

        prod76  = _mm_maddubs_epi16 (srcEve76x8, coef76x8);
        prodSum = _mm_add_epi16 (prodSum, prod76);

        _mm_storeu_si128 ((__m128i *)(dst), prodSum);

        srcEve10x8 = srcEve32x8;
        srcEve32x8 = srcEve54x8;
        srcEve54x8 = srcEve76x8;

        prod10  = _mm_maddubs_epi16 (srcOdd10x8, coef10x8);
        prodSum = _mm_sub_epi16 (prod10, roundx8);
        prod32  = _mm_maddubs_epi16 (srcOdd32x8, coef32x8);
        prodSum = _mm_add_epi16 (prodSum, prod32);
        prod54  = _mm_maddubs_epi16 (srcOdd54x8, coef54x8);
        prodSum = _mm_add_epi16 (prodSum, prod54);

        srcOdd76x8 = _mm_unpacklo_epi8 (src7x8, src8x8);

        prod76  = _mm_maddubs_epi16 (srcOdd76x8, coef76x8);
        prodSum = _mm_add_epi16 (prodSum, prod76);

        _mm_storeu_si128 ((__m128i *)(dst + dstStride), prodSum);

        srcOdd10x8 = srcOdd32x8;
        srcOdd32x8 = srcOdd54x8;
        srcOdd54x8 = srcOdd76x8;

        dst       += dstStride*2;
        src6x8     = src8x8;

    }

}

void Xin266LumaInterpVet16xHU8S16_SSSE3 (
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
    UINT32   row;
    __m128i  src0x16, src1x16, src2x16;
    __m128i  src3x16, src4x16, src5x16;
    __m128i  src6x16, src7x16, src8x16;
    __m128i  srcEveLo10x8, srcEveLo32x8, srcEveLo54x8, srcEveLo76x8;
    __m128i  srcEveHi10x8, srcEveHi32x8, srcEveHi54x8, srcEveHi76x8;
    __m128i  srcOddLo10x8, srcOddLo32x8, srcOddLo54x8, srcOddLo76x8;
    __m128i  srcOddHi10x8, srcOddHi32x8, srcOddHi54x8, srcOddHi76x8;
    __m128i  coef10x8, coef32x8, coef54x8, coef76x8;
    __m128i  prod10, prod32, prod54, prod76;
    __m128i  prodSum;
    __m128i  roundx8, coef8x16;

    (void)width;

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

    roundx8  = _mm_loadu_si128 ((__m128i *)roundOffset3);
    coef8x16 = _mm_loadl_epi64 ((__m128i *)fltCoeff);
    coef8x16 = _mm_unpacklo_epi16 (coef8x16, coef8x16);

    coef10x8 = _mm_shuffle_epi32 (coef8x16, 0x00);
    coef32x8 = _mm_shuffle_epi32 (coef8x16, 0x55);
    coef54x8 = _mm_shuffle_epi32 (coef8x16, 0xAA);
    coef76x8 = _mm_shuffle_epi32 (coef8x16, 0xFF);

    src0x16  = _mm_lddqu_si128 ((__m128i *) src);
    src1x16  = _mm_lddqu_si128 ((__m128i *)(src + srcStride));
    src     += srcStride*2;
    src2x16  = _mm_lddqu_si128 ((__m128i *) src);
    src3x16  = _mm_lddqu_si128 ((__m128i *)(src + srcStride));
    src     += srcStride*2;
    src4x16  = _mm_lddqu_si128 ((__m128i *) src);
    src5x16  = _mm_lddqu_si128 ((__m128i *)(src + srcStride));
    src6x16  = _mm_lddqu_si128 ((__m128i *)(src + srcStride*2));
    src     += srcStride*3;

    srcEveLo10x8 = _mm_unpacklo_epi8 (src0x16, src1x16);
    srcEveLo32x8 = _mm_unpacklo_epi8 (src2x16, src3x16);
    srcEveLo54x8 = _mm_unpacklo_epi8 (src4x16, src5x16);

    srcOddLo10x8 = _mm_unpacklo_epi8 (src1x16, src2x16);
    srcOddLo32x8 = _mm_unpacklo_epi8 (src3x16, src4x16);
    srcOddLo54x8 = _mm_unpacklo_epi8 (src5x16, src6x16);

    srcEveHi10x8 = _mm_unpackhi_epi8 (src0x16, src1x16);
    srcEveHi32x8 = _mm_unpackhi_epi8 (src2x16, src3x16);
    srcEveHi54x8 = _mm_unpackhi_epi8 (src4x16, src5x16);

    srcOddHi10x8 = _mm_unpackhi_epi8 (src1x16, src2x16);
    srcOddHi32x8 = _mm_unpackhi_epi8 (src3x16, src4x16);
    srcOddHi54x8 = _mm_unpackhi_epi8 (src5x16, src6x16);

    for (row = 0; row < height; row += 2)
    {
        src7x16  = _mm_lddqu_si128 ((__m128i *) src);
        src8x16  = _mm_lddqu_si128 ((__m128i *)(src + srcStride));
        src     += srcStride*2;

        prod10  = _mm_maddubs_epi16 (srcEveLo10x8, coef10x8);
        prodSum = _mm_sub_epi16 (prod10, roundx8);
        prod32  = _mm_maddubs_epi16 (srcEveLo32x8, coef32x8);
        prodSum = _mm_add_epi16 (prodSum, prod32);
        prod54  = _mm_maddubs_epi16 (srcEveLo54x8, coef54x8);
        prodSum = _mm_add_epi16 (prodSum, prod54);

        srcEveLo76x8 = _mm_unpacklo_epi8 (src6x16, src7x16);

        prod76  = _mm_maddubs_epi16 (srcEveLo76x8, coef76x8);
        prodSum = _mm_add_epi16 (prodSum, prod76);

        _mm_storeu_si128 ((__m128i *)(dst), prodSum);

        srcEveLo10x8 = srcEveLo32x8;
        srcEveLo32x8 = srcEveLo54x8;
        srcEveLo54x8 = srcEveLo76x8;

        prod10  = _mm_maddubs_epi16 (srcEveHi10x8, coef10x8);
        prodSum = _mm_sub_epi16 (prod10, roundx8);
        prod32  = _mm_maddubs_epi16 (srcEveHi32x8, coef32x8);
        prodSum = _mm_add_epi16 (prodSum, prod32);
        prod54  = _mm_maddubs_epi16 (srcEveHi54x8, coef54x8);
        prodSum = _mm_add_epi16 (prodSum, prod54);

        srcEveHi76x8 = _mm_unpackhi_epi8 (src6x16, src7x16);

        prod76  = _mm_maddubs_epi16 (srcEveHi76x8, coef76x8);
        prodSum = _mm_add_epi16 (prodSum, prod76);

        _mm_storeu_si128 ((__m128i *)(dst + 8), prodSum);

        srcEveHi10x8 = srcEveHi32x8;
        srcEveHi32x8 = srcEveHi54x8;
        srcEveHi54x8 = srcEveHi76x8;

        prod10  = _mm_maddubs_epi16 (srcOddLo10x8, coef10x8);
        prodSum = _mm_sub_epi16 (prod10, roundx8);
        prod32  = _mm_maddubs_epi16 (srcOddLo32x8, coef32x8);
        prodSum = _mm_add_epi16 (prodSum, prod32);
        prod54  = _mm_maddubs_epi16 (srcOddLo54x8, coef54x8);
        prodSum = _mm_add_epi16 (prodSum, prod54);

        srcOddLo76x8 = _mm_unpacklo_epi8 (src7x16, src8x16);

        prod76  = _mm_maddubs_epi16 (srcOddLo76x8, coef76x8);
        prodSum = _mm_add_epi16 (prodSum, prod76);

        _mm_storeu_si128 ((__m128i *)(dst + dstStride), prodSum);

        srcOddLo10x8 = srcOddLo32x8;
        srcOddLo32x8 = srcOddLo54x8;
        srcOddLo54x8 = srcOddLo76x8;

        prod10  = _mm_maddubs_epi16 (srcOddHi10x8, coef10x8);
        prodSum = _mm_sub_epi16 (prod10, roundx8);
        prod32  = _mm_maddubs_epi16 (srcOddHi32x8, coef32x8);
        prodSum = _mm_add_epi16 (prodSum, prod32);
        prod54  = _mm_maddubs_epi16 (srcOddHi54x8, coef54x8);
        prodSum = _mm_add_epi16 (prodSum, prod54);

        srcOddHi76x8 = _mm_unpackhi_epi8 (src7x16, src8x16);

        prod76  = _mm_maddubs_epi16 (srcOddHi76x8, coef76x8);
        prodSum = _mm_add_epi16 (prodSum, prod76);

        _mm_storeu_si128 ((__m128i *)(dst + 8 + dstStride), prodSum);

        srcOddHi10x8 = srcOddHi32x8;
        srcOddHi32x8 = srcOddHi54x8;
        srcOddHi54x8 = srcOddHi76x8;

        dst     += dstStride*2;
        src6x16 = src8x16;

    }

}

void Xin266LumaInterpVetGt16xHU8S16_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
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
    SINT16   *dstRow;
    __m128i  src0x16, src1x16, src2x16;
    __m128i  src3x16, src4x16, src5x16;
    __m128i  src6x16, src7x16, src8x16;
    __m128i  srcEveLo10x8, srcEveLo32x8, srcEveLo54x8, srcEveLo76x8;
    __m128i  srcEveHi10x8, srcEveHi32x8, srcEveHi54x8, srcEveHi76x8;
    __m128i  srcOddLo10x8, srcOddLo32x8, srcOddLo54x8, srcOddLo76x8;
    __m128i  srcOddHi10x8, srcOddHi32x8, srcOddHi54x8, srcOddHi76x8;
    __m128i  coef10x8, coef32x8, coef54x8, coef76x8;
    __m128i  prod10, prod32, prod54, prod76;
    __m128i  prodSum;
    __m128i  roundx8, coef8x16;

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

    roundx8  = _mm_loadu_si128 ((__m128i *)roundOffset3);
    coef8x16 = _mm_loadl_epi64 ((__m128i *)fltCoeff);
    coef8x16 = _mm_unpacklo_epi16 (coef8x16, coef8x16);

    coef10x8 = _mm_shuffle_epi32 (coef8x16, 0x00);
    coef32x8 = _mm_shuffle_epi32 (coef8x16, 0x55);
    coef54x8 = _mm_shuffle_epi32 (coef8x16, 0xAA);
    coef76x8 = _mm_shuffle_epi32 (coef8x16, 0xFF);

    for (col = 0; col < width; col += 16)
    {
        srcRow   = src + col;
        dstRow   = dst + col;

        src0x16  = _mm_lddqu_si128 ((__m128i *) srcRow);
        src1x16  = _mm_lddqu_si128 ((__m128i *)(srcRow + srcStride));
        srcRow  += srcStride*2;
        src2x16  = _mm_lddqu_si128 ((__m128i *) srcRow);
        src3x16  = _mm_lddqu_si128 ((__m128i *)(srcRow + srcStride));
        srcRow  += srcStride*2;
        src4x16  = _mm_lddqu_si128 ((__m128i *) srcRow);
        src5x16  = _mm_lddqu_si128 ((__m128i *)(srcRow + srcStride));
        src6x16  = _mm_lddqu_si128 ((__m128i *)(srcRow + srcStride*2));
        srcRow  += srcStride*3;

        srcEveLo10x8 = _mm_unpacklo_epi8 (src0x16, src1x16);
        srcEveLo32x8 = _mm_unpacklo_epi8 (src2x16, src3x16);
        srcEveLo54x8 = _mm_unpacklo_epi8 (src4x16, src5x16);

        srcOddLo10x8 = _mm_unpacklo_epi8 (src1x16, src2x16);
        srcOddLo32x8 = _mm_unpacklo_epi8 (src3x16, src4x16);
        srcOddLo54x8 = _mm_unpacklo_epi8 (src5x16, src6x16);

        srcEveHi10x8 = _mm_unpackhi_epi8 (src0x16, src1x16);
        srcEveHi32x8 = _mm_unpackhi_epi8 (src2x16, src3x16);
        srcEveHi54x8 = _mm_unpackhi_epi8 (src4x16, src5x16);

        srcOddHi10x8 = _mm_unpackhi_epi8 (src1x16, src2x16);
        srcOddHi32x8 = _mm_unpackhi_epi8 (src3x16, src4x16);
        srcOddHi54x8 = _mm_unpackhi_epi8 (src5x16, src6x16);

        for (row = 0; row < height; row += 2)
        {
            src7x16  = _mm_lddqu_si128 ((__m128i *) srcRow);
            src8x16  = _mm_lddqu_si128 ((__m128i *)(srcRow + srcStride));
            srcRow  += srcStride*2;

            prod10  = _mm_maddubs_epi16 (srcEveLo10x8, coef10x8);
            prodSum = _mm_sub_epi16 (prod10, roundx8);
            prod32  = _mm_maddubs_epi16 (srcEveLo32x8, coef32x8);
            prodSum = _mm_add_epi16 (prodSum, prod32);
            prod54  = _mm_maddubs_epi16 (srcEveLo54x8, coef54x8);
            prodSum = _mm_add_epi16 (prodSum, prod54);

            srcEveLo76x8 = _mm_unpacklo_epi8 (src6x16, src7x16);

            prod76  = _mm_maddubs_epi16 (srcEveLo76x8, coef76x8);
            prodSum = _mm_add_epi16 (prodSum, prod76);

            _mm_storeu_si128 ((__m128i *)(dstRow), prodSum);

            srcEveLo10x8 = srcEveLo32x8;
            srcEveLo32x8 = srcEveLo54x8;
            srcEveLo54x8 = srcEveLo76x8;

            prod10  = _mm_maddubs_epi16 (srcEveHi10x8, coef10x8);
            prodSum = _mm_sub_epi16 (prod10, roundx8);
            prod32  = _mm_maddubs_epi16 (srcEveHi32x8, coef32x8);
            prodSum = _mm_add_epi16 (prodSum, prod32);
            prod54  = _mm_maddubs_epi16 (srcEveHi54x8, coef54x8);
            prodSum = _mm_add_epi16 (prodSum, prod54);

            srcEveHi76x8 = _mm_unpackhi_epi8 (src6x16, src7x16);

            prod76  = _mm_maddubs_epi16 (srcEveHi76x8, coef76x8);
            prodSum = _mm_add_epi16 (prodSum, prod76);

            _mm_storeu_si128 ((__m128i *)(dstRow + 8), prodSum);

            srcEveHi10x8 = srcEveHi32x8;
            srcEveHi32x8 = srcEveHi54x8;
            srcEveHi54x8 = srcEveHi76x8;

            prod10  = _mm_maddubs_epi16 (srcOddLo10x8, coef10x8);
            prodSum = _mm_sub_epi16 (prod10, roundx8);
            prod32  = _mm_maddubs_epi16 (srcOddLo32x8, coef32x8);
            prodSum = _mm_add_epi16 (prodSum, prod32);
            prod54  = _mm_maddubs_epi16 (srcOddLo54x8, coef54x8);
            prodSum = _mm_add_epi16 (prodSum, prod54);

            srcOddLo76x8 = _mm_unpacklo_epi8 (src7x16, src8x16);

            prod76  = _mm_maddubs_epi16 (srcOddLo76x8, coef76x8);
            prodSum = _mm_add_epi16 (prodSum, prod76);

            _mm_storeu_si128 ((__m128i *)(dstRow + dstStride), prodSum);

            srcOddLo10x8 = srcOddLo32x8;
            srcOddLo32x8 = srcOddLo54x8;
            srcOddLo54x8 = srcOddLo76x8;

            prod10  = _mm_maddubs_epi16 (srcOddHi10x8, coef10x8);
            prodSum = _mm_sub_epi16 (prod10, roundx8);
            prod32  = _mm_maddubs_epi16 (srcOddHi32x8, coef32x8);
            prodSum = _mm_add_epi16 (prodSum, prod32);
            prod54  = _mm_maddubs_epi16 (srcOddHi54x8, coef54x8);
            prodSum = _mm_add_epi16 (prodSum, prod54);

            srcOddHi76x8 = _mm_unpackhi_epi8 (src7x16, src8x16);

            prod76  = _mm_maddubs_epi16 (srcOddHi76x8, coef76x8);
            prodSum = _mm_add_epi16 (prodSum, prod76);

            _mm_storeu_si128 ((__m128i *)(dstRow + 8 + dstStride), prodSum);

            srcOddHi10x8 = srcOddHi32x8;
            srcOddHi32x8 = srcOddHi54x8;
            srcOddHi54x8 = srcOddHi76x8;

            dstRow  += dstStride*2;
            src6x16  = src8x16;

        }
    }

}

void Xin266InterpCopy4xHU8S16_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex)
{
    UINT32 row;
    __m128i roundx8;
    __m128i srcTx8, srcBx8;
    __m128i dstTx8, dstBx8;
    __m128i allZero;

    (void)frac;
    (void)width;
    (void)filterIndex;

    roundx8  = _mm_loadu_si128 ((__m128i *)roundOffset3);
    allZero  = _mm_setzero_si128 ();

    for (row = 0; row < height; row += 2)
    {
        srcTx8 = _mm_cvtsi32_si128 (*((UINT32 *)(src)));
        srcBx8 = _mm_cvtsi32_si128 (*((UINT32 *)(src + srcStride)));

        srcTx8 = _mm_unpacklo_epi8 (srcTx8, allZero);
        srcBx8 = _mm_unpacklo_epi8 (srcBx8, allZero);

        dstTx8 = _mm_slli_epi16 (srcTx8, INTERP_SHIFT);
        dstBx8 = _mm_slli_epi16 (srcBx8, INTERP_SHIFT);

        dstTx8 = _mm_sub_epi16 (dstTx8, roundx8);
        dstBx8 = _mm_sub_epi16 (dstBx8, roundx8);

        _mm_storel_epi64 ((__m128i *)(dst),             dstTx8);
        _mm_storel_epi64 ((__m128i *)(dst + dstStride), dstBx8);

        src += srcStride*2;
        dst += dstStride*2;

    }

}

void Xin266InterpCopy8xHU8S16_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex)
{
    UINT32 row;
    __m128i roundx8;
    __m128i srcTx8, srcBx8;
    __m128i dstTx8, dstBx8;
    __m128i allZero;

    (void)frac;
    (void)width;
    (void)filterIndex;

    roundx8  = _mm_loadu_si128 ((__m128i *)roundOffset3);
    allZero  = _mm_setzero_si128 ();

    for (row = 0; row < height; row += 2)
    {
        srcTx8 = _mm_loadl_epi64 ((__m128i *)(src));
        srcBx8 = _mm_loadl_epi64 ((__m128i *)(src + srcStride));

        srcTx8 = _mm_unpacklo_epi8 (srcTx8, allZero);
        srcBx8 = _mm_unpacklo_epi8 (srcBx8, allZero);

        dstTx8 = _mm_slli_epi16 (srcTx8, INTERP_SHIFT);
        dstBx8 = _mm_slli_epi16 (srcBx8, INTERP_SHIFT);

        dstTx8 = _mm_sub_epi16 (dstTx8, roundx8);
        dstBx8 = _mm_sub_epi16 (dstBx8, roundx8);

        _mm_storeu_si128 ((__m128i *)(dst),             dstTx8);
        _mm_storeu_si128 ((__m128i *)(dst + dstStride), dstBx8);

        src += srcStride*2;
        dst += dstStride*2;

    }

}

void Xin266InterpCopyGt8xHU8S16_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex)
{
    UINT32  rowIdx, colIdx;
    __m128i roundx8;
    __m128i srcTx8, srcBx8;
    __m128i dstTLx8, dstTRx8;
    __m128i dstBLx8, dstBRx8;
    __m128i allZero;
    SINT16  *dstRow;
    const PIXEL *srcRow;

    (void)frac;
    (void)filterIndex;

    roundx8  = _mm_loadu_si128 ((__m128i *)roundOffset3);
    allZero  = _mm_setzero_si128 ();

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        for (colIdx = 0; colIdx < width; colIdx += 16)
        {
            dstRow = dst + colIdx;
            srcRow = src + colIdx;

            srcTx8 = _mm_lddqu_si128 ((__m128i *)(srcRow));
            srcBx8 = _mm_lddqu_si128 ((__m128i *)(srcRow + srcStride));

            dstTLx8 = _mm_unpacklo_epi8 (srcTx8, allZero);
            dstTRx8 = _mm_unpackhi_epi8 (srcTx8, allZero);
            dstBLx8 = _mm_unpacklo_epi8 (srcBx8, allZero);
            dstBRx8 = _mm_unpackhi_epi8 (srcBx8, allZero);

            dstTLx8 = _mm_slli_epi16 (dstTLx8, INTERP_SHIFT);
            dstTRx8 = _mm_slli_epi16 (dstTRx8, INTERP_SHIFT);
            dstBLx8 = _mm_slli_epi16 (dstBLx8, INTERP_SHIFT);
            dstBRx8 = _mm_slli_epi16 (dstBRx8, INTERP_SHIFT);

            dstTLx8 = _mm_sub_epi16 (dstTLx8, roundx8);
            dstTRx8 = _mm_sub_epi16 (dstTRx8, roundx8);
            dstBLx8 = _mm_sub_epi16 (dstBLx8, roundx8);
            dstBRx8 = _mm_sub_epi16 (dstBRx8, roundx8);

            _mm_storeu_si128 ((__m128i *)(dstRow),                 dstTLx8);
            _mm_storeu_si128 ((__m128i *)(dstRow + 8),             dstTRx8);
            _mm_storeu_si128 ((__m128i *)(dstRow + dstStride),     dstBLx8);
            _mm_storeu_si128 ((__m128i *)(dstRow + 8 + dstStride), dstBRx8);

        }

        src += srcStride*2;
        dst += dstStride*2;

    }

}

void Xin266LumaInterpVet4xHS16S16_SSSE3 (
    const SINT16 *src,
    intptr_t     srcStride,
    SINT16       *dst,
    intptr_t     dstStride,
    SINT32       frac,
    UINT32       width,
    UINT32       height,
    UINT32      filterIndex)
{
    const SINT16 *srcRow;
    const SINT16 *fltCoeff;
    SINT16   *dstRow;
    UINT32   row;
    __m128i  src0x4, src1x4, src2x4;
    __m128i  src3x4, src4x4, src5x4;
    __m128i  src6x4, src7x4, src8x4;
    __m128i  srcEve10x4, srcEve32x4, srcEve54x4, srcEve76x4;
    __m128i  srcOdd10x4, srcOdd32x4, srcOdd54x4, srcOdd76x4;
    __m128i  coef10x8, coef32x8, coef54x8, coef76x8;
    __m128i  prod10, prod32, prod54, prod76;
    __m128i  outLox4;
    __m128i  prodSum, outx8;
    __m128i  coef8x16;

    (void)width;

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

    coef8x16 = _mm_lddqu_si128 ((__m128i *)fltCoeff);

    coef10x8 = _mm_shuffle_epi32 (coef8x16, 0x00);
    coef32x8 = _mm_shuffle_epi32 (coef8x16, 0x55);
    coef54x8 = _mm_shuffle_epi32 (coef8x16, 0xAA);
    coef76x8 = _mm_shuffle_epi32 (coef8x16, 0xFF);

    srcRow   = src;
    dstRow   = dst;

    src0x4  = _mm_loadl_epi64 ((__m128i *)(srcRow));
    src1x4  = _mm_loadl_epi64 ((__m128i *)(srcRow + srcStride));
    srcRow += srcStride*2;
    src2x4  = _mm_loadl_epi64 ((__m128i *)(srcRow));
    src3x4  = _mm_loadl_epi64 ((__m128i *)(srcRow + srcStride));
    srcRow += srcStride*2;
    src4x4  = _mm_loadl_epi64 ((__m128i *)(srcRow));
    src5x4  = _mm_loadl_epi64 ((__m128i *)(srcRow + srcStride));
    src6x4  = _mm_loadl_epi64 ((__m128i *)(srcRow + srcStride*2));
    srcRow += srcStride*3;

    srcEve10x4 = _mm_unpacklo_epi16 (src0x4, src1x4);
    srcEve32x4 = _mm_unpacklo_epi16 (src2x4, src3x4);
    srcEve54x4 = _mm_unpacklo_epi16 (src4x4, src5x4);

    srcOdd10x4 = _mm_unpacklo_epi16 (src1x4, src2x4);
    srcOdd32x4 = _mm_unpacklo_epi16 (src3x4, src4x4);
    srcOdd54x4 = _mm_unpacklo_epi16 (src5x4, src6x4);

    for (row = 0; row < height; row += 2)
    {
        src7x4   = _mm_loadl_epi64 ((__m128i *)(srcRow));
        src8x4   = _mm_loadl_epi64 ((__m128i *)(srcRow + srcStride));
        srcRow  += srcStride*2;

        prod10  = _mm_madd_epi16 (srcEve10x4, coef10x8);
        prod32  = _mm_madd_epi16 (srcEve32x4, coef32x8);
        prodSum = _mm_add_epi32 (prod10, prod32);
        prod54  = _mm_madd_epi16 (srcEve54x4, coef54x8);
        prodSum = _mm_add_epi32 (prodSum, prod54);

        srcEve76x4 = _mm_unpacklo_epi16 (src6x4, src7x4);

        prod76  = _mm_madd_epi16 (srcEve76x4, coef76x8);
        prodSum = _mm_add_epi32 (prodSum, prod76);
        outLox4 = _mm_srai_epi32 (prodSum, INTERP_SHIFT);
        outx8   = _mm_packs_epi32 (outLox4, outLox4);

        _mm_storel_epi64 ((__m128i *)(dstRow), outx8);

        srcEve10x4 = srcEve32x4;
        srcEve32x4 = srcEve54x4;
        srcEve54x4 = srcEve76x4;

        prod10  = _mm_madd_epi16 (srcOdd10x4, coef10x8);
        prod32  = _mm_madd_epi16 (srcOdd32x4, coef32x8);
        prodSum = _mm_add_epi32 (prod10, prod32);
        prod54  = _mm_madd_epi16 (srcOdd54x4, coef54x8);
        prodSum = _mm_add_epi32 (prodSum, prod54);

        srcOdd76x4 = _mm_unpacklo_epi16 (src7x4, src8x4);

        prod76  = _mm_madd_epi16 (srcOdd76x4, coef76x8);
        prodSum = _mm_add_epi32 (prodSum, prod76);
        outLox4 = _mm_srai_epi32 (prodSum, INTERP_SHIFT);
        outx8   = _mm_packs_epi32 (outLox4, outLox4);

        _mm_storel_epi64 ((__m128i *)(dstRow + dstStride), outx8);

        srcOdd10x4 = srcOdd32x4;
        srcOdd32x4 = srcOdd54x4;
        srcOdd54x4 = srcOdd76x4;

        dstRow += dstStride*2;
        src6x4  = src8x4;

    }

}

void Xin266LumaInterpVetS16S16_SSSE3 (
    const SINT16 *src,
    intptr_t     srcStride,
    SINT16       *dst,
    intptr_t     dstStride,
    SINT32       frac,
    UINT32       width,
    UINT32       height,
    UINT32      filterIndex)
{
    const SINT16 *srcRow;
    const SINT16 *fltCoeff;
    SINT16   *dstRow;
    UINT32   row, col;
    __m128i  src0x8, src1x8, src2x8;
    __m128i  src3x8, src4x8, src5x8;
    __m128i  src6x8, src7x8, src8x8;
    __m128i  srcEveLo10x4, srcEveLo32x4, srcEveLo54x4, srcEveLo76x4;
    __m128i  srcEveHi10x4, srcEveHi32x4, srcEveHi54x4, srcEveHi76x4;
    __m128i  srcOddLo10x4, srcOddLo32x4, srcOddLo54x4, srcOddLo76x4;
    __m128i  srcOddHi10x4, srcOddHi32x4, srcOddHi54x4, srcOddHi76x4;
    __m128i  coef10x8, coef32x8, coef54x8, coef76x8;
    __m128i  prod10, prod32, prod54, prod76;
    __m128i  outLox4, outHix4;
    __m128i  prodSum, outx8;
    __m128i  coef8x16;

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

    coef8x16 = _mm_lddqu_si128 ((__m128i *)fltCoeff);

    coef10x8 = _mm_shuffle_epi32 (coef8x16, 0x00);
    coef32x8 = _mm_shuffle_epi32 (coef8x16, 0x55);
    coef54x8 = _mm_shuffle_epi32 (coef8x16, 0xAA);
    coef76x8 = _mm_shuffle_epi32 (coef8x16, 0xFF);

    for (col = 0; col < width; col += 8)
    {
        srcRow   = src + col;
        dstRow   = dst + col;

        src0x8  = _mm_lddqu_si128 ((__m128i *)(srcRow));
        src1x8  = _mm_lddqu_si128 ((__m128i *)(srcRow + srcStride));
        srcRow += srcStride*2;
        src2x8  = _mm_lddqu_si128 ((__m128i *)(srcRow));
        src3x8  = _mm_lddqu_si128 ((__m128i *)(srcRow + srcStride));
        srcRow += srcStride*2;
        src4x8  = _mm_lddqu_si128 ((__m128i *)(srcRow));
        src5x8  = _mm_lddqu_si128 ((__m128i *)(srcRow + srcStride));
        src6x8  = _mm_lddqu_si128 ((__m128i *)(srcRow + srcStride*2));
        srcRow += srcStride*3;

        srcEveLo10x4 = _mm_unpacklo_epi16 (src0x8, src1x8);
        srcEveLo32x4 = _mm_unpacklo_epi16 (src2x8, src3x8);
        srcEveLo54x4 = _mm_unpacklo_epi16 (src4x8, src5x8);

        srcEveHi10x4 = _mm_unpackhi_epi16 (src0x8, src1x8);
        srcEveHi32x4 = _mm_unpackhi_epi16 (src2x8, src3x8);
        srcEveHi54x4 = _mm_unpackhi_epi16 (src4x8, src5x8);

        srcOddLo10x4 = _mm_unpacklo_epi16 (src1x8, src2x8);
        srcOddLo32x4 = _mm_unpacklo_epi16 (src3x8, src4x8);
        srcOddLo54x4 = _mm_unpacklo_epi16 (src5x8, src6x8);

        srcOddHi10x4 = _mm_unpackhi_epi16 (src1x8, src2x8);
        srcOddHi32x4 = _mm_unpackhi_epi16 (src3x8, src4x8);
        srcOddHi54x4 = _mm_unpackhi_epi16 (src5x8, src6x8);

        for (row = 0; row < height; row += 2)
        {
            src7x8   = _mm_lddqu_si128 ((__m128i *)(srcRow));
            src8x8   = _mm_lddqu_si128 ((__m128i *)(srcRow + srcStride));
            srcRow  += srcStride*2;

            prod10  = _mm_madd_epi16 (srcEveLo10x4, coef10x8);
            prod32  = _mm_madd_epi16 (srcEveLo32x4, coef32x8);
            prodSum = _mm_add_epi32 (prod10, prod32);
            prod54  = _mm_madd_epi16 (srcEveLo54x4, coef54x8);
            prodSum = _mm_add_epi32 (prodSum, prod54);

            srcEveLo76x4 = _mm_unpacklo_epi16 (src6x8, src7x8);

            prod76  = _mm_madd_epi16 (srcEveLo76x4, coef76x8);
            prodSum = _mm_add_epi32 (prodSum, prod76);
            outLox4 = _mm_srai_epi32 (prodSum, INTERP_SHIFT);

            srcEveLo10x4 = srcEveLo32x4;
            srcEveLo32x4 = srcEveLo54x4;
            srcEveLo54x4 = srcEveLo76x4;

            prod10  = _mm_madd_epi16 (srcEveHi10x4, coef10x8);
            prod32  = _mm_madd_epi16 (srcEveHi32x4, coef32x8);
            prodSum = _mm_add_epi32 (prod10, prod32);
            prod54  = _mm_madd_epi16 (srcEveHi54x4, coef54x8);
            prodSum = _mm_add_epi32 (prodSum, prod54);

            srcEveHi76x4 = _mm_unpackhi_epi16 (src6x8, src7x8);

            prod76  = _mm_madd_epi16 (srcEveHi76x4, coef76x8);
            prodSum = _mm_add_epi32 (prodSum, prod76);
            outHix4 = _mm_srai_epi32 (prodSum, INTERP_SHIFT);

            srcEveHi10x4 = srcEveHi32x4;
            srcEveHi32x4 = srcEveHi54x4;
            srcEveHi54x4 = srcEveHi76x4;

            outx8 = _mm_packs_epi32 (outLox4, outHix4);
            _mm_storeu_si128 ((__m128i *)(dstRow), outx8);

            prod10  = _mm_madd_epi16 (srcOddLo10x4, coef10x8);
            prod32  = _mm_madd_epi16 (srcOddLo32x4, coef32x8);
            prodSum = _mm_add_epi32 (prod10, prod32);
            prod54  = _mm_madd_epi16 (srcOddLo54x4, coef54x8);
            prodSum = _mm_add_epi32 (prodSum, prod54);

            srcOddLo76x4 = _mm_unpacklo_epi16 (src7x8, src8x8);

            prod76  = _mm_madd_epi16 (srcOddLo76x4, coef76x8);
            prodSum = _mm_add_epi32 (prodSum, prod76);
            outLox4 = _mm_srai_epi32 (prodSum, INTERP_SHIFT);

            srcOddLo10x4 = srcOddLo32x4;
            srcOddLo32x4 = srcOddLo54x4;
            srcOddLo54x4 = srcOddLo76x4;

            prod10  = _mm_madd_epi16 (srcOddHi10x4, coef10x8);
            prod32  = _mm_madd_epi16 (srcOddHi32x4, coef32x8);
            prodSum = _mm_add_epi32 (prod10, prod32);
            prod54  = _mm_madd_epi16 (srcOddHi54x4, coef54x8);
            prodSum = _mm_add_epi32 (prodSum, prod54);

            srcOddHi76x4 = _mm_unpackhi_epi16 (src7x8, src8x8);

            prod76  = _mm_madd_epi16 (srcOddHi76x4, coef76x8);
            prodSum = _mm_add_epi32 (prodSum, prod76);
            outHix4 = _mm_srai_epi32 (prodSum, INTERP_SHIFT);

            srcOddHi10x4 = srcOddHi32x4;
            srcOddHi32x4 = srcOddHi54x4;
            srcOddHi54x4 = srcOddHi76x4;

            outx8 = _mm_packs_epi32 (outLox4, outHix4);
            _mm_storeu_si128 ((__m128i *)(dstRow + dstStride), outx8);

            dstRow += dstStride*2;
            src6x8  = src8x8;

        }
    }

}

void Xin266LumaInterpHorVet4xHU8S16_SSSE3 (
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

    Xin266LumaInterpHor4xHU8S16_SSSE3 (
        src,
        srcStride,
        firstPassDst,
        width,
        frac,
        width,
        height + LUMA_INTERP_TAPS - 1,
        filterIndex);

    Xin266LumaInterpVet4xHS16S16_SSSE3 (
        firstPassDst + (LUMA_INTERP_TAPS / 2 - 1) * width,
        width,
        dst,
        dstStride,
        frac,
        width,
        height,
        filterIndex);

}

void Xin266LumaInterpHorVet8xHU8S16_SSSE3 (
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

    Xin266LumaInterpHor8xHU8S16_SSSE3 (
        src,
        srcStride,
        firstPassDst,
        width,
        frac,
        width,
        height + LUMA_INTERP_TAPS - 1,
        filterIndex);

    Xin266LumaInterpVetS16S16_SSSE3 (
        firstPassDst + (LUMA_INTERP_TAPS / 2 - 1) * width,
        width,
        dst,
        dstStride,
        frac,
        width,
        height,
        filterIndex);

}

void Xin266LumaInterpHorVetGt8xHU8S16_SSSE3 (
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

    Xin266LumaInterpHorGt8xHU8S16_SSSE3 (
        src,
        srcStride,
        firstPassDst,
        width,
        frac,
        width,
        height + LUMA_INTERP_TAPS - 1,
        filterIndex);

    Xin266LumaInterpVetS16S16_SSSE3 (
        firstPassDst + (LUMA_INTERP_TAPS / 2 - 1) * width,
        width,
        dst,
        dstStride,
        frac,
        width,
        height,
        filterIndex);

}

void Xin266InterpAvg8xHS16U8_SSSE3 (
    const SINT16 *src0,
    intptr_t     src0Stride,
    const SINT16 *src1,
    intptr_t     src1Stride,
    PIXEL        *dst,
    intptr_t     dstStride,
    UINT32       width,
    UINT32       height)
{
    UINT32 rowIdx;
    __m128i c256x8;
    __m128i c128x8;
    __m128i src0Tx8, src0Bx8;
    __m128i src1Tx8, src1Bx8;
    __m128i sumTx8, sumBx8;
    __m128i sumx16;

    (void)width;
    c256x8 = _mm_set1_epi16 (256);
    c128x8 = _mm_srli_epi16 (c256x8, 1);

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        src0Tx8 = _mm_lddqu_si128 ((__m128i *)src0);
        src0Bx8 = _mm_lddqu_si128 ((__m128i *)(src0 + src0Stride));

        src1Tx8 = _mm_lddqu_si128 ((__m128i *)src1);
        src1Bx8 = _mm_lddqu_si128 ((__m128i *)(src1 + src1Stride));

        sumTx8 = _mm_add_epi16 (src0Tx8, src1Tx8);
        sumBx8 = _mm_add_epi16 (src0Bx8, src1Bx8);

        sumTx8 = _mm_mulhrs_epi16 (sumTx8, c256x8);
        sumBx8 = _mm_mulhrs_epi16 (sumBx8, c256x8);

        sumTx8 = _mm_add_epi16 (sumTx8, c128x8);
        sumBx8 = _mm_add_epi16 (sumBx8, c128x8);

        sumx16 = _mm_packus_epi16 (sumTx8, sumBx8);

        _mm_storel_epi64 ((__m128i *)(dst),             sumx16);
        _mm_storel_epi64 ((__m128i *)(dst + dstStride), _mm_srli_si128 (sumx16, 8));

        src0 += src0Stride*2;
        src1 += src1Stride*2;
        dst  += dstStride*2;

    }

}

void Xin266InterpAvgGt8xHS16U8_SSSE3 (
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
    __m128i c256x8;
    __m128i c128x8;
    __m128i src0Lx8, src0Rx8;
    __m128i src1Lx8, src1Rx8;
    __m128i sumLx8, sumRx8;
    __m128i sumx16;
    UINT8   *dstRow;
    const SINT16  *src0Row;
    const SINT16  *src1Row;

    c256x8 = _mm_set1_epi16 (256);
    c128x8 = _mm_srli_epi16 (c256x8, 1);

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        for (colIdx = 0; colIdx < width; colIdx += 16)
        {
            src0Row = src0 + colIdx;
            src1Row = src1 + colIdx;
            dstRow  = dst + colIdx;

            src0Lx8 = _mm_lddqu_si128 ((__m128i *)src0Row);
            src0Rx8 = _mm_lddqu_si128 ((__m128i *)(src0Row + 8));

            src1Lx8 = _mm_lddqu_si128 ((__m128i *)src1Row);
            src1Rx8 = _mm_lddqu_si128 ((__m128i *)(src1Row + 8));

            sumLx8 = _mm_add_epi16 (src0Lx8, src1Lx8);
            sumRx8 = _mm_add_epi16 (src0Rx8, src1Rx8);

            sumLx8 = _mm_mulhrs_epi16 (sumLx8, c256x8);
            sumRx8 = _mm_mulhrs_epi16 (sumRx8, c256x8);

            sumLx8 = _mm_add_epi16 (sumLx8, c128x8);
            sumRx8 = _mm_add_epi16 (sumRx8, c128x8);

            sumx16 = _mm_packus_epi16 (sumLx8, sumRx8);

            _mm_storeu_si128 ((__m128i *)(dstRow), sumx16);

        }

        src0 += src0Stride;
        src1 += src1Stride;
        dst  += dstStride;

    }

}

void  Xin266InterpWeightGt4xHS16U8_SSSE3 (
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
    __m128i weightABx4;
    __m128i src0Tx8, src0Bx8, src1Tx8, src1Bx8;
    __m128i srcItlT0x4, srcItlT1x4;
    __m128i srcItlB0x4, srcItlB1x4;
    __m128i dstT0x4, dstT1x4, dstB0x4, dstB1x4;
    __m128i offsetx4;
    __m128i dstTx8, dstBx8;
    SINT32  shift;
    SINT32  offset;
    UINT32  rowIdx, colIdx;

    shift      = XIN_MAX (2, INTERP_PRECISION - XIN_INTERNAL_BIT_DEPTH) + XIN_BCW_LOG_WGT_BASE;
    offset     = (1 << (shift - 1)) + (INTERP_PREC_OFFSET << XIN_BCW_LOG_WGT_BASE);
    weightABx4 = _mm_set1_epi32 ((weightB << 16) | (weightA & 0xFFFF));
    offsetx4   = _mm_set1_epi32 (offset);

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        for (colIdx = 0; colIdx < width; colIdx += 8)
        {
            src0Tx8 = _mm_lddqu_si128 ((__m128i *)(src0 + colIdx));
            src0Bx8 = _mm_lddqu_si128 ((__m128i *)(src0 + colIdx + src0Stride));

            src1Tx8 = _mm_lddqu_si128 ((__m128i *)(src1 + colIdx));
            src1Bx8 = _mm_lddqu_si128 ((__m128i *)(src1 + colIdx + src1Stride));

            srcItlT0x4 = _mm_unpacklo_epi16 (src0Tx8, src1Tx8);
            srcItlT1x4 = _mm_unpackhi_epi16 (src0Tx8, src1Tx8);

            srcItlB0x4 = _mm_unpacklo_epi16 (src0Bx8, src1Bx8);
            srcItlB1x4 = _mm_unpackhi_epi16 (src0Bx8, src1Bx8);

            dstT0x4 = _mm_madd_epi16 (srcItlT0x4, weightABx4);
            dstT1x4 = _mm_madd_epi16 (srcItlT1x4, weightABx4);

            dstB0x4 = _mm_madd_epi16 (srcItlB0x4, weightABx4);
            dstB1x4 = _mm_madd_epi16 (srcItlB1x4, weightABx4);

            dstT0x4 = _mm_add_epi32 (dstT0x4, offsetx4);
            dstT1x4 = _mm_add_epi32 (dstT1x4, offsetx4);

            dstB0x4 = _mm_add_epi32 (dstB0x4, offsetx4);
            dstB1x4 = _mm_add_epi32 (dstB1x4, offsetx4);

            dstT0x4 = _mm_srai_epi32 (dstT0x4, shift);
            dstT1x4 = _mm_srai_epi32 (dstT1x4, shift);

            dstB0x4 = _mm_srai_epi32 (dstB0x4, shift);
            dstB1x4 = _mm_srai_epi32 (dstB1x4, shift);

            dstTx8 = _mm_packs_epi32 (dstT0x4, dstT1x4);
            dstBx8 = _mm_packs_epi32 (dstB0x4, dstB1x4);

            dstTx8 = _mm_packus_epi16 (dstTx8, dstTx8);
            dstBx8 = _mm_packus_epi16 (dstBx8, dstBx8);

            _mm_storel_epi64 ((__m128i *)(dst + colIdx),             dstTx8);
            _mm_storel_epi64 ((__m128i *)(dst + colIdx + dstStride), dstBx8);
            
        }

        src0 += 2*src0Stride;
        src1 += 2*src1Stride;
        dst  += 2*dstStride;

    }
    
}


