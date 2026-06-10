/***************************************************************************//**
 *
 * @file          h265p_intra_pred_filter_avx2.c
 * @brief         av1 intra prediction filter subroutines (AVX2).
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
#include "h26x_common_data.h"
#include "basic_macro.h"
#include "string.h"
#include "assert.h"
#include "h265p_intra_pred_filter.h"

static UINT8 EvenOddMaskx[8][16] =
{
    { 0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15 },
    { 0, 1, 3, 5, 7, 9, 11, 13, 0, 2, 4, 6, 8, 10, 12, 14 },
    { 0, 0, 2, 4, 6, 8, 10, 12, 0, 0, 3, 5, 7, 9, 11, 13 },
    { 0, 0, 0, 3, 5, 7, 9, 11, 0, 0, 0, 4, 6, 8, 10, 12 },
    { 0, 0, 0, 0, 4, 6, 8, 10, 0, 0, 0, 0, 5, 7, 9, 11 },
    { 0, 0, 0, 0, 0, 5, 7, 9, 0, 0, 0, 0, 0, 6, 8, 10 },
    { 0, 0, 0, 0, 0, 0, 6, 8, 0, 0, 0, 0, 0, 0, 7, 9 },
    { 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 8 }
};

static UINT8 LoadMaskx[16][16] =
{
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
    { 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 },
    { 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 },
    { 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 },
    { 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 },
    { 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 },
    { 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};

static UINT8 BaseMask[33][32] =
{
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },
    {
        0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },
    {
        0xff, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0,    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },
    {
        0xff, 0xff, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0,    0,    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },
    {
        0xff, 0xff, 0xff, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0,    0,    0,    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0,    0,    0,    0,    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0,    0,    0,    0,    0,    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0,    0,    0,    0,    0,    0,    0,    0, 0, 0, 0, 0, 0, 0, 0, 0
    },
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0, 0, 0, 0, 0, 0, 0, 0,
        0,    0,    0,    0,    0,    0,    0,    0,    0, 0, 0, 0, 0, 0, 0, 0
    },
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0, 0, 0, 0, 0, 0, 0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 0, 0, 0, 0, 0, 0
    },
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0
    },
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0
    },
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0
    },
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0,    0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0
    },
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0,    0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0
    },
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0
    },
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0,    0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0
    },
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0
    },
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0
    },
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0
    },
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0
    },
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0
    },
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0
    },
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0,    0,    0,    0,    0,    0,    0,    0,    0
    },
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0,    0,    0,    0,    0,    0,    0,    0
    },
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0,    0,    0,    0,    0,    0,    0
    },
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0,    0,    0,    0,    0,    0
    },
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0,    0,    0,    0,    0
    },
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0,    0,    0,    0
    },
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0,    0,    0
    },
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0,    0
    },
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0
    },
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    },
};

static SINT32 LoadMaskz2[8][8] =
{
    { -1,  0,  0,  0,  0,  0,  0,  0},
    { -1, -1,  0,  0,  0,  0,  0,  0},
    { -1, -1, -1,  0,  0,  0,  0,  0},
    { -1, -1, -1, -1,  0,  0,  0,  0},
    { -1, -1, -1, -1, -1,  0,  0,  0},
    { -1, -1, -1, -1, -1, -1,  0,  0},
    { -1, -1, -1, -1, -1, -1, -1,  0},
    { -1, -1, -1, -1, -1, -1, -1, -1},
};

// Directional prediction, zone 2: 90 < angle < 180
void Xin265pIntraPred4xHDrZ2_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf,
    BOOL     upSampleTop,
    BOOL     upSampleLft,
    SINT32   dx,
    SINT32   dy)
{
    const int min_base_x = -(1 << upSampleTop);
    const int min_base_y = -(1 << upSampleLft);
    const int frac_bits_x = 6 - upSampleTop;
    const int frac_bits_y = 6 - upSampleLft;
    SINT16 base_y_c[8];

    assert(dx > 0);
    (void)width;

    // pre-filter above pixels
    // store in temp buffers:
    //   above[x] * 32 + 16
    //   above[x+1] - above[x]
    // final pixels will be calculated as:
    //   (above[x] * 32 + 16 + (above[x+1] - above[x]) * shift) >> 5
    __m128i a0_x, a1_x, a32, a16, diff;
    __m128i c3f, min_base_y128, c1234, dy128;

    a16 = _mm_set1_epi16(16);
    c3f = _mm_set1_epi16(0x3f);
    min_base_y128 = _mm_set1_epi16((SINT16)min_base_y);
    c1234 = _mm_setr_epi16(0, 1, 2, 3, 4, 0, 0, 0);
    dy128 = _mm_set1_epi16((SINT16)dy);

    for (int r = 0; r < height; r++)
    {
        __m128i b, res, shift, r6, ydx;
        __m128i resx, resy, resxy;
        __m128i a0_x128, a1_x128;
        int y = r + 1;
        int base_x = (-y * dx) >> frac_bits_x;
        int base_shift = 0;
        if (base_x < (min_base_x - 1))
        {
            base_shift = (min_base_x - base_x - 1) >> upSampleTop;
        }
        int base_min_diff =
            (min_base_x - base_x + upSampleTop) >> upSampleTop;
        if (base_min_diff > 4)
        {
            base_min_diff = 4;
        }
        else
        {
            if (base_min_diff < 0) base_min_diff = 0;
        }

        if (base_shift > 3)
        {
            a0_x = _mm_setzero_si128();
            a1_x = _mm_setzero_si128();
            shift = _mm_setzero_si128();
        }
        else
        {
            a0_x128 = _mm_loadu_si128((__m128i *)(topBuf + base_x + base_shift));
            ydx = _mm_set1_epi16((SINT16)(y * dx));
            r6 = _mm_slli_epi16(c1234, 6);

            if (upSampleTop)
            {
                a0_x128 =
                    _mm_shuffle_epi8(a0_x128, *(__m128i *)EvenOddMaskx[base_shift]);
                a1_x128 = _mm_srli_si128(a0_x128, 8);

                shift = _mm_srli_epi16(
                            _mm_and_si128(
                                _mm_slli_epi16(_mm_sub_epi16(r6, ydx), upSampleTop), c3f),
                            1);
            }
            else
            {
                a0_x128 = _mm_shuffle_epi8(a0_x128, *(__m128i *)LoadMaskx[base_shift]);
                a1_x128 = _mm_srli_si128(a0_x128, 1);

                shift = _mm_srli_epi16(_mm_and_si128(_mm_sub_epi16(r6, ydx), c3f), 1);
            }
            a0_x = _mm_cvtepu8_epi16(a0_x128);
            a1_x = _mm_cvtepu8_epi16(a1_x128);
        }
        // y calc
        __m128i a0_y, a1_y, shifty;
        if (base_x < min_base_x)
        {

            __m128i y_c128, base_y_c128, mask128, c1234_;
            c1234_ = _mm_srli_si128(c1234, 2);
            r6 = _mm_set1_epi16((SINT16)(r << 6));
            y_c128 = _mm_sub_epi16(r6, _mm_mullo_epi16(c1234_, dy128));
            base_y_c128 = _mm_srai_epi16(y_c128, frac_bits_y);
            mask128 = _mm_cmpgt_epi16(min_base_y128, base_y_c128);
            base_y_c128 = _mm_andnot_si128(mask128, base_y_c128);
            _mm_storeu_si128((__m128i *)base_y_c, base_y_c128);

            a0_y = _mm_setr_epi16(lftBuf[base_y_c[0]], lftBuf[base_y_c[1]],
                                  lftBuf[base_y_c[2]], lftBuf[base_y_c[3]], 0, 0, 0, 0);
            base_y_c128 = _mm_add_epi16(base_y_c128, _mm_srli_epi16(a16, 4));
            _mm_storeu_si128((__m128i *)base_y_c, base_y_c128);
            a1_y = _mm_setr_epi16(lftBuf[base_y_c[0]], lftBuf[base_y_c[1]],
                                  lftBuf[base_y_c[2]], lftBuf[base_y_c[3]], 0, 0, 0, 0);

            if (upSampleLft)
            {
                shifty = _mm_srli_epi16(
                             _mm_and_si128(_mm_slli_epi16(y_c128, upSampleLft), c3f), 1);
            }
            else
            {
                shifty = _mm_srli_epi16(_mm_and_si128(y_c128, c3f), 1);
            }
            a0_x = _mm_unpacklo_epi64(a0_x, a0_y);
            a1_x = _mm_unpacklo_epi64(a1_x, a1_y);
            shift = _mm_unpacklo_epi64(shift, shifty);
        }

        diff = _mm_sub_epi16(a1_x, a0_x);  // a[x+1] - a[x]
        a32 = _mm_slli_epi16(a0_x, 5);     // a[x] * 32
        a32 = _mm_add_epi16(a32, a16);     // a[x] * 32 + 16

        b = _mm_mullo_epi16(diff, shift);
        res = _mm_add_epi16(a32, b);
        res = _mm_srli_epi16(res, 5);

        resx = _mm_packus_epi16(res, res);
        resy = _mm_srli_si128(resx, 4);

        resxy = _mm_blendv_epi8(resx, resy, *(__m128i *)BaseMask[base_min_diff]);
        *(uint32_t *)(dst) = _mm_cvtsi128_si32(resxy);
        dst += dstStride;
    }
}

void Xin265pIntraPred8xHDrZ2_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf,
    BOOL     upSampleTop,
    BOOL     upSampleLft,
    SINT32   dx,
    SINT32   dy)

{
    const int min_base_x = -(1 << upSampleTop);
    const int min_base_y = -(1 << upSampleLft);
    const int frac_bits_x = 6 - upSampleTop;
    const int frac_bits_y = 6 - upSampleLft;

    SINT16 base_y_c[16];

    // pre-filter above pixels
    // store in temp buffers:
    //   above[x] * 32 + 16
    //   above[x+1] - above[x]
    // final pixels will be calculated as:
    //   (above[x] * 32 + 16 + (above[x+1] - above[x]) * shift) >> 5
    __m256i diff, a32, a16;
    __m256i a0_x, a1_x;
    __m128i a0_x128, a1_x128, min_base_y128, c3f;
    __m128i c1234, dy128;

    (void)width;
    a16 = _mm256_set1_epi16(16);
    c3f = _mm_set1_epi16(0x3f);
    min_base_y128 = _mm_set1_epi16((SINT16)min_base_y);
    dy128 = _mm_set1_epi16((SINT16)dy);
    c1234 = _mm_setr_epi16(1, 2, 3, 4, 5, 6, 7, 8);

    for (int r = 0; r < height; r++)
    {
        __m256i b, res, shift;
        __m128i resx, resy, resxy, r6, ydx;

        int y = r + 1;
        int base_x = (-y * dx) >> frac_bits_x;
        int base_shift = 0;
        if (base_x < (min_base_x - 1))
        {
            base_shift = (min_base_x - base_x - 1) >> upSampleTop;
        }
        int base_min_diff =
            (min_base_x - base_x + upSampleTop) >> upSampleTop;
        if (base_min_diff > 8)
        {
            base_min_diff = 8;
        }
        else
        {
            if (base_min_diff < 0) base_min_diff = 0;
        }

        if (base_shift > 7)
        {
            a0_x = _mm256_setzero_si256();
            a1_x = _mm256_setzero_si256();
            shift = _mm256_setzero_si256();
        }
        else
        {
            a0_x128 = _mm_loadu_si128((__m128i *)(topBuf + base_x + base_shift));
            ydx = _mm_set1_epi16((SINT16)(y * dx));
            r6 = _mm_slli_epi16(_mm_srli_si128(c1234, 2), 6);

            if (upSampleTop)
            {
                a0_x128 =
                    _mm_shuffle_epi8(a0_x128, *(__m128i *)EvenOddMaskx[base_shift]);
                a1_x128 = _mm_srli_si128(a0_x128, 8);

                shift = _mm256_castsi128_si256(_mm_srli_epi16(
                                                   _mm_and_si128(
                                                       _mm_slli_epi16(_mm_sub_epi16(r6, ydx), upSampleTop), c3f),
                                                   1));
            }
            else
            {
                a1_x128 = _mm_srli_si128(a0_x128, 1);
                a0_x128 = _mm_shuffle_epi8(a0_x128, *(__m128i *)LoadMaskx[base_shift]);
                a1_x128 = _mm_shuffle_epi8(a1_x128, *(__m128i *)LoadMaskx[base_shift]);

                shift = _mm256_castsi128_si256 (
                            _mm_srli_epi16(_mm_and_si128(_mm_sub_epi16(r6, ydx), c3f), 1));
            }
            a0_x = _mm256_castsi128_si256(_mm_cvtepu8_epi16(a0_x128));
            a1_x = _mm256_castsi128_si256(_mm_cvtepu8_epi16(a1_x128));
        }

        // y calc
        __m128i a0_y, a1_y, shifty;
        if (base_x < min_base_x)
        {
            __m128i y_c128, base_y_c128, mask128;
            r6 = _mm_set1_epi16((SINT16)(r << 6));
            y_c128 = _mm_sub_epi16(r6, _mm_mullo_epi16(c1234, dy128));
            base_y_c128 = _mm_srai_epi16(y_c128, frac_bits_y);
            mask128 = _mm_cmpgt_epi16(min_base_y128, base_y_c128);
            base_y_c128 = _mm_andnot_si128(mask128, base_y_c128);
            _mm_storeu_si128((__m128i *)base_y_c, base_y_c128);

            a0_y = _mm_setr_epi16(lftBuf[base_y_c[0]], lftBuf[base_y_c[1]],
                                  lftBuf[base_y_c[2]], lftBuf[base_y_c[3]],
                                  lftBuf[base_y_c[4]], lftBuf[base_y_c[5]],
                                  lftBuf[base_y_c[6]], lftBuf[base_y_c[7]]);
            base_y_c128 = _mm_add_epi16(
                              base_y_c128, _mm_srli_epi16(_mm256_castsi256_si128(a16), 4));
            _mm_storeu_si128((__m128i *)base_y_c, base_y_c128);

            a1_y = _mm_setr_epi16(lftBuf[base_y_c[0]], lftBuf[base_y_c[1]],
                                  lftBuf[base_y_c[2]], lftBuf[base_y_c[3]],
                                  lftBuf[base_y_c[4]], lftBuf[base_y_c[5]],
                                  lftBuf[base_y_c[6]], lftBuf[base_y_c[7]]);

            if (upSampleLft)
            {
                shifty = _mm_srli_epi16(
                             _mm_and_si128(_mm_slli_epi16(y_c128, upSampleLft), c3f), 1);
            }
            else
            {
                shifty = _mm_srli_epi16(_mm_and_si128(y_c128, c3f), 1);
            }

            a0_x = _mm256_inserti128_si256(a0_x, a0_y, 1);
            a1_x = _mm256_inserti128_si256(a1_x, a1_y, 1);
            shift = _mm256_inserti128_si256(shift, shifty, 1);
        }

        diff = _mm256_sub_epi16(a1_x, a0_x);  // a[x+1] - a[x]
        a32 = _mm256_slli_epi16(a0_x, 5);     // a[x] * 32
        a32 = _mm256_add_epi16(a32, a16);     // a[x] * 32 + 16

        b = _mm256_mullo_epi16(diff, shift);
        res = _mm256_add_epi16(a32, b);
        res = _mm256_srli_epi16(res, 5);

        resx = _mm_packus_epi16(_mm256_castsi256_si128(res),
                                _mm256_castsi256_si128(res));
        resy = _mm256_extracti128_si256(res, 1);
        resy = _mm_packus_epi16(resy, resy);

        resxy = _mm_blendv_epi8(resx, resy, *(__m128i *)BaseMask[base_min_diff]);
        _mm_storel_epi64((__m128i *)(dst), resxy);
        dst += dstStride;
    }

}

void Xin265pIntraPredWxHDrZ2_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf,
    BOOL     upSampleTop,
    BOOL     upSampleLft,
    SINT32   dx,
    SINT32   dy)
{
    // here upsample_above and upsample_left are 0 by design of
    // av1_use_intra_edge_upsample
    const int min_base_x = -1;
    const int min_base_y = -1;
    (void)upSampleTop;
    (void)upSampleLft;
    const int frac_bits_x = 6;
    const int frac_bits_y = 6;

    __m256i a0_x, a1_x, a0_y, a1_y, a32, a16, c1234, c0123;
    __m256i diff, min_base_y256, c3f, shifty, dy256, c1;
    __m128i a0_x128, a1_x128;

    SINT16 base_y_c[16];
    a16 = _mm256_set1_epi16(16);
    c1 = _mm256_srli_epi16(a16, 4);
    min_base_y256 = _mm256_set1_epi16((SINT16)min_base_y);
    c3f = _mm256_set1_epi16(0x3f);
    dy256 = _mm256_set1_epi16((SINT16)dy);
    c0123 =
        _mm256_setr_epi16(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    c1234 = _mm256_add_epi16(c0123, c1);

    for (int r = 0; r < height; r++)
    {
        __m256i b, res, shift, j256, r6, ydx;
        __m128i resx, resy;
        __m128i resxy;
        int y = r + 1;
        ydx = _mm256_set1_epi16((uint16_t)(y * dx));

        int base_x = (-y * dx) >> frac_bits_x;
        for (int j = 0; j < width; j += 16)
        {
            j256 = _mm256_set1_epi16((SINT16)j);
            int base_shift = 0;
            if ((base_x + j) < (min_base_x - 1))
            {
                base_shift = (min_base_x - (base_x + j) - 1);
            }
            int base_min_diff = (min_base_x - base_x - j);
            if (base_min_diff > 16)
            {
                base_min_diff = 16;
            }
            else
            {
                if (base_min_diff < 0) base_min_diff = 0;
            }

            if (base_shift < 16)
            {
                a0_x128 = _mm_loadu_si128((__m128i *)(topBuf + base_x + base_shift + j));
                a1_x128 =
                    _mm_loadu_si128((__m128i *)(topBuf + base_x + base_shift + 1 + j));
                a0_x128 = _mm_shuffle_epi8(a0_x128, *(__m128i *)LoadMaskx[base_shift]);
                a1_x128 = _mm_shuffle_epi8(a1_x128, *(__m128i *)LoadMaskx[base_shift]);

                a0_x = _mm256_cvtepu8_epi16(a0_x128);
                a1_x = _mm256_cvtepu8_epi16(a1_x128);

                r6 = _mm256_slli_epi16(_mm256_add_epi16(c0123, j256), 6);
                shift = _mm256_srli_epi16(
                            _mm256_and_si256(_mm256_sub_epi16(r6, ydx), c3f), 1);

                diff = _mm256_sub_epi16(a1_x, a0_x);  // a[x+1] - a[x]
                a32 = _mm256_slli_epi16(a0_x, 5);     // a[x] * 32
                a32 = _mm256_add_epi16(a32, a16);     // a[x] * 32 + 16

                b = _mm256_mullo_epi16(diff, shift);
                res = _mm256_add_epi16(a32, b);
                res = _mm256_srli_epi16(res, 5);  // 16 16-bit values
                resx = _mm256_castsi256_si128(_mm256_packus_epi16(
                                                  res, _mm256_castsi128_si256(_mm256_extracti128_si256(res, 1))));
            }
            else
            {
                resx = _mm_setzero_si128();
            }

            // y calc
            if (base_x < min_base_x)
            {
                __m256i c256, y_c256, base_y_c256, mask256, mul16;
                r6 = _mm256_set1_epi16((SINT16)(r << 6));
                c256 = _mm256_add_epi16(j256, c1234);
                mul16 = _mm256_min_epu16(_mm256_mullo_epi16(c256, dy256),
                                         _mm256_srli_epi16(min_base_y256, 1));
                y_c256 = _mm256_sub_epi16(r6, mul16);

                base_y_c256 = _mm256_srai_epi16(y_c256, frac_bits_y);
                mask256 = _mm256_cmpgt_epi16(min_base_y256, base_y_c256);

                base_y_c256 = _mm256_blendv_epi8(base_y_c256, min_base_y256, mask256);
                int16_t min_y = (int16_t)_mm_extract_epi16(
                                    _mm256_extracti128_si256(base_y_c256, 1), 7);
                int16_t max_y =
                    (int16_t)_mm_extract_epi16(_mm256_castsi256_si128(base_y_c256), 0);
                int16_t offset_diff = max_y - min_y;

                if (offset_diff < 16)
                {
                    __m256i min_y256 = _mm256_set1_epi16(min_y);

                    __m256i base_y_offset = _mm256_sub_epi16(base_y_c256, min_y256);
                    __m128i base_y_offset128 =
                        _mm_packs_epi16(_mm256_extracti128_si256(base_y_offset, 0),
                                        _mm256_extracti128_si256(base_y_offset, 1));

                    __m128i a0_y128 = _mm_maskload_epi32(
                                          (int *)(lftBuf + min_y), *(__m128i *)LoadMaskz2[offset_diff / 4]);
                    __m128i a1_y128 =
                        _mm_maskload_epi32((int *)(lftBuf + min_y + 1),
                                           *(__m128i *)LoadMaskz2[offset_diff / 4]);
                    a0_y128 = _mm_shuffle_epi8(a0_y128, base_y_offset128);
                    a1_y128 = _mm_shuffle_epi8(a1_y128, base_y_offset128);
                    a0_y = _mm256_cvtepu8_epi16(a0_y128);
                    a1_y = _mm256_cvtepu8_epi16(a1_y128);
                }
                else
                {
                    base_y_c256 = _mm256_andnot_si256(mask256, base_y_c256);
                    _mm256_storeu_si256((__m256i *)base_y_c, base_y_c256);

                    a0_y = _mm256_setr_epi16(
                               lftBuf[base_y_c[0]], lftBuf[base_y_c[1]], lftBuf[base_y_c[2]],
                               lftBuf[base_y_c[3]], lftBuf[base_y_c[4]], lftBuf[base_y_c[5]],
                               lftBuf[base_y_c[6]], lftBuf[base_y_c[7]], lftBuf[base_y_c[8]],
                               lftBuf[base_y_c[9]], lftBuf[base_y_c[10]], lftBuf[base_y_c[11]],
                               lftBuf[base_y_c[12]], lftBuf[base_y_c[13]], lftBuf[base_y_c[14]],
                               lftBuf[base_y_c[15]]);
                    base_y_c256 = _mm256_add_epi16(base_y_c256, c1);
                    _mm256_storeu_si256((__m256i *)base_y_c, base_y_c256);

                    a1_y = _mm256_setr_epi16(
                               lftBuf[base_y_c[0]], lftBuf[base_y_c[1]], lftBuf[base_y_c[2]],
                               lftBuf[base_y_c[3]], lftBuf[base_y_c[4]], lftBuf[base_y_c[5]],
                               lftBuf[base_y_c[6]], lftBuf[base_y_c[7]], lftBuf[base_y_c[8]],
                               lftBuf[base_y_c[9]], lftBuf[base_y_c[10]], lftBuf[base_y_c[11]],
                               lftBuf[base_y_c[12]], lftBuf[base_y_c[13]], lftBuf[base_y_c[14]],
                               lftBuf[base_y_c[15]]);
                }
                shifty = _mm256_srli_epi16(_mm256_and_si256(y_c256, c3f), 1);

                diff = _mm256_sub_epi16(a1_y, a0_y);  // a[x+1] - a[x]
                a32 = _mm256_slli_epi16(a0_y, 5);     // a[x] * 32
                a32 = _mm256_add_epi16(a32, a16);     // a[x] * 32 + 16

                b = _mm256_mullo_epi16(diff, shifty);
                res = _mm256_add_epi16(a32, b);
                res = _mm256_srli_epi16(res, 5);  // 16 16-bit values
                resy = _mm256_castsi256_si128(_mm256_packus_epi16(
                                                  res, _mm256_castsi128_si256(_mm256_extracti128_si256(res, 1))));
            }
            else
            {
                resy = _mm_setzero_si128();
            }
            resxy = _mm_blendv_epi8(resx, resy, *(__m128i *)BaseMask[base_min_diff]);
            _mm_storeu_si128((__m128i *)(dst + j), resxy);
        }  // for j

        dst += dstStride;
    }

}

static void dr_prediction_z1_HxW_internal_avx2 (
    SINT32  height,
    SINT32  width,
    __m128i *dst,
    PIXEL   *topBuf,
    BOOL    upSampleTop,
    SINT32  dx)
{
    const int frac_bits = 6 - upSampleTop;
    const int max_base_x = ((width + height) - 1) << upSampleTop;

    assert(dx > 0);
    // pre-filter above pixels
    // store in temp buffers:
    //   above[x] * 32 + 16
    //   above[x+1] - above[x]
    // final pixels will be calculated as:
    //   (above[x] * 32 + 16 + (above[x+1] - above[x]) * shift) >> 5
    __m256i a0, a1, a32, a16;
    __m256i diff, c3f;
    __m128i a_mbase_x;

    a16 = _mm256_set1_epi16(16);
    a_mbase_x = _mm_set1_epi8(topBuf[max_base_x]);
    c3f = _mm256_set1_epi16(0x3f);

    int x = dx;
    for (int r = 0; r < width; r++)
    {
        __m256i b, res, shift;
        __m128i res1, a0_128, a1_128;

        int base = x >> frac_bits;
        int base_max_diff = (max_base_x - base) >> upSampleTop;
        if (base_max_diff <= 0)
        {
            for (int i = r; i < width; ++i)
            {
                dst[i] = a_mbase_x;  // save 4 values
            }
            return;
        }
        if (base_max_diff > height) base_max_diff = height;
        a0_128 = _mm_loadu_si128((__m128i *)(topBuf + base));
        a1_128 = _mm_loadu_si128((__m128i *)(topBuf + base + 1));

        if (upSampleTop)
        {
            a0_128 = _mm_shuffle_epi8(a0_128, *(__m128i *)EvenOddMaskx[0]);
            a1_128 = _mm_srli_si128(a0_128, 8);

            shift = _mm256_srli_epi16(
                        _mm256_and_si256(
                            _mm256_slli_epi16(_mm256_set1_epi16((SINT16)x), upSampleTop), c3f),
                        1);
        }
        else
        {
            shift = _mm256_srli_epi16(_mm256_and_si256(_mm256_set1_epi16((SINT16)x), c3f), 1);
        }
        a0 = _mm256_cvtepu8_epi16(a0_128);
        a1 = _mm256_cvtepu8_epi16(a1_128);

        diff = _mm256_sub_epi16(a1, a0);   // a[x+1] - a[x]
        a32 = _mm256_slli_epi16(a0, 5);    // a[x] * 32
        a32 = _mm256_add_epi16(a32, a16);  // a[x] * 32 + 16

        b = _mm256_mullo_epi16(diff, shift);
        res = _mm256_add_epi16(a32, b);
        res = _mm256_srli_epi16(res, 5);

        res = _mm256_packus_epi16(
                  res, _mm256_castsi128_si256(
                      _mm256_extracti128_si256(res, 1)));  // goto 8 bit
        res1 = _mm256_castsi256_si128(res);               // 16 8bit values

        dst[r] =
            _mm_blendv_epi8(a_mbase_x, res1, *(__m128i *)BaseMask[base_max_diff]);
        x += dx;
    }
}

void Xin265pIntraPred4xHDrZ1_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf,
    BOOL     upSampleTop,
    SINT32   dx,
    SINT32   dy)
{
    __m128i dstvec[16];

    (void)width;
    (void)dy;
    (void)lftBuf;

    dr_prediction_z1_HxW_internal_avx2 (
        4,
        height,
        dstvec,
        topBuf,
        upSampleTop,
        dx);

    for (int i = 0; i < height; i++)
    {
        *(UINT32 *)(dst + dstStride * i) = _mm_cvtsi128_si32(dstvec[i]);
    }

}

void Xin265pIntraPred8xHDrZ1_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf,
    BOOL     upSampleTop,
    SINT32   dx,
    SINT32   dy)
{
    __m128i dstvec[32];

    (void)width;
    (void)dy;
    (void)lftBuf;

    dr_prediction_z1_HxW_internal_avx2 (
        8,
        height,
        dstvec,
        topBuf,
        upSampleTop,
        dx);

    for (int i = 0; i < height; i++)
    {
        _mm_storel_epi64((__m128i *)(dst + dstStride * i), dstvec[i]);
    }

}

void Xin265pIntraPred16xHDrZ1_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf,
    BOOL     upSampleTop,
    SINT32   dx,
    SINT32   dy)
{
    __m128i dstvec[64];

    (void)width;
    (void)dy;
    (void)lftBuf;

    dr_prediction_z1_HxW_internal_avx2 (
        16,
        height,
        dstvec,
        topBuf,
        upSampleTop,
        dx);

    for (int i = 0; i < height; i++)
    {
        _mm_storeu_si128((__m128i *)(dst + dstStride * i), dstvec[i]);
    }

}

void Xin265pIntraPred32xHDrZ1_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf,
    BOOL     upSampleTop,
    SINT32   dx,
    SINT32   dy)
{
    const int frac_bits = 6;
    const int max_base_x = ((32 + height) - 1);
    __m256i dstx32;

    // pre-filter above pixels
    // store in temp buffers:
    //   above[x] * 32 + 16
    //   above[x+1] - above[x]
    // final pixels will be calculated as:
    //   (above[x] * 32 + 16 + (above[x+1] - above[x]) * shift) >> 5
    __m256i a0, a1, a32, a16;
    __m256i a_mbase_x, diff, c3f;

    (void)upSampleTop;
    (void)lftBuf;
    (void)dy;
    (void)width;

    a16 = _mm256_set1_epi16(16);
    a_mbase_x = _mm256_set1_epi8(topBuf[max_base_x]);
    c3f = _mm256_set1_epi16(0x3f);

    int x = dx;
    for (int r = 0; r < height; r++)
    {
        __m256i b, res, res16[2];
        __m128i a0_128, a1_128;

        int base = x >> frac_bits;
        int base_max_diff = (max_base_x - base);
        if (base_max_diff <= 0)
        {
            for (int i = r; i < height; ++i)
            {
                _mm256_storeu_si256 ((__m256i *)dst, a_mbase_x);  // save 32 values

                dst += dstStride;
            }
            return;
        }

        if (base_max_diff > 32) base_max_diff = 32;
        __m256i shift =
            _mm256_srli_epi16(_mm256_and_si256(_mm256_set1_epi16((SINT16)x), c3f), 1);

        for (int j = 0, jj = 0; j < 32; j += 16, jj++)
        {
            int mdiff = base_max_diff - j;
            if (mdiff <= 0)
            {
                res16[jj] = a_mbase_x;
            }
            else
            {
                a0_128 = _mm_loadu_si128((__m128i *)(topBuf + base + j));
                a1_128 = _mm_loadu_si128((__m128i *)(topBuf + base + j + 1));
                a0 = _mm256_cvtepu8_epi16(a0_128);
                a1 = _mm256_cvtepu8_epi16(a1_128);

                diff = _mm256_sub_epi16(a1, a0);   // a[x+1] - a[x]
                a32 = _mm256_slli_epi16(a0, 5);    // a[x] * 32
                a32 = _mm256_add_epi16(a32, a16);  // a[x] * 32 + 16
                b = _mm256_mullo_epi16(diff, shift);

                res = _mm256_add_epi16(a32, b);
                res = _mm256_srli_epi16(res, 5);
                res16[jj] = _mm256_packus_epi16(
                                res, _mm256_castsi128_si256(
                                    _mm256_extracti128_si256(res, 1)));  // 16 8bit values
            }
        }

        res16[1] =
            _mm256_inserti128_si256(res16[0], _mm256_castsi256_si128(res16[1]),
                                    1);  // 32 8bit values

        dstx32 = _mm256_blendv_epi8(
                     a_mbase_x, res16[1],
                     *(__m256i *)BaseMask[base_max_diff]);  // 32 8bit values

        _mm256_storeu_si256 ((__m256i *)dst, dstx32);

        dst += dstStride;
        x += dx;

    }

}

void Xin265pIntraPred64xHDrZ1_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf,
    BOOL     upSampleTop,
    SINT32   dx,
    SINT32   dy)
{
    const int frac_bits = 6;
    const int max_base_x = ((64 + height) - 1);

    // pre-filter above pixels
    // store in temp buffers:
    //   above[x] * 32 + 16
    //   above[x+1] - above[x]
    // final pixels will be calculated as:
    //   (above[x] * 32 + 16 + (above[x+1] - above[x]) * shift) >> 5
    __m256i a0, a1, a32, a16;
    __m256i a_mbase_x, diff, c3f;
    __m128i max_base_x128, base_inc128, mask128;

    (void)upSampleTop;
    (void)lftBuf;
    (void)dy;
    (void)width;

    a16 = _mm256_set1_epi16(16);
    a_mbase_x = _mm256_set1_epi8(topBuf[max_base_x]);
    max_base_x128 = _mm_set1_epi8((SINT8)max_base_x);
    c3f = _mm256_set1_epi16(0x3f);

    int x = dx;
    for (int r = 0; r < height; r++, dst += dstStride)
    {
        __m256i b, res;
        int base = x >> frac_bits;
        if (base >= max_base_x)
        {
            for (int i = r; i < height; ++i)
            {
                _mm256_storeu_si256((__m256i *)dst, a_mbase_x);  // save 32 values
                _mm256_storeu_si256((__m256i *)(dst + 32), a_mbase_x);
                dst += dstStride;
            }
            return;
        }

        __m256i shift =
            _mm256_srli_epi16(_mm256_and_si256(_mm256_set1_epi16((SINT16)x), c3f), 1);

        __m128i a0_128, a1_128, res128;
        for (int j = 0; j < 64; j += 16)
        {
            int mdif = max_base_x - (base + j);
            if (mdif <= 0)
            {
                _mm_storeu_si128((__m128i *)(dst + j),
                                 _mm256_castsi256_si128(a_mbase_x));
            }
            else
            {
                a0_128 = _mm_loadu_si128((__m128i *)(topBuf + base + j));
                a1_128 = _mm_loadu_si128((__m128i *)(topBuf + base + 1 + j));
                a0 = _mm256_cvtepu8_epi16(a0_128);
                a1 = _mm256_cvtepu8_epi16(a1_128);

                diff = _mm256_sub_epi16(a1, a0);   // a[x+1] - a[x]
                a32 = _mm256_slli_epi16(a0, 5);    // a[x] * 32
                a32 = _mm256_add_epi16(a32, a16);  // a[x] * 32 + 16
                b = _mm256_mullo_epi16(diff, shift);

                res = _mm256_add_epi16(a32, b);
                res = _mm256_srli_epi16(res, 5);
                res = _mm256_packus_epi16(
                          res, _mm256_castsi128_si256(
                              _mm256_extracti128_si256(res, 1)));  // 16 8bit values

                base_inc128 =
                    _mm_setr_epi8((uint8_t)(base + j), (uint8_t)(base + j + 1),
                                  (uint8_t)(base + j + 2), (uint8_t)(base + j + 3),
                                  (uint8_t)(base + j + 4), (uint8_t)(base + j + 5),
                                  (uint8_t)(base + j + 6), (uint8_t)(base + j + 7),
                                  (uint8_t)(base + j + 8), (uint8_t)(base + j + 9),
                                  (uint8_t)(base + j + 10), (uint8_t)(base + j + 11),
                                  (uint8_t)(base + j + 12), (uint8_t)(base + j + 13),
                                  (uint8_t)(base + j + 14), (uint8_t)(base + j + 15));

                mask128 = _mm_cmpgt_epi8(_mm_subs_epu8(max_base_x128, base_inc128),
                                         _mm_setzero_si128());
                res128 = _mm_blendv_epi8(_mm256_castsi256_si128(a_mbase_x),
                                         _mm256_castsi256_si128(res), mask128);
                _mm_storeu_si128((__m128i *)(dst + j), res128);
            }
        }
        x += dx;
    }

}

void Xin265pIntraPredVer32xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    SINT32  rowIdx;
    __m256i topx32;

    (void)lftBuf;
    (void)width;
    topx32 = _mm256_loadu_si256 ((__m256i *)topBuf);

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        _mm256_storeu_si256 ((__m256i *)(dst),             topx32);
        _mm256_storeu_si256 ((__m256i *)(dst + dstStride), topx32);

        dst += 2*dstStride;
    }

}

void Xin265pIntraPredVer64xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    SINT32  rowIdx;
    __m256i top0x32, top1x32;

    (void)lftBuf;
    (void)width;
    top0x32 = _mm256_loadu_si256((__m256i *)topBuf);
    top1x32 = _mm256_loadu_si256((__m256i *)(topBuf + 32));

    for (rowIdx = 0; rowIdx < height; rowIdx += 2)
    {
        _mm256_storeu_si256((__m256i *)(dst),                  top0x32);
        _mm256_storeu_si256((__m256i *)(dst + 32),             top1x32);
        _mm256_storeu_si256((__m256i *)(dst + dstStride),      top0x32);
        _mm256_storeu_si256((__m256i *)(dst + dstStride + 32), top1x32);

        dst += 2 * dstStride;
    }

}

void Xin265pIntraPredHor32xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    SINT32  row;
    __m256i lft0x32;
    __m256i lft1x32;

    (void)topBuf;
    (void)width;


    for (row = 0; row < height; row += 2)
    {
        lft0x32 = _mm256_set1_epi8 (lftBuf[row]);
        lft1x32 = _mm256_set1_epi8 (lftBuf[row+1]);

        _mm256_storeu_si256 ((__m256i *)(dst),             lft0x32);
        _mm256_storeu_si256 ((__m256i *)(dst + dstStride), lft1x32);

        dst += dstStride*2;
    }

}

void Xin265pIntraPredHor64xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    SINT32  row;
    __m256i lft0x32;
    __m256i lft1x32;

    (void)topBuf;
    (void)width;

    for (row = 0; row < height; row += 2)
    {
        lft0x32 = _mm256_set1_epi8 (lftBuf[row]);
        lft1x32 = _mm256_set1_epi8 (lftBuf[row+1]);

        _mm256_storeu_si256 ((__m256i *)(dst),                  lft0x32);
        _mm256_storeu_si256 ((__m256i *)(dst + 32),             lft0x32);
        _mm256_storeu_si256 ((__m256i *)(dst + dstStride),      lft1x32);
        _mm256_storeu_si256 ((__m256i *)(dst + dstStride + 32), lft1x32);

        dst += dstStride*2;
    }

}

void Xin265pIntraPredPaeth32xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    SINT32    rowIdx;
    __m256i   tlx16;
    __m256i   lftx16;
    __m128i   top0;
    __m128i   top1;
    __m256i   top0x16;
    __m256i   top1x16;
    __m256i   base0x16;
    __m256i   base1x16;
    __m256i   plx16;
    __m256i   ptx16;
    __m256i   ptlx16;
    __m256i   ltlx16;
    __m256i   mask0x16;
    __m256i   mask1x16;
    __m256i   dst0x16, dst1x16;
    __m256i   dstx32;

    (void)width;
    tlx16   = _mm256_set1_epi16 (topBuf[-1]);
    top0    = _mm_loadu_si128 ((__m128i *)(topBuf));
    top1    = _mm_loadu_si128 ((__m128i *)(topBuf + 16));
    top0x16 = _mm256_cvtepu8_epi16 (top0);
    top1x16 = _mm256_cvtepu8_epi16 (top1);

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        lftx16 = _mm256_set1_epi16 (lftBuf[rowIdx]);
        ltlx16 = _mm256_sub_epi16 (lftx16, tlx16);

        base0x16 = _mm256_add_epi16 (top0x16,  ltlx16);
        base1x16 = _mm256_add_epi16 (top1x16,  ltlx16);

        plx16  = _mm256_abs_epi16 (_mm256_sub_epi16 (base0x16, lftx16));
        ptx16  = _mm256_abs_epi16 (_mm256_sub_epi16 (base0x16, top0x16));
        ptlx16 = _mm256_abs_epi16 (_mm256_sub_epi16 (base0x16, tlx16));

        mask0x16 = _mm256_or_si256 (_mm256_cmpgt_epi16 (plx16, ptx16), _mm256_cmpgt_epi16 (plx16, ptlx16));
        mask1x16 = _mm256_cmpgt_epi16 (ptx16, ptlx16);

        plx16  = _mm256_andnot_si256 (mask0x16, lftx16);
        ptlx16 = _mm256_and_si256 (mask1x16, tlx16);
        ptx16  = _mm256_andnot_si256 (mask1x16, top0x16);
        ptx16  = _mm256_or_si256 (ptx16, ptlx16);
        ptx16  = _mm256_and_si256 (mask0x16, ptx16);

        dst0x16 = _mm256_or_si256 (ptx16, plx16);

        plx16  = _mm256_abs_epi16 (_mm256_sub_epi16 (base1x16, lftx16));
        ptx16  = _mm256_abs_epi16 (_mm256_sub_epi16 (base1x16, top1x16));
        ptlx16 = _mm256_abs_epi16 (_mm256_sub_epi16 (base1x16, tlx16));

        mask0x16 = _mm256_or_si256 (_mm256_cmpgt_epi16 (plx16, ptx16), _mm256_cmpgt_epi16 (plx16, ptlx16));
        mask1x16 = _mm256_cmpgt_epi16 (ptx16, ptlx16);

        plx16  = _mm256_andnot_si256 (mask0x16, lftx16);
        ptlx16 = _mm256_and_si256 (mask1x16, tlx16);
        ptx16  = _mm256_andnot_si256 (mask1x16, top1x16);
        ptx16  = _mm256_or_si256 (ptx16, ptlx16);
        ptx16  = _mm256_and_si256 (mask0x16, ptx16);

        dst1x16 = _mm256_or_si256 (ptx16, plx16);

        dstx32  = _mm256_permute4x64_epi64 (_mm256_packus_epi16 (dst0x16, dst1x16), 0xD8);

        _mm256_storeu_si256 ((__m256i *)(dst), dstx32);

        dst += dstStride;

    }

}

void Xin265pIntraPredPaeth64xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    SINT32    rowIdx;
    __m256i   tlx16;
    __m256i   lftx16;
    __m128i   top0;
    __m128i   top1;
    __m128i   top2;
    __m128i   top3;
    __m256i   top0x16;
    __m256i   top1x16;
    __m256i   top2x16;
    __m256i   top3x16;
    __m256i   base0x16;
    __m256i   base1x16;
    __m256i   base2x16;
    __m256i   base3x16;
    __m256i   plx16;
    __m256i   ptx16;
    __m256i   ptlx16;
    __m256i   ltlx16;
    __m256i   mask0x16;
    __m256i   mask1x16;
    __m256i   dst0x16, dst1x16;
    __m256i   dst2x16, dst3x16;
    __m256i   dstx32;

    (void)width;
    tlx16   = _mm256_set1_epi16 (topBuf[-1]);
    top0    = _mm_loadu_si128 ((__m128i *)(topBuf));
    top1    = _mm_loadu_si128 ((__m128i *)(topBuf + 16));
    top2    = _mm_loadu_si128 ((__m128i *)(topBuf + 32));
    top3    = _mm_loadu_si128 ((__m128i *)(topBuf + 48));
    top0x16 = _mm256_cvtepu8_epi16 (top0);
    top1x16 = _mm256_cvtepu8_epi16 (top1);
    top2x16 = _mm256_cvtepu8_epi16 (top2);
    top3x16 = _mm256_cvtepu8_epi16 (top3);

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        lftx16 = _mm256_set1_epi16 (lftBuf[rowIdx]);
        ltlx16 = _mm256_sub_epi16 (lftx16, tlx16);

        base0x16 = _mm256_add_epi16 (top0x16,  ltlx16);
        base1x16 = _mm256_add_epi16 (top1x16,  ltlx16);
        base2x16 = _mm256_add_epi16 (top2x16,  ltlx16);
        base3x16 = _mm256_add_epi16 (top3x16,  ltlx16);

        plx16  = _mm256_abs_epi16 (_mm256_sub_epi16 (base0x16, lftx16));
        ptx16  = _mm256_abs_epi16 (_mm256_sub_epi16 (base0x16, top0x16));
        ptlx16 = _mm256_abs_epi16 (_mm256_sub_epi16 (base0x16, tlx16));

        mask0x16 = _mm256_or_si256 (_mm256_cmpgt_epi16 (plx16, ptx16), _mm256_cmpgt_epi16 (plx16, ptlx16));
        mask1x16 = _mm256_cmpgt_epi16 (ptx16, ptlx16);

        plx16  = _mm256_andnot_si256 (mask0x16, lftx16);
        ptlx16 = _mm256_and_si256 (mask1x16, tlx16);
        ptx16  = _mm256_andnot_si256 (mask1x16, top0x16);
        ptx16  = _mm256_or_si256 (ptx16, ptlx16);
        ptx16  = _mm256_and_si256 (mask0x16, ptx16);

        dst0x16 = _mm256_or_si256 (ptx16, plx16);

        plx16  = _mm256_abs_epi16 (_mm256_sub_epi16 (base1x16, lftx16));
        ptx16  = _mm256_abs_epi16 (_mm256_sub_epi16 (base1x16, top1x16));
        ptlx16 = _mm256_abs_epi16 (_mm256_sub_epi16 (base1x16, tlx16));

        mask0x16 = _mm256_or_si256 (_mm256_cmpgt_epi16 (plx16, ptx16), _mm256_cmpgt_epi16 (plx16, ptlx16));
        mask1x16 = _mm256_cmpgt_epi16 (ptx16, ptlx16);

        plx16  = _mm256_andnot_si256 (mask0x16, lftx16);
        ptlx16 = _mm256_and_si256 (mask1x16, tlx16);
        ptx16  = _mm256_andnot_si256 (mask1x16, top1x16);
        ptx16  = _mm256_or_si256 (ptx16, ptlx16);
        ptx16  = _mm256_and_si256 (mask0x16, ptx16);

        dst1x16 = _mm256_or_si256 (ptx16, plx16);

        dstx32  = _mm256_permute4x64_epi64 (_mm256_packus_epi16 (dst0x16, dst1x16), 0xD8);

        _mm256_storeu_si256 ((__m256i *)(dst), dstx32);

        plx16  = _mm256_abs_epi16 (_mm256_sub_epi16 (base2x16, lftx16));
        ptx16  = _mm256_abs_epi16 (_mm256_sub_epi16 (base2x16, top2x16));
        ptlx16 = _mm256_abs_epi16 (_mm256_sub_epi16 (base2x16, tlx16));

        mask0x16 = _mm256_or_si256 (_mm256_cmpgt_epi16 (plx16, ptx16), _mm256_cmpgt_epi16 (plx16, ptlx16));
        mask1x16 = _mm256_cmpgt_epi16 (ptx16, ptlx16);

        plx16  = _mm256_andnot_si256 (mask0x16, lftx16);
        ptlx16 = _mm256_and_si256 (mask1x16, tlx16);
        ptx16  = _mm256_andnot_si256 (mask1x16, top2x16);
        ptx16  = _mm256_or_si256 (ptx16, ptlx16);
        ptx16  = _mm256_and_si256 (mask0x16, ptx16);

        dst2x16 = _mm256_or_si256 (ptx16, plx16);

        plx16  = _mm256_abs_epi16 (_mm256_sub_epi16 (base3x16, lftx16));
        ptx16  = _mm256_abs_epi16 (_mm256_sub_epi16 (base3x16, top3x16));
        ptlx16 = _mm256_abs_epi16 (_mm256_sub_epi16 (base3x16, tlx16));

        mask0x16 = _mm256_or_si256 (_mm256_cmpgt_epi16 (plx16, ptx16), _mm256_cmpgt_epi16 (plx16, ptlx16));
        mask1x16 = _mm256_cmpgt_epi16 (ptx16, ptlx16);

        plx16  = _mm256_andnot_si256 (mask0x16, lftx16);
        ptlx16 = _mm256_and_si256 (mask1x16, tlx16);
        ptx16  = _mm256_andnot_si256 (mask1x16, top3x16);
        ptx16  = _mm256_or_si256 (ptx16, ptlx16);
        ptx16  = _mm256_and_si256 (mask0x16, ptx16);

        dst3x16 = _mm256_or_si256 (ptx16, plx16);

        dstx32  = _mm256_permute4x64_epi64 (_mm256_packus_epi16 (dst2x16, dst3x16), 0xD8);

        _mm256_storeu_si256 ((__m256i *)(dst + 32), dstx32);

        dst += dstStride;

    }

}

void Xin265pIntraPredSmV16xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    UINT16       scale;
    SINT32       rowIdx;
    const UINT16 *smWeight;
    __m256i      botx16;
    __m256i      wghtx8;
    __m256i      topx16;
    __m256i      tb0x8;
    __m256i      tb1x8;
    __m256i      pred0x8;
    __m256i      pred1x8;
    __m256i      predx16;
    __m256i      rndx8;
    __m256i      allZero;

    (void)width;
    allZero  = _mm256_setzero_si256 ();
    botx16   = _mm256_set1_epi16 (lftBuf[height - 1]);
    topx16   = _mm256_cvtepu8_epi16 (_mm_loadu_si128 ((__m128i *)(topBuf)));
    tb0x8    = _mm256_unpacklo_epi16 (topx16, botx16);
    tb1x8    = _mm256_unpackhi_epi16 (topx16, botx16);
    smWeight = intraSmWeightU16 + height;
    scale    = 1 << XIN_LOG_SM_WEIGHT_SCALE;
    rndx8    = _mm256_set1_epi32 (scale>>1);

    for (rowIdx = 0; rowIdx < height; ++rowIdx)
    {
        wghtx8 = _mm256_set1_epi32 ((smWeight[rowIdx]) | ((scale - smWeight[rowIdx]) << 16));

        pred0x8 = _mm256_madd_epi16 (tb0x8, wghtx8);
        pred1x8 = _mm256_madd_epi16 (tb1x8, wghtx8);

        pred0x8 = _mm256_add_epi32 (pred0x8, rndx8);
        pred1x8 = _mm256_add_epi32 (pred1x8, rndx8);

        pred0x8 = _mm256_srli_epi16 (pred0x8, XIN_LOG_SM_WEIGHT_SCALE);
        pred1x8 = _mm256_srli_epi16 (pred1x8, XIN_LOG_SM_WEIGHT_SCALE);

        predx16 = _mm256_packus_epi32 (pred0x8, pred1x8);
        predx16 = _mm256_packus_epi16 (predx16, allZero);
        predx16 = _mm256_permute4x64_epi64 (predx16, 0xD8);

        _mm_storeu_si128 ((__m128i *)dst, _mm256_castsi256_si128 (predx16));

        dst += dstStride;

    }

}

void Xin265pIntraPredSmV32xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    UINT16       scale;
    SINT32       rowIdx;
    const UINT16 *smWeight;
    __m256i      botx16;
    __m256i      wghtx8;
    __m256i      topx16;
    __m256i      tb0x8;
    __m256i      tb1x8;
    __m256i      tb2x8;
    __m256i      tb3x8;
    __m256i      pred0x8;
    __m256i      pred1x8;
    __m256i      pred0x16;
    __m256i      pred1x16;
    __m256i      predx32;
    __m256i      rndx8;

    (void)width;
    botx16   = _mm256_set1_epi16 (lftBuf[height - 1]);
    topx16   = _mm256_cvtepu8_epi16 (_mm_loadu_si128 ((__m128i *)(topBuf)));
    tb0x8    = _mm256_unpacklo_epi16 (topx16, botx16);
    tb1x8    = _mm256_unpackhi_epi16 (topx16, botx16);
    topx16   = _mm256_cvtepu8_epi16 (_mm_loadu_si128 ((__m128i *)(topBuf + 16)));
    tb2x8    = _mm256_unpacklo_epi16 (topx16, botx16);
    tb3x8    = _mm256_unpackhi_epi16 (topx16, botx16);
    smWeight = intraSmWeightU16 + height;
    scale    = 1 << XIN_LOG_SM_WEIGHT_SCALE;
    rndx8    = _mm256_set1_epi32 (scale>>1);

    for (rowIdx = 0; rowIdx < height; ++rowIdx)
    {
        wghtx8 = _mm256_set1_epi32 ((smWeight[rowIdx]) | ((scale - smWeight[rowIdx]) << 16));

        pred0x8 = _mm256_madd_epi16 (tb0x8, wghtx8);
        pred1x8 = _mm256_madd_epi16 (tb1x8, wghtx8);

        pred0x8 = _mm256_add_epi32 (pred0x8, rndx8);
        pred1x8 = _mm256_add_epi32 (pred1x8, rndx8);

        pred0x8 = _mm256_srli_epi16 (pred0x8, XIN_LOG_SM_WEIGHT_SCALE);
        pred1x8 = _mm256_srli_epi16 (pred1x8, XIN_LOG_SM_WEIGHT_SCALE);

        pred0x16 = _mm256_packus_epi32 (pred0x8, pred1x8);

        pred0x8 = _mm256_madd_epi16 (tb2x8, wghtx8);
        pred1x8 = _mm256_madd_epi16 (tb3x8, wghtx8);

        pred0x8 = _mm256_add_epi32 (pred0x8, rndx8);
        pred1x8 = _mm256_add_epi32 (pred1x8, rndx8);

        pred0x8 = _mm256_srli_epi16 (pred0x8, XIN_LOG_SM_WEIGHT_SCALE);
        pred1x8 = _mm256_srli_epi16 (pred1x8, XIN_LOG_SM_WEIGHT_SCALE);

        pred1x16 = _mm256_packus_epi32 (pred0x8, pred1x8);

        predx32 = _mm256_packus_epi16 (pred0x16, pred1x16);
        predx32 = _mm256_permute4x64_epi64 (predx32, 0xD8);

        _mm256_storeu_si256 ((__m256i *)(dst), predx32);

        dst += dstStride;

    }

}

void Xin265pIntraPredSmV64xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    UINT16       scale;
    SINT32       rowIdx;
    const UINT16 *smWeight;
    __m256i      botx16;
    __m256i      wghtx8;
    __m256i      topx16;
    __m256i      tb0x8;
    __m256i      tb1x8;
    __m256i      tb2x8;
    __m256i      tb3x8;
    __m256i      tb4x8;
    __m256i      tb5x8;
    __m256i      tb6x8;
    __m256i      tb7x8;
    __m256i      pred0x8;
    __m256i      pred1x8;
    __m256i      pred0x16;
    __m256i      pred1x16;
    __m256i      predx32;
    __m256i      rndx8;

    (void)width;
    botx16   = _mm256_set1_epi16 (lftBuf[height - 1]);
    topx16   = _mm256_cvtepu8_epi16 (_mm_loadu_si128 ((__m128i *)(topBuf)));
    tb0x8    = _mm256_unpacklo_epi16 (topx16, botx16);
    tb1x8    = _mm256_unpackhi_epi16 (topx16, botx16);
    topx16   = _mm256_cvtepu8_epi16 (_mm_loadu_si128 ((__m128i *)(topBuf + 16)));
    tb2x8    = _mm256_unpacklo_epi16 (topx16, botx16);
    tb3x8    = _mm256_unpackhi_epi16 (topx16, botx16);
    topx16   = _mm256_cvtepu8_epi16 (_mm_loadu_si128 ((__m128i *)(topBuf + 32)));
    tb4x8    = _mm256_unpacklo_epi16 (topx16, botx16);
    tb5x8    = _mm256_unpackhi_epi16 (topx16, botx16);
    topx16   = _mm256_cvtepu8_epi16 (_mm_loadu_si128 ((__m128i *)(topBuf + 48)));
    tb6x8    = _mm256_unpacklo_epi16 (topx16, botx16);
    tb7x8    = _mm256_unpackhi_epi16 (topx16, botx16);
    smWeight = intraSmWeightU16 + height;
    scale    = 1 << XIN_LOG_SM_WEIGHT_SCALE;
    rndx8    = _mm256_set1_epi32 (scale>>1);

    for (rowIdx = 0; rowIdx < height; ++rowIdx)
    {
        wghtx8 = _mm256_set1_epi32 ((smWeight[rowIdx]) | ((scale - smWeight[rowIdx]) << 16));

        pred0x8 = _mm256_madd_epi16 (tb0x8, wghtx8);
        pred1x8 = _mm256_madd_epi16 (tb1x8, wghtx8);

        pred0x8 = _mm256_add_epi32 (pred0x8, rndx8);
        pred1x8 = _mm256_add_epi32 (pred1x8, rndx8);

        pred0x8 = _mm256_srli_epi16 (pred0x8, XIN_LOG_SM_WEIGHT_SCALE);
        pred1x8 = _mm256_srli_epi16 (pred1x8, XIN_LOG_SM_WEIGHT_SCALE);

        pred0x16 = _mm256_packus_epi32 (pred0x8, pred1x8);

        pred0x8 = _mm256_madd_epi16 (tb2x8, wghtx8);
        pred1x8 = _mm256_madd_epi16 (tb3x8, wghtx8);

        pred0x8 = _mm256_add_epi32 (pred0x8, rndx8);
        pred1x8 = _mm256_add_epi32 (pred1x8, rndx8);

        pred0x8 = _mm256_srli_epi16 (pred0x8, XIN_LOG_SM_WEIGHT_SCALE);
        pred1x8 = _mm256_srli_epi16 (pred1x8, XIN_LOG_SM_WEIGHT_SCALE);

        pred1x16 = _mm256_packus_epi32 (pred0x8, pred1x8);

        predx32 = _mm256_packus_epi16 (pred0x16, pred1x16);
        predx32 = _mm256_permute4x64_epi64 (predx32, 0xD8);

        _mm256_storeu_si256 ((__m256i *)(dst), predx32);

        wghtx8 = _mm256_set1_epi32 ((smWeight[rowIdx]) | ((scale - smWeight[rowIdx]) << 16));

        pred0x8 = _mm256_madd_epi16 (tb4x8, wghtx8);
        pred1x8 = _mm256_madd_epi16 (tb5x8, wghtx8);

        pred0x8 = _mm256_add_epi32 (pred0x8, rndx8);
        pred1x8 = _mm256_add_epi32 (pred1x8, rndx8);

        pred0x8 = _mm256_srli_epi16 (pred0x8, XIN_LOG_SM_WEIGHT_SCALE);
        pred1x8 = _mm256_srli_epi16 (pred1x8, XIN_LOG_SM_WEIGHT_SCALE);

        pred0x16 = _mm256_packus_epi32 (pred0x8, pred1x8);

        pred0x8 = _mm256_madd_epi16 (tb6x8, wghtx8);
        pred1x8 = _mm256_madd_epi16 (tb7x8, wghtx8);

        pred0x8 = _mm256_add_epi32 (pred0x8, rndx8);
        pred1x8 = _mm256_add_epi32 (pred1x8, rndx8);

        pred0x8 = _mm256_srli_epi16 (pred0x8, XIN_LOG_SM_WEIGHT_SCALE);
        pred1x8 = _mm256_srli_epi16 (pred1x8, XIN_LOG_SM_WEIGHT_SCALE);

        pred1x16 = _mm256_packus_epi32 (pred0x8, pred1x8);

        predx32 = _mm256_packus_epi16 (pred0x16, pred1x16);
        predx32 = _mm256_permute4x64_epi64 (predx32, 0xD8);

        _mm256_storeu_si256 ((__m256i *)(dst + 32), predx32);

        dst += dstStride;

    }

}

void Xin265pIntraPredSm16xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    PIXEL        rgt;
    __m256i      botx16;
    __m256i      topx16;
    __m256i      tb0x8;
    __m256i      tb1x8;
    __m256i      rndx8;
    __m256i      rlx8;
    __m256i      wghtHx8;
    __m256i      wghtWx16;
    __m256i      wghtW0x8;
    __m256i      wghtW1x8;
    __m256i      scalex16;
    __m256i      pred0x8;
    __m256i      pred1x8;
    __m256i      predx16;
    __m256i      allZero;
    UINT16       scale;
    SINT32       rowIdx;
    const UINT16 *smWeightW;
    const UINT16 *smWeightH;

    rgt       = topBuf[width - 1];
    smWeightW = intraSmWeightU16 + width;
    smWeightH = intraSmWeightU16 + height;
    allZero   = _mm256_setzero_si256 ();
    scale     = 1 << XIN_LOG_SM_WEIGHT_SCALE;
    botx16    = _mm256_set1_epi16 (lftBuf[height - 1]);
    topx16    = _mm256_cvtepu8_epi16 (_mm_loadu_si128 ((__m128i *)(topBuf)));
    tb0x8     = _mm256_unpacklo_epi16 (topx16, botx16);
    tb1x8     = _mm256_unpackhi_epi16 (topx16, botx16);
    scalex16  = _mm256_set1_epi16 ((SINT16)scale);
    rndx8     = _mm256_set1_epi32 (scale);
    wghtWx16  = _mm256_loadu_si256 ((__m256i *)(smWeightW));
    wghtW0x8  = _mm256_unpacklo_epi16 (wghtWx16, _mm256_sub_epi16 (scalex16, wghtWx16));
    wghtW1x8  = _mm256_unpackhi_epi16 (wghtWx16, _mm256_sub_epi16 (scalex16, wghtWx16));

    for (rowIdx = 0; rowIdx < height; ++rowIdx)
    {
        wghtHx8 = _mm256_set1_epi32 ((smWeightH[rowIdx]) | ((scale - smWeightH[rowIdx]) << 16));
        rlx8    = _mm256_set1_epi32 (lftBuf[rowIdx] | ((SINT32)(rgt << 16)));

        pred0x8 = _mm256_add_epi32 (_mm256_madd_epi16 (rlx8, wghtW0x8), rndx8);
        pred1x8 = _mm256_add_epi32 (_mm256_madd_epi16 (rlx8, wghtW1x8), rndx8);
        pred0x8 = _mm256_add_epi32 (pred0x8, _mm256_madd_epi16 (tb0x8, wghtHx8));
        pred1x8 = _mm256_add_epi32 (pred1x8, _mm256_madd_epi16 (tb1x8, wghtHx8));

        pred0x8 = _mm256_srli_epi32 (pred0x8, XIN_LOG_SM_WEIGHT_SCALE + 1);
        pred1x8 = _mm256_srli_epi32 (pred1x8, XIN_LOG_SM_WEIGHT_SCALE + 1);

        predx16 = _mm256_packus_epi32 (pred0x8, pred1x8);
        predx16 = _mm256_packus_epi16 (predx16, allZero);
        predx16 = _mm256_permute4x64_epi64 (predx16, 0xD8);

        _mm_storeu_si128 ((__m128i *)dst, _mm256_castsi256_si128 (predx16));

        dst += dstStride;

    }

}

void Xin265pIntraPredSm32xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    PIXEL        rgt;
    __m256i      botx16;
    __m256i      topx16;
    __m256i      tb0x8;
    __m256i      tb1x8;
    __m256i      tb2x8;
    __m256i      tb3x8;
    __m256i      rndx8;
    __m256i      rlx8;
    __m256i      wghtHx8;
    __m256i      wghtWx16;
    __m256i      wghtW0x8;
    __m256i      wghtW1x8;
    __m256i      wghtW2x8;
    __m256i      wghtW3x8;
    __m256i      scalex16;
    __m256i      pred0x8;
    __m256i      pred1x8;
    __m256i      pred2x8;
    __m256i      pred3x8;
    __m256i      pred0x16;
    __m256i      pred1x16;
    __m256i      predx32;
    UINT16       scale;
    SINT32       rowIdx;
    const UINT16 *smWeightW;
    const UINT16 *smWeightH;

    rgt       = topBuf[width - 1];
    smWeightW = intraSmWeightU16 + width;
    smWeightH = intraSmWeightU16 + height;
    scale     = 1 << XIN_LOG_SM_WEIGHT_SCALE;
    botx16    = _mm256_set1_epi16 (lftBuf[height - 1]);
    topx16    = _mm256_cvtepu8_epi16 (_mm_loadu_si128 ((__m128i *)(topBuf)));
    tb0x8     = _mm256_unpacklo_epi16 (topx16, botx16);
    tb1x8     = _mm256_unpackhi_epi16 (topx16, botx16);
    topx16    = _mm256_cvtepu8_epi16 (_mm_loadu_si128 ((__m128i *)(topBuf + 16)));
    tb2x8     = _mm256_unpacklo_epi16 (topx16, botx16);
    tb3x8     = _mm256_unpackhi_epi16 (topx16, botx16);
    scalex16  = _mm256_set1_epi16 ((SINT16)scale);
    rndx8     = _mm256_set1_epi32 (scale);
    wghtWx16  = _mm256_loadu_si256 ((__m256i *)(smWeightW));
    wghtW0x8  = _mm256_unpacklo_epi16 (wghtWx16, _mm256_sub_epi16 (scalex16, wghtWx16));
    wghtW1x8  = _mm256_unpackhi_epi16 (wghtWx16, _mm256_sub_epi16 (scalex16, wghtWx16));
    wghtWx16  = _mm256_loadu_si256 ((__m256i *)(smWeightW + 16));
    wghtW2x8  = _mm256_unpacklo_epi16 (wghtWx16, _mm256_sub_epi16 (scalex16, wghtWx16));
    wghtW3x8  = _mm256_unpackhi_epi16 (wghtWx16, _mm256_sub_epi16 (scalex16, wghtWx16));

    for (rowIdx = 0; rowIdx < height; ++rowIdx)
    {
        wghtHx8 = _mm256_set1_epi32 ((smWeightH[rowIdx]) | ((scale - smWeightH[rowIdx]) << 16));
        rlx8    = _mm256_set1_epi32 (lftBuf[rowIdx] | ((SINT32)(rgt << 16)));

        pred0x8 = _mm256_add_epi32 (_mm256_madd_epi16 (rlx8, wghtW0x8), rndx8);
        pred1x8 = _mm256_add_epi32 (_mm256_madd_epi16 (rlx8, wghtW1x8), rndx8);
        pred2x8 = _mm256_add_epi32 (_mm256_madd_epi16 (rlx8, wghtW2x8), rndx8);
        pred3x8 = _mm256_add_epi32 (_mm256_madd_epi16 (rlx8, wghtW3x8), rndx8);

        pred0x8 = _mm256_add_epi32 (pred0x8, _mm256_madd_epi16 (tb0x8, wghtHx8));
        pred1x8 = _mm256_add_epi32 (pred1x8, _mm256_madd_epi16 (tb1x8, wghtHx8));
        pred2x8 = _mm256_add_epi32 (pred2x8, _mm256_madd_epi16 (tb2x8, wghtHx8));
        pred3x8 = _mm256_add_epi32 (pred3x8, _mm256_madd_epi16 (tb3x8, wghtHx8));

        pred0x8 = _mm256_srli_epi32 (pred0x8, XIN_LOG_SM_WEIGHT_SCALE + 1);
        pred1x8 = _mm256_srli_epi32 (pred1x8, XIN_LOG_SM_WEIGHT_SCALE + 1);
        pred2x8 = _mm256_srli_epi32 (pred2x8, XIN_LOG_SM_WEIGHT_SCALE + 1);
        pred3x8 = _mm256_srli_epi32 (pred3x8, XIN_LOG_SM_WEIGHT_SCALE + 1);

        pred0x16 = _mm256_packus_epi32 (pred0x8, pred1x8);
        pred1x16 = _mm256_packus_epi32 (pred2x8, pred3x8);

        predx32 = _mm256_packus_epi16 (pred0x16, pred1x16);
        predx32 = _mm256_permute4x64_epi64 (predx32, 0xD8);

        _mm256_storeu_si256 ((__m256i *)(dst), predx32);

        dst += dstStride;

    }

}

void Xin265pIntraPredSm64xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    PIXEL        rgt;
    __m256i      botx16;
    __m256i      topx16;
    __m256i      tb0x8;
    __m256i      tb1x8;
    __m256i      tb2x8;
    __m256i      tb3x8;
    __m256i      tb4x8;
    __m256i      tb5x8;
    __m256i      tb6x8;
    __m256i      tb7x8;
    __m256i      rndx8;
    __m256i      rlx8;
    __m256i      wghtHx8;
    __m256i      wghtWx16;
    __m256i      wghtW0x8;
    __m256i      wghtW1x8;
    __m256i      wghtW2x8;
    __m256i      wghtW3x8;
    __m256i      wghtW4x8;
    __m256i      wghtW5x8;
    __m256i      wghtW6x8;
    __m256i      wghtW7x8;
    __m256i      scalex16;
    __m256i      pred0x8;
    __m256i      pred1x8;
    __m256i      pred2x8;
    __m256i      pred3x8;
    __m256i      pred4x8;
    __m256i      pred5x8;
    __m256i      pred6x8;
    __m256i      pred7x8;
    __m256i      pred0x16;
    __m256i      pred1x16;
    __m256i      pred2x16;
    __m256i      pred3x16;
    __m256i      pred0x32;
    __m256i      pred1x32;
    UINT16       scale;
    SINT32       rowIdx;
    const UINT16 *smWeightW;
    const UINT16 *smWeightH;

    rgt       = topBuf[width - 1];
    smWeightW = intraSmWeightU16 + width;
    smWeightH = intraSmWeightU16 + height;
    scale     = 1 << XIN_LOG_SM_WEIGHT_SCALE;
    botx16    = _mm256_set1_epi16 (lftBuf[height - 1]);
    topx16    = _mm256_cvtepu8_epi16 (_mm_loadu_si128 ((__m128i *)(topBuf)));
    tb0x8     = _mm256_unpacklo_epi16 (topx16, botx16);
    tb1x8     = _mm256_unpackhi_epi16 (topx16, botx16);
    topx16    = _mm256_cvtepu8_epi16 (_mm_loadu_si128 ((__m128i *)(topBuf + 16)));
    tb2x8     = _mm256_unpacklo_epi16 (topx16, botx16);
    tb3x8     = _mm256_unpackhi_epi16 (topx16, botx16);
    topx16    = _mm256_cvtepu8_epi16 (_mm_loadu_si128 ((__m128i *)(topBuf + 32)));
    tb4x8     = _mm256_unpacklo_epi16 (topx16, botx16);
    tb5x8     = _mm256_unpackhi_epi16 (topx16, botx16);
    topx16    = _mm256_cvtepu8_epi16 (_mm_loadu_si128 ((__m128i *)(topBuf + 48)));
    tb6x8     = _mm256_unpacklo_epi16 (topx16, botx16);
    tb7x8     = _mm256_unpackhi_epi16 (topx16, botx16);
    scalex16  = _mm256_set1_epi16 ((SINT16)scale);
    rndx8     = _mm256_set1_epi32 (scale);
    wghtWx16  = _mm256_loadu_si256 ((__m256i *)(smWeightW));
    wghtW0x8  = _mm256_unpacklo_epi16 (wghtWx16, _mm256_sub_epi16 (scalex16, wghtWx16));
    wghtW1x8  = _mm256_unpackhi_epi16 (wghtWx16, _mm256_sub_epi16 (scalex16, wghtWx16));
    wghtWx16  = _mm256_loadu_si256 ((__m256i *)(smWeightW + 16));
    wghtW2x8  = _mm256_unpacklo_epi16 (wghtWx16, _mm256_sub_epi16 (scalex16, wghtWx16));
    wghtW3x8  = _mm256_unpackhi_epi16 (wghtWx16, _mm256_sub_epi16 (scalex16, wghtWx16));
    wghtWx16  = _mm256_loadu_si256 ((__m256i *)(smWeightW + 32));
    wghtW4x8  = _mm256_unpacklo_epi16 (wghtWx16, _mm256_sub_epi16 (scalex16, wghtWx16));
    wghtW5x8  = _mm256_unpackhi_epi16 (wghtWx16, _mm256_sub_epi16 (scalex16, wghtWx16));
    wghtWx16  = _mm256_loadu_si256 ((__m256i *)(smWeightW + 48));
    wghtW6x8  = _mm256_unpacklo_epi16 (wghtWx16, _mm256_sub_epi16 (scalex16, wghtWx16));
    wghtW7x8  = _mm256_unpackhi_epi16 (wghtWx16, _mm256_sub_epi16 (scalex16, wghtWx16));

    for (rowIdx = 0; rowIdx < height; ++rowIdx)
    {
        wghtHx8 = _mm256_set1_epi32 ((smWeightH[rowIdx]) | ((scale - smWeightH[rowIdx]) << 16));
        rlx8    = _mm256_set1_epi32 (lftBuf[rowIdx] | ((SINT32)(rgt << 16)));

        pred0x8 = _mm256_add_epi32 (_mm256_madd_epi16 (rlx8, wghtW0x8), rndx8);
        pred1x8 = _mm256_add_epi32 (_mm256_madd_epi16 (rlx8, wghtW1x8), rndx8);
        pred2x8 = _mm256_add_epi32 (_mm256_madd_epi16 (rlx8, wghtW2x8), rndx8);
        pred3x8 = _mm256_add_epi32 (_mm256_madd_epi16 (rlx8, wghtW3x8), rndx8);
        pred4x8 = _mm256_add_epi32 (_mm256_madd_epi16 (rlx8, wghtW4x8), rndx8);
        pred5x8 = _mm256_add_epi32 (_mm256_madd_epi16 (rlx8, wghtW5x8), rndx8);
        pred6x8 = _mm256_add_epi32 (_mm256_madd_epi16 (rlx8, wghtW6x8), rndx8);
        pred7x8 = _mm256_add_epi32 (_mm256_madd_epi16 (rlx8, wghtW7x8), rndx8);

        pred0x8 = _mm256_add_epi32 (pred0x8, _mm256_madd_epi16 (tb0x8, wghtHx8));
        pred1x8 = _mm256_add_epi32 (pred1x8, _mm256_madd_epi16 (tb1x8, wghtHx8));
        pred2x8 = _mm256_add_epi32 (pred2x8, _mm256_madd_epi16 (tb2x8, wghtHx8));
        pred3x8 = _mm256_add_epi32 (pred3x8, _mm256_madd_epi16 (tb3x8, wghtHx8));
        pred4x8 = _mm256_add_epi32 (pred4x8, _mm256_madd_epi16 (tb4x8, wghtHx8));
        pred5x8 = _mm256_add_epi32 (pred5x8, _mm256_madd_epi16 (tb5x8, wghtHx8));
        pred6x8 = _mm256_add_epi32 (pred6x8, _mm256_madd_epi16 (tb6x8, wghtHx8));
        pred7x8 = _mm256_add_epi32 (pred7x8, _mm256_madd_epi16 (tb7x8, wghtHx8));

        pred0x8 = _mm256_srli_epi32 (pred0x8, XIN_LOG_SM_WEIGHT_SCALE + 1);
        pred1x8 = _mm256_srli_epi32 (pred1x8, XIN_LOG_SM_WEIGHT_SCALE + 1);
        pred2x8 = _mm256_srli_epi32 (pred2x8, XIN_LOG_SM_WEIGHT_SCALE + 1);
        pred3x8 = _mm256_srli_epi32 (pred3x8, XIN_LOG_SM_WEIGHT_SCALE + 1);
        pred4x8 = _mm256_srli_epi32 (pred4x8, XIN_LOG_SM_WEIGHT_SCALE + 1);
        pred5x8 = _mm256_srli_epi32 (pred5x8, XIN_LOG_SM_WEIGHT_SCALE + 1);
        pred6x8 = _mm256_srli_epi32 (pred6x8, XIN_LOG_SM_WEIGHT_SCALE + 1);
        pred7x8 = _mm256_srli_epi32 (pred7x8, XIN_LOG_SM_WEIGHT_SCALE + 1);

        pred0x16 = _mm256_packus_epi32 (pred0x8, pred1x8);
        pred1x16 = _mm256_packus_epi32 (pred2x8, pred3x8);
        pred2x16 = _mm256_packus_epi32 (pred4x8, pred5x8);
        pred3x16 = _mm256_packus_epi32 (pred6x8, pred7x8);

        pred0x32 = _mm256_packus_epi16 (pred0x16, pred1x16);
        pred0x32 = _mm256_permute4x64_epi64 (pred0x32, 0xD8);
        pred1x32 = _mm256_packus_epi16 (pred2x16, pred3x16);
        pred1x32 = _mm256_permute4x64_epi64 (pred1x32, 0xD8);

        _mm256_storeu_si256 ((__m256i *)(dst),      pred0x32);
        _mm256_storeu_si256 ((__m256i *)(dst + 32), pred1x32);

        dst += dstStride;

    }

}

void Xin265pIntraPredSmH16xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    PIXEL        rgt;
    UINT16       scale;
    SINT32       rowIdx;
    const UINT16 *smWeight;
    __m256i      wghtx16;
    __m256i      wght0x8, wght1x8;
    __m256i      pred0x8, pred1x8;
    __m256i      scalex16;
    __m256i      rlx8;
    __m256i      rndx8;
    __m256i      predx16;

    rgt      = topBuf[width - 1];
    smWeight = intraSmWeightU16 + width;
    scale    = 1 << XIN_LOG_SM_WEIGHT_SCALE;
    scalex16 = _mm256_set1_epi16 ((SINT16)scale);
    rndx8    = _mm256_set1_epi32 (scale>>1);
    wghtx16  = _mm256_loadu_si256 ((__m256i *)(smWeight));
    wght0x8  = _mm256_unpacklo_epi16 (wghtx16, _mm256_sub_epi16 (scalex16, wghtx16));
    wght1x8  = _mm256_unpackhi_epi16 (wghtx16, _mm256_sub_epi16 (scalex16, wghtx16));

    for (rowIdx = 0; rowIdx < height; ++rowIdx)
    {
        rlx8  = _mm256_set1_epi32 (lftBuf[rowIdx] | ((SINT32)(rgt << 16)));

        pred0x8 = _mm256_add_epi32 (_mm256_madd_epi16 (rlx8, wght0x8), rndx8);
        pred1x8 = _mm256_add_epi32 (_mm256_madd_epi16 (rlx8, wght1x8), rndx8);

        pred0x8 = _mm256_srli_epi32 (pred0x8, XIN_LOG_SM_WEIGHT_SCALE);
        pred1x8 = _mm256_srli_epi32 (pred1x8, XIN_LOG_SM_WEIGHT_SCALE);

        predx16 = _mm256_packus_epi32 (pred0x8, pred1x8);
        predx16 = _mm256_packus_epi16 (predx16, predx16);
        predx16 = _mm256_permute4x64_epi64 (predx16, 0xD8);

        _mm_storeu_si128 ((__m128i *)dst, _mm256_castsi256_si128 (predx16));

        dst += dstStride;
    }

}

void Xin265pIntraPredSmH32xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    PIXEL        rgt;
    UINT16       scale;
    SINT32       rowIdx;
    const UINT16 *smWeight;
    __m256i      wghtx16;
    __m256i      wght0x8, wght1x8;
    __m256i      wght2x8, wght3x8;
    __m256i      pred0x8, pred1x8;
    __m256i      pred2x8, pred3x8;
    __m256i      scalex16;
    __m256i      rlx8;
    __m256i      rndx8;
    __m256i      pred0x16, pred1x16;
    __m256i      predx32;

    rgt      = topBuf[width - 1];
    smWeight = intraSmWeightU16 + width;
    scale    = 1 << XIN_LOG_SM_WEIGHT_SCALE;
    scalex16 = _mm256_set1_epi16 ((SINT16)scale);
    rndx8    = _mm256_set1_epi32 (scale>>1);
    wghtx16  = _mm256_loadu_si256 ((__m256i *)(smWeight));
    wght0x8  = _mm256_unpacklo_epi16 (wghtx16, _mm256_sub_epi16 (scalex16, wghtx16));
    wght1x8  = _mm256_unpackhi_epi16 (wghtx16, _mm256_sub_epi16 (scalex16, wghtx16));
    wghtx16  = _mm256_loadu_si256 ((__m256i *)(smWeight + 16));
    wght2x8  = _mm256_unpacklo_epi16 (wghtx16, _mm256_sub_epi16 (scalex16, wghtx16));
    wght3x8  = _mm256_unpackhi_epi16 (wghtx16, _mm256_sub_epi16 (scalex16, wghtx16));

    for (rowIdx = 0; rowIdx < height; ++rowIdx)
    {
        rlx8  = _mm256_set1_epi32 (lftBuf[rowIdx] | ((SINT32)(rgt << 16)));

        pred0x8 = _mm256_add_epi32 (_mm256_madd_epi16 (rlx8, wght0x8), rndx8);
        pred1x8 = _mm256_add_epi32 (_mm256_madd_epi16 (rlx8, wght1x8), rndx8);
        pred2x8 = _mm256_add_epi32 (_mm256_madd_epi16 (rlx8, wght2x8), rndx8);
        pred3x8 = _mm256_add_epi32 (_mm256_madd_epi16 (rlx8, wght3x8), rndx8);

        pred0x8 = _mm256_srli_epi32 (pred0x8, XIN_LOG_SM_WEIGHT_SCALE);
        pred1x8 = _mm256_srli_epi32 (pred1x8, XIN_LOG_SM_WEIGHT_SCALE);
        pred2x8 = _mm256_srli_epi32 (pred2x8, XIN_LOG_SM_WEIGHT_SCALE);
        pred3x8 = _mm256_srli_epi32 (pred3x8, XIN_LOG_SM_WEIGHT_SCALE);

        pred0x16 = _mm256_packus_epi32 (pred0x8, pred1x8);
        pred1x16 = _mm256_packus_epi32 (pred2x8, pred3x8);

        predx32 = _mm256_packus_epi16 (pred0x16, pred1x16);
        predx32 = _mm256_permute4x64_epi64 (predx32, 0xD8);

        _mm256_storeu_si256 ((__m256i *)(dst), predx32);

        dst += dstStride;

    }

}

void Xin265pIntraPredSmH64xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf)
{
    PIXEL        rgt;
    UINT16       scale;
    SINT32       rowIdx;
    const UINT16 *smWeight;
    __m256i      wghtx16;
    __m256i      wght0x8, wght1x8;
    __m256i      wght2x8, wght3x8;
    __m256i      wght4x8, wght5x8;
    __m256i      wght6x8, wght7x8;
    __m256i      pred0x8, pred1x8;
    __m256i      pred2x8, pred3x8;
    __m256i      pred4x8, pred5x8;
    __m256i      pred6x8, pred7x8;
    __m256i      scalex16;
    __m256i      rlx8;
    __m256i      rndx8;
    __m256i      pred0x16, pred1x16;
    __m256i      pred2x16, pred3x16;
    __m256i      pred0x32, pred1x32;

    rgt      = topBuf[width - 1];
    smWeight = intraSmWeightU16 + width;
    scale    = 1 << XIN_LOG_SM_WEIGHT_SCALE;
    scalex16 = _mm256_set1_epi16 ((SINT16)scale);
    rndx8    = _mm256_set1_epi32 (scale>>1);
    wghtx16  = _mm256_loadu_si256 ((__m256i *)(smWeight));
    wght0x8  = _mm256_unpacklo_epi16 (wghtx16, _mm256_sub_epi16 (scalex16, wghtx16));
    wght1x8  = _mm256_unpackhi_epi16 (wghtx16, _mm256_sub_epi16 (scalex16, wghtx16));
    wghtx16  = _mm256_loadu_si256 ((__m256i *)(smWeight + 16));
    wght2x8  = _mm256_unpacklo_epi16 (wghtx16, _mm256_sub_epi16 (scalex16, wghtx16));
    wght3x8  = _mm256_unpackhi_epi16 (wghtx16, _mm256_sub_epi16 (scalex16, wghtx16));
    wghtx16  = _mm256_loadu_si256 ((__m256i *)(smWeight + 32));
    wght4x8  = _mm256_unpacklo_epi16 (wghtx16, _mm256_sub_epi16 (scalex16, wghtx16));
    wght5x8  = _mm256_unpackhi_epi16 (wghtx16, _mm256_sub_epi16 (scalex16, wghtx16));
    wghtx16  = _mm256_loadu_si256 ((__m256i *)(smWeight + 48));
    wght6x8  = _mm256_unpacklo_epi16 (wghtx16, _mm256_sub_epi16 (scalex16, wghtx16));
    wght7x8  = _mm256_unpackhi_epi16 (wghtx16, _mm256_sub_epi16 (scalex16, wghtx16));

    for (rowIdx = 0; rowIdx < height; ++rowIdx)
    {
        rlx8  = _mm256_set1_epi32 (lftBuf[rowIdx] | ((SINT32)(rgt << 16)));

        pred0x8 = _mm256_add_epi32 (_mm256_madd_epi16 (rlx8, wght0x8), rndx8);
        pred1x8 = _mm256_add_epi32 (_mm256_madd_epi16 (rlx8, wght1x8), rndx8);
        pred2x8 = _mm256_add_epi32 (_mm256_madd_epi16 (rlx8, wght2x8), rndx8);
        pred3x8 = _mm256_add_epi32 (_mm256_madd_epi16 (rlx8, wght3x8), rndx8);
        pred4x8 = _mm256_add_epi32 (_mm256_madd_epi16 (rlx8, wght4x8), rndx8);
        pred5x8 = _mm256_add_epi32 (_mm256_madd_epi16 (rlx8, wght5x8), rndx8);
        pred6x8 = _mm256_add_epi32 (_mm256_madd_epi16 (rlx8, wght6x8), rndx8);
        pred7x8 = _mm256_add_epi32 (_mm256_madd_epi16 (rlx8, wght7x8), rndx8);

        pred0x8 = _mm256_srli_epi32 (pred0x8, XIN_LOG_SM_WEIGHT_SCALE);
        pred1x8 = _mm256_srli_epi32 (pred1x8, XIN_LOG_SM_WEIGHT_SCALE);
        pred2x8 = _mm256_srli_epi32 (pred2x8, XIN_LOG_SM_WEIGHT_SCALE);
        pred3x8 = _mm256_srli_epi32 (pred3x8, XIN_LOG_SM_WEIGHT_SCALE);
        pred4x8 = _mm256_srli_epi32 (pred4x8, XIN_LOG_SM_WEIGHT_SCALE);
        pred5x8 = _mm256_srli_epi32 (pred5x8, XIN_LOG_SM_WEIGHT_SCALE);
        pred6x8 = _mm256_srli_epi32 (pred6x8, XIN_LOG_SM_WEIGHT_SCALE);
        pred7x8 = _mm256_srli_epi32 (pred7x8, XIN_LOG_SM_WEIGHT_SCALE);

        pred0x16 = _mm256_packus_epi32 (pred0x8, pred1x8);
        pred1x16 = _mm256_packus_epi32 (pred2x8, pred3x8);
        pred2x16 = _mm256_packus_epi32 (pred4x8, pred5x8);
        pred3x16 = _mm256_packus_epi32 (pred6x8, pred7x8);

        pred0x32 = _mm256_packus_epi16 (pred0x16, pred1x16);
        pred0x32 = _mm256_permute4x64_epi64 (pred0x32, 0xD8);
        pred1x32 = _mm256_packus_epi16 (pred2x16, pred3x16);
        pred1x32 = _mm256_permute4x64_epi64 (pred1x32, 0xD8);

        _mm256_storeu_si256 ((__m256i *)(dst),      pred0x32);
        _mm256_storeu_si256 ((__m256i *)(dst + 32), pred1x32);

        dst += dstStride;

    }

}



