/***************************************************************************//**
*
* @file          h265p_inverse_trans_avx2.c
* @brief         av1 inverse transform subroutines.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#include "immintrin.h"
#include "xin_typedef.h"
#include "h26x_common_data.h"
#include "basic_macro.h"
#include "string.h"
#include "stdint.h"
#include "h265p_inverse_1d_trans.h"
#include "h265p_trans_context.h"
#include "h265p_inverse_trans.h"
#include "h265p_trans_utility.h"

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
        NULL,
        NULL,
        NULL
    },
    {
        Xin265pIdct16_AVX2,
        Xin265pIadst16_AVX2,
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

static inline __m256i load_32bit_to_16bit_w16_avx2(const int32_t *a)
{
    const __m256i a_low = _mm256_lddqu_si256((const __m256i *)a);
    const __m256i b = _mm256_packs_epi32(a_low, *(const __m256i *)(a + 8));
    return _mm256_permute4x64_epi64(b, 0xD8);
}

static inline void load_buffer_32bit_to_16bit_w16_avx2(const int32_t *in,
        intptr_t stride, __m256i *out,
        int out_size)
{
    for (int i = 0; i < out_size; ++i)
    {
        out[i] = load_32bit_to_16bit_w16_avx2(in + i * stride);
    }
}

static inline void round_shift_avx2(const __m256i *input, __m256i *output,
                                    int size)
{
    const __m256i scale = _mm256_set1_epi16(XIN_INV_SQRT2 * 8);
    for (int i = 0; i < size; ++i)
    {
        output[i] = _mm256_mulhrs_epi16(input[i], scale);
    }
}

static inline void lowbd_write_buffer_16xn_avx2(__m256i *in, int16_t *output,
        intptr_t stride, int flipud,
        int height)
{
    int j = flipud ? (height - 1) : 0;
    const int step = flipud ? -1 : 1;
    for (int i = 0; i < height; ++i, j += step)
    {
        _mm256_storeu_si256( (__m256i *)(output + i * stride), in[j]);
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

void Xin265pIDctNoIdentityWxH_AVX2 (
    SINT32      *input,
    intptr_t    inputStride,
    SINT16      *output,
    intptr_t    outputStride,
    UINT32      tranType,
    UINT32      tranSize,
    SINT32      *tempBuffer)
{
    const SINT8 *shift;
    __m256i buf1[64 * 16];
    __m256i buf0[64];
    __m256i scale0;
    SINT32  cosBitHor;
    SINT32  cosBitVer;
    SINT32  width;
    SINT32  height;
    SINT32  lgWidth;
    SINT32  lgHeight;
    SINT32  widthDiv16;
    SINT32  heightDiv16;
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
    widthDiv16  = width >> 4;
    heightDiv16 = height >> 4;
    scale0      = _mm256_set1_epi16(1 << (15 + shift[0]));
    t1dHor      = tx1dHor[tranType];
    t1dVer      = tx1dVer[tranType];

    Xin265pGetFlip (
        tranType,
        &ud_flip,
        &lr_flip);

    for (int i = 0; i < heightDiv16; i++)
    {
        const SINT32 *input_row = input + (i << 4) * inputStride;

        for (int j = 0; j < widthDiv16; ++j)
        {
            __m256i *buf0_cur = buf0 + j * 16;
            const int32_t *input_cur = input_row + j * 16;

            load_buffer_32bit_to_16bit_w16_avx2 (
                input_cur,
                inputStride,
                buf0_cur,
                16);

            transpose_16bit_16x16_avx2 (
                (SINT16 *)buf0_cur,
                (SINT16 *)buf0_cur);
        }

        if ((height == 2*width) || (width == 2*height))
        {
            round_shift_avx2 (
                buf0,
                buf0,
                width);  // rect special code
        }

        xinIdctNonIdentity[lgWidth][t1dVer] (
            (SINT16 *)buf0,
            (SINT16 *)buf0,
            (SINT8)cosBitVer);

        for (int j = 0; j < width; ++j)
        {
            buf0[j] = _mm256_mulhrs_epi16(buf0[j], scale0);
        }

        __m256i *buf1_cur = buf1 + (i << 4);

        if (lr_flip)
        {
            for (int j = 0; j < widthDiv16; ++j)
            {
                __m256i temp[16];
                flip_buf_avx2(buf0 + 16 * j, temp, 16);
                int offset = height * (widthDiv16 - 1 - j);

                transpose_16bit_16x16_avx2 (
                    (SINT16 *)(temp),
                    (SINT16 *)(buf1_cur + offset));
            }
        }
        else
        {
            for (int j = 0; j < widthDiv16; ++j)
            {
                transpose_16bit_16x16_avx2 (
                    (SINT16 *)(buf0 + 16 * j),
                    (SINT16 *)(buf1_cur + height * j));
            }
        }

    }

    const __m256i scale1 = _mm256_set1_epi16(1 << (15 + shift[1]));

    for (int i = 0; i < widthDiv16; i++)
    {
        __m256i *buf1_cur = buf1 + i * height;

        xinIdctNonIdentity[lgHeight][t1dHor] (
            (SINT16 *)buf1_cur,
            (SINT16 *)buf1_cur,
            (SINT8)cosBitHor);

        for (int j = 0; j < height; ++j)
        {
            buf1_cur[j] = _mm256_mulhrs_epi16 (buf1_cur[j], scale1);
        }
    }

    for (int i = 0; i < widthDiv16; i++)
    {
        lowbd_write_buffer_16xn_avx2 (
            buf1 + i * height,
            output + 16 * i,
            outputStride,
            ud_flip,
            height);
    }

}

void Xin265pIDctGt8xGt8_AVX2 (
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

        Xin265pIDctNoIdentityWxH_AVX2 (
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

