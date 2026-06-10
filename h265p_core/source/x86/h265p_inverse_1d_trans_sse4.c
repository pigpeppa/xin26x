/***************************************************************************//**
 *
 * @file          h265p_inverse_1d_trans_sse4.c
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
#include "h265p_trans_context.h"
#include "h265p_inverse_1d_trans.h"
#include "h265p_trans_utility.h"

#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif

#define btf_16_4p_sse2(w0, w1, in0, in1, out0, out1) \
  {                                                  \
    __m128i t0 = _mm_unpacklo_epi16(in0, in1);       \
    __m128i u0 = _mm_madd_epi16(t0, w0);             \
    __m128i v0 = _mm_madd_epi16(t0, w1);             \
                                                     \
    __m128i a0 = _mm_add_epi32(u0, __rounding);      \
    __m128i b0 = _mm_add_epi32(v0, __rounding);      \
                                                     \
    __m128i c0 = _mm_srai_epi32(a0, cos_bit);        \
    __m128i d0 = _mm_srai_epi32(b0, cos_bit);        \
                                                     \
    out0 = _mm_packs_epi32(c0, c0);                  \
    out1 = _mm_packs_epi32(d0, d0);                  \
  }

#define btf_16_adds_subs_out_sse2(out0, out1, in0, in1) \
  do {                                                  \
    const __m128i _in0 = in0;                           \
    const __m128i _in1 = in1;                           \
    out0 = _mm_adds_epi16(_in0, _in1);                  \
    out1 = _mm_subs_epi16(_in0, _in1);                  \
  } while (0)

#define btf_16_adds_subs_sse2(in0, in1) \
    do {                                  \
      const __m128i _in0 = in0;           \
      const __m128i _in1 = in1;           \
      in0 = _mm_adds_epi16(_in0, _in1);   \
      in1 = _mm_subs_epi16(_in0, _in1);   \
    } while (0)

#define btf_16_subs_adds_sse2(in0, in1) \
      do {                                  \
        const __m128i _in0 = in0;           \
        const __m128i _in1 = in1;           \
        in1 = _mm_subs_epi16(_in0, _in1);   \
        in0 = _mm_adds_epi16(_in0, _in1);   \
      } while (0)

#define btf_16_ssse3(w0, w1, in, out0, out1)    \
        do {                                          \
          const __m128i _w0 = _mm_set1_epi16((int16_t)(w0 * 8)); \
          const __m128i _w1 = _mm_set1_epi16((int16_t)(w1 * 8)); \
          const __m128i _in = in;                     \
          out0 = _mm_mulhrs_epi16(_in, _w0);          \
          out1 = _mm_mulhrs_epi16(_in, _w1);          \
        } while (0)


#define pair_set_epi16(a, b) _mm_set1_epi32((SINT32)(((UINT16)(a)) | (((SINT32)(b)) << 16)))

static const SINT32 *XinGetCosPi (
    SINT32 n)
{
    return cosPiData[n - XIN_COS_BIT_MIN];
}

static const SINT32 *XinGetSinPi (
    SINT32 n)
{
    return sinPiData[n - XIN_COS_BIT_MIN];
}

void Xin265pIdct4P4_SSE2 (
    const SINT16 *in,
    SINT16       *out,
    SINT8        cos_bit)
{
    (void)cos_bit;
    const int32_t *cospi = XinGetCosPi(XIN_INV_COS_BIT);
    const __m128i __rounding = _mm_set1_epi32(1 << (XIN_INV_COS_BIT - 1));
    __m128i x[4];
    __m128i *input = (__m128i *)in;
    __m128i *output = (__m128i *)out;

    const __m128i cospi_p32_p32 = pair_set_epi16(cospi[32], cospi[32]);
    const __m128i cospi_p32_m32 = pair_set_epi16(cospi[32], -cospi[32]);
    const __m128i cospi_p48_m16 = pair_set_epi16(cospi[48], -cospi[16]);
    const __m128i cospi_p16_p48 = pair_set_epi16(cospi[16], cospi[48]);

    // stage 1

    x[0] = input[0];
    x[1] = input[2];
    x[2] = input[1];
    x[3] = input[3];

    // stage 2
    btf_16_4p_sse2(cospi_p32_p32, cospi_p32_m32, x[0], x[1], x[0], x[1]);
    btf_16_4p_sse2(cospi_p48_m16, cospi_p16_p48, x[2], x[3], x[2], x[3]);

    // stage 3
    btf_16_adds_subs_out_sse2(output[0], output[3], x[0], x[3]);
    btf_16_adds_subs_out_sse2(output[1], output[2], x[1], x[2]);

}

void Xin265pIadst4P4_SSE2 (
    const SINT16 *in,
    SINT16       *out,
    SINT8        cos_bit)
{
    (void)cos_bit;
    const int32_t *sinpi = XinGetSinPi(XIN_INV_COS_BIT);
    __m128i *input = (__m128i *)in;
    __m128i *output = (__m128i *)out;
    const __m128i sinpi_p01_p04 = pair_set_epi16(sinpi[1], sinpi[4]);
    const __m128i sinpi_p02_m01 = pair_set_epi16(sinpi[2], -sinpi[1]);
    const __m128i sinpi_p03_p02 = pair_set_epi16(sinpi[3], sinpi[2]);
    const __m128i sinpi_p03_m04 = pair_set_epi16(sinpi[3], -sinpi[4]);
    const __m128i sinpi_p03_m03 = pair_set_epi16(sinpi[3], -sinpi[3]);
    const __m128i sinpi_0_p03 = pair_set_epi16(0, sinpi[3]);
    const __m128i sinpi_p04_p02 = pair_set_epi16(sinpi[4], sinpi[2]);
    const __m128i sinpi_m03_m01 = pair_set_epi16(-sinpi[3], -sinpi[1]);
    __m128i x0[4];
    __m128i u[2];
    __m128i x1[8];
    __m128i x2[4];

    x0[0] = input[0];
    x0[1] = input[1];
    x0[2] = input[2];
    x0[3] = input[3];


    u[0] = _mm_unpacklo_epi16(x0[0], x0[2]);
    u[1] = _mm_unpacklo_epi16(x0[1], x0[3]);


    x1[0] = _mm_madd_epi16(u[0], sinpi_p01_p04);  // x0*sin1 + x2*sin4
    x1[1] = _mm_madd_epi16(u[0], sinpi_p02_m01);  // x0*sin2 - x2*sin1
    x1[2] = _mm_madd_epi16(u[1], sinpi_p03_p02);  // x1*sin3 + x3*sin2
    x1[3] = _mm_madd_epi16(u[1], sinpi_p03_m04);  // x1*sin3 - x3*sin4
    x1[4] = _mm_madd_epi16(u[0], sinpi_p03_m03);  // x0*sin3 - x2*sin3
    x1[5] = _mm_madd_epi16(u[1], sinpi_0_p03);    // x2*sin3
    x1[6] = _mm_madd_epi16(u[0], sinpi_p04_p02);  // x0*sin4 + x2*sin2
    x1[7] = _mm_madd_epi16(u[1], sinpi_m03_m01);  // -x1*sin3 - x3*sin1

    x2[0] = _mm_add_epi32(x1[0], x1[2]);  // x0*sin1 + x2*sin4 + x1*sin3 + x3*sin2
    x2[1] = _mm_add_epi32(x1[1], x1[3]);  // x0*sin2 - x2*sin1 + x1*sin3 - x3*sin4
    x2[2] = _mm_add_epi32(x1[4], x1[5]);  // x0*sin3 - x2*sin3 + x3*sin3
    x2[3] = _mm_add_epi32(x1[6], x1[7]);  // x0*sin4 + x2*sin2 - x1*sin3 - x3*sin1

    const __m128i rounding = _mm_set1_epi32(1 << (XIN_INV_COS_BIT - 1));
    for (int i = 0; i < 4; ++i)
    {
        __m128i out0 = _mm_add_epi32(x2[i], rounding);
        out0 = _mm_srai_epi32(out0, XIN_INV_COS_BIT);
        output[i] = _mm_packs_epi32(out0, out0);
    }

}

void Xin265pIidentity4_SSSE3 (
    const SINT16 *in,
    SINT16       *out,
    SINT8        cos_bit)
{
    (void)cos_bit;
    __m128i *output = (__m128i *)out;
    __m128i *input = (__m128i *)in;
    const int16_t scale_fractional = (XIN_FWD_SQRT2 - (1 << XIN_INV_COS_BIT));
    const __m128i scale = _mm_set1_epi16(scale_fractional << (15 - XIN_INV_COS_BIT));
    for (int i = 0; i < 4; ++i)
    {
        __m128i x = _mm_mulhrs_epi16(input[i], scale);
        output[i] = _mm_adds_epi16(x, input[i]);
    }
}

void Xin265pIdct4P8_SSE2 (
    const SINT16 *in,
    SINT16       *out,
    SINT8        cos_bit)
{
    (void)cos_bit;
    const int32_t *cospi = XinGetCosPi(XIN_INV_COS_BIT);
    const __m128i __rounding = _mm_set1_epi32(1 << (XIN_INV_COS_BIT - 1));

    __m128i *input = (__m128i *)in;
    __m128i *output = (__m128i *)out;

    const __m128i cospi_p32_p32 = pair_set_epi16(cospi[32], cospi[32]);
    const __m128i cospi_p32_m32 = pair_set_epi16(cospi[32], -cospi[32]);
    const __m128i cospi_p48_m16 = pair_set_epi16(cospi[48], -cospi[16]);
    const __m128i cospi_p16_p48 = pair_set_epi16(cospi[16], cospi[48]);

    // stage 1
    __m128i x[4];
    x[0] = input[0];
    x[1] = input[2];
    x[2] = input[1];
    x[3] = input[3];

    // stage 2
    btf_16_sse2(cospi_p32_p32, cospi_p32_m32, x[0], x[1], x[0], x[1]);
    btf_16_sse2(cospi_p48_m16, cospi_p16_p48, x[2], x[3], x[2], x[3]);

    // stage 3
    btf_16_adds_subs_out_sse2(output[0], output[3], x[0], x[3]);
    btf_16_adds_subs_out_sse2(output[1], output[2], x[1], x[2]);

}

void Xin265pIadst4P8_SSE2 (
    const SINT16 *in,
    SINT16       *out,
    SINT8        cos_bit)
{
    (void)cos_bit;
    const int32_t *sinpi = XinGetSinPi(XIN_INV_COS_BIT);

    __m128i *input = (__m128i *)in;
    __m128i *output = (__m128i *)out;
    const __m128i sinpi_p01_p04 = pair_set_epi16(sinpi[1], sinpi[4]);
    const __m128i sinpi_p02_m01 = pair_set_epi16(sinpi[2], -sinpi[1]);
    const __m128i sinpi_p03_p02 = pair_set_epi16(sinpi[3], sinpi[2]);
    const __m128i sinpi_p03_m04 = pair_set_epi16(sinpi[3], -sinpi[4]);
    const __m128i sinpi_p03_m03 = pair_set_epi16(sinpi[3], -sinpi[3]);
    const __m128i sinpi_0_p03 = pair_set_epi16(0, sinpi[3]);
    const __m128i sinpi_p04_p02 = pair_set_epi16(sinpi[4], sinpi[2]);
    const __m128i sinpi_m03_m01 = pair_set_epi16(-sinpi[3], -sinpi[1]);
    __m128i x0[4];
    x0[0] = input[0];
    x0[1] = input[1];
    x0[2] = input[2];
    x0[3] = input[3];

    __m128i u[4];
    u[0] = _mm_unpacklo_epi16(x0[0], x0[2]);
    u[1] = _mm_unpackhi_epi16(x0[0], x0[2]);
    u[2] = _mm_unpacklo_epi16(x0[1], x0[3]);
    u[3] = _mm_unpackhi_epi16(x0[1], x0[3]);

    __m128i x1[16];
    x1[0] = _mm_madd_epi16(u[0], sinpi_p01_p04);  // x0*sin1 + x2*sin4
    x1[1] = _mm_madd_epi16(u[1], sinpi_p01_p04);
    x1[2] = _mm_madd_epi16(u[0], sinpi_p02_m01);  // x0*sin2 - x2*sin1
    x1[3] = _mm_madd_epi16(u[1], sinpi_p02_m01);
    x1[4] = _mm_madd_epi16(u[2], sinpi_p03_p02);  // x1*sin3 + x3*sin2
    x1[5] = _mm_madd_epi16(u[3], sinpi_p03_p02);
    x1[6] = _mm_madd_epi16(u[2], sinpi_p03_m04);  // x1*sin3 - x3*sin4
    x1[7] = _mm_madd_epi16(u[3], sinpi_p03_m04);
    x1[8] = _mm_madd_epi16(u[0], sinpi_p03_m03);  // x0*sin3 - x2*sin3
    x1[9] = _mm_madd_epi16(u[1], sinpi_p03_m03);
    x1[10] = _mm_madd_epi16(u[2], sinpi_0_p03);  // x2*sin3
    x1[11] = _mm_madd_epi16(u[3], sinpi_0_p03);
    x1[12] = _mm_madd_epi16(u[0], sinpi_p04_p02);  // x0*sin4 + x2*sin2
    x1[13] = _mm_madd_epi16(u[1], sinpi_p04_p02);
    x1[14] = _mm_madd_epi16(u[2], sinpi_m03_m01);  // -x1*sin3 - x3*sin1
    x1[15] = _mm_madd_epi16(u[3], sinpi_m03_m01);

    __m128i x2[8];
    x2[0] = _mm_add_epi32(x1[0], x1[4]);  // x0*sin1 +x2*sin4 +x1*sin3 +x3*sin2
    x2[1] = _mm_add_epi32(x1[1], x1[5]);
    x2[2] = _mm_add_epi32(x1[2], x1[6]);  // x0*sin2 -x2*sin1 +x1*sin3 -x3*sin4
    x2[3] = _mm_add_epi32(x1[3], x1[7]);
    x2[4] = _mm_add_epi32(x1[8], x1[10]);  // x0*sin3 -x2*sin3 +x3*sin3
    x2[5] = _mm_add_epi32(x1[9], x1[11]);
    x2[6] = _mm_add_epi32(x1[12], x1[14]);  // x0*sin1 +x2*sin4 +x0*sin2 -x2*sin1
    x2[7] = _mm_add_epi32(x1[13], x1[15]);

    const __m128i rounding = _mm_set1_epi32(1 << (XIN_INV_COS_BIT - 1));
    for (int i = 0; i < 4; ++i)
    {
        __m128i out0 = _mm_add_epi32(x2[2 * i], rounding);
        __m128i out1 = _mm_add_epi32(x2[2 * i + 1], rounding);
        out0 = _mm_srai_epi32(out0, XIN_INV_COS_BIT);
        out1 = _mm_srai_epi32(out1, XIN_INV_COS_BIT);
        output[i] = _mm_packs_epi32(out0, out1);
    }

}

void Xin265pIdct8P4_SSE2 (
    const SINT16 *in,
    SINT16       *out,
    SINT8        cos_bit)
{
    (void)cos_bit;
    const int32_t *cospi = XinGetCosPi(XIN_INV_COS_BIT);
    const __m128i __rounding = _mm_set1_epi32(1 << (XIN_INV_COS_BIT - 1));

    __m128i *input = (__m128i *)in;
    __m128i *output = (__m128i *)out;

    const __m128i cospi_p56_m08 = pair_set_epi16(cospi[56], -cospi[8]);
    const __m128i cospi_p08_p56 = pair_set_epi16(cospi[8], cospi[56]);
    const __m128i cospi_p24_m40 = pair_set_epi16(cospi[24], -cospi[40]);
    const __m128i cospi_p40_p24 = pair_set_epi16(cospi[40], cospi[24]);
    const __m128i cospi_p32_p32 = pair_set_epi16(cospi[32], cospi[32]);
    const __m128i cospi_p32_m32 = pair_set_epi16(cospi[32], -cospi[32]);
    const __m128i cospi_p48_m16 = pair_set_epi16(cospi[48], -cospi[16]);
    const __m128i cospi_p16_p48 = pair_set_epi16(cospi[16], cospi[48]);
    const __m128i cospi_m32_p32 = pair_set_epi16(-cospi[32], cospi[32]);

    // stage 1
    __m128i x[8];
    x[0] = input[0];
    x[1] = input[4];
    x[2] = input[2];
    x[3] = input[6];
    x[4] = input[1];
    x[5] = input[5];
    x[6] = input[3];
    x[7] = input[7];

    // stage 2
    btf_16_4p_sse2 (cospi_p56_m08, cospi_p08_p56, x[4], x[7], x[4], x[7]);
    btf_16_4p_sse2 (cospi_p24_m40, cospi_p40_p24, x[5], x[6], x[5], x[6]);

    // stage 3
    btf_16_4p_sse2 (cospi_p32_p32, cospi_p32_m32, x[0], x[1], x[0], x[1]);
    btf_16_4p_sse2 (cospi_p48_m16, cospi_p16_p48, x[2], x[3], x[2], x[3]);
    btf_16_adds_subs_sse2(x[4], x[5]);
    btf_16_subs_adds_sse2(x[7], x[6]);

    // stage 4
    btf_16_adds_subs_sse2(x[0], x[3]);
    btf_16_adds_subs_sse2(x[1], x[2]);
    btf_16_4p_sse2 (cospi_m32_p32, cospi_p32_p32, x[5], x[6], x[5], x[6]);

    // stage 5
    btf_16_adds_subs_out_sse2(output[0], output[7], x[0], x[7]);
    btf_16_adds_subs_out_sse2(output[1], output[6], x[1], x[6]);
    btf_16_adds_subs_out_sse2(output[2], output[5], x[2], x[5]);
    btf_16_adds_subs_out_sse2(output[3], output[4], x[3], x[4]);

}

void Xin265pIadst8P4_SSE2 (
    const SINT16 *in,
    SINT16       *out,
    SINT8        cos_bit)
{
    (void)cos_bit;
    const int32_t *cospi = XinGetCosPi(XIN_INV_COS_BIT);
    const __m128i __zero = _mm_setzero_si128();
    const __m128i __rounding = _mm_set1_epi32(1 << (XIN_INV_COS_BIT - 1));

    __m128i *input = (__m128i *)in;
    __m128i *output = (__m128i *)out;

    const __m128i cospi_p04_p60 = pair_set_epi16(cospi[4], cospi[60]);
    const __m128i cospi_p60_m04 = pair_set_epi16(cospi[60], -cospi[4]);
    const __m128i cospi_p20_p44 = pair_set_epi16(cospi[20], cospi[44]);
    const __m128i cospi_p44_m20 = pair_set_epi16(cospi[44], -cospi[20]);
    const __m128i cospi_p36_p28 = pair_set_epi16(cospi[36], cospi[28]);
    const __m128i cospi_p28_m36 = pair_set_epi16(cospi[28], -cospi[36]);
    const __m128i cospi_p52_p12 = pair_set_epi16(cospi[52], cospi[12]);
    const __m128i cospi_p12_m52 = pair_set_epi16(cospi[12], -cospi[52]);
    const __m128i cospi_p16_p48 = pair_set_epi16(cospi[16], cospi[48]);
    const __m128i cospi_p48_m16 = pair_set_epi16(cospi[48], -cospi[16]);
    const __m128i cospi_m48_p16 = pair_set_epi16(-cospi[48], cospi[16]);
    const __m128i cospi_p32_p32 = pair_set_epi16(cospi[32], cospi[32]);
    const __m128i cospi_p32_m32 = pair_set_epi16(cospi[32], -cospi[32]);

    // stage 1
    __m128i x[8];
    x[0] = input[7];
    x[1] = input[0];
    x[2] = input[5];
    x[3] = input[2];
    x[4] = input[3];
    x[5] = input[4];
    x[6] = input[1];
    x[7] = input[6];

    // stage 2
    btf_16_4p_sse2(cospi_p04_p60, cospi_p60_m04, x[0], x[1], x[0], x[1]);
    btf_16_4p_sse2(cospi_p20_p44, cospi_p44_m20, x[2], x[3], x[2], x[3]);
    btf_16_4p_sse2(cospi_p36_p28, cospi_p28_m36, x[4], x[5], x[4], x[5]);
    btf_16_4p_sse2(cospi_p52_p12, cospi_p12_m52, x[6], x[7], x[6], x[7]);

    // stage 3
    btf_16_adds_subs_sse2(x[0], x[4]);
    btf_16_adds_subs_sse2(x[1], x[5]);
    btf_16_adds_subs_sse2(x[2], x[6]);
    btf_16_adds_subs_sse2(x[3], x[7]);

    // stage 4
    btf_16_4p_sse2(cospi_p16_p48, cospi_p48_m16, x[4], x[5], x[4], x[5]);
    btf_16_4p_sse2(cospi_m48_p16, cospi_p16_p48, x[6], x[7], x[6], x[7]);

    // stage 5
    btf_16_adds_subs_sse2(x[0], x[2]);
    btf_16_adds_subs_sse2(x[1], x[3]);
    btf_16_adds_subs_sse2(x[4], x[6]);
    btf_16_adds_subs_sse2(x[5], x[7]);

    // stage 6
    btf_16_4p_sse2(cospi_p32_p32, cospi_p32_m32, x[2], x[3], x[2], x[3]);
    btf_16_4p_sse2(cospi_p32_p32, cospi_p32_m32, x[6], x[7], x[6], x[7]);

    // stage 7
    output[0] = x[0];
    output[1] = _mm_subs_epi16(__zero, x[4]);
    output[2] = x[6];
    output[3] = _mm_subs_epi16(__zero, x[2]);
    output[4] = x[3];
    output[5] = _mm_subs_epi16(__zero, x[7]);
    output[6] = x[5];
    output[7] = _mm_subs_epi16(__zero, x[1]);

}

void Xin265pIidentity8_SSSE3 (
    const SINT16 *in,
    SINT16       *out,
    SINT8        cos_bit)
{
    (void)cos_bit;
    __m128i *input;
    __m128i *output;

    input = (__m128i *)in;
    output = (__m128i *)out;

    for (int i = 0; i < 8; ++i)
    {
        output[i] = _mm_adds_epi16(input[i], input[i]);
    }
}

void Xin265pIdct8P8_SSE2 (
    const SINT16 *in,
    SINT16       *out,
    SINT8        cos_bit)
{
    (void)cos_bit;
    const int32_t *cospi = XinGetCosPi(XIN_INV_COS_BIT);
    const __m128i __rounding = _mm_set1_epi32(1 << (XIN_INV_COS_BIT - 1));

    __m128i *input = (__m128i *)in;
    __m128i *output = (__m128i *)out;

    const __m128i cospi_p56_m08 = pair_set_epi16(cospi[56], -cospi[8]);
    const __m128i cospi_p08_p56 = pair_set_epi16(cospi[8], cospi[56]);
    const __m128i cospi_p24_m40 = pair_set_epi16(cospi[24], -cospi[40]);
    const __m128i cospi_p40_p24 = pair_set_epi16(cospi[40], cospi[24]);
    const __m128i cospi_p32_p32 = pair_set_epi16(cospi[32], cospi[32]);
    const __m128i cospi_p32_m32 = pair_set_epi16(cospi[32], -cospi[32]);
    const __m128i cospi_p48_m16 = pair_set_epi16(cospi[48], -cospi[16]);
    const __m128i cospi_p16_p48 = pair_set_epi16(cospi[16], cospi[48]);
    const __m128i cospi_m32_p32 = pair_set_epi16(-cospi[32], cospi[32]);

    // stage 1
    __m128i x[8];
    x[0] = input[0];
    x[1] = input[4];
    x[2] = input[2];
    x[3] = input[6];
    x[4] = input[1];
    x[5] = input[5];
    x[6] = input[3];
    x[7] = input[7];

    // stage 2
    btf_16_sse2(cospi_p56_m08, cospi_p08_p56, x[4], x[7], x[4], x[7]);
    btf_16_sse2(cospi_p24_m40, cospi_p40_p24, x[5], x[6], x[5], x[6]);

    // stage 3
    btf_16_sse2(cospi_p32_p32, cospi_p32_m32, x[0], x[1], x[0], x[1]);
    btf_16_sse2(cospi_p48_m16, cospi_p16_p48, x[2], x[3], x[2], x[3]);
    btf_16_adds_subs_sse2(x[4], x[5]);
    btf_16_subs_adds_sse2(x[7], x[6]);

    // stage 4
    btf_16_adds_subs_sse2(x[0], x[3]);
    btf_16_adds_subs_sse2(x[1], x[2]);
    btf_16_sse2(cospi_m32_p32, cospi_p32_p32, x[5], x[6], x[5], x[6]);

    // stage 5
    btf_16_adds_subs_out_sse2(output[0], output[7], x[0], x[7]);
    btf_16_adds_subs_out_sse2(output[1], output[6], x[1], x[6]);
    btf_16_adds_subs_out_sse2(output[2], output[5], x[2], x[5]);
    btf_16_adds_subs_out_sse2(output[3], output[4], x[3], x[4]);

}

void Xin265pIadst8P8_SSE2 (
    const SINT16 *in,
    SINT16       *out,
    SINT8        cos_bit)
{
    (void)cos_bit;
    const int32_t *cospi = XinGetCosPi(XIN_INV_COS_BIT);
    const __m128i __zero = _mm_setzero_si128();
    const __m128i __rounding = _mm_set1_epi32(1 << (XIN_INV_COS_BIT - 1));

    __m128i *input = (__m128i *)in;
    __m128i *output = (__m128i *)out;

    const __m128i cospi_p04_p60 = pair_set_epi16(cospi[4], cospi[60]);
    const __m128i cospi_p60_m04 = pair_set_epi16(cospi[60], -cospi[4]);
    const __m128i cospi_p20_p44 = pair_set_epi16(cospi[20], cospi[44]);
    const __m128i cospi_p44_m20 = pair_set_epi16(cospi[44], -cospi[20]);
    const __m128i cospi_p36_p28 = pair_set_epi16(cospi[36], cospi[28]);
    const __m128i cospi_p28_m36 = pair_set_epi16(cospi[28], -cospi[36]);
    const __m128i cospi_p52_p12 = pair_set_epi16(cospi[52], cospi[12]);
    const __m128i cospi_p12_m52 = pair_set_epi16(cospi[12], -cospi[52]);
    const __m128i cospi_p16_p48 = pair_set_epi16(cospi[16], cospi[48]);
    const __m128i cospi_p48_m16 = pair_set_epi16(cospi[48], -cospi[16]);
    const __m128i cospi_m48_p16 = pair_set_epi16(-cospi[48], cospi[16]);
    const __m128i cospi_p32_p32 = pair_set_epi16(cospi[32], cospi[32]);
    const __m128i cospi_p32_m32 = pair_set_epi16(cospi[32], -cospi[32]);

    // stage 1
    __m128i x[8];
    x[0] = input[7];
    x[1] = input[0];
    x[2] = input[5];
    x[3] = input[2];
    x[4] = input[3];
    x[5] = input[4];
    x[6] = input[1];
    x[7] = input[6];

    // stage 2
    btf_16_sse2(cospi_p04_p60, cospi_p60_m04, x[0], x[1], x[0], x[1]);
    btf_16_sse2(cospi_p20_p44, cospi_p44_m20, x[2], x[3], x[2], x[3]);
    btf_16_sse2(cospi_p36_p28, cospi_p28_m36, x[4], x[5], x[4], x[5]);
    btf_16_sse2(cospi_p52_p12, cospi_p12_m52, x[6], x[7], x[6], x[7]);

    // stage 3
    btf_16_adds_subs_sse2(x[0], x[4]);
    btf_16_adds_subs_sse2(x[1], x[5]);
    btf_16_adds_subs_sse2(x[2], x[6]);
    btf_16_adds_subs_sse2(x[3], x[7]);

    // stage 4
    btf_16_sse2(cospi_p16_p48, cospi_p48_m16, x[4], x[5], x[4], x[5]);
    btf_16_sse2(cospi_m48_p16, cospi_p16_p48, x[6], x[7], x[6], x[7]);

    // stage 5
    btf_16_adds_subs_sse2(x[0], x[2]);
    btf_16_adds_subs_sse2(x[1], x[3]);
    btf_16_adds_subs_sse2(x[4], x[6]);
    btf_16_adds_subs_sse2(x[5], x[7]);

    // stage 6
    btf_16_sse2(cospi_p32_p32, cospi_p32_m32, x[2], x[3], x[2], x[3]);
    btf_16_sse2(cospi_p32_p32, cospi_p32_m32, x[6], x[7], x[6], x[7]);

    // stage 7
    output[0] = x[0];
    output[1] = _mm_subs_epi16(__zero, x[4]);
    output[2] = x[6];
    output[3] = _mm_subs_epi16(__zero, x[2]);
    output[4] = x[3];
    output[5] = _mm_subs_epi16(__zero, x[7]);
    output[6] = x[5];
    output[7] = _mm_subs_epi16(__zero, x[1]);

}

static inline void idct16_stage5_sse2(__m128i *x, const int32_t *cospi,
                                      const __m128i __rounding,
                                      int8_t cos_bit)
{
    const __m128i cospi_m32_p32 = pair_set_epi16(-cospi[32], cospi[32]);
    const __m128i cospi_p32_p32 = pair_set_epi16(cospi[32], cospi[32]);
    btf_16_adds_subs_sse2(x[0], x[3]);
    btf_16_adds_subs_sse2(x[1], x[2]);
    btf_16_sse2(cospi_m32_p32, cospi_p32_p32, x[5], x[6], x[5], x[6]);
    btf_16_adds_subs_sse2(x[8], x[11]);
    btf_16_adds_subs_sse2(x[9], x[10]);
    btf_16_subs_adds_sse2(x[15], x[12]);
    btf_16_subs_adds_sse2(x[14], x[13]);
}

static inline void idct16_stage6_sse2(__m128i *x, const int32_t *cospi,
                                      const __m128i __rounding,
                                      int8_t cos_bit)
{
    const __m128i cospi_m32_p32 = pair_set_epi16(-cospi[32], cospi[32]);
    const __m128i cospi_p32_p32 = pair_set_epi16(cospi[32], cospi[32]);
    btf_16_adds_subs_sse2(x[0], x[7]);
    btf_16_adds_subs_sse2(x[1], x[6]);
    btf_16_adds_subs_sse2(x[2], x[5]);
    btf_16_adds_subs_sse2(x[3], x[4]);
    btf_16_sse2(cospi_m32_p32, cospi_p32_p32, x[10], x[13], x[10], x[13]);
    btf_16_sse2(cospi_m32_p32, cospi_p32_p32, x[11], x[12], x[11], x[12]);
}

static inline void idct16_stage7_sse2(__m128i *output, __m128i *x)
{
    btf_16_adds_subs_out_sse2(output[0], output[15], x[0], x[15]);
    btf_16_adds_subs_out_sse2(output[1], output[14], x[1], x[14]);
    btf_16_adds_subs_out_sse2(output[2], output[13], x[2], x[13]);
    btf_16_adds_subs_out_sse2(output[3], output[12], x[3], x[12]);
    btf_16_adds_subs_out_sse2(output[4], output[11], x[4], x[11]);
    btf_16_adds_subs_out_sse2(output[5], output[10], x[5], x[10]);
    btf_16_adds_subs_out_sse2(output[6], output[9], x[6], x[9]);
    btf_16_adds_subs_out_sse2(output[7], output[8], x[7], x[8]);
}

void Xin265pIdct16_SSE2 (
    const SINT16 *in,
    SINT16       *out,
    SINT8        cos_bit)
{
    (void)cos_bit;
    const int32_t *cospi = XinGetCosPi(XIN_INV_COS_BIT);
    const __m128i __rounding = _mm_set1_epi32(1 << (XIN_INV_COS_BIT - 1));

    __m128i *input = (__m128i *)in;
    __m128i *output = (__m128i *)out;

    const __m128i cospi_p60_m04 = pair_set_epi16(cospi[60], -cospi[4]);
    const __m128i cospi_p04_p60 = pair_set_epi16(cospi[4], cospi[60]);
    const __m128i cospi_p28_m36 = pair_set_epi16(cospi[28], -cospi[36]);
    const __m128i cospi_p36_p28 = pair_set_epi16(cospi[36], cospi[28]);
    const __m128i cospi_p44_m20 = pair_set_epi16(cospi[44], -cospi[20]);
    const __m128i cospi_p20_p44 = pair_set_epi16(cospi[20], cospi[44]);
    const __m128i cospi_p12_m52 = pair_set_epi16(cospi[12], -cospi[52]);
    const __m128i cospi_p52_p12 = pair_set_epi16(cospi[52], cospi[12]);
    const __m128i cospi_p56_m08 = pair_set_epi16(cospi[56], -cospi[8]);
    const __m128i cospi_p08_p56 = pair_set_epi16(cospi[8], cospi[56]);
    const __m128i cospi_p24_m40 = pair_set_epi16(cospi[24], -cospi[40]);
    const __m128i cospi_p40_p24 = pair_set_epi16(cospi[40], cospi[24]);
    const __m128i cospi_p32_p32 = pair_set_epi16(cospi[32], cospi[32]);
    const __m128i cospi_p32_m32 = pair_set_epi16(cospi[32], -cospi[32]);
    const __m128i cospi_p48_m16 = pair_set_epi16(cospi[48], -cospi[16]);
    const __m128i cospi_p16_p48 = pair_set_epi16(cospi[16], cospi[48]);
    const __m128i cospi_m16_p48 = pair_set_epi16(-cospi[16], cospi[48]);
    const __m128i cospi_p48_p16 = pair_set_epi16(cospi[48], cospi[16]);
    const __m128i cospi_m48_m16 = pair_set_epi16(-cospi[48], -cospi[16]);

    // stage 1
    __m128i x[16];
    x[0] = input[0];
    x[1] = input[8];
    x[2] = input[4];
    x[3] = input[12];
    x[4] = input[2];
    x[5] = input[10];
    x[6] = input[6];
    x[7] = input[14];
    x[8] = input[1];
    x[9] = input[9];
    x[10] = input[5];
    x[11] = input[13];
    x[12] = input[3];
    x[13] = input[11];
    x[14] = input[7];
    x[15] = input[15];

    // stage 2
    btf_16_sse2(cospi_p60_m04, cospi_p04_p60, x[8], x[15], x[8], x[15]);
    btf_16_sse2(cospi_p28_m36, cospi_p36_p28, x[9], x[14], x[9], x[14]);
    btf_16_sse2(cospi_p44_m20, cospi_p20_p44, x[10], x[13], x[10], x[13]);
    btf_16_sse2(cospi_p12_m52, cospi_p52_p12, x[11], x[12], x[11], x[12]);

    // stage 3
    btf_16_sse2(cospi_p56_m08, cospi_p08_p56, x[4], x[7], x[4], x[7]);
    btf_16_sse2(cospi_p24_m40, cospi_p40_p24, x[5], x[6], x[5], x[6]);
    btf_16_adds_subs_sse2(x[8], x[9]);
    btf_16_subs_adds_sse2(x[11], x[10]);
    btf_16_adds_subs_sse2(x[12], x[13]);
    btf_16_subs_adds_sse2(x[15], x[14]);

    // stage 4
    btf_16_sse2(cospi_p32_p32, cospi_p32_m32, x[0], x[1], x[0], x[1]);
    btf_16_sse2(cospi_p48_m16, cospi_p16_p48, x[2], x[3], x[2], x[3]);
    btf_16_adds_subs_sse2(x[4], x[5]);
    btf_16_subs_adds_sse2(x[7], x[6]);
    btf_16_sse2(cospi_m16_p48, cospi_p48_p16, x[9], x[14], x[9], x[14]);
    btf_16_sse2(cospi_m48_m16, cospi_m16_p48, x[10], x[13], x[10], x[13]);

    // stage 5~7
    idct16_stage5_sse2(x, cospi, __rounding, cos_bit);
    idct16_stage6_sse2(x, cospi, __rounding, cos_bit);
    idct16_stage7_sse2(output, x);

}


static inline void iadst16_stage3_ssse3(__m128i *x)
{
    btf_16_adds_subs_sse2(x[0], x[8]);
    btf_16_adds_subs_sse2(x[1], x[9]);
    btf_16_adds_subs_sse2(x[2], x[10]);
    btf_16_adds_subs_sse2(x[3], x[11]);
    btf_16_adds_subs_sse2(x[4], x[12]);
    btf_16_adds_subs_sse2(x[5], x[13]);
    btf_16_adds_subs_sse2(x[6], x[14]);
    btf_16_adds_subs_sse2(x[7], x[15]);
}

static inline void iadst16_stage4_ssse3(__m128i *x, const int32_t *cospi,
                                        const __m128i __rounding,
                                        int8_t cos_bit)
{
    const __m128i cospi_p08_p56 = pair_set_epi16(cospi[8], cospi[56]);
    const __m128i cospi_p56_m08 = pair_set_epi16(cospi[56], -cospi[8]);
    const __m128i cospi_p40_p24 = pair_set_epi16(cospi[40], cospi[24]);
    const __m128i cospi_p24_m40 = pair_set_epi16(cospi[24], -cospi[40]);
    const __m128i cospi_m56_p08 = pair_set_epi16(-cospi[56], cospi[8]);
    const __m128i cospi_m24_p40 = pair_set_epi16(-cospi[24], cospi[40]);
    btf_16_sse2(cospi_p08_p56, cospi_p56_m08, x[8], x[9], x[8], x[9]);
    btf_16_sse2(cospi_p40_p24, cospi_p24_m40, x[10], x[11], x[10], x[11]);
    btf_16_sse2(cospi_m56_p08, cospi_p08_p56, x[12], x[13], x[12], x[13]);
    btf_16_sse2(cospi_m24_p40, cospi_p40_p24, x[14], x[15], x[14], x[15]);
}

static inline void iadst16_stage5_ssse3(__m128i *x)
{
    btf_16_adds_subs_sse2(x[0], x[4]);
    btf_16_adds_subs_sse2(x[1], x[5]);
    btf_16_adds_subs_sse2(x[2], x[6]);
    btf_16_adds_subs_sse2(x[3], x[7]);
    btf_16_adds_subs_sse2(x[8], x[12]);
    btf_16_adds_subs_sse2(x[9], x[13]);
    btf_16_adds_subs_sse2(x[10], x[14]);
    btf_16_adds_subs_sse2(x[11], x[15]);
}

static inline void iadst16_stage6_ssse3(__m128i *x, const int32_t *cospi,
                                        const __m128i __rounding,
                                        int8_t cos_bit)
{
    const __m128i cospi_p16_p48 = pair_set_epi16(cospi[16], cospi[48]);
    const __m128i cospi_p48_m16 = pair_set_epi16(cospi[48], -cospi[16]);
    const __m128i cospi_m48_p16 = pair_set_epi16(-cospi[48], cospi[16]);
    btf_16_sse2(cospi_p16_p48, cospi_p48_m16, x[4], x[5], x[4], x[5]);
    btf_16_sse2(cospi_m48_p16, cospi_p16_p48, x[6], x[7], x[6], x[7]);
    btf_16_sse2(cospi_p16_p48, cospi_p48_m16, x[12], x[13], x[12], x[13]);
    btf_16_sse2(cospi_m48_p16, cospi_p16_p48, x[14], x[15], x[14], x[15]);
}

static inline void iadst16_stage7_ssse3(__m128i *x)
{
    btf_16_adds_subs_sse2(x[0], x[2]);
    btf_16_adds_subs_sse2(x[1], x[3]);
    btf_16_adds_subs_sse2(x[4], x[6]);
    btf_16_adds_subs_sse2(x[5], x[7]);
    btf_16_adds_subs_sse2(x[8], x[10]);
    btf_16_adds_subs_sse2(x[9], x[11]);
    btf_16_adds_subs_sse2(x[12], x[14]);
    btf_16_adds_subs_sse2(x[13], x[15]);
}

static inline void iadst16_stage8_ssse3(__m128i *x, const int32_t *cospi,
                                        const __m128i __rounding,
                                        int8_t cos_bit)
{
    const __m128i cospi_p32_p32 = pair_set_epi16(cospi[32], cospi[32]);
    const __m128i cospi_p32_m32 = pair_set_epi16(cospi[32], -cospi[32]);
    btf_16_sse2(cospi_p32_p32, cospi_p32_m32, x[2], x[3], x[2], x[3]);
    btf_16_sse2(cospi_p32_p32, cospi_p32_m32, x[6], x[7], x[6], x[7]);
    btf_16_sse2(cospi_p32_p32, cospi_p32_m32, x[10], x[11], x[10], x[11]);
    btf_16_sse2(cospi_p32_p32, cospi_p32_m32, x[14], x[15], x[14], x[15]);
}

static inline void iadst16_stage9_ssse3(__m128i *output, __m128i *x)
{
    const __m128i __zero = _mm_setzero_si128();
    output[0] = x[0];
    output[1] = _mm_subs_epi16(__zero, x[8]);
    output[2] = x[12];
    output[3] = _mm_subs_epi16(__zero, x[4]);
    output[4] = x[6];
    output[5] = _mm_subs_epi16(__zero, x[14]);
    output[6] = x[10];
    output[7] = _mm_subs_epi16(__zero, x[2]);
    output[8] = x[3];
    output[9] = _mm_subs_epi16(__zero, x[11]);
    output[10] = x[15];
    output[11] = _mm_subs_epi16(__zero, x[7]);
    output[12] = x[5];
    output[13] = _mm_subs_epi16(__zero, x[13]);
    output[14] = x[9];
    output[15] = _mm_subs_epi16(__zero, x[1]);
}

void Xin265pIadst16_SSSE3 (
    const SINT16 *in,
    SINT16       *out,
    SINT8        cos_bit)
{
    (void)cos_bit;
    const int32_t *cospi = XinGetCosPi(XIN_INV_COS_BIT);
    const __m128i __rounding = _mm_set1_epi32(1 << (XIN_INV_COS_BIT - 1));

    __m128i *input = (__m128i *)in;
    __m128i *output = (__m128i *)out;
    
    const __m128i cospi_p02_p62 = pair_set_epi16(cospi[2], cospi[62]);
    const __m128i cospi_p62_m02 = pair_set_epi16(cospi[62], -cospi[2]);
    const __m128i cospi_p10_p54 = pair_set_epi16(cospi[10], cospi[54]);
    const __m128i cospi_p54_m10 = pair_set_epi16(cospi[54], -cospi[10]);
    const __m128i cospi_p18_p46 = pair_set_epi16(cospi[18], cospi[46]);
    const __m128i cospi_p46_m18 = pair_set_epi16(cospi[46], -cospi[18]);
    const __m128i cospi_p26_p38 = pair_set_epi16(cospi[26], cospi[38]);
    const __m128i cospi_p38_m26 = pair_set_epi16(cospi[38], -cospi[26]);
    const __m128i cospi_p34_p30 = pair_set_epi16(cospi[34], cospi[30]);
    const __m128i cospi_p30_m34 = pair_set_epi16(cospi[30], -cospi[34]);
    const __m128i cospi_p42_p22 = pair_set_epi16(cospi[42], cospi[22]);
    const __m128i cospi_p22_m42 = pair_set_epi16(cospi[22], -cospi[42]);
    const __m128i cospi_p50_p14 = pair_set_epi16(cospi[50], cospi[14]);
    const __m128i cospi_p14_m50 = pair_set_epi16(cospi[14], -cospi[50]);
    const __m128i cospi_p58_p06 = pair_set_epi16(cospi[58], cospi[6]);
    const __m128i cospi_p06_m58 = pair_set_epi16(cospi[6], -cospi[58]);

    // stage 1
    __m128i x[16];
    x[0] = input[15];
    x[1] = input[0];
    x[2] = input[13];
    x[3] = input[2];
    x[4] = input[11];
    x[5] = input[4];
    x[6] = input[9];
    x[7] = input[6];
    x[8] = input[7];
    x[9] = input[8];
    x[10] = input[5];
    x[11] = input[10];
    x[12] = input[3];
    x[13] = input[12];
    x[14] = input[1];
    x[15] = input[14];

    // stage 2
    btf_16_sse2(cospi_p02_p62, cospi_p62_m02, x[0], x[1], x[0], x[1]);
    btf_16_sse2(cospi_p10_p54, cospi_p54_m10, x[2], x[3], x[2], x[3]);
    btf_16_sse2(cospi_p18_p46, cospi_p46_m18, x[4], x[5], x[4], x[5]);
    btf_16_sse2(cospi_p26_p38, cospi_p38_m26, x[6], x[7], x[6], x[7]);
    btf_16_sse2(cospi_p34_p30, cospi_p30_m34, x[8], x[9], x[8], x[9]);
    btf_16_sse2(cospi_p42_p22, cospi_p22_m42, x[10], x[11], x[10], x[11]);
    btf_16_sse2(cospi_p50_p14, cospi_p14_m50, x[12], x[13], x[12], x[13]);
    btf_16_sse2(cospi_p58_p06, cospi_p06_m58, x[14], x[15], x[14], x[15]);

    // stage 3~9
    iadst16_stage3_ssse3(x);
    iadst16_stage4_ssse3(x, cospi, __rounding, cos_bit);
    iadst16_stage5_ssse3(x);
    iadst16_stage6_ssse3(x, cospi, __rounding, cos_bit);
    iadst16_stage7_ssse3(x);
    iadst16_stage8_ssse3(x, cospi, __rounding, cos_bit);
    iadst16_stage9_ssse3(output, x);
}



