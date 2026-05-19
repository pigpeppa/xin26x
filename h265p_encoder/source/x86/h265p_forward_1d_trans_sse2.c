/***************************************************************************//**
*
* @file          h265p_forward_1d_trans_sse2.c
* @brief         av1 forward transform subroutines.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#include "emmintrin.h"
#include "xin_typedef.h"
#include "h265p_trans_context.h"
#include "h265p_forward_1d_trans.h"
#include "h265p_trans_utility.h"

#define pair_set_epi16(a, b) _mm_set1_epi32((SINT32)(((UINT16)(a)) | (((SINT32)(b)) << 16)))

static inline void btf_16_w4_sse2 (
    __m128i *w0,
    __m128i *w1,
    __m128i __rounding,
    SINT8   cosBit,
    __m128i *in0,
    __m128i *in1,
    __m128i *out0,
    __m128i *out1)
{
    __m128i t0;
    __m128i u0;
    __m128i v0;
    __m128i a0;
    __m128i b0;
    __m128i c0;
    __m128i d0;

    t0 = _mm_unpacklo_epi16 (*in0, *in1);
    u0 = _mm_madd_epi16 (t0, *w0);
    v0 = _mm_madd_epi16 (t0, *w1);
    a0 = _mm_add_epi32 (u0, __rounding);
    b0 = _mm_add_epi32 (v0, __rounding);
    c0 = _mm_srai_epi32 (a0, cosBit);
    d0 = _mm_srai_epi32 (b0, cosBit);

    *out0 = _mm_packs_epi32 (c0, c0);
    *out1 = _mm_packs_epi32 (d0, c0);

}


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

static inline __m128i scale_round_sse2 (const __m128i a, const int scale)
{
    const __m128i scale_rounding = pair_set_epi16(scale, 1 << (XIN_SQRT2_BITS - 1));
    const __m128i b = _mm_madd_epi16(a, scale_rounding);

    return _mm_srai_epi32(b, XIN_SQRT2_BITS);
}

void Xin265pFdct4x4_SSE2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit)
{
    const SINT32  *cosPi;
    __m128i       cospi_p32_p32;
    __m128i       cospi_p32_m32;
    __m128i       cospi_p16_p48;
    __m128i       cospi_p48_m16;
    __m128i       __rounding;
    __m128i       u[4], v[4];
    __m128i       *input128;
    __m128i       *output128;

    cosPi = XinGetCosPi(cosBit);

    cospi_p32_p32 = pair_set_epi16 (cosPi[32],  cosPi[32]);
    cospi_p32_m32 = pair_set_epi16 (cosPi[32], -cosPi[32]);
    cospi_p16_p48 = pair_set_epi16 (cosPi[16],  cosPi[48]);
    cospi_p48_m16 = pair_set_epi16 (cosPi[48], -cosPi[16]);
    __rounding    = _mm_set1_epi32 (1 << (cosBit - 1));
    input128      = (__m128i *)input;
    output128     = (__m128i *)output;

    u[0] = _mm_unpacklo_epi16 (input128[0], input128[1]);
    u[1] = _mm_unpacklo_epi16 (input128[3], input128[2]);

    v[0] = _mm_add_epi16 (u[0], u[1]);
    v[1] = _mm_sub_epi16 (u[0], u[1]);

    u[0] = _mm_madd_epi16 (v[0], cospi_p32_p32);  // 0
    u[1] = _mm_madd_epi16 (v[0], cospi_p32_m32);  // 2
    u[2] = _mm_madd_epi16 (v[1], cospi_p16_p48);  // 1
    u[3] = _mm_madd_epi16 (v[1], cospi_p48_m16);  // 3

    v[0] = _mm_add_epi32 (u[0], __rounding);
    v[1] = _mm_add_epi32 (u[1], __rounding);
    v[2] = _mm_add_epi32 (u[2], __rounding);
    v[3] = _mm_add_epi32 (u[3], __rounding);
    u[0] = _mm_srai_epi32 (v[0], cosBit);
    u[1] = _mm_srai_epi32 (v[1], cosBit);
    u[2] = _mm_srai_epi32 (v[2], cosBit);
    u[3] = _mm_srai_epi32 (v[3], cosBit);

    output128[0] = _mm_packs_epi32 (u[0], u[1]);
    output128[1] = _mm_packs_epi32 (u[2], u[3]);
    output128[2] = _mm_srli_si128 (output128[0], 8);
    output128[3] = _mm_srli_si128 (output128[1], 8);

}

void Xin265pFadst4x4_SSE2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit)
{
    const SINT32  *sinPi;
    __m128i       sinpi_p01_p02;
    __m128i       sinpi_p04_m01;
    __m128i       sinpi_p03_p04;
    __m128i       sinpi_m03_p02;
    __m128i       sinpi_p03_p03;
    __m128i       *input128;
    __m128i       *output128;
    __m128i       __zero;
    __m128i       in7;
    __m128i       __rounding;
    __m128i       u[8], v[8];

    sinPi = XinGetSinPi(cosBit);

    sinpi_p01_p02 = pair_set_epi16 ( sinPi[1],  sinPi[2]);
    sinpi_p04_m01 = pair_set_epi16 ( sinPi[4], -sinPi[1]);
    sinpi_p03_p04 = pair_set_epi16 ( sinPi[3],  sinPi[4]);
    sinpi_m03_p02 = pair_set_epi16 (-sinPi[3],  sinPi[2]);
    sinpi_p03_p03 = _mm_set1_epi16 ((SINT16)sinPi[3]);
    __zero        = _mm_set1_epi16 (0);
    __rounding    = _mm_set1_epi32 (1 << (cosBit - 1));
    input128      = (__m128i *)input;
    output128     = (__m128i *)output;
    in7           = _mm_add_epi16 (input128[0], output128[1]);

    u[0] = _mm_unpacklo_epi16 (input128[0], input128[1]);
    u[1] = _mm_unpacklo_epi16 (input128[2], input128[3]);
    u[2] = _mm_unpacklo_epi16 (in7,         __zero);
    u[3] = _mm_unpacklo_epi16 (input128[2], __zero);
    u[4] = _mm_unpacklo_epi16 (input128[3], __zero);

    v[0] = _mm_madd_epi16 (u[0], sinpi_p01_p02);  // s0 + s2
    v[1] = _mm_madd_epi16 (u[1], sinpi_p03_p04);  // s4 + s5
    v[2] = _mm_madd_epi16 (u[2], sinpi_p03_p03);  // x1
    v[3] = _mm_madd_epi16 (u[0], sinpi_p04_m01);  // s1 - s3
    v[4] = _mm_madd_epi16 (u[1], sinpi_m03_p02);  // -s4 + s6
    v[5] = _mm_madd_epi16 (u[3], sinpi_p03_p03);  // s4
    v[6] = _mm_madd_epi16 (u[4], sinpi_p03_p03);

    u[0] = _mm_add_epi32 (v[0], v[1]);
    u[1] = _mm_sub_epi32 (v[2], v[6]);
    u[2] = _mm_add_epi32 (v[3], v[4]);
    u[3] = _mm_sub_epi32 (u[2], u[0]);
    u[4] = _mm_slli_epi32 (v[5], 2);
    u[5] = _mm_sub_epi32 (u[4], v[5]);
    u[6] = _mm_add_epi32 (u[3], u[5]);

    v[0] = _mm_add_epi32 (u[0], __rounding);
    v[1] = _mm_add_epi32 (u[1], __rounding);
    v[2] = _mm_add_epi32 (u[2], __rounding);
    v[3] = _mm_add_epi32 (u[6], __rounding);

    u[0] = _mm_srai_epi32 (v[0], cosBit);
    u[1] = _mm_srai_epi32 (v[1], cosBit);
    u[2] = _mm_srai_epi32 (v[2], cosBit);
    u[3] = _mm_srai_epi32 (v[3], cosBit);

    output128[0] = _mm_packs_epi32 (u[0], u[2]);
    output128[1] = _mm_packs_epi32 (u[1], u[3]);
    output128[2] = _mm_srli_si128 (output128[0], 8);
    output128[3] = _mm_srli_si128 (output128[1], 8);

}

void Xin265pFidentity4x4_SSE2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit)
{
    __m128i one;
    __m128i a;
    __m128i b;
    __m128i *input128;
    __m128i *output128;

    (void)cosBit;
    one       = _mm_set1_epi16(1);
    input128  = (__m128i *)input;
    output128 = (__m128i *)output;

    for (int i = 0; i < 4; ++i)
    {
        a = _mm_unpacklo_epi16(input128[i], one);
        b = scale_round_sse2(a, XIN_FWD_SQRT2);

        output128[i] = _mm_packs_epi32(b, b);
    }

}

void Xin265pFdct8x4_SSE2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cos_bit)
{
    const SINT32 *cosPi;
    __m128i      cospi_p32_p32;
    __m128i      cospi_p32_m32;
    __m128i      cospi_p48_p16;
    __m128i      cospi_m16_p48;
    __m128i      __rounding;
    __m128i      x1[4];
    __m128i      x2[4];
    __m128i      *input128;
    __m128i      *output128;

	cosPi = XinGetCosPi(cos_bit);

    input128      = (__m128i *)input;
    output128     = (__m128i *)output;
	__rounding    = _mm_set1_epi32(1 << (cos_bit - 1));
    cospi_p32_p32 = pair_set_epi16 ( cosPi[32],  cosPi[32]);
    cospi_p32_m32 = pair_set_epi16 ( cosPi[32], -cosPi[32]);
    cospi_p48_p16 = pair_set_epi16 ( cosPi[48],  cosPi[16]);
    cospi_m16_p48 = pair_set_epi16 (-cosPi[16],  cosPi[48]);

    // stage 1
    x1[0] = _mm_adds_epi16 (input128[0], input128[3]);
    x1[3] = _mm_subs_epi16 (input128[0], input128[3]);
    x1[1] = _mm_adds_epi16 (input128[1], input128[2]);
    x1[2] = _mm_subs_epi16 (input128[1], input128[2]);

    // stage 2
    btf_16_sse2 (cospi_p32_p32, cospi_p32_m32, x1[0], x1[1], x2[0], x2[1]);
    btf_16_sse2 (cospi_p48_p16, cospi_m16_p48, x1[2], x1[3], x2[2], x2[3]);

    // stage 3
    output128[0] = x2[0];
    output128[1] = x2[2];
    output128[2] = x2[1];
    output128[3] = x2[3];

}

void Xin265pFadst8x4_SSE2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit)
{
    const SINT32 *sinPi;
    __m128i      sinpi_p01_p02;
    __m128i      sinpi_p04_m01;
    __m128i      sinpi_p03_p04;
    __m128i      sinpi_m03_p02;
    __m128i      sinpi_p03_p03;
    __m128i      __zero;
    __m128i      __rounding;
    __m128i      in7;
    __m128i      u_lo[8], u_hi[8], v_lo[8], v_hi[8];
    __m128i      *input128;
    __m128i      *output128;

    sinPi = XinGetSinPi(cosBit);

    input128      = (__m128i *)input;
    output128     = (__m128i *)output;
    sinpi_p01_p02 = pair_set_epi16 ( sinPi[1],  sinPi[2]);
    sinpi_p04_m01 = pair_set_epi16 ( sinPi[4], -sinPi[1]);
    sinpi_p03_p04 = pair_set_epi16 ( sinPi[3],  sinPi[4]);
    sinpi_m03_p02 = pair_set_epi16 (-sinPi[3],  sinPi[2]);
    sinpi_p03_p03 = _mm_set1_epi16 ((SINT16)sinPi[3]);
    __zero        = _mm_set1_epi16(0);
    __rounding    = _mm_set1_epi32(1 << (cosBit - 1));
    in7           = _mm_add_epi16(input128[0], input128[1]);

    u_lo[0] = _mm_unpacklo_epi16 (input128[0], input128[1]);
    u_hi[0] = _mm_unpackhi_epi16 (input128[0], input128[1]);
    u_lo[1] = _mm_unpacklo_epi16 (input128[2], input128[3]);
    u_hi[1] = _mm_unpackhi_epi16 (input128[2], input128[3]);
    u_lo[2] = _mm_unpacklo_epi16 (in7, __zero);
    u_hi[2] = _mm_unpackhi_epi16 (in7, __zero);
    u_lo[3] = _mm_unpacklo_epi16 (input128[2], __zero);
    u_hi[3] = _mm_unpackhi_epi16 (input128[2], __zero);
    u_lo[4] = _mm_unpacklo_epi16 (input128[3], __zero);
    u_hi[4] = _mm_unpackhi_epi16 (input128[3], __zero);

    v_lo[0] = _mm_madd_epi16 (u_lo[0], sinpi_p01_p02);  // s0 + s2
    v_hi[0] = _mm_madd_epi16 (u_hi[0], sinpi_p01_p02);  // s0 + s2
    v_lo[1] = _mm_madd_epi16 (u_lo[1], sinpi_p03_p04);  // s4 + s5
    v_hi[1] = _mm_madd_epi16 (u_hi[1], sinpi_p03_p04);  // s4 + s5
    v_lo[2] = _mm_madd_epi16 (u_lo[2], sinpi_p03_p03);  // x1
    v_hi[2] = _mm_madd_epi16 (u_hi[2], sinpi_p03_p03);  // x1
    v_lo[3] = _mm_madd_epi16 (u_lo[0], sinpi_p04_m01);  // s1 - s3
    v_hi[3] = _mm_madd_epi16 (u_hi[0], sinpi_p04_m01);  // s1 - s3
    v_lo[4] = _mm_madd_epi16 (u_lo[1], sinpi_m03_p02);  // -s4 + s6
    v_hi[4] = _mm_madd_epi16 (u_hi[1], sinpi_m03_p02);  // -s4 + s6
    v_lo[5] = _mm_madd_epi16 (u_lo[3], sinpi_p03_p03);  // s4
    v_hi[5] = _mm_madd_epi16 (u_hi[3], sinpi_p03_p03);  // s4
    v_lo[6] = _mm_madd_epi16 (u_lo[4], sinpi_p03_p03);
    v_hi[6] = _mm_madd_epi16 (u_hi[4], sinpi_p03_p03);

    u_lo[0] = _mm_add_epi32 (v_lo[0], v_lo[1]);
    u_hi[0] = _mm_add_epi32 (v_hi[0], v_hi[1]);
    u_lo[1] = _mm_sub_epi32 (v_lo[2], v_lo[6]);
    u_hi[1] = _mm_sub_epi32 (v_hi[2], v_hi[6]);
    u_lo[2] = _mm_add_epi32 (v_lo[3], v_lo[4]);
    u_hi[2] = _mm_add_epi32 (v_hi[3], v_hi[4]);
    u_lo[3] = _mm_sub_epi32 (u_lo[2], u_lo[0]);
    u_hi[3] = _mm_sub_epi32 (u_hi[2], u_hi[0]);
    u_lo[4] = _mm_slli_epi32 (v_lo[5], 2);
    u_hi[4] = _mm_slli_epi32 (v_hi[5], 2);
    u_lo[5] = _mm_sub_epi32 (u_lo[4], v_lo[5]);
    u_hi[5] = _mm_sub_epi32 (u_hi[4], v_hi[5]);
    u_lo[6] = _mm_add_epi32 (u_lo[3], u_lo[5]);
    u_hi[6] = _mm_add_epi32 (u_hi[3], u_hi[5]);

    v_lo[0] = _mm_add_epi32 (u_lo[0], __rounding);
    v_hi[0] = _mm_add_epi32 (u_hi[0], __rounding);
    v_lo[1] = _mm_add_epi32 (u_lo[1], __rounding);
    v_hi[1] = _mm_add_epi32 (u_hi[1], __rounding);
    v_lo[2] = _mm_add_epi32 (u_lo[2], __rounding);
    v_hi[2] = _mm_add_epi32 (u_hi[2], __rounding);
    v_lo[3] = _mm_add_epi32 (u_lo[6], __rounding);
    v_hi[3] = _mm_add_epi32 (u_hi[6], __rounding);

    u_lo[0] = _mm_srai_epi32(v_lo[0], cosBit);
    u_hi[0] = _mm_srai_epi32(v_hi[0], cosBit);
    u_lo[1] = _mm_srai_epi32(v_lo[1], cosBit);
    u_hi[1] = _mm_srai_epi32(v_hi[1], cosBit);
    u_lo[2] = _mm_srai_epi32(v_lo[2], cosBit);
    u_hi[2] = _mm_srai_epi32(v_hi[2], cosBit);
    u_lo[3] = _mm_srai_epi32(v_lo[3], cosBit);
    u_hi[3] = _mm_srai_epi32(v_hi[3], cosBit);

    output128[0] = _mm_packs_epi32(u_lo[0], u_hi[0]);
    output128[1] = _mm_packs_epi32(u_lo[1], u_hi[1]);
    output128[2] = _mm_packs_epi32(u_lo[2], u_hi[2]);
    output128[3] = _mm_packs_epi32(u_lo[3], u_hi[3]);

}

void Xin265pFidentity8x4_SSE2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit)
{
    __m128i one;
    SINT32  idx;
    __m128i *input128;
    __m128i *output128;
    __m128i a_lo;
    __m128i a_hi;
    __m128i b_lo;
    __m128i b_hi;

    (void)cosBit;
    one       = _mm_set1_epi16(1);
    input128  = (__m128i *)input;
    output128 = (__m128i *)output;

    for (idx = 0; idx < 4; ++idx)
    {
        a_lo = _mm_unpacklo_epi16 (input128[idx], one);
        a_hi = _mm_unpackhi_epi16 (input128[idx], one);
        b_lo = scale_round_sse2 (a_lo, XIN_FWD_SQRT2);
        b_hi = scale_round_sse2 (a_hi, XIN_FWD_SQRT2);

        output128[idx] = _mm_packs_epi32 (b_lo, b_hi);
    }

}

void Xin265pFdct4x8_SSE2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit)
{
    const SINT32 *cosPi;
    __m128i      __rounding;
    __m128i      cospi_m32_p32;
    __m128i      cospi_p32_p32;
    __m128i      cospi_p32_m32;
    __m128i      cospi_p48_p16;
    __m128i      cospi_m16_p48;
    __m128i      cospi_p56_p08;
    __m128i      cospi_m08_p56;
    __m128i      cospi_p24_p40;
    __m128i      cospi_m40_p24;
    __m128i      x1[8];
    __m128i      x2[8];
    __m128i      x3[8];
    __m128i      x4[8];
    __m128i      *input128;
    __m128i      *output128;

    cosPi = XinGetCosPi(cosBit);

    __rounding    = _mm_set1_epi32 (1 << (cosBit - 1));
    cospi_m32_p32 = pair_set_epi16 (-cosPi[32],  cosPi[32]);
    cospi_p32_p32 = pair_set_epi16 ( cosPi[32],  cosPi[32]);
    cospi_p32_m32 = pair_set_epi16 ( cosPi[32], -cosPi[32]);
    cospi_p48_p16 = pair_set_epi16 ( cosPi[48],  cosPi[16]);
    cospi_m16_p48 = pair_set_epi16 (-cosPi[16],  cosPi[48]);
    cospi_p56_p08 = pair_set_epi16 ( cosPi[56],  cosPi[8]);
    cospi_m08_p56 = pair_set_epi16 (-cosPi[8],   cosPi[56]);
    cospi_p24_p40 = pair_set_epi16 ( cosPi[24],  cosPi[40]);
    cospi_m40_p24 = pair_set_epi16 (-cosPi[40],  cosPi[24]);
    input128      = (__m128i *)input;
    output128     = (__m128i *)output;

    // stage 1

    x1[0] = _mm_adds_epi16 (input128[0], input128[7]);
    x1[7] = _mm_subs_epi16 (input128[0], input128[7]);
    x1[1] = _mm_adds_epi16 (input128[1], input128[6]);
    x1[6] = _mm_subs_epi16 (input128[1], input128[6]);
    x1[2] = _mm_adds_epi16 (input128[2], input128[5]);
    x1[5] = _mm_subs_epi16 (input128[2], input128[5]);
    x1[3] = _mm_adds_epi16 (input128[3], input128[4]);
    x1[4] = _mm_subs_epi16 (input128[3], input128[4]);

    // stage 2
    x2[0] = _mm_adds_epi16 (x1[0], x1[3]);
    x2[3] = _mm_subs_epi16 (x1[0], x1[3]);
    x2[1] = _mm_adds_epi16 (x1[1], x1[2]);
    x2[2] = _mm_subs_epi16 (x1[1], x1[2]);
    x2[4] = x1[4];

    btf_16_w4_sse2 (
        &cospi_m32_p32,
        &cospi_p32_p32,
        __rounding,
        cosBit,
        &x1[5],
        &x1[6],
        &x2[5],
        &x2[6]);

    x2[7] = x1[7];

    // stage 3
    btf_16_w4_sse2 (
        &cospi_p32_p32,
        &cospi_p32_m32,
        __rounding,
        cosBit,
        &x2[0],
        &x2[1],
        &x3[0],
        &x3[1]);

    btf_16_w4_sse2 (
        &cospi_p48_p16,
        &cospi_m16_p48,
        __rounding,
        cosBit,
        &x2[2],
        &x2[3],
        &x3[2],
        &x3[3]);

    x3[4] = _mm_adds_epi16 (x2[4], x2[5]);
    x3[5] = _mm_subs_epi16 (x2[4], x2[5]);
    x3[6] = _mm_subs_epi16 (x2[7], x2[6]);
    x3[7] = _mm_adds_epi16 (x2[7], x2[6]);

    // stage 4
    x4[0] = x3[0];
    x4[1] = x3[1];
    x4[2] = x3[2];
    x4[3] = x3[3];

    btf_16_w4_sse2 (
        &cospi_p56_p08,
        &cospi_m08_p56,
        __rounding,
        cosBit,
        &x3[4],
        &x3[7],
        &x4[4],
        &x4[7]);

    btf_16_w4_sse2 (
        &cospi_p24_p40,
        &cospi_m40_p24,
        __rounding,
        cosBit,
        &x3[5],
        &x3[6],
        &x4[5],
        &x4[6]);

    // stage 5
    output128[0] = x4[0];
    output128[1] = x4[4];
    output128[2] = x4[2];
    output128[3] = x4[6];
    output128[4] = x4[1];
    output128[5] = x4[5];
    output128[6] = x4[3];
    output128[7] = x4[7];

}

void Xin265pFadst4x8_SSE2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit)
{
    const SINT32 *cosPi;
    __m128i      __zero;
    __m128i      __rounding;
    __m128i      cospi_p32_p32;
    __m128i      cospi_p32_m32;
    __m128i      cospi_p16_p48;
    __m128i      cospi_p48_m16;
    __m128i      cospi_m48_p16;
    __m128i      cospi_p04_p60;
    __m128i      cospi_p60_m04;
    __m128i      cospi_p20_p44;
    __m128i      cospi_p44_m20;
    __m128i      cospi_p36_p28;
    __m128i      cospi_p28_m36;
    __m128i      cospi_p52_p12;
    __m128i      cospi_p12_m52;
    __m128i      x1[8];
    __m128i      x2[8];
    __m128i      x3[8];
    __m128i      x4[8];
    __m128i      x5[8];
    __m128i      x6[8];
    __m128i      *input128;
    __m128i      *output128;

    cosPi = XinGetCosPi(cosBit);

    __zero        = _mm_setzero_si128 ();
    __rounding    = _mm_set1_epi32 (1 << (cosBit - 1));
    cospi_p32_p32 = pair_set_epi16 ( cosPi[32],  cosPi[32]);
    cospi_p32_m32 = pair_set_epi16 ( cosPi[32], -cosPi[32]);
    cospi_p16_p48 = pair_set_epi16 ( cosPi[16],  cosPi[48]);
    cospi_p48_m16 = pair_set_epi16 ( cosPi[48], -cosPi[16]);
    cospi_m48_p16 = pair_set_epi16 (-cosPi[48],  cosPi[16]);
    cospi_p04_p60 = pair_set_epi16 ( cosPi[4],   cosPi[60]);
    cospi_p60_m04 = pair_set_epi16 ( cosPi[60], -cosPi[4]);
    cospi_p20_p44 = pair_set_epi16 ( cosPi[20],  cosPi[44]);
    cospi_p44_m20 = pair_set_epi16 ( cosPi[44], -cosPi[20]);
    cospi_p36_p28 = pair_set_epi16 ( cosPi[36],  cosPi[28]);
    cospi_p28_m36 = pair_set_epi16 ( cosPi[28], -cosPi[36]);
    cospi_p52_p12 = pair_set_epi16 ( cosPi[52],  cosPi[12]);
    cospi_p12_m52 = pair_set_epi16 ( cosPi[12], -cosPi[52]);
    input128      = (__m128i *)input;
    output128     = (__m128i *)output;

    // stage 1
    x1[0] = input128[0];
    x1[1] = _mm_subs_epi16 (__zero, input128[7]);
    x1[2] = _mm_subs_epi16 (__zero, input128[3]);
    x1[3] = input128[4];
    x1[4] = _mm_subs_epi16 (__zero, input128[1]);
    x1[5] = input128[6];
    x1[6] = input128[2];
    x1[7] = _mm_subs_epi16 (__zero, input128[5]);

    // stage 2

    x2[0] = x1[0];
    x2[1] = x1[1];

    btf_16_w4_sse2 (
        &cospi_p32_p32,
        &cospi_p32_m32,
        __rounding,
        cosBit,
        &x1[2],
        &x1[3],
        &x2[2],
        &x2[3]);

    x2[4] = x1[4];
    x2[5] = x1[5];

    btf_16_w4_sse2 (
        &cospi_p32_p32,
        &cospi_p32_m32,
        __rounding,
        cosBit,
        &x1[6],
        &x1[7],
        &x2[6],
        &x2[7]);

    // stage 3
    x3[0] = _mm_adds_epi16 (x2[0], x2[2]);
    x3[2] = _mm_subs_epi16 (x2[0], x2[2]);
    x3[1] = _mm_adds_epi16 (x2[1], x2[3]);
    x3[3] = _mm_subs_epi16 (x2[1], x2[3]);
    x3[4] = _mm_adds_epi16 (x2[4], x2[6]);
    x3[6] = _mm_subs_epi16 (x2[4], x2[6]);
    x3[5] = _mm_adds_epi16 (x2[5], x2[7]);
    x3[7] = _mm_subs_epi16 (x2[5], x2[7]);

    // stage 4
    x4[0] = x3[0];
    x4[1] = x3[1];
    x4[2] = x3[2];
    x4[3] = x3[3];

    btf_16_w4_sse2 (
        &cospi_p16_p48,
        &cospi_p48_m16,
        __rounding,
        cosBit,
        &x3[4],
        &x3[5],
        &x4[4],
        &x4[5]);

    btf_16_w4_sse2 (
        &cospi_m48_p16,
        &cospi_p16_p48,
        __rounding,
        cosBit,
        &x3[6],
        &x3[7],
        &x4[6],
        &x4[7]);

    // stage 5

    x5[0] = _mm_adds_epi16 (x4[0], x4[4]);
    x5[4] = _mm_subs_epi16 (x4[0], x4[4]);
    x5[1] = _mm_adds_epi16 (x4[1], x4[5]);
    x5[5] = _mm_subs_epi16 (x4[1], x4[5]);
    x5[2] = _mm_adds_epi16 (x4[2], x4[6]);
    x5[6] = _mm_subs_epi16 (x4[2], x4[6]);
    x5[3] = _mm_adds_epi16 (x4[3], x4[7]);
    x5[7] = _mm_subs_epi16 (x4[3], x4[7]);

    // stage 6

    btf_16_w4_sse2 (
        &cospi_p04_p60,
        &cospi_p60_m04,
        __rounding,
        cosBit,
        &x5[0],
        &x5[1],
        &x6[0],
        &x6[1]);

    btf_16_w4_sse2 (
        &cospi_p20_p44,
        &cospi_p44_m20,
        __rounding,
        cosBit,
        &x5[2],
        &x5[3],
        &x6[2],
        &x6[3]);

    btf_16_w4_sse2 (
        &cospi_p36_p28,
        &cospi_p28_m36,
        __rounding,
        cosBit,
        &x5[4],
        &x5[5],
        &x6[4],
        &x6[5]);

    btf_16_w4_sse2 (
        &cospi_p52_p12,
        &cospi_p12_m52,
        __rounding,
        cosBit,
        &x5[6],
        &x5[7],
        &x6[6],
        &x6[7]);

    // stage 7
    output128[0] = x6[1];
    output128[1] = x6[6];
    output128[2] = x6[3];
    output128[3] = x6[4];
    output128[4] = x6[5];
    output128[5] = x6[2];
    output128[6] = x6[7];
    output128[7] = x6[0];

}

void Xin265pFidentity8x8_SSE2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit)
{
    __m128i *input128;
    __m128i *output128;

    (void)cosBit;
    input128  = (__m128i *)input;
    output128 = (__m128i *)output;

    output128[0] = _mm_adds_epi16 (input128[0], input128[0]);
    output128[1] = _mm_adds_epi16 (input128[1], input128[1]);
    output128[2] = _mm_adds_epi16 (input128[2], input128[2]);
    output128[3] = _mm_adds_epi16 (input128[3], input128[3]);
    output128[4] = _mm_adds_epi16 (input128[4], input128[4]);
    output128[5] = _mm_adds_epi16 (input128[5], input128[5]);
    output128[6] = _mm_adds_epi16 (input128[6], input128[6]);
    output128[7] = _mm_adds_epi16 (input128[7], input128[7]);

}

void Xin265pFdct8x8_SSE2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cos_bit)
{
    const SINT32 *cosPi;
    __m128i      __rounding;
    __m128i      cospi_m32_p32;
    __m128i      cospi_p32_p32;
    __m128i      cospi_p32_m32;
    __m128i      cospi_p48_p16;
    __m128i      cospi_m16_p48;
    __m128i      cospi_p56_p08;
    __m128i      cospi_m08_p56;
    __m128i      cospi_p24_p40;
    __m128i      cospi_m40_p24;
    __m128i      x1[8];
    __m128i      x2[8];
    __m128i      x3[8];
    __m128i      x4[8];
    __m128i      *input128;
    __m128i      *output128;

	cosPi = XinGetCosPi(cos_bit);

    input128      = (__m128i *)input;
    output128     = (__m128i *)output;
	__rounding    = _mm_set1_epi32(1 << (cos_bit - 1));
    cospi_m32_p32 = pair_set_epi16 (-cosPi[32],  cosPi[32]);
    cospi_p32_p32 = pair_set_epi16 ( cosPi[32],  cosPi[32]);
    cospi_p32_m32 = pair_set_epi16 ( cosPi[32], -cosPi[32]);
    cospi_p48_p16 = pair_set_epi16 ( cosPi[48],  cosPi[16]);
    cospi_m16_p48 = pair_set_epi16 (-cosPi[16],  cosPi[48]);
    cospi_p56_p08 = pair_set_epi16 ( cosPi[56],  cosPi[8]);
    cospi_m08_p56 = pair_set_epi16 (-cosPi[8],   cosPi[56]);
    cospi_p24_p40 = pair_set_epi16 ( cosPi[24],  cosPi[40]);
    cospi_m40_p24 = pair_set_epi16 (-cosPi[40],  cosPi[24]);

    // stage 1
    x1[0] = _mm_adds_epi16 (input128[0], input128[7]);
    x1[7] = _mm_subs_epi16 (input128[0], input128[7]);
    x1[1] = _mm_adds_epi16 (input128[1], input128[6]);
    x1[6] = _mm_subs_epi16 (input128[1], input128[6]);
    x1[2] = _mm_adds_epi16 (input128[2], input128[5]);
    x1[5] = _mm_subs_epi16 (input128[2], input128[5]);
    x1[3] = _mm_adds_epi16 (input128[3], input128[4]);
    x1[4] = _mm_subs_epi16 (input128[3], input128[4]);

    // stage 2
    x2[0] = _mm_adds_epi16 (x1[0], x1[3]);
    x2[3] = _mm_subs_epi16 (x1[0], x1[3]);
    x2[1] = _mm_adds_epi16 (x1[1], x1[2]);
    x2[2] = _mm_subs_epi16 (x1[1], x1[2]);
    x2[4] = x1[4];

    btf_16_sse2 (
        cospi_m32_p32,
        cospi_p32_p32,
        x1[5],
        x1[6],
        x2[5],
        x2[6]);

    x2[7] = x1[7];

    // stage 3
    btf_16_sse2 (
        cospi_p32_p32,
        cospi_p32_m32,
        x2[0],
        x2[1],
        x3[0],
        x3[1]);

    btf_16_sse2 (
        cospi_p48_p16,
        cospi_m16_p48,
        x2[2],
        x2[3],
        x3[2],
        x3[3]);

    x3[4] = _mm_adds_epi16 (x2[4], x2[5]);
    x3[5] = _mm_subs_epi16 (x2[4], x2[5]);
    x3[6] = _mm_subs_epi16 (x2[7], x2[6]);
    x3[7] = _mm_adds_epi16 (x2[7], x2[6]);

    // stage 4
    x4[0] = x3[0];
    x4[1] = x3[1];
    x4[2] = x3[2];
    x4[3] = x3[3];

    btf_16_sse2 (
        cospi_p56_p08,
        cospi_m08_p56,
        x3[4],
        x3[7],
        x4[4],
        x4[7]);

    btf_16_sse2 (
        cospi_p24_p40,
        cospi_m40_p24,
        x3[5],
        x3[6],
        x4[5],
        x4[6]);

    // stage 5
    output128[0] = x4[0];
    output128[1] = x4[4];
    output128[2] = x4[2];
    output128[3] = x4[6];
    output128[4] = x4[1];
    output128[5] = x4[5];
    output128[6] = x4[3];
    output128[7] = x4[7];

}

void Xin265pFadst8x8_SSE2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cos_bit)

{
    const SINT32 *cosPi;
    __m128i      __zero;
    __m128i      __rounding;
    __m128i      cospi_p32_p32;
    __m128i      cospi_p32_m32;
    __m128i      cospi_p16_p48;
    __m128i      cospi_p48_m16;
    __m128i      cospi_m48_p16;
    __m128i      cospi_p04_p60;
    __m128i      cospi_p60_m04;
    __m128i      cospi_p20_p44;
    __m128i      cospi_p44_m20;
    __m128i      cospi_p36_p28;
    __m128i      cospi_p28_m36;
    __m128i      cospi_p52_p12;
    __m128i      cospi_p12_m52;
    __m128i      *input128;
    __m128i      *output128;
    __m128i      x1[8];
    __m128i      x2[8];
    __m128i      x3[8];
    __m128i      x4[8];
    __m128i      x5[8];
    __m128i      x6[8];

    cosPi = XinGetCosPi(cos_bit);

    input128      = (__m128i *)input;
    output128     = (__m128i *)output;
    __zero        = _mm_setzero_si128 ();
    __rounding    = _mm_set1_epi32 (1 << (cos_bit - 1));
    cospi_p32_p32 = pair_set_epi16 ( cosPi[32],  cosPi[32]);
    cospi_p32_m32 = pair_set_epi16 ( cosPi[32], -cosPi[32]);
    cospi_p16_p48 = pair_set_epi16 ( cosPi[16],  cosPi[48]);
    cospi_p48_m16 = pair_set_epi16 ( cosPi[48], -cosPi[16]);
    cospi_m48_p16 = pair_set_epi16 (-cosPi[48],  cosPi[16]);
    cospi_p04_p60 = pair_set_epi16 ( cosPi[4],   cosPi[60]);
    cospi_p60_m04 = pair_set_epi16 ( cosPi[60], -cosPi[4]);
    cospi_p20_p44 = pair_set_epi16 ( cosPi[20],  cosPi[44]);
    cospi_p44_m20 = pair_set_epi16 ( cosPi[44], -cosPi[20]);
    cospi_p36_p28 = pair_set_epi16 ( cosPi[36],  cosPi[28]);
    cospi_p28_m36 = pair_set_epi16 ( cosPi[28], -cosPi[36]);
    cospi_p52_p12 = pair_set_epi16 ( cosPi[52],  cosPi[12]);
    cospi_p12_m52 = pair_set_epi16 ( cosPi[12], -cosPi[52]);

    // stage 1
    x1[0] = input128[0];
    x1[1] = _mm_subs_epi16 (__zero, input128[7]);
    x1[2] = _mm_subs_epi16 (__zero, input128[3]);
    x1[3] = input128[4];
    x1[4] = _mm_subs_epi16 (__zero, input128[1]);
    x1[5] = input128[6];
    x1[6] = input128[2];
    x1[7] = _mm_subs_epi16 (__zero, input128[5]);

    // stage 2
    x2[0] = x1[0];
    x2[1] = x1[1];

    btf_16_sse2 (
        cospi_p32_p32,
        cospi_p32_m32,
        x1[2],
        x1[3],
        x2[2],
        x2[3]);

    x2[4] = x1[4];
    x2[5] = x1[5];

    btf_16_sse2 (
        cospi_p32_p32,
        cospi_p32_m32,
        x1[6],
        x1[7],
        x2[6],
        x2[7]);

    // stage 3
    x3[0] = _mm_adds_epi16 (x2[0], x2[2]);
    x3[2] = _mm_subs_epi16 (x2[0], x2[2]);
    x3[1] = _mm_adds_epi16 (x2[1], x2[3]);
    x3[3] = _mm_subs_epi16 (x2[1], x2[3]);
    x3[4] = _mm_adds_epi16 (x2[4], x2[6]);
    x3[6] = _mm_subs_epi16 (x2[4], x2[6]);
    x3[5] = _mm_adds_epi16 (x2[5], x2[7]);
    x3[7] = _mm_subs_epi16 (x2[5], x2[7]);

    // stage 4
    x4[0] = x3[0];
    x4[1] = x3[1];
    x4[2] = x3[2];
    x4[3] = x3[3];

    btf_16_sse2 (
        cospi_p16_p48,
        cospi_p48_m16,
        x3[4],
        x3[5],
        x4[4],
        x4[5]);

    btf_16_sse2 (
        cospi_m48_p16,
        cospi_p16_p48,
        x3[6],
        x3[7],
        x4[6],
        x4[7]);

    // stage 5
    x5[0] = _mm_adds_epi16 (x4[0], x4[4]);
    x5[4] = _mm_subs_epi16 (x4[0], x4[4]);
    x5[1] = _mm_adds_epi16 (x4[1], x4[5]);
    x5[5] = _mm_subs_epi16 (x4[1], x4[5]);
    x5[2] = _mm_adds_epi16 (x4[2], x4[6]);
    x5[6] = _mm_subs_epi16 (x4[2], x4[6]);
    x5[3] = _mm_adds_epi16 (x4[3], x4[7]);
    x5[7] = _mm_subs_epi16 (x4[3], x4[7]);

    // stage 6
    btf_16_sse2 (
        cospi_p04_p60,
        cospi_p60_m04,
        x5[0],
        x5[1],
        x6[0],
        x6[1]);

    btf_16_sse2 (
        cospi_p20_p44, 
        cospi_p44_m20, 
        x5[2], 
        x5[3], 
        x6[2], 
        x6[3]);
    
    btf_16_sse2 (
        cospi_p36_p28, 
        cospi_p28_m36, 
        x5[4], 
        x5[5], 
        x6[4], 
        x6[5]);
    
    btf_16_sse2 (
        cospi_p52_p12, 
        cospi_p12_m52, 
        x5[6], 
        x5[7], 
        x6[6], 
        x6[7]);

    // stage 7
    output128[0] = x6[1];
    output128[1] = x6[6];
    output128[2] = x6[3];
    output128[3] = x6[4];
    output128[4] = x6[5];
    output128[5] = x6[2];
    output128[6] = x6[7];
    output128[7] = x6[0];
    
}

