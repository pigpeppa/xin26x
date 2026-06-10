/***************************************************************************//**
 *
 * @file          h265p_inverse_trans_sse4.c
 * @brief         av1 inverse transform subroutines (SSE4).
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
#include "smmintrin.h"
#include "xin_typedef.h"
#include "h26x_common_data.h"
#include "basic_macro.h"
#include "string.h"
#include "stdint.h"
#include "h265p_inverse_1d_trans.h"
#include "h265p_trans_context.h"
#include "h265p_inverse_trans.h"
#include "h265p_trans_utility.h"

static Xin265pIdctOpt xinIdct4P4[XIN_TX_1D_NUM] =
{
    Xin265pIdct4P4_SSE2,
    Xin265pIadst4P4_SSE2,
    Xin265pIidentity4_SSSE3
};

static Xin265pIdctOpt xinIdct4P8[XIN_TX_1D_NUM] =
{
    Xin265pIdct4P8_SSE2,
    Xin265pIadst4P8_SSE2,
    Xin265pIidentity4_SSSE3
};

static Xin265pIdctOpt xinIdct8P4[XIN_TX_1D_NUM] =
{
    Xin265pIdct8P4_SSE2,
    Xin265pIadst8P4_SSE2,
    Xin265pIidentity8_SSSE3
};

static Xin265pIdctOpt xinIdctNonIdentity[7][XIN_TX_1D_NUM] =
{
    {
        NULL,
        NULL,
        NULL
    },
    {
        NULL,
        NULL,
        NULL
    },
    {
        NULL,
        NULL,
        NULL
    },
    {
        Xin265pIdct8P8_SSE2,
        Xin265pIadst8P8_SSE2,
        Xin265pIidentity8_SSSE3
    },
    {
        Xin265pIdct16_SSE2,
        Xin265pIadst16_SSSE3,
        NULL
    },
    {
        Xin265pIdct32_AVX2,
        NULL,
        NULL
    },
    {
        Xin265pIdct64_AVX2,
        NULL,
        NULL
    }
};

static inline __m128i load_32bit_to_16bit_w4(const int32_t *a)
{
    const __m128i a_low = _mm_load_si128((const __m128i *)a);
    return _mm_packs_epi32(a_low, a_low);
}

static inline __m128i load_32bit_to_16bit(const int32_t *a)
{
    const __m128i a_low = _mm_load_si128((const __m128i *)a);
    return _mm_packs_epi32(a_low, *(const __m128i *)(a + 4));
}

static inline void load_buffer_32bit_to_16bit_w4(const int32_t *in, intptr_t stride,
        __m128i *out, int out_size)
{
    for (int i = 0; i < out_size; ++i)
    {
        out[i] = load_32bit_to_16bit_w4(in + i * stride);
    }
}

static inline void load_buffer_32bit_to_16bit(const int32_t *in, intptr_t stride,
        __m128i *out, int out_size)
{
    for (int i = 0; i < out_size; ++i)
    {
        out[i] = load_32bit_to_16bit(in + i * stride);
    }
}

static inline void round_shift_16bit_ssse3 (__m128i *in, int size, int bit)
{
    if (bit < 0)
    {
        const __m128i scale = _mm_set1_epi16(1 << (15 + bit));
        for (int i = 0; i < size; ++i)
        {
            in[i] = _mm_mulhrs_epi16(in[i], scale);
        }
    }
    else if (bit > 0)
    {
        for (int i = 0; i < size; ++i)
        {
            in[i] = _mm_slli_epi16(in[i], bit);
        }
    }
}

static inline void lowbd_write_buffer_4xn_sse2 (__m128i *in, int16_t *output,
        intptr_t stride, int flipud,
        const int height)
{
    int j = flipud ? (height - 1) : 0;
    const int step = flipud ? -1 : 1;

    for (int i = 0; i < height; ++i, j += step)
    {
        _mm_storel_epi64 ( (__m128i *)(output + i * stride), in[j]);
    }
}

static inline void lowbd_write_buffer_8xn_sse2(__m128i *in, int16_t *output,
        intptr_t stride, int flipud,
        const int height)
{
    int j = flipud ? (height - 1) : 0;
    const int step = flipud ? -1 : 1;

    for (int i = 0; i < height; ++i, j += step)
    {
        _mm_storeu_si128((__m128i *)(output + i * stride), in[j]);
    }
}

static inline void lowbd_write_buffer_16xn_sse2(__m128i *in, int16_t *output,
        intptr_t stride, int flipud,
        int height)
{
    int j = flipud ? (height - 1) : 0;
    const int step = flipud ? -1 : 1;
    for (int i = 0; i < height; ++i, j += step)
    {
        _mm_storeu_si128((__m128i *)(output + i * stride), in[j]);
        _mm_storeu_si128((__m128i *)(output + 8 + i * stride), in[j + height]);
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

static inline void round_shift_ssse3 (
    const __m128i *input,
    __m128i *output,
    int size)
{
    const __m128i scale = _mm_set1_epi16(XIN_INV_SQRT2 * 8);
    for (int i = 0; i < size; ++i)
    {
        output[i] = _mm_mulhrs_epi16(input[i], scale);
    }
}

void Xin265pIDct4x4_SSE4 (
    SINT32      *input,
    intptr_t    inputStride,
    SINT16      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer)
{
    const SINT8 *shift;
    __m128i buf[4];
    __m128i temp[4];
    SINT32  cosBitHor;
    SINT32  cosBitVer;
    SINT32  width;
    SINT32  height;
    SINT32  lgWidth;
    SINT32  lgHeight;
    SINT32  ud_flip, lr_flip;
    UINT32  t1dHor, t1dVer;

    (void)tempBuffer;
    shift       = invTxShift[tranSize];
    width       = txSize2TxDim[tranSize][0];
    height      = txSize2TxDim[tranSize][1];
    lgWidth     = calcLog2[width];
    lgHeight    = calcLog2[height];
    cosBitHor   = invCosBitHor[lgHeight][lgWidth];
    cosBitVer   = invCosBitVer[lgHeight][lgWidth];
    t1dHor      = tx1dHor[tranType];
    t1dVer      = tx1dVer[tranType];

    Xin265pGetFlip (
        tranType,
        &ud_flip,
        &lr_flip);

    load_buffer_32bit_to_16bit_w4 (
        input,
        inputStride,
        buf,
        height);

    transpose_16bit_4x4 (
        (SINT16 *)buf,
        (SINT16 *)buf);

    xinIdct4P4[t1dVer] (
        (SINT16 *)buf,
        (SINT16 *)buf,
        (SINT8)cosBitVer);

    if (lr_flip)
    {
        flip_buf_sse2 (
            buf,
            temp,
            width);

        transpose_16bit_4x4 (
            (SINT16 *)temp,
            (SINT16 *)buf);
    }
    else
    {
        transpose_16bit_4x4 (
            (SINT16 *)buf,
            (SINT16 *)buf);
    }

    xinIdct4P4[t1dHor] (
        (SINT16 *)buf,
        (SINT16 *)buf,
        (SINT8)cosBitHor);

    round_shift_16bit_ssse3 (
        buf,
        height,
        shift[1]);

    lowbd_write_buffer_4xn_sse2 (
        buf,
        output,
        outputStride,
        ud_flip,
        height);

}


void Xin265pIDct4x8_SSE4 (
    SINT32      *input,
    intptr_t    inputStride,
    SINT16      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer)
{
    const SINT8 *shift;
    __m128i buf[8];
    __m128i temp[4];
    SINT32  cosBitHor;
    SINT32  cosBitVer;
    SINT32  width;
    SINT32  height;
    SINT32  lgWidth;
    SINT32  lgHeight;
    SINT32  ud_flip, lr_flip;
    UINT32  t1dHor, t1dVer;

    (void)tempBuffer;
    shift     = invTxShift[tranSize];
    width     = txSize2TxDim[tranSize][0];
    height    = txSize2TxDim[tranSize][1];
    lgWidth   = calcLog2[width];
    lgHeight  = calcLog2[height];
    cosBitHor = invCosBitHor[lgHeight][lgWidth];
    cosBitVer = invCosBitVer[lgHeight][lgWidth];
    t1dHor    = tx1dHor[tranType];
    t1dVer    = tx1dVer[tranType];

    Xin265pGetFlip (
        tranType,
        &ud_flip,
        &lr_flip);

    load_buffer_32bit_to_16bit_w4 (
        input,
        inputStride,
        buf,
        height);

    transpose_16bit_4x8 (
        (SINT16 *)buf,
        (SINT16 *)buf);

    round_shift_ssse3 (
        buf,
        buf,
        width);  // rect special code

    xinIdct4P8[t1dVer] (
        (SINT16 *)buf,
        (SINT16 *)buf,
        (SINT8)cosBitVer);

    // round_shift_16bit_ssse3(buf, txfm_size_col, shift[0]);// shift[0] is 0
    if (lr_flip)
    {
        flip_buf_sse2 (
            buf,
            temp,
            width);

        transpose_16bit_8x4 (
            (SINT16 *)temp,
            (SINT16 *)buf);
    }
    else
    {
        transpose_16bit_8x4 (
            (SINT16 *)buf,
            (SINT16 *)buf);
    }

    xinIdct8P4[t1dHor] (
        (SINT16 *)buf,
        (SINT16 *)buf,
        (SINT8)cosBitHor);

    round_shift_16bit_ssse3 (
        buf,
        height,
        shift[1]);

    lowbd_write_buffer_4xn_sse2 (
        buf,
        output,
        outputStride,
        ud_flip,
        height);

}

void Xin265pIDct8x4_SSE4 (
    SINT32      *input,
    intptr_t    inputStride,
    SINT16      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer)
{
    const SINT8 *shift;
    __m128i buf[8];
    __m128i temp[8];
    SINT32  cosBitHor;
    SINT32  cosBitVer;
    SINT32  width;
    SINT32  height;
    SINT32  lgWidth;
    SINT32  lgHeight;
    SINT32  ud_flip, lr_flip;
    UINT32  t1dHor, t1dVer;

    (void)tempBuffer;
    shift     = invTxShift[tranSize];
    width     = txSize2TxDim[tranSize][0];
    height    = txSize2TxDim[tranSize][1];
    lgWidth   = calcLog2[width];
    lgHeight  = calcLog2[height];
    cosBitHor = invCosBitHor[lgHeight][lgWidth];
    cosBitVer = invCosBitVer[lgHeight][lgWidth];
    t1dHor    = tx1dHor[tranType];
    t1dVer    = tx1dVer[tranType];

    Xin265pGetFlip (
        tranType,
        &ud_flip,
        &lr_flip);

    load_buffer_32bit_to_16bit (
        input,
        inputStride,
        buf,
        height);

    transpose_16bit_8x4 (
        (SINT16 *)buf,
        (SINT16 *)buf);

    round_shift_ssse3 (
        buf,
        buf,
        width);  // rect special code

    xinIdct8P4[t1dVer] (
        (SINT16 *)buf,
        (SINT16 *)buf,
        (SINT8)cosBitVer);

    // round_shift_16bit_ssse3(buf, txfm_size_col, shift[0]); // shift[0] is 0
    if (lr_flip)
    {
        flip_buf_sse2 (
            buf,
            temp,
            width);

        transpose_16bit_4x8 (
            (SINT16 *)temp,
            (SINT16 *)buf);
    }
    else
    {
        transpose_16bit_4x8 (
            (SINT16 *)buf,
            (SINT16 *)buf);
    }

    xinIdct4P8[t1dHor] (
        (SINT16 *)buf,
        (SINT16 *)buf,
        (SINT8)cosBitHor);

    round_shift_16bit_ssse3 (
        buf,
        height,
        shift[1]);

    lowbd_write_buffer_8xn_sse2 (
        buf,
        output,
        outputStride,
        ud_flip,
        height);

}

void Xin265pIDctNoIdentityWxH_SSE4 (
    SINT32      *input,
    intptr_t    inputStride,
    SINT16      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer)
{

    const SINT8 *shift;
    __m128i buf1[64 * 8];
    __m128i buf0[64];
    __m128i temp[8];
    SINT32  cosBitHor;
    SINT32  cosBitVer;
    SINT32  width;
    SINT32  height;
    SINT32  lgWidth;
    SINT32  lgHeight;
    SINT32  widthDiv8;
    SINT32  heightDiv8;
    SINT32  ud_flip, lr_flip;
    UINT32  t1dHor, t1dVer;

    (void)tempBuffer;
    shift      = invTxShift[tranSize];
    width      = txSize2TxDim[tranSize][0];
    height     = txSize2TxDim[tranSize][1];
    lgWidth    = calcLog2[width];
    lgHeight   = calcLog2[height];
    cosBitHor  = invCosBitHor[lgHeight][lgWidth];
    cosBitVer  = invCosBitVer[lgHeight][lgWidth];
    widthDiv8  = width >> 3;
    heightDiv8 = height >> 3;
    t1dHor     = tx1dHor[tranType];
    t1dVer     = tx1dVer[tranType];

    Xin265pGetFlip (
        tranType,
        &ud_flip,
        &lr_flip);

    for (int i = 0; i < heightDiv8; i++)
    {
        const int32_t *input_row = input + i * inputStride* 8;
        for (int j = 0; j < widthDiv8; ++j)
        {
            __m128i *buf0_cur = buf0 + j * 8;

            load_buffer_32bit_to_16bit (
                input_row + j * 8,
                inputStride,
                buf0_cur,
                8);

            transpose_16bit_8x8 (
                (SINT16 *)buf0_cur,
                (SINT16 *)buf0_cur);
        }

        if ((height == 2*width) || (width == 2*height))
        {
            round_shift_ssse3 (
                buf0,
                buf0,
                width);
        }

        xinIdctNonIdentity[lgWidth][t1dVer] (
            (SINT16 *)buf0,
            (SINT16 *)buf0,
            (SINT8)cosBitVer);

        round_shift_16bit_ssse3 (
            buf0,
            width,
            shift[0]);

        __m128i *_buf1 = buf1 + i * 8;

        if (lr_flip)
        {
            for (int j = 0; j < widthDiv8; ++j)
            {
                flip_buf_sse2 (
                    buf0 + 8 * j,
                    temp,
                    8);

                transpose_16bit_8x8 (
                    (SINT16 *)temp,
                    (SINT16 *)(_buf1 + height * (widthDiv8 - 1 - j)));
            }
        }
        else
        {
            for (int j = 0; j < widthDiv8; ++j)
            {
                transpose_16bit_8x8 (
                    (SINT16 *)(buf0 + 8 * j),
                    (SINT16 *)(_buf1 + height * j));
            }
        }

    }

    for (int i = 0; i < widthDiv8; i++)
    {
        xinIdctNonIdentity[lgHeight][t1dHor] (
            (SINT16 *)(buf1 + i * height),
            (SINT16 *)(buf1 + i * height),
            (SINT8)cosBitHor);

        round_shift_16bit_ssse3 (
            buf1 + i * height,
            height,
            shift[1]);
    }

    if (width >= 16)
    {
        for (int i = 0; i < (width >> 4); i++)
        {
            lowbd_write_buffer_16xn_sse2 (
                buf1 + i * height * 2,
                output + 16 * i,
                outputStride,
                ud_flip,
                height);
        }
    }
    else if (width == 8)
    {
        lowbd_write_buffer_8xn_sse2 (
            buf1,
            output,
            outputStride,
            ud_flip,
            height);
    }

}

void Xin265pIDct8x8_SSE4 (
    SINT32      *input,
    intptr_t    inputStride,
    SINT16      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer)
{
    switch (tranType)
    {
    case XIN_DCT_DCT:
    case XIN_ADST_DCT:   // ADST in vertical, DCT in horizontal
    case XIN_DCT_ADST:   // DCT  in vertical, ADST in horizontal
    case XIN_ADST_ADST:  // ADST in both directions
    case XIN_FLIPADST_DCT:
    case XIN_DCT_FLIPADST:
    case XIN_FLIPADST_FLIPADST:
    case XIN_ADST_FLIPADST:
    case XIN_FLIPADST_ADST:

        Xin265pIDctNoIdentityWxH_SSE4 (
            input,
            inputStride,
            output,
            outputStride,
            tranType,
            tranSize,
            tempBuffer);

        break;

    default:

        Xin265pIDctWxH (
            input,
            inputStride,
            output,
            outputStride,
            tranType,
            tranSize,
            tempBuffer);

        break;

    }
}


