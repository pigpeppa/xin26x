/***************************************************************************//**
 *
 * @file          h265p_trans_utility_avx2.c
 * @brief         av1 transform-related common subroutines (AVX2).
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
#ifdef __linux__
#include "xin_avx2_linux_patch.h"
#endif

static inline void transpose2_8x8_avx2 (
    __m256i *in,
    __m256i *out)
{
    __m256i t[16], u[16];
    // (1st, 2nd) ==> (lo, hi)
    //   (0, 1)   ==>  (0, 1)
    //   (2, 3)   ==>  (2, 3)
    //   (4, 5)   ==>  (4, 5)
    //   (6, 7)   ==>  (6, 7)
    for (int i = 0; i < 4; i++)
    {
        t[2 * i] = _mm256_unpacklo_epi16(in[2 * i], in[2 * i + 1]);
        t[2 * i + 1] = _mm256_unpackhi_epi16(in[2 * i], in[2 * i + 1]);
    }

    // (1st, 2nd) ==> (lo, hi)
    //   (0, 2)   ==>  (0, 2)
    //   (1, 3)   ==>  (1, 3)
    //   (4, 6)   ==>  (4, 6)
    //   (5, 7)   ==>  (5, 7)
    for (int i = 0; i < 2; i++)
    {
        u[i] = _mm256_unpacklo_epi32(t[i], t[i + 2]);
        u[i + 2] = _mm256_unpackhi_epi32(t[i], t[i + 2]);

        u[i + 4] = _mm256_unpacklo_epi32(t[i + 4], t[i + 6]);
        u[i + 6] = _mm256_unpackhi_epi32(t[i + 4], t[i + 6]);
    }

    // (1st, 2nd) ==> (lo, hi)
    //   (0, 4)   ==>  (0, 1)
    //   (1, 5)   ==>  (4, 5)
    //   (2, 6)   ==>  (2, 3)
    //   (3, 7)   ==>  (6, 7)
    for (int i = 0; i < 2; i++)
    {
        out[2 * i] = _mm256_unpacklo_epi64(u[2 * i], u[2 * i + 4]);
        out[2 * i + 1] = _mm256_unpackhi_epi64(u[2 * i], u[2 * i + 4]);

        out[2 * i + 4] = _mm256_unpacklo_epi64(u[2 * i + 1], u[2 * i + 5]);
        out[2 * i + 5] = _mm256_unpackhi_epi64(u[2 * i + 1], u[2 * i + 5]);
    }

}

void transpose_16bit_16x16_avx2 (
    SINT16 *input,
    SINT16 *output)
{
    __m256i t[16];
    __m256i *in;
    __m256i *out;

    in  = (__m256i *)input;
    out = (__m256i *)output;

#define LOADL(idx)                                                            \
  t[idx] = _mm256_castsi128_si256(_mm_load_si128((__m128i const *)&in[idx])); \
  t[idx] = _mm256_inserti128_si256(                                           \
      t[idx], _mm_load_si128((__m128i const *)&in[idx + 8]), 1);

#define LOADR(idx)                                                           \
  t[8 + idx] =                                                               \
      _mm256_castsi128_si256(_mm_load_si128((__m128i const *)&in[idx] + 1)); \
  t[8 + idx] = _mm256_inserti128_si256(                                      \
      t[8 + idx], _mm_load_si128((__m128i const *)&in[idx + 8] + 1), 1);

    // load left 8x16
    LOADL(0);
    LOADL(1);
    LOADL(2);
    LOADL(3);
    LOADL(4);
    LOADL(5);
    LOADL(6);
    LOADL(7);

    // load right 8x16
    LOADR(0);
    LOADR(1);
    LOADR(2);
    LOADR(3);
    LOADR(4);
    LOADR(5);
    LOADR(6);
    LOADR(7);

    // get the top 16x8 result
    transpose2_8x8_avx2(t, out);
    // get the bottom 16x8 result
    transpose2_8x8_avx2(&t[8], &out[8]);

}

void transpose_32_8x8_avx2 (
    SINT32   *input,
    SINT32   *output,
    intptr_t stride32)
{
    __m256i *in  = (__m256i *)input;
    __m256i *out = (__m256i *)output;
    __m256i temp0 = _mm256_unpacklo_epi32(in[0], in[2]);
    __m256i temp1 = _mm256_unpackhi_epi32(in[0], in[2]);
    __m256i temp2 = _mm256_unpacklo_epi32(in[1], in[3]);
    __m256i temp3 = _mm256_unpackhi_epi32(in[1], in[3]);
    __m256i temp4 = _mm256_unpacklo_epi32(in[4], in[6]);
    __m256i temp5 = _mm256_unpackhi_epi32(in[4], in[6]);
    __m256i temp6 = _mm256_unpacklo_epi32(in[5], in[7]);
    __m256i temp7 = _mm256_unpackhi_epi32(in[5], in[7]);

    __m256i t0 = _mm256_unpacklo_epi32(temp0, temp2);
    __m256i t1 = _mm256_unpackhi_epi32(temp0, temp2);
    __m256i t2 = _mm256_unpacklo_epi32(temp1, temp3);
    __m256i t3 = _mm256_unpackhi_epi32(temp1, temp3);
    __m256i t4 = _mm256_unpacklo_epi32(temp4, temp6);
    __m256i t5 = _mm256_unpackhi_epi32(temp4, temp6);
    __m256i t6 = _mm256_unpacklo_epi32(temp5, temp7);
    __m256i t7 = _mm256_unpackhi_epi32(temp5, temp7);

    intptr_t stride = stride32 >> 3;

    out[0 * stride] = _mm256_permute2x128_si256(t0, t4, 0x20);
    out[1 * stride] = _mm256_permute2x128_si256(t1, t5, 0x20);
    out[2 * stride] = _mm256_permute2x128_si256(t2, t6, 0x20);
    out[3 * stride] = _mm256_permute2x128_si256(t3, t7, 0x20);
    out[4 * stride] = _mm256_permute2x128_si256(t0, t4, 0x31);
    out[5 * stride] = _mm256_permute2x128_si256(t1, t5, 0x31);
    out[6 * stride] = _mm256_permute2x128_si256(t2, t6, 0x31);
    out[7 * stride] = _mm256_permute2x128_si256(t3, t7, 0x31);

}

void transpose_16bit_16x8_avx2 (
    SINT16 *input,
    SINT16 *output)
{
    __m256i *in  = (__m256i *)input;
    __m256i *out = (__m256i *)output;

    const __m256i a0 = _mm256_unpacklo_epi16(in[0], in[1]);
    const __m256i a1 = _mm256_unpacklo_epi16(in[2], in[3]);
    const __m256i a2 = _mm256_unpacklo_epi16(in[4], in[5]);
    const __m256i a3 = _mm256_unpacklo_epi16(in[6], in[7]);
    const __m256i a4 = _mm256_unpackhi_epi16(in[0], in[1]);
    const __m256i a5 = _mm256_unpackhi_epi16(in[2], in[3]);
    const __m256i a6 = _mm256_unpackhi_epi16(in[4], in[5]);
    const __m256i a7 = _mm256_unpackhi_epi16(in[6], in[7]);

    const __m256i b0 = _mm256_unpacklo_epi32(a0, a1);
    const __m256i b1 = _mm256_unpacklo_epi32(a2, a3);
    const __m256i b2 = _mm256_unpacklo_epi32(a4, a5);
    const __m256i b3 = _mm256_unpacklo_epi32(a6, a7);
    const __m256i b4 = _mm256_unpackhi_epi32(a0, a1);
    const __m256i b5 = _mm256_unpackhi_epi32(a2, a3);
    const __m256i b6 = _mm256_unpackhi_epi32(a4, a5);
    const __m256i b7 = _mm256_unpackhi_epi32(a6, a7);

    out[0] = _mm256_unpacklo_epi64(b0, b1);
    out[1] = _mm256_unpackhi_epi64(b0, b1);
    out[2] = _mm256_unpacklo_epi64(b4, b5);
    out[3] = _mm256_unpackhi_epi64(b4, b5);
    out[4] = _mm256_unpacklo_epi64(b2, b3);
    out[5] = _mm256_unpackhi_epi64(b2, b3);
    out[6] = _mm256_unpacklo_epi64(b6, b7);
    out[7] = _mm256_unpackhi_epi64(b6, b7);

}

