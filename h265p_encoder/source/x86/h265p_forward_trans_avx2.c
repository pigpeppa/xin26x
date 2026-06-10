/***************************************************************************//**
 *
 * @file          h265p_forward_trans_avx2.c
 * @brief         av1 forward transform subroutines (AVX2).
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
#include "assert.h"
#include "xin_typedef.h"
#include "basic_macro.h"
#include "string.h"
#include "stdint.h"
#include "h265p_forward_1d_trans.h"
#include "h265p_trans_context.h"
#include "h265p_definition.h"
#include "h265p_constant.h"
#include "h265p_common_data.h"
#include "h265p_forward_trans.h"
#include "h265p_trans_utility.h"

#define pair_set_epi16(a, b) _mm_set1_epi32((SINT32)(((UINT16)(a)) | (((SINT32)(b)) << 16)))

static const Xin265pFdctOpt xinFdct16x32Hor[XIN_TX_2D_NUM] =
{
    Xin265pFdct16x32_AVX2,      // DCT_DCT
    NULL,                       // ADST_DCT
    NULL,                       // DCT_ADST
    NULL,                       // ADST_ADST
    NULL,                       // FLIPADST_DCT
    NULL,                       // DCT_FLIPADST
    NULL,                       // FLIPADST_FLIPADST
    NULL,                       // ADST_FLIPADST
    NULL,                       // FLIPADST_ADST
    Xin265pFidentity16x32_AVX2, // IDTX
    Xin265pFdct16x32_AVX2,      // V_DCT
    Xin265pFidentity16x32_AVX2, // H_DCT
    NULL,                       // V_ADST
    NULL,                       // H_ADST
    NULL,                       // V_FLIPADST
    NULL                        // H_FLIPADST
};

static const Xin265pFdctOpt xinFdct16x32Ver[XIN_TX_2D_NUM] =
{
    Xin265pFdct16x32_AVX2,      // DCT_DCT
    NULL,                       // ADST_DCT
    NULL,                       // DCT_ADST
    NULL,                       // ADST_ADST
    NULL,                       // FLIPADST_DCT
    NULL,                       // DCT_FLIPADST
    NULL,                       // FLIPADST_FLIPADST
    NULL,                       // ADST_FLIPADST
    NULL,                       // FLIPADST_ADST
    Xin265pFidentity16x32_AVX2, // IDTX
    Xin265pFidentity16x32_AVX2, // V_DCT
    Xin265pFdct16x32_AVX2,      // H_DCT
    NULL,                       // V_ADST
    NULL,                       // H_ADST
    NULL,                       // V_FLIPADST
    NULL                        // H_FLIPADST
};

static const Xin265pFdctOpt xinFdct16x16Hor[XIN_TX_2D_NUM] =
{
    Xin265pFdct16x16_AVX2,       // DCT_DCT
    Xin265pFadst16x16_AVX2,      // ADST_DCT
    Xin265pFdct16x16_AVX2,       // DCT_ADST
    Xin265pFadst16x16_AVX2,      // ADST_ADST
    Xin265pFadst16x16_AVX2,      // FLIPADST_DCT
    Xin265pFdct16x16_AVX2,       // DCT_FLIPADST
    Xin265pFadst16x16_AVX2,      // FLIPADST_FLIPADST
    Xin265pFadst16x16_AVX2,      // ADST_FLIPADST
    Xin265pFadst16x16_AVX2,      // FLIPADST_ADST
    Xin265pFidentity16x16_AVX2,  // IDTX
    Xin265pFdct16x16_AVX2,       // V_DCT
    Xin265pFidentity16x16_AVX2,  // H_DCT
    Xin265pFadst16x16_AVX2,      // V_ADST
    Xin265pFidentity16x16_AVX2,  // H_ADST
    Xin265pFadst16x16_AVX2,      // V_FLIPADST
    Xin265pFidentity16x16_AVX2   // H_FLIPADST
};

static const Xin265pFdctOpt xinFdct16x16Ver[XIN_TX_2D_NUM] =
{
    Xin265pFdct16x16_AVX2,       // DCT_DCT
    Xin265pFdct16x16_AVX2,       // ADST_DCT
    Xin265pFadst16x16_AVX2,      // DCT_ADST
    Xin265pFadst16x16_AVX2,      // ADST_ADST
    Xin265pFdct16x16_AVX2,       // FLIPADST_DCT
    Xin265pFadst16x16_AVX2,      // DCT_FLIPADST
    Xin265pFadst16x16_AVX2,      // FLIPADST_FLIPADST
    Xin265pFadst16x16_AVX2,      // ADST_FLIPADST
    Xin265pFadst16x16_AVX2,      // FLIPADST_ADST
    Xin265pFidentity16x16_AVX2,  // IDTX
    Xin265pFidentity16x16_AVX2,  // V_DCT
    Xin265pFdct16x16_AVX2,       // H_DCT
    Xin265pFidentity16x16_AVX2,  // V_ADST
    Xin265pFadst16x16_AVX2,      // H_ADST
    Xin265pFidentity16x16_AVX2,  // V_FLIPADST
    Xin265pFadst16x16_AVX2       // H_FLIPADST
};

static const Xin265pFdctOpt xinFdct8x16Hor[XIN_TX_2D_NUM] =
{
    Xin265pFdct8x16_AVX2,       // DCT_DCT
    Xin265pFadst8x16_AVX2,      // ADST_DCT
    Xin265pFdct8x16_AVX2,       // DCT_ADST
    Xin265pFadst8x16_AVX2,      // ADST_ADST
    Xin265pFadst8x16_AVX2,      // FLIPADST_DCT
    Xin265pFdct8x16_AVX2,       // DCT_FLIPADST
    Xin265pFadst8x16_AVX2,      // FLIPADST_FLIPADST
    Xin265pFadst8x16_AVX2,      // ADST_FLIPADST
    Xin265pFadst8x16_AVX2,      // FLIPADST_ADST
    Xin265pFidentity8x16_AVX2,  // IDTX
    Xin265pFdct8x16_AVX2,       // V_DCT
    Xin265pFidentity8x16_AVX2,  // H_DCT
    Xin265pFadst8x16_AVX2,      // V_ADST
    Xin265pFidentity8x16_AVX2,  // H_ADST
    Xin265pFadst8x16_AVX2,      // V_FLIPADST
    Xin265pFidentity8x16_AVX2   // H_FLIPADST
};

static const Xin265pFdctOpt xinFdct8x16Ver[XIN_TX_2D_NUM] =
{
    Xin265pFdctDual8x8_AVX2,       // DCT_DCT
    Xin265pFdctDual8x8_AVX2,       // ADST_DCT
    Xin265pFadstDual8x8_AVX2,      // DCT_ADST
    Xin265pFadstDual8x8_AVX2,      // ADST_ADST
    Xin265pFdctDual8x8_AVX2,       // FLIPADST_DCT
    Xin265pFadstDual8x8_AVX2,      // DCT_FLIPADST
    Xin265pFadstDual8x8_AVX2,      // FLIPADST_FLIPADST
    Xin265pFadstDual8x8_AVX2,      // ADST_FLIPADST
    Xin265pFadstDual8x8_AVX2,      // FLIPADST_ADST
    Xin265pFidentityDual8x8_AVX2,  // IDTX
    Xin265pFidentityDual8x8_AVX2,  // V_DCT
    Xin265pFdctDual8x8_AVX2,       // H_DCT
    Xin265pFidentityDual8x8_AVX2,  // V_ADST
    Xin265pFadstDual8x8_AVX2,      // H_ADST
    Xin265pFidentityDual8x8_AVX2,  // V_FLIPADST
    Xin265pFadstDual8x8_AVX2       // H_FLIPADST
};

static const Xin265pFdctOpt xinFdct16x8Hor[XIN_TX_2D_NUM] =
{
    Xin265pFdctDual8x8_AVX2,       // DCT_DCT
    Xin265pFadstDual8x8_AVX2,      // ADST_DCT
    Xin265pFdctDual8x8_AVX2,       // DCT_ADST
    Xin265pFadstDual8x8_AVX2,      // ADST_ADST
    Xin265pFadstDual8x8_AVX2,      // FLIPADST_DCT
    Xin265pFdctDual8x8_AVX2,       // DCT_FLIPADST
    Xin265pFadstDual8x8_AVX2,      // FLIPADST_FLIPADST
    Xin265pFadstDual8x8_AVX2,      // ADST_FLIPADST
    Xin265pFadstDual8x8_AVX2,      // FLIPADST_ADST
    Xin265pFidentityDual8x8_AVX2,  // IDTX
    Xin265pFdctDual8x8_AVX2,       // V_DCT
    Xin265pFidentityDual8x8_AVX2,  // H_DCT
    Xin265pFadstDual8x8_AVX2,      // V_ADST
    Xin265pFidentityDual8x8_AVX2,  // H_ADST
    Xin265pFadstDual8x8_AVX2,      // V_FLIPADST
    Xin265pFidentityDual8x8_AVX2,  // H_FLIPADST
};

static const Xin265pFdctOpt xinFdct16x8Ver[XIN_TX_2D_NUM] =
{
    Xin265pFdct8x16_AVX2,       // DCT_DCT
    Xin265pFdct8x16_AVX2,       // ADST_DCT
    Xin265pFadst8x16_AVX2,      // DCT_ADST
    Xin265pFadst8x16_AVX2,      // ADST_ADST
    Xin265pFdct8x16_AVX2,       // FLIPADST_DCT
    Xin265pFadst8x16_AVX2,      // DCT_FLIPADST
    Xin265pFadst8x16_AVX2,      // FLIPADST_FLIPADST
    Xin265pFadst8x16_AVX2,      // ADST_FLIPADST
    Xin265pFadst8x16_AVX2,      // FLIPADST_ADST
    Xin265pFidentity8x16_AVX2,  // IDTX
    Xin265pFidentity8x16_AVX2,  // V_DCT
    Xin265pFdct8x16_AVX2,       // H_DCT
    Xin265pFidentity8x16_AVX2,  // V_ADST
    Xin265pFadst8x16_AVX2,      // H_ADST
    Xin265pFidentity8x16_AVX2,  // V_FLIPADST
    Xin265pFadst8x16_AVX2       // H_FLIPADST
};

static inline __m256i pair_set_w16_epi16 (
    SINT32 a,
    SINT32 b)
{
    return _mm256_set1_epi32 ((SINT32)(((UINT16)(a)) | (((UINT32)(b)) << 16)));
}

static inline void flip_buf_sse2 (
    __m128i *in,
    __m128i *out,
    SINT32  size)
{
    SINT32 idx;

    for (idx = 0; idx < size; ++idx)
    {
        out[size - idx - 1] = in[idx];
    }
}

static inline void pack_reg(const __m128i *in1, const __m128i *in2,
                            __m256i *out)
{
    out[0] = _mm256_insertf128_si256(_mm256_castsi128_si256(in1[0]), in2[0], 0x1);
    out[1] = _mm256_insertf128_si256(_mm256_castsi128_si256(in1[1]), in2[1], 0x1);
    out[2] = _mm256_insertf128_si256(_mm256_castsi128_si256(in1[2]), in2[2], 0x1);
    out[3] = _mm256_insertf128_si256(_mm256_castsi128_si256(in1[3]), in2[3], 0x1);
    out[4] = _mm256_insertf128_si256(_mm256_castsi128_si256(in1[4]), in2[4], 0x1);
    out[5] = _mm256_insertf128_si256(_mm256_castsi128_si256(in1[5]), in2[5], 0x1);
    out[6] = _mm256_insertf128_si256(_mm256_castsi128_si256(in1[6]), in2[6], 0x1);
    out[7] = _mm256_insertf128_si256(_mm256_castsi128_si256(in1[7]), in2[7], 0x1);
}

static inline void extract_reg(const __m256i *in, __m128i *out1)
{
    out1[0] = _mm256_castsi256_si128(in[0]);
    out1[1] = _mm256_castsi256_si128(in[1]);
    out1[2] = _mm256_castsi256_si128(in[2]);
    out1[3] = _mm256_castsi256_si128(in[3]);
    out1[4] = _mm256_castsi256_si128(in[4]);
    out1[5] = _mm256_castsi256_si128(in[5]);
    out1[6] = _mm256_castsi256_si128(in[6]);
    out1[7] = _mm256_castsi256_si128(in[7]);

    out1[8] = _mm256_extracti128_si256(in[0], 0x01);
    out1[9] = _mm256_extracti128_si256(in[1], 0x01);
    out1[10] = _mm256_extracti128_si256(in[2], 0x01);
    out1[11] = _mm256_extracti128_si256(in[3], 0x01);
    out1[12] = _mm256_extracti128_si256(in[4], 0x01);
    out1[13] = _mm256_extracti128_si256(in[5], 0x01);
    out1[14] = _mm256_extracti128_si256(in[6], 0x01);
    out1[15] = _mm256_extracti128_si256(in[7], 0x01);
}

static inline __m128i load_16bit_to_16bit (SINT16 *a)
{
    return _mm_load_si128((__m128i *)a);
}

static inline void load_buffer_16bit_to_16bit_flip (
    SINT16   *in,
    intptr_t stride,
    __m128i  *out,
    SINT32   out_size)
{
    SINT32 idx;

    for (idx = 0; idx < out_size; ++idx)
    {
        out[out_size - idx - 1] = load_16bit_to_16bit(in + idx * stride);
    }
}

static inline void round_shift_16bit_w16_avx2 (
    __m256i *in,
    SINT32  size,
    SINT32  bit)
{
    SINT32 idx;

    if (bit < 0)
    {
        bit = -bit;
        __m256i round = _mm256_set1_epi16(1 << (bit - 1));
        for (idx = 0; idx < size; ++idx)
        {
            in[idx] = _mm256_adds_epi16(in[idx], round);
            in[idx] = _mm256_srai_epi16(in[idx], bit);
        }
    }
    else if (bit > 0)
    {
        for (idx = 0; idx < size; ++idx)
        {
            in[idx] = _mm256_slli_epi16(in[idx], bit);
        }
    }

}

static inline __m256i load_16bit_to_16bit_avx2 (
    int16_t *a)
{
    return _mm256_loadu_si256((__m256i *)a);
}

static inline __m256i av1_round_shift_32_avx2 (
    __m256i vec,
    SINT32 bit)
{
    __m256i tmp, round;
    round = _mm256_set1_epi32(1 << (bit - 1));
    tmp = _mm256_add_epi32(vec, round);
    return _mm256_srai_epi32(tmp, bit);
}

static inline void av1_round_shift_array_32_avx2 (
    __m256i *input,
    __m256i *output,
    SINT32 size,
    SINT32 bit)
{
    SINT32 idx;

    if (bit > 0)
    {
        for (idx = 0; idx < size; idx++)
        {
            output[idx] = av1_round_shift_32_avx2(input[idx], bit);
        }
    }
    else
    {
        for (idx = 0; idx < size; idx++)
        {
            output[idx] = _mm256_slli_epi32(input[idx], -bit);
        }
    }

}

static void av1_round_shift_rect_array_32_avx2(__m256i *input,
        __m256i *output,
        const int size,
        const int bit,
        const int val)
{
    const __m256i sqrt2 = _mm256_set1_epi32(val);
    if (bit > 0)
    {
        int i;
        for (i = 0; i < size; i++)
        {
            const __m256i r0 = av1_round_shift_32_avx2(input[i], bit);
            const __m256i r1 = _mm256_mullo_epi32(sqrt2, r0);
            output[i] = av1_round_shift_32_avx2(r1, XIN_SQRT2_BITS);
        }
    }
    else
    {
        int i;
        for (i = 0; i < size; i++)
        {
            const __m256i r0 = _mm256_slli_epi32(input[i], -bit);
            const __m256i r1 = _mm256_mullo_epi32(sqrt2, r0);
            output[i] = av1_round_shift_32_avx2(r1, XIN_SQRT2_BITS);
        }
    }
}

static inline void load_buffer_16bit_to_16bit_avx2 (
    SINT16   *in,
    intptr_t stride,
    __m256i  *out,
    SINT32    out_size)
{
    SINT32 idx;

    for (idx = 0; idx < out_size; ++idx)
    {
        out[idx] = load_16bit_to_16bit_avx2(in + idx * stride);
    }
}

static inline void load_buffer_16bit_to_16bit_flip_avx2 (
    SINT16   *in,
    intptr_t stride,
    __m256i  *out,
    SINT32   out_size)
{
    SINT32 idx;

    for (idx = 0; idx < out_size; ++idx)
    {
        out[out_size - idx - 1] = load_16bit_to_16bit_avx2(in + idx * stride);
    }
}

static inline void load_buffer_16bit_to_16bit (
    SINT16   *in,
    intptr_t stride,
    __m128i  *out,
    SINT32   out_size)
{
    SINT32 idx;

    for (idx = 0; idx < out_size; ++idx)
    {
        out[idx] = load_16bit_to_16bit(in + idx * stride);
    }
}

static inline __m128i scale_round_sse2 (const __m128i a, const int scale)
{
    const __m128i scale_rounding = pair_set_epi16(scale, 1 << (XIN_SQRT2_BITS - 1));
    const __m128i b = _mm_madd_epi16(a, scale_rounding);

    return _mm_srai_epi32(b, XIN_SQRT2_BITS);
}

static inline void store_rect_16bit_to_32bit (
    __m128i a,
    SINT32  *b)
{
    const __m128i one = _mm_set1_epi16(1);
    const __m128i a_lo = _mm_unpacklo_epi16(a, one);
    const __m128i a_hi = _mm_unpackhi_epi16(a, one);

    const __m128i b_lo = scale_round_sse2(a_lo, XIN_FWD_SQRT2);
    const __m128i b_hi = scale_round_sse2(a_hi, XIN_FWD_SQRT2);

    _mm_store_si128((__m128i *)b, b_lo);
    _mm_store_si128((__m128i *)(b + 4), b_hi);
}

static inline void store_rect_buffer_16bit_to_32bit_w8 (
    __m128i  *in,
    SINT32   *out,
    intptr_t stride,
    SINT32   out_size)
{
    SINT32 idx;

    for (idx = 0; idx < out_size; ++idx)
    {
        store_rect_16bit_to_32bit (
            in[idx],
            out + idx * stride);
    }
}

// Store 8 16 bit values. Sign extend the values.
static inline void store_buffer_16bit_to_32bit_w16_avx2 (
    __m256i  *in,
    SINT32   *out,
    intptr_t stride,
    SINT32   out_size)
{
    for (int i = 0; i < out_size; ++i)
    {
        _mm256_storeu_si256((__m256i *)(out),
                            _mm256_cvtepi16_epi32(_mm256_castsi256_si128(in[i])));
        _mm256_storeu_si256(
            (__m256i *)(out + 8),
            _mm256_cvtepi16_epi32(_mm256_extracti128_si256(in[i], 1)));
        out += stride;
    }
}

static inline void flip_buf_avx2 (
    __m256i *in,
    __m256i *out,
    SINT32  size)
{
    SINT32 idx;

    for (idx = 0; idx < size; ++idx)
    {
        out[size - idx - 1] = in[idx];
    }
}

static inline void round_shift_16bit (
    __m128i *in,
    int     size,
    int     bit)
{
    SINT32  idx;
    __m128i rounding;

    if (bit < 0)
    {
        bit      = -bit;
        rounding = _mm_set1_epi16 (1 << (bit - 1));

        for (idx = 0; idx < size; ++idx)
        {
            in[idx] = _mm_adds_epi16 (in[idx], rounding);
            in[idx] = _mm_srai_epi16 (in[idx], bit);
        }
    }
    else if (bit > 0)
    {
        for (idx = 0; idx < size; ++idx)
        {
            in[idx] = _mm_slli_epi16 (in[idx], bit);
        }
    }

}

static inline __m256i scale_round_avx2 (
    __m256i a,
    SINT32 scale)
{
    const __m256i scale_rounding =
        pair_set_w16_epi16(scale, 1 << (XIN_SQRT2_BITS - 1));
    const __m256i b = _mm256_madd_epi16(a, scale_rounding);
    return _mm256_srai_epi32(b, XIN_SQRT2_BITS);
}

static inline void store_rect_16bit_to_32bit_avx2(const __m256i a,
        int32_t *const b)
{
    const __m256i one = _mm256_set1_epi16(1);
    const __m256i a_reoder = _mm256_permute4x64_epi64(a, 0xd8);
    const __m256i a_lo = _mm256_unpacklo_epi16(a_reoder, one);
    const __m256i a_hi = _mm256_unpackhi_epi16(a_reoder, one);
    const __m256i b_lo = scale_round_avx2(a_lo, XIN_FWD_SQRT2);
    const __m256i b_hi = scale_round_avx2(a_hi, XIN_FWD_SQRT2);
    _mm256_storeu_si256((__m256i *)b, b_lo);
    _mm256_storeu_si256((__m256i *)(b + 8), b_hi);
}

static inline void store_rect_buffer_16bit_to_32bit_w16_avx2(
    const __m256i *const in, int32_t *const out, const intptr_t stride,
    const int out_size)
{
    for (int i = 0; i < out_size; ++i)
    {
        store_rect_16bit_to_32bit_avx2(in[i], out + i * stride);
    }
}

static inline void store_rect_16bit_to_32bit_w8_avx2(const __m256i a,
        int32_t *const b, intptr_t stride)
{
    const __m256i one = _mm256_set1_epi16(1);
    const __m256i a_lo = _mm256_unpacklo_epi16(a, one);
    const __m256i a_hi = _mm256_unpackhi_epi16(a, one);
    const __m256i b_lo = scale_round_avx2(a_lo, XIN_FWD_SQRT2);
    const __m256i b_hi = scale_round_avx2(a_hi, XIN_FWD_SQRT2);
    const __m256i temp = _mm256_permute2f128_si256(b_lo, b_hi, 0x31);
    _mm_store_si128((__m128i *)b, _mm256_castsi256_si128(b_lo));
    _mm_store_si128((__m128i *)(b + 4), _mm256_castsi256_si128(b_hi));
    _mm256_storeu_si256((__m256i *)(b + (8*stride)), temp);
}

static inline void store_rect_buffer_16bit_to_32bit_w8_avx2(
    const __m256i *const in, int32_t *const out, intptr_t stride,
    const int out_size)
{
    for (int i = 0; i < out_size; ++i)
    {
        store_rect_16bit_to_32bit_w8_avx2(in[i], out + i * stride, stride);
    }
}

void Xin265pFDct64x64_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    SINT32      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer)
{
    __m256i     buf0[64], buf1[256];
    SINT32      width;
    SINT32      height;
    SINT32      lgWidth;
    SINT32      lgHeight;
    SINT32      widthDiv16;
    SINT32      heightDiv16;
    SINT32      rowIdx;
    SINT32      colIdx;
    SINT32      cosBitHor;
    SINT32      cosBitVer;
    __m256i     bufA[64];
    __m256i     bufB[64];
    const SINT8 *shift;

    (void)tranType;
    (void)tempBuffer;
    shift       = fwdTxShift[tranSize];
    width       = 64;
    height      = 64;
    lgWidth     = 6;
    lgHeight    = 6;
    widthDiv16  = width >> 4;
    heightDiv16 = height >> 4;
    cosBitHor   = fwdCosBitHor[lgWidth][lgHeight];
    cosBitVer   = fwdCosBitVer[lgWidth][lgHeight];

    for (colIdx = 0; colIdx < widthDiv16; colIdx++)
    {
        load_buffer_16bit_to_16bit_avx2 (
            input + 16 * colIdx,
            inputStride,
            buf0,
            height);

        round_shift_16bit_w16_avx2 (
            buf0,
            height,
            shift[0]);

        Xin265pFdct16x64_AVX2 (
            (SINT16 *)buf0,
            (SINT16 *)buf0,
            (SINT8)cosBitHor);

        round_shift_16bit_w16_avx2 (
            buf0,
            height,
            shift[1]);

        for (rowIdx = 0; rowIdx < heightDiv16; ++rowIdx)
        {
            transpose_16bit_16x16_avx2 (
                (SINT16 *)(buf0 + rowIdx * 16),
                (SINT16 *)(buf1 + rowIdx * width + 16 * colIdx));
        }

    }

    for (rowIdx = 0; rowIdx < heightDiv16; rowIdx++)
    {
        __m128i *buf = (__m128i *)(buf1 + width * rowIdx);

        for (int j = 0; j < width; ++j)
        {
            bufA[j] = _mm256_cvtepi16_epi32(buf[j * 2]);
            bufB[j] = _mm256_cvtepi16_epi32(buf[j * 2 + 1]);
        }

        Xin265pFdct64_AVX2 (
            (SINT16 *)bufA,
            (SINT16 *)bufA,
            (SINT8)cosBitVer);

        Xin265pFdct64_AVX2 (
            (SINT16 *)bufB,
            (SINT16 *)bufB,
            (SINT8)cosBitVer);

        av1_round_shift_array_32_avx2 (
            bufA,
            bufA,
            64,
            -shift[2]);

        av1_round_shift_array_32_avx2 (
            bufB,
            bufB,
            64,
            -shift[2]);

        int32_t *output8 = output + 16 * outputStride * rowIdx;

        for (int j = 0; j < 8; ++j)
        {
            __m256i *out = (__m256i *)(output8 + 8 * j);

            transpose_32_8x8_avx2 (
                (SINT32 *)(bufA + 8 * j),
                (SINT32 *)out,
                outputStride);

            transpose_32_8x8_avx2 (
                (SINT32 *)(bufB + 8 * j),
                (SINT32 *)(out + outputStride),
                outputStride);
        }

    }

}

void Xin265pFDct64x32_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    SINT32      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer)
{
    __m256i     buf0[64], buf1[256];
    SINT32      width;
    SINT32      height;
    SINT32      lgWidth;
    SINT32      lgHeight;
    SINT32      widthDiv16;
    SINT32      heightDiv16;
    SINT32      cosBitHor;
    SINT32      cosBitVer;
    __m256i     bufA[64];
    __m256i     bufB[64];
    const SINT8 *shift;

    (void)tempBuffer;
    shift       = fwdTxShift[tranSize];
    width       = 64;
    height      = 32;
    lgWidth     = 6;
    lgHeight    = 5;
    widthDiv16  = width >> 4;
    heightDiv16 = height >> 4;
    cosBitHor   = fwdCosBitHor[lgWidth][lgHeight];
    cosBitVer   = fwdCosBitVer[lgWidth][lgHeight];

    for (int i = 0; i < widthDiv16; i++)
    {
        load_buffer_16bit_to_16bit_avx2 (
            input + 16 * i,
            inputStride,
            buf0,
            height);

        round_shift_16bit_w16_avx2 (
            buf0,
            height,
            shift[0]);

        xinFdct16x32Hor[tranType] (
            (SINT16 *)buf0,
            (SINT16 *)buf0,
            (SINT8)cosBitHor);

        round_shift_16bit_w16_avx2 (
            buf0,
            height,
            shift[1]);

        for (int j = 0; j < heightDiv16; ++j)
        {
            transpose_16bit_16x16_avx2 (
                (SINT16 *)(buf0 + j * 16),
                (SINT16 *)(buf1 + j * width + 16 * i));
        }

    }

    for (int i = 0; i < heightDiv16; i++)
    {
        __m128i *buf = (__m128i *)(buf1 + width * i);

        for (int j = 0; j < width; ++j)
        {
            bufA[j] = _mm256_cvtepi16_epi32(buf[j * 2]);
            bufB[j] = _mm256_cvtepi16_epi32(buf[j * 2 + 1]);
        }

        Xin265pFdct64_AVX2 (
            (SINT16 *)bufA,
            (SINT16 *)bufA,
            (SINT8)cosBitVer);

        Xin265pFdct64_AVX2 (
            (SINT16 *)bufB,
            (SINT16 *)bufB,
            (SINT8)cosBitVer);

        av1_round_shift_rect_array_32_avx2 (
            bufA,
            bufA,
            64,
            -shift[2],
            XIN_FWD_SQRT2);

        av1_round_shift_rect_array_32_avx2 (
            bufB,
            bufB,
            64,
            -shift[2],
            XIN_FWD_SQRT2);

        int32_t *output8 = output + 16 * outputStride * i;
        for (int j = 0; j < 8; ++j)
        {
            __m256i *out = (__m256i *)(output8 + 8 * j);
            transpose_32_8x8_avx2 (
                (SINT32 *)(bufA + 8 * j),
                (SINT32 *)(out),
                outputStride);

            transpose_32_8x8_avx2 (
                (SINT32 *)(bufB + 8 * j),
                (SINT32 *)(out + outputStride),
                outputStride);
        }

    }

}

void Xin265pFDct32x64_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    SINT32      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer)
{
    __m256i     buf0[64], buf1[256];
    SINT32      width;
    SINT32      height;
    SINT32      lgWidth;
    SINT32      lgHeight;
    SINT32      widthDiv16;
    SINT32      heightDiv16;
    SINT32      cosBitHor;
    SINT32      cosBitVer;
    __m256i     bufA[32];
    __m256i     bufB[32];
    const SINT8 *shift;

    (void)tranType;
    (void)tempBuffer;
    shift       = fwdTxShift[tranSize];
    width       = 32;
    height      = 64;
    lgWidth     = 5;
    lgHeight    = 6;
    widthDiv16  = width >> 4;
    heightDiv16 = height >> 4;
    cosBitHor   = fwdCosBitHor[lgWidth][lgHeight];
    cosBitVer   = fwdCosBitVer[lgWidth][lgHeight];

    for (int i = 0; i < widthDiv16; i++)
    {
        load_buffer_16bit_to_16bit_avx2 (
            input + 16 * i,
            inputStride,
            buf0,
            height);

        round_shift_16bit_w16_avx2 (
            buf0,
            height,
            shift[0]);

        Xin265pFdct16x64_AVX2 (
            (SINT16 *)buf0,
            (SINT16 *)buf0,
            (SINT8)cosBitHor);

        round_shift_16bit_w16_avx2 (
            buf0,
            height,
            shift[1]);

        for (int j = 0; j < heightDiv16; ++j)
        {
            transpose_16bit_16x16_avx2 (
                (SINT16 *)(buf0 + j * 16),
                (SINT16 *)(buf1 + j * width + 16 * i));
        }

    }

    for (int i = 0; i < heightDiv16; i++)
    {

        __m128i *buf = (__m128i *)(buf1 + width * i);

        for (int j = 0; j < width; ++j)
        {
            bufA[j] = _mm256_cvtepi16_epi32(buf[j * 2]);
            bufB[j] = _mm256_cvtepi16_epi32(buf[j * 2 + 1]);
        }

        Xin265pFdct32_AVX2 (
            (SINT16 *)bufA,
            (SINT16 *)bufA,
            (SINT8)cosBitVer);

        Xin265pFdct32_AVX2 (
            (SINT16 *)bufB,
            (SINT16 *)bufB,
            (SINT8)cosBitVer);

        av1_round_shift_rect_array_32_avx2 (
            bufA,
            bufA,
            32,
            -shift[2],
            XIN_FWD_SQRT2);

        av1_round_shift_rect_array_32_avx2 (
            bufB,
            bufB,
            32,
            -shift[2],
            XIN_FWD_SQRT2);

        int32_t *output8 = output + 16 * outputStride * i;
        for (int j = 0; j < 4; ++j)
        {
            __m256i *out = (__m256i *)(output8 + 8 * j);
            transpose_32_8x8_avx2 (
                (SINT32 *)(bufA + 8 * j),
                (SINT32 *)out,
                outputStride);

            transpose_32_8x8_avx2 (
                (SINT32 *)(bufB + 8 * j),
                (SINT32 *)(out + outputStride),
                outputStride);
        }

    }

}


void Xin265pFDct32x32_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    SINT32      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer)
{
    const SINT8 *shift;
    SINT32      cosBitHor;
    SINT32      cosBitVer;
    SINT32      lgWidth;
    SINT32      lgHeight;
    SINT32      width;
    SINT32      height;
    SINT32      ud_flip, lr_flip;
    __m256i     buf0[32], buf1[128];

    (void)tempBuffer;
    shift     = fwdTxShift[tranSize];
    width     = 32;
    height    = 32;
    lgWidth   = 5;
    lgHeight  = 5;
    cosBitHor = fwdCosBitHor[lgWidth][lgHeight];
    cosBitVer = fwdCosBitVer[lgWidth][lgHeight];

    Xin265pGetFlip (
        tranType,
        &ud_flip,
        &lr_flip);

    for (int i = 0; i < 2; i++)
    {
        if (ud_flip)
        {
            load_buffer_16bit_to_16bit_flip_avx2 (
                input + 16 * i,
                inputStride,
                buf0,
                height);
        }
        else
        {
            load_buffer_16bit_to_16bit_avx2 (
                input + 16 * i,
                inputStride,
                buf0,
                height);
        }

        round_shift_16bit_w16_avx2 (
            buf0,
            height,
            shift[0]);

        xinFdct16x32Hor[tranType] (
            (SINT16 *)buf0,
            (SINT16 *)buf0,
            (SINT8)cosBitHor);

        round_shift_16bit_w16_avx2 (
            buf0,
            height,
            shift[1]);

        transpose_16bit_16x16_avx2 (
            (SINT16 *)(buf0 + 0 * 16),
            (SINT16 *)(buf1 + 0 * width + 16 * i));

        transpose_16bit_16x16_avx2 (
            (SINT16 *)(buf0 + 1 * 16),
            (SINT16 *)(buf1 + 1 * width + 16 * i));

    }

    for (int i = 0; i < 2; i++)
    {
        __m256i *buf;
        if (lr_flip)
        {
            buf = buf0;
            flip_buf_avx2(buf1 + width * i, buf, width);
        }
        else
        {
            buf = buf1 + width * i;
        }

        xinFdct16x32Ver[tranType] (
            (SINT16 *)buf,
            (SINT16 *)buf,
            (SINT8)cosBitVer);

        round_shift_16bit_w16_avx2 (
            buf,
            width,
            shift[2]);

        transpose_16bit_16x16_avx2 (
            (SINT16 *)(buf),
            (SINT16 *)(buf));

        store_buffer_16bit_to_32bit_w16_avx2 (
            buf,
            output + 16 * outputStride * i,
            outputStride,
            16);

        transpose_16bit_16x16_avx2 (
            (SINT16 *)(buf + 16),
            (SINT16 *)(buf + 16));

        store_buffer_16bit_to_32bit_w16_avx2 (
            buf + 16,
            output + 16 * outputStride * i + 16,
            outputStride,
            16);

    }

}

void Xin265pFDct16x16_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    SINT32      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer)
{
    const SINT8 *shift;
    SINT32      cosBitHor;
    SINT32      cosBitVer;
    SINT32      lgWidth;
    SINT32      lgHeight;
    SINT32      width;
    SINT32      height;
    SINT32      ud_flip, lr_flip;
    __m256i     buf0[16], buf1[16];
    __m256i     *buf;

    (void)tempBuffer;
    shift     = fwdTxShift[tranSize];
    width     = 16;
    height    = 16;
    lgWidth   = 4;
    lgHeight  = 4;
    cosBitHor = fwdCosBitHor[lgWidth][lgHeight];
    cosBitVer = fwdCosBitVer[lgWidth][lgHeight];

    Xin265pGetFlip (
        tranType,
        &ud_flip,
        &lr_flip);

    if (ud_flip)
    {
        load_buffer_16bit_to_16bit_flip_avx2 (
            input,
            inputStride,
            buf0,
            height);
    }
    else
    {
        load_buffer_16bit_to_16bit_avx2 (
            input,
            inputStride,
            buf0,
            height);
    }

    round_shift_16bit_w16_avx2 (
        buf0,
        height,
        shift[0]);

    xinFdct16x16Hor[tranType] (
        (SINT16 *)buf0,
        (SINT16 *)buf0,
        (SINT8)cosBitHor);

    round_shift_16bit_w16_avx2 (
        buf0,
        height,
        shift[1]);

    transpose_16bit_16x16_avx2 (
        (SINT16 *)(buf0),
        (SINT16 *)(buf1 + 0 * width));


    if (lr_flip)
    {
        buf = buf0;

        flip_buf_avx2 (
            buf1,
            buf,
            width);
    }
    else
    {
        buf = buf1;
    }

    xinFdct16x16Ver[tranType] (
        (SINT16 *)buf,
        (SINT16 *)buf,
        (SINT8)cosBitVer);

    round_shift_16bit_w16_avx2 (
        buf,
        width,
        shift[2]);

    transpose_16bit_16x16_avx2 (
        (SINT16 *)buf,
        (SINT16 *)buf);

    store_buffer_16bit_to_32bit_w16_avx2 (
        buf,
        output,
        outputStride,
        16);

}

void Xin265pFDct32x16_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    SINT32      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer)
{
    const SINT8 *shift;
    SINT32      cosBitHor;
    SINT32      cosBitVer;
    SINT32      lgWidth;
    SINT32      lgHeight;
    SINT32      width;
    SINT32      height;
    SINT32      ud_flip, lr_flip;
    __m256i     buf0[32], buf1[64];
    __m256i     *buf;

    (void)tempBuffer;
    shift     = fwdTxShift[tranSize];
    width     = 32;
    height    = 16;
    lgWidth   = 5;
    lgHeight  = 4;
    cosBitHor = fwdCosBitHor[lgWidth][lgHeight];
    cosBitVer = fwdCosBitVer[lgWidth][lgHeight];

    Xin265pGetFlip (
        tranType,
        &ud_flip,
        &lr_flip);

    for (int i = 0; i < 2; i++)
    {
        if (ud_flip)
        {
            load_buffer_16bit_to_16bit_flip_avx2 (
                input + 16 * i,
                inputStride,
                buf0,
                height);
        }
        else
        {
            load_buffer_16bit_to_16bit_avx2 (
                input + 16 * i,
                inputStride,
                buf0,
                height);
        }

        round_shift_16bit_w16_avx2 (
            buf0,
            height,
            shift[0]);

        xinFdct16x16Hor[tranType] (
            (SINT16 *)buf0,
            (SINT16 *)buf0,
            (SINT8)cosBitHor);

        round_shift_16bit_w16_avx2 (
            buf0,
            height,
            shift[1]);

        transpose_16bit_16x16_avx2 (
            (SINT16 *)buf0,
            (SINT16 *)(buf1 + 0 * width + 16 * i));

    }

    if (lr_flip)
    {
        buf = buf0;

        flip_buf_avx2 (
            buf1,
            buf,
            width);
    }
    else
    {
        buf = buf1;
    }

    xinFdct16x32Ver[tranType] (
        (SINT16 *)buf,
        (SINT16 *)buf,
        (SINT8)cosBitVer);

    round_shift_16bit_w16_avx2 (
        buf,
        width,
        shift[2]);

    transpose_16bit_16x16_avx2 (
        (SINT16 *)buf,
        (SINT16 *)buf);

    store_rect_buffer_16bit_to_32bit_w16_avx2 (
        buf,
        output,
        outputStride,
        16);

    transpose_16bit_16x16_avx2 (
        (SINT16 *)(buf + 16),
        (SINT16 *)(buf + 16));

    store_rect_buffer_16bit_to_32bit_w16_avx2 (
        buf + 16,
        output + 16,
        outputStride,
        16);

}

void Xin265pFDct16x32_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    SINT32      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer)
{
    const SINT8 *shift;
    SINT32      cosBitHor;
    SINT32      cosBitVer;
    SINT32      lgWidth;
    SINT32      lgHeight;
    SINT32      width;
    SINT32      height;
    SINT32      ud_flip, lr_flip;
    __m256i     buf0[32], buf1[32];
    __m256i     *buf;

    (void)tempBuffer;
    shift     = fwdTxShift[tranSize];
    width     = 16;
    height    = 32;
    lgWidth   = 4;
    lgHeight  = 5;
    cosBitHor = fwdCosBitHor[lgWidth][lgHeight];
    cosBitVer = fwdCosBitVer[lgWidth][lgHeight];

    Xin265pGetFlip (
        tranType,
        &ud_flip,
        &lr_flip);

    if (ud_flip)
    {
        load_buffer_16bit_to_16bit_flip_avx2 (
            input,
            inputStride,
            buf0,
            height);
    }
    else
    {
        load_buffer_16bit_to_16bit_avx2 (
            input,
            inputStride,
            buf0,
            height);
    }

    round_shift_16bit_w16_avx2 (
        buf0,
        height,
        shift[0]);

    xinFdct16x32Hor[tranType] (
        (SINT16 *)buf0,
        (SINT16 *)buf0,
        (SINT8)cosBitHor);

    round_shift_16bit_w16_avx2 (
        buf0,
        height,
        shift[1]);

    transpose_16bit_16x16_avx2 (
        (SINT16 *)buf0,
        (SINT16 *)buf1);

    transpose_16bit_16x16_avx2 (
        (SINT16 *)(buf0 + 16),
        (SINT16 *)(buf1 + 16));

    for (int i = 0; i < 2; i++)
    {
        if (lr_flip)
        {
            buf = buf0;
            flip_buf_avx2 (
                buf1 + width * i,
                buf,
                width);
        }
        else
        {
            buf = buf1 + width * i;
        }

        xinFdct16x16Ver[tranType] (
            (SINT16 *)buf,
            (SINT16 *)buf,
            (SINT8)cosBitVer);

        round_shift_16bit_w16_avx2 (
            buf,
            width,
            shift[2]);

        transpose_16bit_16x16_avx2 (
            (SINT16 *)buf,
            (SINT16 *)buf);

        store_rect_buffer_16bit_to_32bit_w16_avx2 (
            buf,
            output + 16 * outputStride * i,
            outputStride,
            16);

    }

}

void Xin265pFDct8x16_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    SINT32      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer)
{
    const SINT8 *shift;
    SINT32      cosBitHor;
    SINT32      cosBitVer;
    SINT32      lgWidth;
    SINT32      lgHeight;
    SINT32      width;
    SINT32      height;
    SINT32      ud_flip, lr_flip;
    __m128i     buf0[16], buf1[16];
    __m256i     buf2[8];

    (void)tempBuffer;
    shift     = fwdTxShift[tranSize];
    width     = 8;
    height    = 16;
    lgWidth   = 3;
    lgHeight  = 4;
    cosBitHor = fwdCosBitHor[lgWidth][lgHeight];
    cosBitVer = fwdCosBitVer[lgWidth][lgHeight];

    Xin265pGetFlip (
        tranType,
        &ud_flip,
        &lr_flip);

    if (ud_flip)
    {
        load_buffer_16bit_to_16bit_flip (
            input,
            inputStride,
            buf0,
            height);
    }
    else
    {
        load_buffer_16bit_to_16bit (
            input,
            inputStride,
            buf0,
            height);
    }

    round_shift_16bit (
        buf0,
        height,
        shift[0]);

    xinFdct8x16Hor[tranType] (
        (SINT16 *)buf0,
        (SINT16 *)buf0,
        (SINT8)cosBitHor);

    round_shift_16bit (
        buf0,
        height,
        shift[1]);

    transpose_16bit_8x8 (
        (SINT16 *)buf0,
        (SINT16 *)buf1);

    transpose_16bit_8x8 (
        (SINT16 *)(buf0 + 8),
        (SINT16 *)(buf1 + 8));

    __m128i *bufl, *bufu;
    if (lr_flip)
    {
        bufl = buf0;
        bufu = buf0 + 8;

        flip_buf_sse2 (
            buf1 + width * 0,
            bufl,
            width);

        flip_buf_sse2 (
            buf1 + width * 1,
            bufu,
            width);
    }
    else
    {
        bufl = buf1 + width * 0;
        bufu = buf1 + width * 1;
    }

    pack_reg (
        bufl,
        bufu,
        buf2);

    xinFdct8x16Ver[tranType] (
        (SINT16 *)buf2,
        (SINT16 *)buf2,
        (SINT8)cosBitVer);

    round_shift_16bit_w16_avx2 (
        buf2,
        width,
        shift[2]);

    transpose_16bit_16x8_avx2 (
        (SINT16 *)buf2,
        (SINT16 *)buf2);

    store_rect_buffer_16bit_to_32bit_w8_avx2 (
        buf2,
        output,
        outputStride,
        8);

}

void Xin265pFDct16x8_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    SINT32      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer)
{
    const SINT8 *shift;
    SINT32      cosBitHor;
    SINT32      cosBitVer;
    SINT32      lgWidth;
    SINT32      lgHeight;
    SINT32      width;
    SINT32      height;
    SINT32      ud_flip, lr_flip;
    __m128i     buf0[16], buf1[16];
    __m256i     buf2[8];
    __m128i     *buf;

    (void)tempBuffer;
    shift     = fwdTxShift[tranSize];
    width     = 16;
    height    = 8;
    lgWidth   = 4;
    lgHeight  = 3;
    cosBitHor = fwdCosBitHor[lgWidth][lgHeight];
    cosBitVer = fwdCosBitVer[lgWidth][lgHeight];

    Xin265pGetFlip (
        tranType,
        &ud_flip,
        &lr_flip);

    if (ud_flip)
    {
        load_buffer_16bit_to_16bit_flip (
            input + 8 * 0,
            inputStride,
            buf0,
            height);

        load_buffer_16bit_to_16bit_flip (
            input + 8 * 1,
            inputStride,
            &buf0[8],
            height);
    }
    else
    {
        load_buffer_16bit_to_16bit (
            input + 8 * 0,
            inputStride,
            buf0,
            height);

        load_buffer_16bit_to_16bit (
            input + 8 * 1,
            inputStride,
            &buf0[8],
            height);
    }

    pack_reg (
        buf0,
        &buf0[8],
        buf2);

    round_shift_16bit_w16_avx2 (
        buf2,
        height,
        shift[0]);

    xinFdct16x8Hor[tranType] (
        (SINT16 *)buf2, 
        (SINT16 *)buf2, 
        (SINT8)cosBitHor);
    
    round_shift_16bit_w16_avx2 (
        buf2, 
        height, 
        shift[1]);
    
    transpose_16bit_16x8_avx2 (
        (SINT16 *)buf2, 
        (SINT16 *)buf2);
    
    extract_reg (
        buf2, 
        buf1);

    if (lr_flip)
    {
        buf = buf0;
        
        flip_buf_sse2 (
            buf1, 
            buf, 
            width);
    }
    else
    {
        buf = buf1;
    }
    
    xinFdct16x8Ver[tranType] (
        (SINT16 *)buf, 
        (SINT16 *)buf, 
        (SINT8)cosBitVer);
    
    round_shift_16bit (
        buf, 
        width, 
        shift[2]);
    
    transpose_16bit_8x8 (
        (SINT16 *)buf, 
        (SINT16 *)buf);
    
    store_rect_buffer_16bit_to_32bit_w8 (
        buf, 
        output, 
        outputStride, 
        height);
    
    transpose_16bit_8x8 (
        (SINT16 *)(buf + 8), 
        (SINT16 *)(buf + 8));
    
    store_rect_buffer_16bit_to_32bit_w8 (
        buf + 8, 
        output + 8, 
        outputStride, 
        height);
    
}


