/***************************************************************************//**
 *
 * @file          h265p_forward_trans_sse2.c
 * @brief         av1 forward transform subroutines (SSE2).
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
#include "emmintrin.h"
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

static const Xin265pFdctOpt xinFdct4x4Hor[XIN_TX_2D_NUM] =
{
    Xin265pFdct4x4_SSE2,        // DCT_DCT
    Xin265pFadst4x4_SSE2,       // ADST_DCT
    Xin265pFdct4x4_SSE2,        // DCT_ADST
    Xin265pFadst4x4_SSE2,       // ADST_ADST
    Xin265pFadst4x4_SSE2,       // FLIPADST_DCT
    Xin265pFdct4x4_SSE2,        // DCT_FLIPADST
    Xin265pFadst4x4_SSE2,       // FLIPADST_FLIPADST
    Xin265pFadst4x4_SSE2,       // ADST_FLIPADST
    Xin265pFadst4x4_SSE2,       // FLIPADST_ADST
    Xin265pFidentity4x4_SSE2,   // IDTX
    Xin265pFdct4x4_SSE2,        // V_DCT
    Xin265pFidentity4x4_SSE2,   // H_DCT
    Xin265pFadst4x4_SSE2,       // V_ADST
    Xin265pFidentity4x4_SSE2,   // H_ADST
    Xin265pFadst4x4_SSE2,       // V_FLIPADST
    Xin265pFidentity4x4_SSE2    // H_FLIPADST
};

static const Xin265pFdctOpt xinFdct4x4Ver[XIN_TX_2D_NUM] =
{
    Xin265pFdct4x4_SSE2,        // DCT_DCT
    Xin265pFdct4x4_SSE2,        // ADST_DCT
    Xin265pFadst4x4_SSE2,       // DCT_ADST
    Xin265pFadst4x4_SSE2,       // ADST_ADST
    Xin265pFdct4x4_SSE2,        // FLIPADST_DCT
    Xin265pFadst4x4_SSE2,       // DCT_FLIPADST
    Xin265pFadst4x4_SSE2,       // FLIPADST_FLIPADST
    Xin265pFadst4x4_SSE2,       // ADST_FLIPADST
    Xin265pFadst4x4_SSE2,       // FLIPADST_ADST
    Xin265pFidentity4x4_SSE2,   // IDTX
    Xin265pFidentity4x4_SSE2,   // V_DCT
    Xin265pFdct4x4_SSE2,        // H_DCT
    Xin265pFidentity4x4_SSE2,   // V_ADST
    Xin265pFadst4x4_SSE2,       // H_ADST
    Xin265pFidentity4x4_SSE2,   // V_FLIPADST
    Xin265pFadst4x4_SSE2        // H_FLIPADST
};

static const Xin265pFdctOpt xinFdct8x4Hor[XIN_TX_2D_NUM] =
{
    Xin265pFdct8x4_SSE2,        // DCT_DCT
    Xin265pFadst8x4_SSE2,       // ADST_DCT
    Xin265pFdct8x4_SSE2,        // DCT_ADST
    Xin265pFadst8x4_SSE2,       // ADST_ADST
    Xin265pFadst8x4_SSE2,       // FLIPADST_DCT
    Xin265pFdct8x4_SSE2,        // DCT_FLIPADST
    Xin265pFadst8x4_SSE2,       // FLIPADST_FLIPADST
    Xin265pFadst8x4_SSE2,       // ADST_FLIPADST
    Xin265pFadst8x4_SSE2,       // FLIPADST_ADST
    Xin265pFidentity8x4_SSE2,   // IDTX
    Xin265pFdct8x4_SSE2,        // V_DCT
    Xin265pFidentity8x4_SSE2,   // H_DCT
    Xin265pFadst8x4_SSE2,       // V_ADST
    Xin265pFidentity8x4_SSE2,   // H_ADST
    Xin265pFadst8x4_SSE2,       // V_FLIPADST
    Xin265pFidentity8x4_SSE2    // H_FLIPADST
};

static const Xin265pFdctOpt xinFdct4x8Ver[XIN_TX_2D_NUM] =
{
    Xin265pFdct4x8_SSE2,        // DCT_DCT
    Xin265pFdct4x8_SSE2,        // ADST_DCT
    Xin265pFadst4x8_SSE2,       // DCT_ADST
    Xin265pFadst4x8_SSE2,       // ADST_ADST
    Xin265pFdct4x8_SSE2,        // FLIPADST_DCT
    Xin265pFadst4x8_SSE2,       // DCT_FLIPADST
    Xin265pFadst4x8_SSE2,       // FLIPADST_FLIPADST
    Xin265pFadst4x8_SSE2,       // ADST_FLIPADST
    Xin265pFadst4x8_SSE2,       // FLIPADST_ADST
    Xin265pFidentity8x8_SSE2,   // IDTX
    Xin265pFidentity8x8_SSE2,   // V_DCT
    Xin265pFdct4x8_SSE2,        // H_DCT
    Xin265pFidentity8x8_SSE2,   // V_ADST
    Xin265pFadst4x8_SSE2,       // H_ADST
    Xin265pFidentity8x8_SSE2,   // V_FLIPADST
    Xin265pFadst4x8_SSE2        // H_FLIPADST
};

static const Xin265pFdctOpt xinFdct4x8Hor[XIN_TX_2D_NUM] =
{
    Xin265pFdct4x8_SSE2,        // DCT_DCT
    Xin265pFadst4x8_SSE2,       // ADST_DCT
    Xin265pFdct4x8_SSE2,        // DCT_ADST
    Xin265pFadst4x8_SSE2,       // ADST_ADST
    Xin265pFadst4x8_SSE2,       // FLIPADST_DCT
    Xin265pFdct4x8_SSE2,        // DCT_FLIPADST
    Xin265pFadst4x8_SSE2,       // FLIPADST_FLIPADST
    Xin265pFadst4x8_SSE2,       // ADST_FLIPADST
    Xin265pFadst4x8_SSE2,       // FLIPADST_ADST
    Xin265pFidentity8x8_SSE2,   // IDTX
    Xin265pFdct4x8_SSE2,        // V_DCT
    Xin265pFidentity8x8_SSE2,   // H_DCT
    Xin265pFadst4x8_SSE2,       // V_ADST
    Xin265pFidentity8x8_SSE2,   // H_ADST
    Xin265pFadst4x8_SSE2,       // V_FLIPADST
    Xin265pFidentity8x8_SSE2    // H_FLIPADST
};

static const Xin265pFdctOpt xinFdct8x4Ver[XIN_TX_2D_NUM] =
{
    Xin265pFdct8x4_SSE2,        // DCT_DCT
    Xin265pFdct8x4_SSE2,        // ADST_DCT
    Xin265pFadst8x4_SSE2,       // DCT_ADST
    Xin265pFadst8x4_SSE2,       // ADST_ADST
    Xin265pFdct8x4_SSE2,        // FLIPADST_DCT
    Xin265pFadst8x4_SSE2,       // DCT_FLIPADST
    Xin265pFadst8x4_SSE2,       // FLIPADST_FLIPADST
    Xin265pFadst8x4_SSE2,       // ADST_FLIPADST
    Xin265pFadst8x4_SSE2,       // FLIPADST_ADST
    Xin265pFidentity8x4_SSE2,   // IDTX
    Xin265pFidentity8x4_SSE2,   // V_DCT
    Xin265pFdct8x4_SSE2,        // H_DCT
    Xin265pFidentity8x4_SSE2,   // V_ADST
    Xin265pFadst8x4_SSE2,       // H_ADST
    Xin265pFidentity8x4_SSE2,   // V_FLIPADST
    Xin265pFadst8x4_SSE2        // H_FLIPADST
};

static const Xin265pFdctOpt xinFdct8x8Hor[XIN_TX_2D_NUM] =
{
    Xin265pFdct8x8_SSE2,        // DCT_DCT
    Xin265pFadst8x8_SSE2,       // ADST_DCT
    Xin265pFdct8x8_SSE2,        // DCT_ADST
    Xin265pFadst8x8_SSE2,       // ADST_ADST
    Xin265pFadst8x8_SSE2,       // FLIPADST_DCT
    Xin265pFdct8x8_SSE2,        // DCT_FLIPADST
    Xin265pFadst8x8_SSE2,       // FLIPADST_FLIPADST
    Xin265pFadst8x8_SSE2,       // ADST_FLIPADST
    Xin265pFadst8x8_SSE2,       // FLIPADST_ADST
    Xin265pFidentity8x8_SSE2,   // IDTX
    Xin265pFdct8x8_SSE2,        // V_DCT
    Xin265pFidentity8x8_SSE2,   // H_DCT
    Xin265pFadst8x8_SSE2,       // V_ADST
    Xin265pFidentity8x8_SSE2,   // H_ADST
    Xin265pFadst8x8_SSE2,       // V_FLIPADST
    Xin265pFidentity8x8_SSE2    // H_FLIPADST
};

static const Xin265pFdctOpt xinFdct8x8Ver[XIN_TX_2D_NUM] =
{
    Xin265pFdct8x8_SSE2,        // DCT_DCT
    Xin265pFdct8x8_SSE2,        // ADST_DCT
    Xin265pFadst8x8_SSE2,       // DCT_ADST
    Xin265pFadst8x8_SSE2,       // ADST_ADST
    Xin265pFdct8x8_SSE2,        // FLIPADST_DCT
    Xin265pFadst8x8_SSE2,       // DCT_FLIPADST
    Xin265pFadst8x8_SSE2,       // FLIPADST_FLIPADST
    Xin265pFadst8x8_SSE2,       // ADST_FLIPADST
    Xin265pFadst8x8_SSE2,       // FLIPADST_ADST
    Xin265pFidentity8x8_SSE2,   // IDTX
    Xin265pFidentity8x8_SSE2,   // V_DCT
    Xin265pFdct8x8_SSE2,        // H_DCT
    Xin265pFidentity8x8_SSE2,   // V_ADST
    Xin265pFadst8x8_SSE2,       // H_ADST
    Xin265pFidentity8x8_SSE2,   // V_FLIPADST
    Xin265pFadst8x8_SSE2        // H_FLIPADST
};

static inline void load_buffer_16bit_to_16bit_w4_flip (
    SINT16   *in,
    intptr_t stride,
    __m128i  *out,
    SINT32   out_size)
{
    SINT32 idx;

    for (idx = 0; idx < out_size; ++idx)
    {
        out[out_size - idx - 1] = _mm_loadl_epi64 ((const __m128i *)(in + idx * stride));
    }
}

static inline void load_buffer_16bit_to_16bit_w4(
    SINT16   *in,
    intptr_t stride,
    __m128i  *out,
    SINT32   out_size)
{
    SINT32 idx;

    for (idx = 0; idx < out_size; ++idx)
    {
        out[idx] = _mm_loadl_epi64 ((const __m128i *)(in + idx * stride));
    }
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

// Store 4 16 bit values. Sign extend the values.
static inline void store_16bit_to_32bit_w4 (
    __m128i a,
    SINT32  *b)
{
    __m128i a_lo = _mm_unpacklo_epi16 (a, a);
    __m128i a_1 = _mm_srai_epi32 (a_lo, 16);
    _mm_store_si128((__m128i *)b, a_1);
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

static inline void store_rect_16bit_to_32bit_w4 (
    __m128i a,
    SINT32  *b)
{
    const __m128i one = _mm_set1_epi16(1);
    const __m128i a_lo = _mm_unpacklo_epi16(a, one);
    const __m128i b_lo = scale_round_sse2(a_lo, XIN_FWD_SQRT2);
    _mm_store_si128((__m128i *)b, b_lo);
}

static inline void store_16bit_to_32bit (
    __m128i a,
    SINT32  *b)
{
    const __m128i a_lo = _mm_unpacklo_epi16(a, a);
    const __m128i a_hi = _mm_unpackhi_epi16(a, a);
    const __m128i a_1 = _mm_srai_epi32(a_lo, 16);
    const __m128i a_2 = _mm_srai_epi32(a_hi, 16);
    _mm_store_si128((__m128i *)b, a_1);
    _mm_store_si128((__m128i *)(b + 4), a_2);
}

static inline void store_buffer_16bit_to_32bit_w4 (
    __m128i  *in,
    SINT32   *out,
    intptr_t stride,
    SINT32   out_size)
{
    SINT32 idx;

    for (idx = 0; idx < out_size; ++idx)
    {
        store_16bit_to_32bit_w4 (
            in[idx],
            out + idx * stride);
    }
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

static inline void store_rect_buffer_16bit_to_32bit_w4 (
    __m128i  *in,
    SINT32   *out,
    intptr_t stride,
    SINT32   out_size)
{
    SINT32 idx;

    for (idx = 0; idx < out_size; ++idx)
    {
        store_rect_16bit_to_32bit_w4 (
            in[idx],
            out + idx * stride);
    }
}

static inline void store_buffer_16bit_to_32bit_w8 (
    __m128i  *in,
    SINT32   *out,
    intptr_t stride,
    SINT32   out_size)
{
    SINT32 idx;

    for (idx = 0; idx < out_size; ++idx)
    {
        store_16bit_to_32bit (
            in[idx],
            out + idx * stride);
    }
}


void Xin265pFDct4x4_SSE2 (
    SINT16      *input,
    intptr_t    inputStride,
    SINT32      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer)
{
    __m128i      buf0[4], buf1[4];
    __m128i     *buf;
    const SINT8 *shift;
    SINT32      cosBitHor;
    SINT32      cosBitVer;
    SINT32      lgWidth;
    SINT32      lgHeight;
    SINT32      ud_flip, lr_flip;

    (void)tempBuffer;

    shift     = fwdTxShift[tranSize];
    lgWidth   = 2;
    lgHeight  = 2;
    cosBitHor = fwdCosBitHor[lgWidth][lgHeight];
    cosBitVer = fwdCosBitVer[lgWidth][lgHeight];

    Xin265pGetFlip (
        tranType,
        &ud_flip,
        &lr_flip);

    if (ud_flip)
    {
        load_buffer_16bit_to_16bit_w4_flip (
            input,
            inputStride,
            buf0,
            4);
    }
    else
    {
        load_buffer_16bit_to_16bit_w4 (
            input,
            inputStride,
            buf0,
            4);
    }

    round_shift_16bit (
        buf0,
        4,
        shift[0]);

    xinFdct4x4Hor[tranType] (
        (SINT16 *)buf0,
        (SINT16 *)buf0,
        (SINT8)cosBitHor);

    round_shift_16bit (
        buf0,
        4,
        shift[1]);

    transpose_16bit_4x4 (
        (SINT16 *)buf0,
        (SINT16 *)buf1);

    if (lr_flip)
    {
        buf = buf0;

        flip_buf_sse2 (
            buf1,
            buf,
            4);
    }
    else
    {
        buf = buf1;
    }

    xinFdct4x4Ver[tranType] (
        (SINT16 *)buf,
        (SINT16 *)buf,
        (SINT8)cosBitVer);

    round_shift_16bit (
        buf,
        4,
        shift[2]);

    transpose_16bit_4x4 (
        (SINT16 *)buf,
        (SINT16 *)buf);

    store_buffer_16bit_to_32bit_w4 (
        buf,
        output,
        outputStride,
        4);

}

void Xin265pFDct8x4_SSE2 (
    SINT16      *input,
    intptr_t    inputStride,
    SINT32      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer)
{
    __m128i      buf0[8], buf1[8];
    __m128i     *buf;
    const SINT8 *shift;
    SINT32      cosBitHor;
    SINT32      cosBitVer;
    SINT32      lgWidth;
    SINT32      lgHeight;
    SINT32      width;
    SINT32      height;
    SINT32      ud_flip, lr_flip;

    (void)tempBuffer;
    shift     = fwdTxShift[tranSize];
    width     = 8;
    height    = 4;
    lgWidth   = 3;
    lgHeight  = 2;
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

    xinFdct8x4Hor[tranType] (
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

    xinFdct4x8Ver[tranType] (
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

}

void Xin265pFDct4x8_SSE2 (
    SINT16      *input,
    intptr_t    inputStride,
    SINT32      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer)
{
    __m128i      buf0[8], buf1[8];
    __m128i     *buf;
    const SINT8 *shift;
    SINT32      cosBitHor;
    SINT32      cosBitVer;
    SINT32      lgWidth;
    SINT32      lgHeight;
    SINT32      width;
    SINT32      height;
    SINT32      ud_flip, lr_flip;

    (void)tempBuffer;
    shift     = fwdTxShift[tranSize];
    width     = 4;
    height    = 8;
    lgWidth   = 3;
    lgHeight  = 2;
    cosBitHor = fwdCosBitHor[lgWidth][lgHeight];
    cosBitVer = fwdCosBitVer[lgWidth][lgHeight];

    Xin265pGetFlip (
        tranType,
        &ud_flip,
        &lr_flip);

    if (ud_flip)
    {
        load_buffer_16bit_to_16bit_w4_flip (
            input,
            inputStride,
            buf0,
            height);
    }
    else
    {
        load_buffer_16bit_to_16bit_w4 (
            input,
            inputStride,
            buf0,
            height);
    }

    round_shift_16bit (
        buf0,
        height,
        shift[0]);

    xinFdct4x8Hor[tranType] (
        (SINT16 *)buf0,
        (SINT16 *)buf0,
        (SINT8)cosBitHor);

    round_shift_16bit (
        buf0,
        height,
        shift[1]);

    transpose_16bit_4x8 (
        (SINT16 *)buf0,
        (SINT16 *)buf1);

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

    xinFdct8x4Ver[tranType] (
        (SINT16 *)buf,
        (SINT16 *)buf,
        (SINT8)cosBitVer);

    round_shift_16bit (
        buf,
        width,
        shift[2]);

    transpose_16bit_8x4 (
        (SINT16 *)buf,
        (SINT16 *)buf);

    store_rect_buffer_16bit_to_32bit_w4 (
        buf,
        output,
        outputStride,
        height);

}

void Xin265pFDct8x8_SSE2 (
    SINT16      *input,
    intptr_t    inputStride,
    SINT32      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer)
{
    __m128i      buf0[8], buf1[8];
    __m128i     *buf;
    const SINT8 *shift;
    SINT32      cosBitHor;
    SINT32      cosBitVer;
    SINT32      lgWidth;
    SINT32      lgHeight;
    SINT32      width;
    SINT32      height;
    SINT32      ud_flip, lr_flip;

    (void)tempBuffer;
    shift     = fwdTxShift[tranSize];
    width     = 8;
    height    = 8;
    lgWidth   = 3;
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

    xinFdct8x8Hor[tranType] (
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

    xinFdct8x8Ver[tranType] (
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

    store_buffer_16bit_to_32bit_w8 (
        buf,
        output,
        outputStride,
        height);

}


