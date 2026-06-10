/***************************************************************************//**
 *
 * @file          h265p_trans_utility.h
 * @brief         This file declares av1 transform-related common subroutines.
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
#ifndef _h265p_trans_utility_h_
#define _h265p_trans_utility_h_

#define btf_16_sse2(w0, w1, in0, in1, out0, out1) \
  {                                               \
    __m128i t0 = _mm_unpacklo_epi16(in0, in1);    \
    __m128i t1 = _mm_unpackhi_epi16(in0, in1);    \
    __m128i u0 = _mm_madd_epi16(t0, w0);          \
    __m128i u1 = _mm_madd_epi16(t1, w0);          \
    __m128i v0 = _mm_madd_epi16(t0, w1);          \
    __m128i v1 = _mm_madd_epi16(t1, w1);          \
                                                  \
    __m128i a0 = _mm_add_epi32(u0, __rounding);   \
    __m128i a1 = _mm_add_epi32(u1, __rounding);   \
    __m128i b0 = _mm_add_epi32(v0, __rounding);   \
    __m128i b1 = _mm_add_epi32(v1, __rounding);   \
                                                  \
    __m128i c0 = _mm_srai_epi32(a0, cos_bit);     \
    __m128i c1 = _mm_srai_epi32(a1, cos_bit);     \
    __m128i d0 = _mm_srai_epi32(b0, cos_bit);     \
    __m128i d1 = _mm_srai_epi32(b1, cos_bit);     \
                                                  \
    out0 = _mm_packs_epi32(c0, c1);               \
    out1 = _mm_packs_epi32(d0, d1);               \
  }


void transpose_16bit_4x4 (
    SINT16 *in,
    SINT16 *out);

void transpose_16bit_8x8 (
    SINT16 *in,
    SINT16 *out);

void transpose_16bit_4x8 (
    SINT16 *in,
    SINT16 *out);

void transpose_16bit_8x4 (
    SINT16 *in,
    SINT16 *out);

void transpose_16bit_16x16_avx2 (
    SINT16 *in,
    SINT16 *out);

void transpose_32_8x8_avx2 (
    SINT32   *inputA,
    SINT32   *output,
    intptr_t stride);

void transpose_16bit_16x8_avx2 (
    SINT16 *input,
    SINT16 *output);

#endif

