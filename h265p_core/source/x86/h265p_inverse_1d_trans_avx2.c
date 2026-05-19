/***************************************************************************//**
*
* @file          h265p_inverse_1d_trans_avx2.c
* @brief         av1 forward transform subroutines.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#include "immintrin.h"
#include "xin_typedef.h"
#include "h265p_trans_context.h"
#include "h265p_inverse_1d_trans.h"

static const SINT32 *XinGetCosPi (
    SINT32 n)
{
    return cosPiData[n - XIN_COS_BIT_MIN];
}

static inline __m256i pair_set_w16_epi16 (
    SINT32 a,
    SINT32 b)
{
    return _mm256_set1_epi32 ((SINT32)(((UINT16)(a)) | (((UINT32)(b)) << 16)));
}

static inline void btf_16_w16_avx2 (
    __m256i w0,
    __m256i w1,
    __m256i *in0,
    __m256i *in1,
    __m256i _r,
    SINT32  cos_bit)
{
    __m256i t0 = _mm256_unpacklo_epi16(*in0, *in1);
    __m256i t1 = _mm256_unpackhi_epi16(*in0, *in1);
    __m256i u0 = _mm256_madd_epi16(t0, w0);
    __m256i u1 = _mm256_madd_epi16(t1, w0);
    __m256i v0 = _mm256_madd_epi16(t0, w1);
    __m256i v1 = _mm256_madd_epi16(t1, w1);

    __m256i a0 = _mm256_add_epi32(u0, _r);
    __m256i a1 = _mm256_add_epi32(u1, _r);
    __m256i b0 = _mm256_add_epi32(v0, _r);
    __m256i b1 = _mm256_add_epi32(v1, _r);

    __m256i c0 = _mm256_srai_epi32(a0, cos_bit);
    __m256i c1 = _mm256_srai_epi32(a1, cos_bit);
    __m256i d0 = _mm256_srai_epi32(b0, cos_bit);
    __m256i d1 = _mm256_srai_epi32(b1, cos_bit);

    *in0 = _mm256_packs_epi32(c0, c1);
    *in1 = _mm256_packs_epi32(d0, d1);

}

static inline void btf_16_adds_subs_avx2 (
    __m256i *in0,
    __m256i *in1)
{
    const __m256i _in0 = *in0;
    const __m256i _in1 = *in1;
    *in0 = _mm256_adds_epi16(_in0, _in1);
    *in1 = _mm256_subs_epi16(_in0, _in1);
}

static inline void btf_16_adds_subs_out_avx2 (
    __m256i *out0,
    __m256i *out1,
    __m256i in0,
    __m256i in1)
{
    const __m256i _in0 = in0;
    const __m256i _in1 = in1;
    *out0 = _mm256_adds_epi16 (_in0, _in1);
    *out1 = _mm256_subs_epi16 (_in0, _in1);
}

static inline void idct16_stage5_avx2(__m256i *x1, const int32_t *cospi,
                                      const __m256i _r, int8_t cos_bit)
{
    const __m256i cospi_m32_p32 = pair_set_w16_epi16(-cospi[32], cospi[32]);
    const __m256i cospi_p32_p32 = pair_set_w16_epi16(cospi[32], cospi[32]);
    btf_16_adds_subs_avx2(&x1[0], &x1[3]);
    btf_16_adds_subs_avx2(&x1[1], &x1[2]);
    btf_16_w16_avx2(cospi_m32_p32, cospi_p32_p32, &x1[5], &x1[6], _r, cos_bit);

    btf_16_adds_subs_avx2(&x1[8], &x1[11]);
    btf_16_adds_subs_avx2(&x1[9], &x1[10]);
    btf_16_adds_subs_avx2(&x1[15], &x1[12]);
    btf_16_adds_subs_avx2(&x1[14], &x1[13]);
}

static inline void idct16_stage6_avx2(__m256i *x, const int32_t *cospi,
                                      const __m256i _r, int8_t cos_bit)
{
    const __m256i cospi_m32_p32 = pair_set_w16_epi16(-cospi[32], cospi[32]);
    const __m256i cospi_p32_p32 = pair_set_w16_epi16(cospi[32], cospi[32]);
    btf_16_adds_subs_avx2(&x[0], &x[7]);
    btf_16_adds_subs_avx2(&x[1], &x[6]);
    btf_16_adds_subs_avx2(&x[2], &x[5]);
    btf_16_adds_subs_avx2(&x[3], &x[4]);
    btf_16_w16_avx2(cospi_m32_p32, cospi_p32_p32, &x[10], &x[13], _r, cos_bit);
    btf_16_w16_avx2(cospi_m32_p32, cospi_p32_p32, &x[11], &x[12], _r, cos_bit);
}

static inline void idct16_stage7_avx2(__m256i *output, __m256i *x1)
{
    btf_16_adds_subs_out_avx2(&output[0], &output[15], x1[0], x1[15]);
    btf_16_adds_subs_out_avx2(&output[1], &output[14], x1[1], x1[14]);
    btf_16_adds_subs_out_avx2(&output[2], &output[13], x1[2], x1[13]);
    btf_16_adds_subs_out_avx2(&output[3], &output[12], x1[3], x1[12]);
    btf_16_adds_subs_out_avx2(&output[4], &output[11], x1[4], x1[11]);
    btf_16_adds_subs_out_avx2(&output[5], &output[10], x1[5], x1[10]);
    btf_16_adds_subs_out_avx2(&output[6], &output[9], x1[6], x1[9]);
    btf_16_adds_subs_out_avx2(&output[7], &output[8], x1[7], x1[8]);
}

static inline void iadst16_stage3_avx2(__m256i *x)
{
    btf_16_adds_subs_avx2(&x[0], &x[8]);
    btf_16_adds_subs_avx2(&x[1], &x[9]);
    btf_16_adds_subs_avx2(&x[2], &x[10]);
    btf_16_adds_subs_avx2(&x[3], &x[11]);
    btf_16_adds_subs_avx2(&x[4], &x[12]);
    btf_16_adds_subs_avx2(&x[5], &x[13]);
    btf_16_adds_subs_avx2(&x[6], &x[14]);
    btf_16_adds_subs_avx2(&x[7], &x[15]);
}

static inline void iadst16_stage4_avx2(__m256i *x, const int32_t *cospi,
                                       const __m256i _r, int8_t cos_bit)
{
    const __m256i cospi_p08_p56 = pair_set_w16_epi16(cospi[8], cospi[56]);
    const __m256i cospi_p56_m08 = pair_set_w16_epi16(cospi[56], -cospi[8]);
    const __m256i cospi_p40_p24 = pair_set_w16_epi16(cospi[40], cospi[24]);
    const __m256i cospi_p24_m40 = pair_set_w16_epi16(cospi[24], -cospi[40]);
    const __m256i cospi_m56_p08 = pair_set_w16_epi16(-cospi[56], cospi[8]);
    const __m256i cospi_m24_p40 = pair_set_w16_epi16(-cospi[24], cospi[40]);
    btf_16_w16_avx2(cospi_p08_p56, cospi_p56_m08, &x[8], &x[9], _r, cos_bit);
    btf_16_w16_avx2(cospi_p40_p24, cospi_p24_m40, &x[10], &x[11], _r, cos_bit);
    btf_16_w16_avx2(cospi_m56_p08, cospi_p08_p56, &x[12], &x[13], _r, cos_bit);
    btf_16_w16_avx2(cospi_m24_p40, cospi_p40_p24, &x[14], &x[15], _r, cos_bit);
}

static inline void iadst16_stage5_avx2(__m256i *x)
{
    btf_16_adds_subs_avx2(&x[0], &x[4]);
    btf_16_adds_subs_avx2(&x[1], &x[5]);
    btf_16_adds_subs_avx2(&x[2], &x[6]);
    btf_16_adds_subs_avx2(&x[3], &x[7]);
    btf_16_adds_subs_avx2(&x[8], &x[12]);
    btf_16_adds_subs_avx2(&x[9], &x[13]);
    btf_16_adds_subs_avx2(&x[10], &x[14]);
    btf_16_adds_subs_avx2(&x[11], &x[15]);
}

static inline void iadst16_stage6_avx2(__m256i *x, const int32_t *cospi,
                                       const __m256i _r, int8_t cos_bit)
{
    const __m256i cospi_p16_p48 = pair_set_w16_epi16(cospi[16], cospi[48]);
    const __m256i cospi_p48_m16 = pair_set_w16_epi16(cospi[48], -cospi[16]);
    const __m256i cospi_m48_p16 = pair_set_w16_epi16(-cospi[48], cospi[16]);
    btf_16_w16_avx2(cospi_p16_p48, cospi_p48_m16, &x[4], &x[5], _r, cos_bit);
    btf_16_w16_avx2(cospi_m48_p16, cospi_p16_p48, &x[6], &x[7], _r, cos_bit);
    btf_16_w16_avx2(cospi_p16_p48, cospi_p48_m16, &x[12], &x[13], _r, cos_bit);
    btf_16_w16_avx2(cospi_m48_p16, cospi_p16_p48, &x[14], &x[15], _r, cos_bit);
}

static inline void iadst16_stage7_avx2(__m256i *x)
{
    btf_16_adds_subs_avx2(&x[0], &x[2]);
    btf_16_adds_subs_avx2(&x[1], &x[3]);
    btf_16_adds_subs_avx2(&x[4], &x[6]);
    btf_16_adds_subs_avx2(&x[5], &x[7]);
    btf_16_adds_subs_avx2(&x[8], &x[10]);
    btf_16_adds_subs_avx2(&x[9], &x[11]);
    btf_16_adds_subs_avx2(&x[12], &x[14]);
    btf_16_adds_subs_avx2(&x[13], &x[15]);
}

static inline void iadst16_stage8_avx2(__m256i *x1, const int32_t *cospi,
                                       const __m256i _r, int8_t cos_bit)
{
    const __m256i cospi_p32_p32 = pair_set_w16_epi16(cospi[32], cospi[32]);
    const __m256i cospi_p32_m32 = pair_set_w16_epi16(cospi[32], -cospi[32]);
    btf_16_w16_avx2(cospi_p32_p32, cospi_p32_m32, &x1[2], &x1[3], _r, cos_bit);
    btf_16_w16_avx2(cospi_p32_p32, cospi_p32_m32, &x1[6], &x1[7], _r, cos_bit);
    btf_16_w16_avx2(cospi_p32_p32, cospi_p32_m32, &x1[10], &x1[11], _r, cos_bit);
    btf_16_w16_avx2(cospi_p32_p32, cospi_p32_m32, &x1[14], &x1[15], _r, cos_bit);
}

static inline void iadst16_stage9_avx2(__m256i *output, __m256i *x1)
{
    const __m256i __zero = _mm256_setzero_si256();
    output[0] = x1[0];
    output[1] = _mm256_subs_epi16(__zero, x1[8]);
    output[2] = x1[12];
    output[3] = _mm256_subs_epi16(__zero, x1[4]);
    output[4] = x1[6];
    output[5] = _mm256_subs_epi16(__zero, x1[14]);
    output[6] = x1[10];
    output[7] = _mm256_subs_epi16(__zero, x1[2]);
    output[8] = x1[3];
    output[9] = _mm256_subs_epi16(__zero, x1[11]);
    output[10] = x1[15];
    output[11] = _mm256_subs_epi16(__zero, x1[7]);
    output[12] = x1[5];
    output[13] = _mm256_subs_epi16(__zero, x1[13]);
    output[14] = x1[9];
    output[15] = _mm256_subs_epi16(__zero, x1[1]);

}

static inline void idct32_high16_stage3_avx2(__m256i *x)
{
    btf_16_adds_subs_avx2(&x[16], &x[17]);
    btf_16_adds_subs_avx2(&x[19], &x[18]);
    btf_16_adds_subs_avx2(&x[20], &x[21]);
    btf_16_adds_subs_avx2(&x[23], &x[22]);
    btf_16_adds_subs_avx2(&x[24], &x[25]);
    btf_16_adds_subs_avx2(&x[27], &x[26]);
    btf_16_adds_subs_avx2(&x[28], &x[29]);
    btf_16_adds_subs_avx2(&x[31], &x[30]);
}

static inline void idct32_high16_stage4_avx2(__m256i *x, const int32_t *cospi,
        const __m256i _r, int8_t cos_bit)
{
    const __m256i cospi_m08_p56 = pair_set_w16_epi16(-cospi[8], cospi[56]);
    const __m256i cospi_p56_p08 = pair_set_w16_epi16(cospi[56], cospi[8]);
    const __m256i cospi_m56_m08 = pair_set_w16_epi16(-cospi[56], -cospi[8]);
    const __m256i cospi_m40_p24 = pair_set_w16_epi16(-cospi[40], cospi[24]);
    const __m256i cospi_p24_p40 = pair_set_w16_epi16(cospi[24], cospi[40]);
    const __m256i cospi_m24_m40 = pair_set_w16_epi16(-cospi[24], -cospi[40]);
    btf_16_w16_avx2(cospi_m08_p56, cospi_p56_p08, &x[17], &x[30], _r, cos_bit);
    btf_16_w16_avx2(cospi_m56_m08, cospi_m08_p56, &x[18], &x[29], _r, cos_bit);
    btf_16_w16_avx2(cospi_m40_p24, cospi_p24_p40, &x[21], &x[26], _r, cos_bit);
    btf_16_w16_avx2(cospi_m24_m40, cospi_m40_p24, &x[22], &x[25], _r, cos_bit);
}

static inline void idct32_high24_stage5_avx2(__m256i *x, const int32_t *cospi,
        const __m256i _r, int8_t cos_bit)
{
    const __m256i cospi_m16_p48 = pair_set_w16_epi16(-cospi[16], cospi[48]);
    const __m256i cospi_p48_p16 = pair_set_w16_epi16(cospi[48], cospi[16]);
    const __m256i cospi_m48_m16 = pair_set_w16_epi16(-cospi[48], -cospi[16]);
    btf_16_w16_avx2(cospi_m16_p48, cospi_p48_p16, &x[9], &x[14], _r, cos_bit);
    btf_16_w16_avx2(cospi_m48_m16, cospi_m16_p48, &x[10], &x[13], _r, cos_bit);
    btf_16_adds_subs_avx2(&x[16], &x[19]);
    btf_16_adds_subs_avx2(&x[17], &x[18]);
    btf_16_adds_subs_avx2(&x[23], &x[20]);
    btf_16_adds_subs_avx2(&x[22], &x[21]);
    btf_16_adds_subs_avx2(&x[24], &x[27]);
    btf_16_adds_subs_avx2(&x[25], &x[26]);
    btf_16_adds_subs_avx2(&x[31], &x[28]);
    btf_16_adds_subs_avx2(&x[30], &x[29]);
}

static inline void idct32_high28_stage6_avx2(__m256i *x, const int32_t *cospi,
        const __m256i _r, int8_t cos_bit)
{
    const __m256i cospi_m32_p32 = pair_set_w16_epi16(-cospi[32], cospi[32]);
    const __m256i cospi_p32_p32 = pair_set_w16_epi16(cospi[32], cospi[32]);
    const __m256i cospi_m16_p48 = pair_set_w16_epi16(-cospi[16], cospi[48]);
    const __m256i cospi_p48_p16 = pair_set_w16_epi16(cospi[48], cospi[16]);
    const __m256i cospi_m48_m16 = pair_set_w16_epi16(-cospi[48], -cospi[16]);
    btf_16_w16_avx2(cospi_m32_p32, cospi_p32_p32, &x[5], &x[6], _r, cos_bit);
    btf_16_adds_subs_avx2(&x[8], &x[11]);
    btf_16_adds_subs_avx2(&x[9], &x[10]);
    btf_16_adds_subs_avx2(&x[15], &x[12]);
    btf_16_adds_subs_avx2(&x[14], &x[13]);
    btf_16_w16_avx2(cospi_m16_p48, cospi_p48_p16, &x[18], &x[29], _r, cos_bit);
    btf_16_w16_avx2(cospi_m16_p48, cospi_p48_p16, &x[19], &x[28], _r, cos_bit);
    btf_16_w16_avx2(cospi_m48_m16, cospi_m16_p48, &x[20], &x[27], _r, cos_bit);
    btf_16_w16_avx2(cospi_m48_m16, cospi_m16_p48, &x[21], &x[26], _r, cos_bit);
}

static inline void idct32_stage7_avx2(__m256i *x, const int32_t *cospi,
                                      const __m256i _r, int8_t cos_bit)
{
    const __m256i cospi_m32_p32 = pair_set_w16_epi16(-cospi[32], cospi[32]);
    const __m256i cospi_p32_p32 = pair_set_w16_epi16(cospi[32], cospi[32]);
    btf_16_adds_subs_avx2(&x[0], &x[7]);
    btf_16_adds_subs_avx2(&x[1], &x[6]);
    btf_16_adds_subs_avx2(&x[2], &x[5]);
    btf_16_adds_subs_avx2(&x[3], &x[4]);
    btf_16_w16_avx2(cospi_m32_p32, cospi_p32_p32, &x[10], &x[13], _r, cos_bit);
    btf_16_w16_avx2(cospi_m32_p32, cospi_p32_p32, &x[11], &x[12], _r, cos_bit);
    btf_16_adds_subs_avx2(&x[16], &x[23]);
    btf_16_adds_subs_avx2(&x[17], &x[22]);
    btf_16_adds_subs_avx2(&x[18], &x[21]);
    btf_16_adds_subs_avx2(&x[19], &x[20]);
    btf_16_adds_subs_avx2(&x[31], &x[24]);
    btf_16_adds_subs_avx2(&x[30], &x[25]);
    btf_16_adds_subs_avx2(&x[29], &x[26]);
    btf_16_adds_subs_avx2(&x[28], &x[27]);
}

static inline void idct32_stage8_avx2(__m256i *x, const int32_t *cospi,
                                      const __m256i _r, int8_t cos_bit)
{
    const __m256i cospi_m32_p32 = pair_set_w16_epi16(-cospi[32], cospi[32]);
    const __m256i cospi_p32_p32 = pair_set_w16_epi16(cospi[32], cospi[32]);
    btf_16_adds_subs_avx2(&x[0], &x[15]);
    btf_16_adds_subs_avx2(&x[1], &x[14]);
    btf_16_adds_subs_avx2(&x[2], &x[13]);
    btf_16_adds_subs_avx2(&x[3], &x[12]);
    btf_16_adds_subs_avx2(&x[4], &x[11]);
    btf_16_adds_subs_avx2(&x[5], &x[10]);
    btf_16_adds_subs_avx2(&x[6], &x[9]);
    btf_16_adds_subs_avx2(&x[7], &x[8]);
    btf_16_w16_avx2(cospi_m32_p32, cospi_p32_p32, &x[20], &x[27], _r, cos_bit);
    btf_16_w16_avx2(cospi_m32_p32, cospi_p32_p32, &x[21], &x[26], _r, cos_bit);
    btf_16_w16_avx2(cospi_m32_p32, cospi_p32_p32, &x[22], &x[25], _r, cos_bit);
    btf_16_w16_avx2(cospi_m32_p32, cospi_p32_p32, &x[23], &x[24], _r, cos_bit);
}

static inline void idct32_stage9_avx2(__m256i *output, __m256i *x)
{
    btf_16_adds_subs_out_avx2(&output[0], &output[31], x[0], x[31]);
    btf_16_adds_subs_out_avx2(&output[1], &output[30], x[1], x[30]);
    btf_16_adds_subs_out_avx2(&output[2], &output[29], x[2], x[29]);
    btf_16_adds_subs_out_avx2(&output[3], &output[28], x[3], x[28]);
    btf_16_adds_subs_out_avx2(&output[4], &output[27], x[4], x[27]);
    btf_16_adds_subs_out_avx2(&output[5], &output[26], x[5], x[26]);
    btf_16_adds_subs_out_avx2(&output[6], &output[25], x[6], x[25]);
    btf_16_adds_subs_out_avx2(&output[7], &output[24], x[7], x[24]);
    btf_16_adds_subs_out_avx2(&output[8], &output[23], x[8], x[23]);
    btf_16_adds_subs_out_avx2(&output[9], &output[22], x[9], x[22]);
    btf_16_adds_subs_out_avx2(&output[10], &output[21], x[10], x[21]);
    btf_16_adds_subs_out_avx2(&output[11], &output[20], x[11], x[20]);
    btf_16_adds_subs_out_avx2(&output[12], &output[19], x[12], x[19]);
    btf_16_adds_subs_out_avx2(&output[13], &output[18], x[13], x[18]);
    btf_16_adds_subs_out_avx2(&output[14], &output[17], x[14], x[17]);
    btf_16_adds_subs_out_avx2(&output[15], &output[16], x[15], x[16]);
}

// half input is zero
#define btf_16_w16_0_avx2(w0, w1, in, out0, out1)  \
  {                                                \
    const __m256i _w0 = _mm256_set1_epi16((SINT16)(w0 * 8)); \
    const __m256i _w1 = _mm256_set1_epi16((SINT16)(w1 * 8)); \
    const __m256i _in = in;                        \
    out0 = _mm256_mulhrs_epi16(_in, _w0);          \
    out1 = _mm256_mulhrs_epi16(_in, _w1);          \
  }

static inline void idct64_stage4_high32_avx2(__m256i *x, const int32_t *cospi,
        const __m256i _r, int8_t cos_bit)
{
    (void)cos_bit;
    const __m256i cospi_m04_p60 = pair_set_w16_epi16(-cospi[4], cospi[60]);
    const __m256i cospi_p60_p04 = pair_set_w16_epi16(cospi[60], cospi[4]);
    const __m256i cospi_m60_m04 = pair_set_w16_epi16(-cospi[60], -cospi[4]);
    const __m256i cospi_m36_p28 = pair_set_w16_epi16(-cospi[36], cospi[28]);
    const __m256i cospi_p28_p36 = pair_set_w16_epi16(cospi[28], cospi[36]);
    const __m256i cospi_m28_m36 = pair_set_w16_epi16(-cospi[28], -cospi[36]);
    const __m256i cospi_m20_p44 = pair_set_w16_epi16(-cospi[20], cospi[44]);
    const __m256i cospi_p44_p20 = pair_set_w16_epi16(cospi[44], cospi[20]);
    const __m256i cospi_m44_m20 = pair_set_w16_epi16(-cospi[44], -cospi[20]);
    const __m256i cospi_m52_p12 = pair_set_w16_epi16(-cospi[52], cospi[12]);
    const __m256i cospi_p12_p52 = pair_set_w16_epi16(cospi[12], cospi[52]);
    const __m256i cospi_m12_m52 = pair_set_w16_epi16(-cospi[12], -cospi[52]);
    btf_16_w16_avx2(cospi_m04_p60, cospi_p60_p04, &x[33], &x[62], _r, cos_bit);
    btf_16_w16_avx2(cospi_m60_m04, cospi_m04_p60, &x[34], &x[61], _r, cos_bit);
    btf_16_w16_avx2(cospi_m36_p28, cospi_p28_p36, &x[37], &x[58], _r, cos_bit);
    btf_16_w16_avx2(cospi_m28_m36, cospi_m36_p28, &x[38], &x[57], _r, cos_bit);
    btf_16_w16_avx2(cospi_m20_p44, cospi_p44_p20, &x[41], &x[54], _r, cos_bit);
    btf_16_w16_avx2(cospi_m44_m20, cospi_m20_p44, &x[42], &x[53], _r, cos_bit);
    btf_16_w16_avx2(cospi_m52_p12, cospi_p12_p52, &x[45], &x[50], _r, cos_bit);
    btf_16_w16_avx2(cospi_m12_m52, cospi_m52_p12, &x[46], &x[49], _r, cos_bit);
}

static inline void idct64_stage5_high48_avx2(__m256i *x, const int32_t *cospi,
        const __m256i _r, int8_t cos_bit)
{
    (void)cos_bit;
    const __m256i cospi_m08_p56 = pair_set_w16_epi16(-cospi[8], cospi[56]);
    const __m256i cospi_p56_p08 = pair_set_w16_epi16(cospi[56], cospi[8]);
    const __m256i cospi_m56_m08 = pair_set_w16_epi16(-cospi[56], -cospi[8]);
    const __m256i cospi_m40_p24 = pair_set_w16_epi16(-cospi[40], cospi[24]);
    const __m256i cospi_p24_p40 = pair_set_w16_epi16(cospi[24], cospi[40]);
    const __m256i cospi_m24_m40 = pair_set_w16_epi16(-cospi[24], -cospi[40]);
    btf_16_w16_avx2(cospi_m08_p56, cospi_p56_p08, &x[17], &x[30], _r, cos_bit);
    btf_16_w16_avx2(cospi_m56_m08, cospi_m08_p56, &x[18], &x[29], _r, cos_bit);
    btf_16_w16_avx2(cospi_m40_p24, cospi_p24_p40, &x[21], &x[26], _r, cos_bit);
    btf_16_w16_avx2(cospi_m24_m40, cospi_m40_p24, &x[22], &x[25], _r, cos_bit);
    btf_16_adds_subs_avx2(&x[32], &x[35]);
    btf_16_adds_subs_avx2(&x[33], &x[34]);
    btf_16_adds_subs_avx2(&x[39], &x[36]);
    btf_16_adds_subs_avx2(&x[38], &x[37]);
    btf_16_adds_subs_avx2(&x[40], &x[43]);
    btf_16_adds_subs_avx2(&x[41], &x[42]);
    btf_16_adds_subs_avx2(&x[47], &x[44]);
    btf_16_adds_subs_avx2(&x[46], &x[45]);
    btf_16_adds_subs_avx2(&x[48], &x[51]);
    btf_16_adds_subs_avx2(&x[49], &x[50]);
    btf_16_adds_subs_avx2(&x[55], &x[52]);
    btf_16_adds_subs_avx2(&x[54], &x[53]);
    btf_16_adds_subs_avx2(&x[56], &x[59]);
    btf_16_adds_subs_avx2(&x[57], &x[58]);
    btf_16_adds_subs_avx2(&x[63], &x[60]);
    btf_16_adds_subs_avx2(&x[62], &x[61]);
}

static inline void idct64_stage6_high32_avx2(__m256i *x, const int32_t *cospi,
        const __m256i _r, int8_t cos_bit)
{
    (void)cos_bit;
    const __m256i cospi_m08_p56 = pair_set_w16_epi16(-cospi[8], cospi[56]);
    const __m256i cospi_p56_p08 = pair_set_w16_epi16(cospi[56], cospi[8]);
    const __m256i cospi_m56_m08 = pair_set_w16_epi16(-cospi[56], -cospi[8]);
    const __m256i cospi_m40_p24 = pair_set_w16_epi16(-cospi[40], cospi[24]);
    const __m256i cospi_p24_p40 = pair_set_w16_epi16(cospi[24], cospi[40]);
    const __m256i cospi_m24_m40 = pair_set_w16_epi16(-cospi[24], -cospi[40]);
    btf_16_w16_avx2(cospi_m08_p56, cospi_p56_p08, &x[34], &x[61], _r, cos_bit);
    btf_16_w16_avx2(cospi_m08_p56, cospi_p56_p08, &x[35], &x[60], _r, cos_bit);
    btf_16_w16_avx2(cospi_m56_m08, cospi_m08_p56, &x[36], &x[59], _r, cos_bit);
    btf_16_w16_avx2(cospi_m56_m08, cospi_m08_p56, &x[37], &x[58], _r, cos_bit);
    btf_16_w16_avx2(cospi_m40_p24, cospi_p24_p40, &x[42], &x[53], _r, cos_bit);
    btf_16_w16_avx2(cospi_m40_p24, cospi_p24_p40, &x[43], &x[52], _r, cos_bit);
    btf_16_w16_avx2(cospi_m24_m40, cospi_m40_p24, &x[44], &x[51], _r, cos_bit);
    btf_16_w16_avx2(cospi_m24_m40, cospi_m40_p24, &x[45], &x[50], _r, cos_bit);
}

static inline void idct64_stage6_high48_avx2(__m256i *x, const int32_t *cospi,
        const __m256i _r, int8_t cos_bit)
{
    btf_16_adds_subs_avx2(&x[16], &x[19]);
    btf_16_adds_subs_avx2(&x[17], &x[18]);
    btf_16_adds_subs_avx2(&x[23], &x[20]);
    btf_16_adds_subs_avx2(&x[22], &x[21]);
    btf_16_adds_subs_avx2(&x[24], &x[27]);
    btf_16_adds_subs_avx2(&x[25], &x[26]);
    btf_16_adds_subs_avx2(&x[31], &x[28]);
    btf_16_adds_subs_avx2(&x[30], &x[29]);
    idct64_stage6_high32_avx2(x, cospi, _r, cos_bit);
}

static inline void idct64_stage7_high48_avx2(__m256i *x, const int32_t *cospi,
        const __m256i _r, int8_t cos_bit)
{
    (void)cos_bit;
    const __m256i cospi_m16_p48 = pair_set_w16_epi16(-cospi[16], cospi[48]);
    const __m256i cospi_p48_p16 = pair_set_w16_epi16(cospi[48], cospi[16]);
    const __m256i cospi_m48_m16 = pair_set_w16_epi16(-cospi[48], -cospi[16]);
    btf_16_w16_avx2(cospi_m16_p48, cospi_p48_p16, &x[18], &x[29], _r, cos_bit);
    btf_16_w16_avx2(cospi_m16_p48, cospi_p48_p16, &x[19], &x[28], _r, cos_bit);
    btf_16_w16_avx2(cospi_m48_m16, cospi_m16_p48, &x[20], &x[27], _r, cos_bit);
    btf_16_w16_avx2(cospi_m48_m16, cospi_m16_p48, &x[21], &x[26], _r, cos_bit);
    btf_16_adds_subs_avx2(&x[32], &x[39]);
    btf_16_adds_subs_avx2(&x[33], &x[38]);
    btf_16_adds_subs_avx2(&x[34], &x[37]);
    btf_16_adds_subs_avx2(&x[35], &x[36]);
    btf_16_adds_subs_avx2(&x[47], &x[40]);
    btf_16_adds_subs_avx2(&x[46], &x[41]);
    btf_16_adds_subs_avx2(&x[45], &x[42]);
    btf_16_adds_subs_avx2(&x[44], &x[43]);
    btf_16_adds_subs_avx2(&x[48], &x[55]);
    btf_16_adds_subs_avx2(&x[49], &x[54]);
    btf_16_adds_subs_avx2(&x[50], &x[53]);
    btf_16_adds_subs_avx2(&x[51], &x[52]);
    btf_16_adds_subs_avx2(&x[63], &x[56]);
    btf_16_adds_subs_avx2(&x[62], &x[57]);
    btf_16_adds_subs_avx2(&x[61], &x[58]);
    btf_16_adds_subs_avx2(&x[60], &x[59]);
}

static inline void idct64_stage8_high48_avx2(__m256i *x, const int32_t *cospi,
        const __m256i _r, int8_t cos_bit)
{
    (void)cos_bit;
    const __m256i cospi_m16_p48 = pair_set_w16_epi16(-cospi[16], cospi[48]);
    const __m256i cospi_p48_p16 = pair_set_w16_epi16(cospi[48], cospi[16]);
    const __m256i cospi_m48_m16 = pair_set_w16_epi16(-cospi[48], -cospi[16]);
    btf_16_adds_subs_avx2(&x[16], &x[23]);
    btf_16_adds_subs_avx2(&x[17], &x[22]);
    btf_16_adds_subs_avx2(&x[18], &x[21]);
    btf_16_adds_subs_avx2(&x[19], &x[20]);
    btf_16_adds_subs_avx2(&x[31], &x[24]);
    btf_16_adds_subs_avx2(&x[30], &x[25]);
    btf_16_adds_subs_avx2(&x[29], &x[26]);
    btf_16_adds_subs_avx2(&x[28], &x[27]);
    btf_16_w16_avx2(cospi_m16_p48, cospi_p48_p16, &x[36], &x[59], _r, cos_bit);
    btf_16_w16_avx2(cospi_m16_p48, cospi_p48_p16, &x[37], &x[58], _r, cos_bit);
    btf_16_w16_avx2(cospi_m16_p48, cospi_p48_p16, &x[38], &x[57], _r, cos_bit);
    btf_16_w16_avx2(cospi_m16_p48, cospi_p48_p16, &x[39], &x[56], _r, cos_bit);
    btf_16_w16_avx2(cospi_m48_m16, cospi_m16_p48, &x[40], &x[55], _r, cos_bit);
    btf_16_w16_avx2(cospi_m48_m16, cospi_m16_p48, &x[41], &x[54], _r, cos_bit);
    btf_16_w16_avx2(cospi_m48_m16, cospi_m16_p48, &x[42], &x[53], _r, cos_bit);
    btf_16_w16_avx2(cospi_m48_m16, cospi_m16_p48, &x[43], &x[52], _r, cos_bit);
}

static inline void idct64_stage9_avx2(__m256i *x, const int32_t *cospi,
                                      const __m256i _r, int8_t cos_bit)
{
    (void)cos_bit;
    const __m256i cospi_m32_p32 = pair_set_w16_epi16(-cospi[32], cospi[32]);
    const __m256i cospi_p32_p32 = pair_set_w16_epi16(cospi[32], cospi[32]);
    btf_16_adds_subs_avx2(&x[0], &x[15]);
    btf_16_adds_subs_avx2(&x[1], &x[14]);
    btf_16_adds_subs_avx2(&x[2], &x[13]);
    btf_16_adds_subs_avx2(&x[3], &x[12]);
    btf_16_adds_subs_avx2(&x[4], &x[11]);
    btf_16_adds_subs_avx2(&x[5], &x[10]);
    btf_16_adds_subs_avx2(&x[6], &x[9]);
    btf_16_adds_subs_avx2(&x[7], &x[8]);
    btf_16_w16_avx2(cospi_m32_p32, cospi_p32_p32, &x[20], &x[27], _r, cos_bit);
    btf_16_w16_avx2(cospi_m32_p32, cospi_p32_p32, &x[21], &x[26], _r, cos_bit);
    btf_16_w16_avx2(cospi_m32_p32, cospi_p32_p32, &x[22], &x[25], _r, cos_bit);
    btf_16_w16_avx2(cospi_m32_p32, cospi_p32_p32, &x[23], &x[24], _r, cos_bit);
    btf_16_adds_subs_avx2(&x[32], &x[47]);
    btf_16_adds_subs_avx2(&x[33], &x[46]);
    btf_16_adds_subs_avx2(&x[34], &x[45]);
    btf_16_adds_subs_avx2(&x[35], &x[44]);
    btf_16_adds_subs_avx2(&x[36], &x[43]);
    btf_16_adds_subs_avx2(&x[37], &x[42]);
    btf_16_adds_subs_avx2(&x[38], &x[41]);
    btf_16_adds_subs_avx2(&x[39], &x[40]);
    btf_16_adds_subs_avx2(&x[63], &x[48]);
    btf_16_adds_subs_avx2(&x[62], &x[49]);
    btf_16_adds_subs_avx2(&x[61], &x[50]);
    btf_16_adds_subs_avx2(&x[60], &x[51]);
    btf_16_adds_subs_avx2(&x[59], &x[52]);
    btf_16_adds_subs_avx2(&x[58], &x[53]);
    btf_16_adds_subs_avx2(&x[57], &x[54]);
    btf_16_adds_subs_avx2(&x[56], &x[55]);
}

static inline void idct64_stage10_avx2(__m256i *x, const int32_t *cospi,
                                       const __m256i _r, int8_t cos_bit)
{
    (void)cos_bit;
    const __m256i cospi_m32_p32 = pair_set_w16_epi16(-cospi[32], cospi[32]);
    const __m256i cospi_p32_p32 = pair_set_w16_epi16(cospi[32], cospi[32]);
    btf_16_adds_subs_avx2(&x[0], &x[31]);
    btf_16_adds_subs_avx2(&x[1], &x[30]);
    btf_16_adds_subs_avx2(&x[2], &x[29]);
    btf_16_adds_subs_avx2(&x[3], &x[28]);
    btf_16_adds_subs_avx2(&x[4], &x[27]);
    btf_16_adds_subs_avx2(&x[5], &x[26]);
    btf_16_adds_subs_avx2(&x[6], &x[25]);
    btf_16_adds_subs_avx2(&x[7], &x[24]);
    btf_16_adds_subs_avx2(&x[8], &x[23]);
    btf_16_adds_subs_avx2(&x[9], &x[22]);
    btf_16_adds_subs_avx2(&x[10], &x[21]);
    btf_16_adds_subs_avx2(&x[11], &x[20]);
    btf_16_adds_subs_avx2(&x[12], &x[19]);
    btf_16_adds_subs_avx2(&x[13], &x[18]);
    btf_16_adds_subs_avx2(&x[14], &x[17]);
    btf_16_adds_subs_avx2(&x[15], &x[16]);
    btf_16_w16_avx2(cospi_m32_p32, cospi_p32_p32, &x[40], &x[55], _r, cos_bit);
    btf_16_w16_avx2(cospi_m32_p32, cospi_p32_p32, &x[41], &x[54], _r, cos_bit);
    btf_16_w16_avx2(cospi_m32_p32, cospi_p32_p32, &x[42], &x[53], _r, cos_bit);
    btf_16_w16_avx2(cospi_m32_p32, cospi_p32_p32, &x[43], &x[52], _r, cos_bit);
    btf_16_w16_avx2(cospi_m32_p32, cospi_p32_p32, &x[44], &x[51], _r, cos_bit);
    btf_16_w16_avx2(cospi_m32_p32, cospi_p32_p32, &x[45], &x[50], _r, cos_bit);
    btf_16_w16_avx2(cospi_m32_p32, cospi_p32_p32, &x[46], &x[49], _r, cos_bit);
    btf_16_w16_avx2(cospi_m32_p32, cospi_p32_p32, &x[47], &x[48], _r, cos_bit);
}

static inline void idct64_stage11_avx2(__m256i *output, __m256i *x)
{
    btf_16_adds_subs_out_avx2(&output[0], &output[63], x[0], x[63]);
    btf_16_adds_subs_out_avx2(&output[1], &output[62], x[1], x[62]);
    btf_16_adds_subs_out_avx2(&output[2], &output[61], x[2], x[61]);
    btf_16_adds_subs_out_avx2(&output[3], &output[60], x[3], x[60]);
    btf_16_adds_subs_out_avx2(&output[4], &output[59], x[4], x[59]);
    btf_16_adds_subs_out_avx2(&output[5], &output[58], x[5], x[58]);
    btf_16_adds_subs_out_avx2(&output[6], &output[57], x[6], x[57]);
    btf_16_adds_subs_out_avx2(&output[7], &output[56], x[7], x[56]);
    btf_16_adds_subs_out_avx2(&output[8], &output[55], x[8], x[55]);
    btf_16_adds_subs_out_avx2(&output[9], &output[54], x[9], x[54]);
    btf_16_adds_subs_out_avx2(&output[10], &output[53], x[10], x[53]);
    btf_16_adds_subs_out_avx2(&output[11], &output[52], x[11], x[52]);
    btf_16_adds_subs_out_avx2(&output[12], &output[51], x[12], x[51]);
    btf_16_adds_subs_out_avx2(&output[13], &output[50], x[13], x[50]);
    btf_16_adds_subs_out_avx2(&output[14], &output[49], x[14], x[49]);
    btf_16_adds_subs_out_avx2(&output[15], &output[48], x[15], x[48]);
    btf_16_adds_subs_out_avx2(&output[16], &output[47], x[16], x[47]);
    btf_16_adds_subs_out_avx2(&output[17], &output[46], x[17], x[46]);
    btf_16_adds_subs_out_avx2(&output[18], &output[45], x[18], x[45]);
    btf_16_adds_subs_out_avx2(&output[19], &output[44], x[19], x[44]);
    btf_16_adds_subs_out_avx2(&output[20], &output[43], x[20], x[43]);
    btf_16_adds_subs_out_avx2(&output[21], &output[42], x[21], x[42]);
    btf_16_adds_subs_out_avx2(&output[22], &output[41], x[22], x[41]);
    btf_16_adds_subs_out_avx2(&output[23], &output[40], x[23], x[40]);
    btf_16_adds_subs_out_avx2(&output[24], &output[39], x[24], x[39]);
    btf_16_adds_subs_out_avx2(&output[25], &output[38], x[25], x[38]);
    btf_16_adds_subs_out_avx2(&output[26], &output[37], x[26], x[37]);
    btf_16_adds_subs_out_avx2(&output[27], &output[36], x[27], x[36]);
    btf_16_adds_subs_out_avx2(&output[28], &output[35], x[28], x[35]);
    btf_16_adds_subs_out_avx2(&output[29], &output[34], x[29], x[34]);
    btf_16_adds_subs_out_avx2(&output[30], &output[33], x[30], x[33]);
    btf_16_adds_subs_out_avx2(&output[31], &output[32], x[31], x[32]);
}

void Xin265pIdct16_AVX2 (
    const SINT16 *input,
    SINT16       *output,
    SINT8        cosBit)
{
    const SINT32  *cosPi;
    __m256i cospi_p60_m04;
    __m256i cospi_p04_p60;
    __m256i cospi_p28_m36;
    __m256i cospi_p36_p28;
    __m256i cospi_p44_m20;
    __m256i cospi_p20_p44;
    __m256i cospi_p12_m52;
    __m256i cospi_p52_p12;
    __m256i cospi_p56_m08;
    __m256i cospi_p08_p56;
    __m256i cospi_p24_m40;
    __m256i cospi_p40_p24;
    __m256i cospi_p32_p32;
    __m256i cospi_p32_m32;
    __m256i cospi_p48_m16;
    __m256i cospi_p16_p48;
    __m256i cospi_m16_p48;
    __m256i cospi_p48_p16;
    __m256i cospi_m48_m16;
    __m256i _r;
    __m256i x1[16];
    __m256i *input256;
    __m256i *output256;

    (void)(cosBit);
    cosPi         = XinGetCosPi (XIN_INV_COS_BIT);
    _r            = _mm256_set1_epi32(1 << (XIN_INV_COS_BIT - 1));
    cospi_p60_m04 = pair_set_w16_epi16(cosPi[60], -cosPi[4]);
    cospi_p04_p60 = pair_set_w16_epi16(cosPi[4], cosPi[60]);
    cospi_p28_m36 = pair_set_w16_epi16(cosPi[28], -cosPi[36]);
    cospi_p36_p28 = pair_set_w16_epi16(cosPi[36], cosPi[28]);
    cospi_p44_m20 = pair_set_w16_epi16(cosPi[44], -cosPi[20]);
    cospi_p20_p44 = pair_set_w16_epi16(cosPi[20], cosPi[44]);
    cospi_p12_m52 = pair_set_w16_epi16(cosPi[12], -cosPi[52]);
    cospi_p52_p12 = pair_set_w16_epi16(cosPi[52], cosPi[12]);
    cospi_p56_m08 = pair_set_w16_epi16(cosPi[56], -cosPi[8]);
    cospi_p08_p56 = pair_set_w16_epi16(cosPi[8], cosPi[56]);
    cospi_p24_m40 = pair_set_w16_epi16(cosPi[24], -cosPi[40]);
    cospi_p40_p24 = pair_set_w16_epi16(cosPi[40], cosPi[24]);
    cospi_p32_p32 = pair_set_w16_epi16(cosPi[32], cosPi[32]);
    cospi_p32_m32 = pair_set_w16_epi16(cosPi[32], -cosPi[32]);
    cospi_p48_m16 = pair_set_w16_epi16(cosPi[48], -cosPi[16]);
    cospi_p16_p48 = pair_set_w16_epi16(cosPi[16], cosPi[48]);
    cospi_m16_p48 = pair_set_w16_epi16(-cosPi[16], cosPi[48]);
    cospi_p48_p16 = pair_set_w16_epi16(cosPi[48], cosPi[16]);
    cospi_m48_m16 = pair_set_w16_epi16(-cosPi[48], -cosPi[16]);
    input256      = (__m256i *)input;
    output256     = (__m256i *)output;

    // stage 1
    x1[0] = input256[0];
    x1[1] = input256[8];
    x1[2] = input256[4];
    x1[3] = input256[12];
    x1[4] = input256[2];
    x1[5] = input256[10];
    x1[6] = input256[6];
    x1[7] = input256[14];
    x1[8] = input256[1];
    x1[9] = input256[9];
    x1[10] = input256[5];
    x1[11] = input256[13];
    x1[12] = input256[3];
    x1[13] = input256[11];
    x1[14] = input256[7];
    x1[15] = input256[15];

    // stage 2
    btf_16_w16_avx2(cospi_p60_m04, cospi_p04_p60, &x1[8], &x1[15], _r, cosBit);
    btf_16_w16_avx2(cospi_p28_m36, cospi_p36_p28, &x1[9], &x1[14], _r, cosBit);
    btf_16_w16_avx2(cospi_p44_m20, cospi_p20_p44, &x1[10], &x1[13], _r, cosBit);
    btf_16_w16_avx2(cospi_p12_m52, cospi_p52_p12, &x1[11], &x1[12], _r, cosBit);

    // stage 3
    btf_16_w16_avx2(cospi_p56_m08, cospi_p08_p56, &x1[4], &x1[7], _r, cosBit);
    btf_16_w16_avx2(cospi_p24_m40, cospi_p40_p24, &x1[5], &x1[6], _r, cosBit);
    btf_16_adds_subs_avx2(&x1[8], &x1[9]);
    btf_16_adds_subs_avx2(&x1[11], &x1[10]);
    btf_16_adds_subs_avx2(&x1[12], &x1[13]);
    btf_16_adds_subs_avx2(&x1[15], &x1[14]);

    // stage 4
    btf_16_w16_avx2(cospi_p32_p32, cospi_p32_m32, &x1[0], &x1[1], _r, cosBit);
    btf_16_w16_avx2(cospi_p48_m16, cospi_p16_p48, &x1[2], &x1[3], _r, cosBit);
    btf_16_adds_subs_avx2(&x1[4], &x1[5]);
    btf_16_adds_subs_avx2(&x1[7], &x1[6]);
    btf_16_w16_avx2(cospi_m16_p48, cospi_p48_p16, &x1[9], &x1[14], _r, cosBit);
    btf_16_w16_avx2(cospi_m48_m16, cospi_m16_p48, &x1[10], &x1[13], _r, cosBit);

    idct16_stage5_avx2(x1, cosPi, _r, cosBit);
    idct16_stage6_avx2(x1, cosPi, _r, cosBit);
    idct16_stage7_avx2(output256, x1);

}

void Xin265pIadst16_AVX2 (
    const SINT16 *in,
    SINT16       *out,
    SINT8        cos_bit)
{
    (void)(cos_bit);
    const int32_t *cospi = XinGetCosPi(XIN_INV_COS_BIT);

    const __m256i _r = _mm256_set1_epi32(1 << (XIN_INV_COS_BIT - 1));

    __m256i *input  = (__m256i *)in;
    __m256i *output = (__m256i *)out;
    __m256i cospi_p02_p62 = pair_set_w16_epi16(cospi[2], cospi[62]);
    __m256i cospi_p62_m02 = pair_set_w16_epi16(cospi[62], -cospi[2]);
    __m256i cospi_p10_p54 = pair_set_w16_epi16(cospi[10], cospi[54]);
    __m256i cospi_p54_m10 = pair_set_w16_epi16(cospi[54], -cospi[10]);
    __m256i cospi_p18_p46 = pair_set_w16_epi16(cospi[18], cospi[46]);
    __m256i cospi_p46_m18 = pair_set_w16_epi16(cospi[46], -cospi[18]);
    __m256i cospi_p26_p38 = pair_set_w16_epi16(cospi[26], cospi[38]);
    __m256i cospi_p38_m26 = pair_set_w16_epi16(cospi[38], -cospi[26]);
    __m256i cospi_p34_p30 = pair_set_w16_epi16(cospi[34], cospi[30]);
    __m256i cospi_p30_m34 = pair_set_w16_epi16(cospi[30], -cospi[34]);
    __m256i cospi_p42_p22 = pair_set_w16_epi16(cospi[42], cospi[22]);
    __m256i cospi_p22_m42 = pair_set_w16_epi16(cospi[22], -cospi[42]);
    __m256i cospi_p50_p14 = pair_set_w16_epi16(cospi[50], cospi[14]);
    __m256i cospi_p14_m50 = pair_set_w16_epi16(cospi[14], -cospi[50]);
    __m256i cospi_p58_p06 = pair_set_w16_epi16(cospi[58], cospi[6]);
    __m256i cospi_p06_m58 = pair_set_w16_epi16(cospi[6], -cospi[58]);

    // stage 1
    __m256i x1[16];
    x1[0] = input[15];
    x1[1] = input[0];
    x1[2] = input[13];
    x1[3] = input[2];
    x1[4] = input[11];
    x1[5] = input[4];
    x1[6] = input[9];
    x1[7] = input[6];
    x1[8] = input[7];
    x1[9] = input[8];
    x1[10] = input[5];
    x1[11] = input[10];
    x1[12] = input[3];
    x1[13] = input[12];
    x1[14] = input[1];
    x1[15] = input[14];

    // stage 2
    btf_16_w16_avx2(cospi_p02_p62, cospi_p62_m02, &x1[0], &x1[1], _r, cos_bit);
    btf_16_w16_avx2(cospi_p10_p54, cospi_p54_m10, &x1[2], &x1[3], _r, cos_bit);
    btf_16_w16_avx2(cospi_p18_p46, cospi_p46_m18, &x1[4], &x1[5], _r, cos_bit);
    btf_16_w16_avx2(cospi_p26_p38, cospi_p38_m26, &x1[6], &x1[7], _r, cos_bit);
    btf_16_w16_avx2(cospi_p34_p30, cospi_p30_m34, &x1[8], &x1[9], _r, cos_bit);
    btf_16_w16_avx2(cospi_p42_p22, cospi_p22_m42, &x1[10], &x1[11], _r, cos_bit);
    btf_16_w16_avx2(cospi_p50_p14, cospi_p14_m50, &x1[12], &x1[13], _r, cos_bit);
    btf_16_w16_avx2(cospi_p58_p06, cospi_p06_m58, &x1[14], &x1[15], _r, cos_bit);

    iadst16_stage3_avx2(x1);
    iadst16_stage4_avx2(x1, cospi, _r, cos_bit);
    iadst16_stage5_avx2(x1);
    iadst16_stage6_avx2(x1, cospi, _r, cos_bit);
    iadst16_stage7_avx2(x1);
    iadst16_stage8_avx2(x1, cospi, _r, cos_bit);
    iadst16_stage9_avx2(output, x1);
}

void Xin265pIdct32_AVX2 (
    const SINT16 *in,
    SINT16       *out,
    SINT8        cos_bit)
{
    (void)(cos_bit);
    const int32_t *cospi = XinGetCosPi(XIN_INV_COS_BIT);
    const __m256i _r = _mm256_set1_epi32(1 << (XIN_INV_COS_BIT - 1));

    __m256i *input  = (__m256i *)in;
    __m256i *output = (__m256i *)out;

    __m256i cospi_p62_m02 = pair_set_w16_epi16(cospi[62], -cospi[2]);
    __m256i cospi_p02_p62 = pair_set_w16_epi16(cospi[2], cospi[62]);
    __m256i cospi_p30_m34 = pair_set_w16_epi16(cospi[30], -cospi[34]);
    __m256i cospi_p34_p30 = pair_set_w16_epi16(cospi[34], cospi[30]);
    __m256i cospi_p46_m18 = pair_set_w16_epi16(cospi[46], -cospi[18]);
    __m256i cospi_p18_p46 = pair_set_w16_epi16(cospi[18], cospi[46]);
    __m256i cospi_p14_m50 = pair_set_w16_epi16(cospi[14], -cospi[50]);
    __m256i cospi_p50_p14 = pair_set_w16_epi16(cospi[50], cospi[14]);
    __m256i cospi_p54_m10 = pair_set_w16_epi16(cospi[54], -cospi[10]);
    __m256i cospi_p10_p54 = pair_set_w16_epi16(cospi[10], cospi[54]);
    __m256i cospi_p22_m42 = pair_set_w16_epi16(cospi[22], -cospi[42]);
    __m256i cospi_p42_p22 = pair_set_w16_epi16(cospi[42], cospi[22]);
    __m256i cospi_p38_m26 = pair_set_w16_epi16(cospi[38], -cospi[26]);
    __m256i cospi_p26_p38 = pair_set_w16_epi16(cospi[26], cospi[38]);
    __m256i cospi_p06_m58 = pair_set_w16_epi16(cospi[6], -cospi[58]);
    __m256i cospi_p58_p06 = pair_set_w16_epi16(cospi[58], cospi[6]);
    __m256i cospi_p60_m04 = pair_set_w16_epi16(cospi[60], -cospi[4]);
    __m256i cospi_p04_p60 = pair_set_w16_epi16(cospi[4], cospi[60]);
    __m256i cospi_p28_m36 = pair_set_w16_epi16(cospi[28], -cospi[36]);
    __m256i cospi_p36_p28 = pair_set_w16_epi16(cospi[36], cospi[28]);
    __m256i cospi_p44_m20 = pair_set_w16_epi16(cospi[44], -cospi[20]);
    __m256i cospi_p20_p44 = pair_set_w16_epi16(cospi[20], cospi[44]);
    __m256i cospi_p12_m52 = pair_set_w16_epi16(cospi[12], -cospi[52]);
    __m256i cospi_p52_p12 = pair_set_w16_epi16(cospi[52], cospi[12]);
    __m256i cospi_p56_m08 = pair_set_w16_epi16(cospi[56], -cospi[8]);
    __m256i cospi_p08_p56 = pair_set_w16_epi16(cospi[8], cospi[56]);
    __m256i cospi_p24_m40 = pair_set_w16_epi16(cospi[24], -cospi[40]);
    __m256i cospi_p40_p24 = pair_set_w16_epi16(cospi[40], cospi[24]);
    __m256i cospi_p32_p32 = pair_set_w16_epi16(cospi[32], cospi[32]);
    __m256i cospi_p32_m32 = pair_set_w16_epi16(cospi[32], -cospi[32]);
    __m256i cospi_p48_m16 = pair_set_w16_epi16(cospi[48], -cospi[16]);
    __m256i cospi_p16_p48 = pair_set_w16_epi16(cospi[16], cospi[48]);

    // stage 1
    __m256i x1[32];
    x1[0] = input[0];
    x1[1] = input[16];
    x1[2] = input[8];
    x1[3] = input[24];
    x1[4] = input[4];
    x1[5] = input[20];
    x1[6] = input[12];
    x1[7] = input[28];
    x1[8] = input[2];
    x1[9] = input[18];
    x1[10] = input[10];
    x1[11] = input[26];
    x1[12] = input[6];
    x1[13] = input[22];
    x1[14] = input[14];
    x1[15] = input[30];
    x1[16] = input[1];
    x1[17] = input[17];
    x1[18] = input[9];
    x1[19] = input[25];
    x1[20] = input[5];
    x1[21] = input[21];
    x1[22] = input[13];
    x1[23] = input[29];
    x1[24] = input[3];
    x1[25] = input[19];
    x1[26] = input[11];
    x1[27] = input[27];
    x1[28] = input[7];
    x1[29] = input[23];
    x1[30] = input[15];
    x1[31] = input[31];

    // stage 2
    btf_16_w16_avx2(cospi_p62_m02, cospi_p02_p62, &x1[16], &x1[31], _r, cos_bit);
    btf_16_w16_avx2(cospi_p30_m34, cospi_p34_p30, &x1[17], &x1[30], _r, cos_bit);
    btf_16_w16_avx2(cospi_p46_m18, cospi_p18_p46, &x1[18], &x1[29], _r, cos_bit);
    btf_16_w16_avx2(cospi_p14_m50, cospi_p50_p14, &x1[19], &x1[28], _r, cos_bit);
    btf_16_w16_avx2(cospi_p54_m10, cospi_p10_p54, &x1[20], &x1[27], _r, cos_bit);
    btf_16_w16_avx2(cospi_p22_m42, cospi_p42_p22, &x1[21], &x1[26], _r, cos_bit);
    btf_16_w16_avx2(cospi_p38_m26, cospi_p26_p38, &x1[22], &x1[25], _r, cos_bit);
    btf_16_w16_avx2(cospi_p06_m58, cospi_p58_p06, &x1[23], &x1[24], _r, cos_bit);

    // stage 3
    btf_16_w16_avx2(cospi_p60_m04, cospi_p04_p60, &x1[8], &x1[15], _r, cos_bit);
    btf_16_w16_avx2(cospi_p28_m36, cospi_p36_p28, &x1[9], &x1[14], _r, cos_bit);
    btf_16_w16_avx2(cospi_p44_m20, cospi_p20_p44, &x1[10], &x1[13], _r, cos_bit);
    btf_16_w16_avx2(cospi_p12_m52, cospi_p52_p12, &x1[11], &x1[12], _r, cos_bit);
    idct32_high16_stage3_avx2(x1);

    // stage 4
    btf_16_w16_avx2(cospi_p56_m08, cospi_p08_p56, &x1[4], &x1[7], _r, cos_bit);
    btf_16_w16_avx2(cospi_p24_m40, cospi_p40_p24, &x1[5], &x1[6], _r, cos_bit);
    btf_16_adds_subs_avx2(&x1[8], &x1[9]);
    btf_16_adds_subs_avx2(&x1[11], &x1[10]);
    btf_16_adds_subs_avx2(&x1[12], &x1[13]);
    btf_16_adds_subs_avx2(&x1[15], &x1[14]);
    idct32_high16_stage4_avx2(x1, cospi, _r, cos_bit);

    // stage 5
    btf_16_w16_avx2(cospi_p32_p32, cospi_p32_m32, &x1[0], &x1[1], _r, cos_bit);
    btf_16_w16_avx2(cospi_p48_m16, cospi_p16_p48, &x1[2], &x1[3], _r, cos_bit);
    btf_16_adds_subs_avx2(&x1[4], &x1[5]);
    btf_16_adds_subs_avx2(&x1[7], &x1[6]);
    idct32_high24_stage5_avx2(x1, cospi, _r, cos_bit);

    // stage 6
    btf_16_adds_subs_avx2(&x1[0], &x1[3]);
    btf_16_adds_subs_avx2(&x1[1], &x1[2]);
    idct32_high28_stage6_avx2(x1, cospi, _r, cos_bit);

    idct32_stage7_avx2(x1, cospi, _r, cos_bit);
    idct32_stage8_avx2(x1, cospi, _r, cos_bit);
    idct32_stage9_avx2(output, x1);

}

void Xin265pIdct64_AVX2 (
    const SINT16 *in,
    SINT16       *out,
    SINT8        cos_bit)
{
    (void)cos_bit;
    const int32_t *cospi = XinGetCosPi(XIN_INV_COS_BIT);
    const __m256i _r = _mm256_set1_epi32(1 << (XIN_INV_COS_BIT - 1));

    const __m256i cospi_p32_p32 = pair_set_w16_epi16(cospi[32], cospi[32]);
    const __m256i cospi_m16_p48 = pair_set_w16_epi16(-cospi[16], cospi[48]);
    const __m256i cospi_p48_p16 = pair_set_w16_epi16(cospi[48], cospi[16]);
    const __m256i cospi_m48_m16 = pair_set_w16_epi16(-cospi[48], -cospi[16]);
    const __m256i cospi_m32_p32 = pair_set_w16_epi16(-cospi[32], cospi[32]);

    __m256i *input = (__m256i *)in;
    __m256i *output = (__m256i *)out;

    // stage 1
    __m256i x[64];
    x[0] = input[0];
    x[2] = input[16];
    x[4] = input[8];
    x[6] = input[24];
    x[8] = input[4];
    x[10] = input[20];
    x[12] = input[12];
    x[14] = input[28];
    x[16] = input[2];
    x[18] = input[18];
    x[20] = input[10];
    x[22] = input[26];
    x[24] = input[6];
    x[26] = input[22];
    x[28] = input[14];
    x[30] = input[30];
    x[32] = input[1];
    x[34] = input[17];
    x[36] = input[9];
    x[38] = input[25];
    x[40] = input[5];
    x[42] = input[21];
    x[44] = input[13];
    x[46] = input[29];
    x[48] = input[3];
    x[50] = input[19];
    x[52] = input[11];
    x[54] = input[27];
    x[56] = input[7];
    x[58] = input[23];
    x[60] = input[15];
    x[62] = input[31];

    // stage 2
    btf_16_w16_0_avx2(cospi[63], cospi[1], x[32], x[32], x[63]);
    btf_16_w16_0_avx2(-cospi[33], cospi[31], x[62], x[33], x[62]);
    btf_16_w16_0_avx2(cospi[47], cospi[17], x[34], x[34], x[61]);
    btf_16_w16_0_avx2(-cospi[49], cospi[15], x[60], x[35], x[60]);
    btf_16_w16_0_avx2(cospi[55], cospi[9], x[36], x[36], x[59]);
    btf_16_w16_0_avx2(-cospi[41], cospi[23], x[58], x[37], x[58]);
    btf_16_w16_0_avx2(cospi[39], cospi[25], x[38], x[38], x[57]);
    btf_16_w16_0_avx2(-cospi[57], cospi[7], x[56], x[39], x[56]);
    btf_16_w16_0_avx2(cospi[59], cospi[5], x[40], x[40], x[55]);
    btf_16_w16_0_avx2(-cospi[37], cospi[27], x[54], x[41], x[54]);
    btf_16_w16_0_avx2(cospi[43], cospi[21], x[42], x[42], x[53]);
    btf_16_w16_0_avx2(-cospi[53], cospi[11], x[52], x[43], x[52]);
    btf_16_w16_0_avx2(cospi[51], cospi[13], x[44], x[44], x[51]);
    btf_16_w16_0_avx2(-cospi[45], cospi[19], x[50], x[45], x[50]);
    btf_16_w16_0_avx2(cospi[35], cospi[29], x[46], x[46], x[49]);
    btf_16_w16_0_avx2(-cospi[61], cospi[3], x[48], x[47], x[48]);

    // stage 3
    btf_16_w16_0_avx2(cospi[62], cospi[2], x[16], x[16], x[31]);
    btf_16_w16_0_avx2(-cospi[34], cospi[30], x[30], x[17], x[30]);
    btf_16_w16_0_avx2(cospi[46], cospi[18], x[18], x[18], x[29]);
    btf_16_w16_0_avx2(-cospi[50], cospi[14], x[28], x[19], x[28]);
    btf_16_w16_0_avx2(cospi[54], cospi[10], x[20], x[20], x[27]);
    btf_16_w16_0_avx2(-cospi[42], cospi[22], x[26], x[21], x[26]);
    btf_16_w16_0_avx2(cospi[38], cospi[26], x[22], x[22], x[25]);
    btf_16_w16_0_avx2(-cospi[58], cospi[6], x[24], x[23], x[24]);
    btf_16_adds_subs_avx2(&x[32], &x[33]);
    btf_16_adds_subs_avx2(&x[35], &x[34]);
    btf_16_adds_subs_avx2(&x[36], &x[37]);
    btf_16_adds_subs_avx2(&x[39], &x[38]);
    btf_16_adds_subs_avx2(&x[40], &x[41]);
    btf_16_adds_subs_avx2(&x[43], &x[42]);
    btf_16_adds_subs_avx2(&x[44], &x[45]);
    btf_16_adds_subs_avx2(&x[47], &x[46]);
    btf_16_adds_subs_avx2(&x[48], &x[49]);
    btf_16_adds_subs_avx2(&x[51], &x[50]);
    btf_16_adds_subs_avx2(&x[52], &x[53]);
    btf_16_adds_subs_avx2(&x[55], &x[54]);
    btf_16_adds_subs_avx2(&x[56], &x[57]);
    btf_16_adds_subs_avx2(&x[59], &x[58]);
    btf_16_adds_subs_avx2(&x[60], &x[61]);
    btf_16_adds_subs_avx2(&x[63], &x[62]);

    // stage 4
    btf_16_w16_0_avx2(cospi[60], cospi[4], x[8], x[8], x[15]);
    btf_16_w16_0_avx2(-cospi[36], cospi[28], x[14], x[9], x[14]);
    btf_16_w16_0_avx2(cospi[44], cospi[20], x[10], x[10], x[13]);
    btf_16_w16_0_avx2(-cospi[52], cospi[12], x[12], x[11], x[12]);
    btf_16_adds_subs_avx2(&x[16], &x[17]);
    btf_16_adds_subs_avx2(&x[19], &x[18]);
    btf_16_adds_subs_avx2(&x[20], &x[21]);
    btf_16_adds_subs_avx2(&x[23], &x[22]);
    btf_16_adds_subs_avx2(&x[24], &x[25]);
    btf_16_adds_subs_avx2(&x[27], &x[26]);
    btf_16_adds_subs_avx2(&x[28], &x[29]);
    btf_16_adds_subs_avx2(&x[31], &x[30]);
    idct64_stage4_high32_avx2(x, cospi, _r, cos_bit);

    // stage 5
    btf_16_w16_0_avx2(cospi[56], cospi[8], x[4], x[4], x[7]);
    btf_16_w16_0_avx2(-cospi[40], cospi[24], x[6], x[5], x[6]);
    btf_16_adds_subs_avx2(&x[8], &x[9]);
    btf_16_adds_subs_avx2(&x[11], &x[10]);
    btf_16_adds_subs_avx2(&x[12], &x[13]);
    btf_16_adds_subs_avx2(&x[15], &x[14]);
    idct64_stage5_high48_avx2(x, cospi, _r, cos_bit);

    // stage 6
    btf_16_w16_0_avx2(cospi[32], cospi[32], x[0], x[0], x[1]);
    btf_16_w16_0_avx2(cospi[48], cospi[16], x[2], x[2], x[3]);
    btf_16_adds_subs_avx2(&x[4], &x[5]);
    btf_16_adds_subs_avx2(&x[7], &x[6]);
    btf_16_w16_avx2(cospi_m16_p48, cospi_p48_p16, &x[9], &x[14], _r, cos_bit);
    btf_16_w16_avx2(cospi_m48_m16, cospi_m16_p48, &x[10], &x[13], _r, cos_bit);
    idct64_stage6_high48_avx2(x, cospi, _r, cos_bit);

    // stage 7
    btf_16_adds_subs_avx2(&x[0], &x[3]);
    btf_16_adds_subs_avx2(&x[1], &x[2]);
    btf_16_w16_avx2(cospi_m32_p32, cospi_p32_p32, &x[5], &x[6], _r, cos_bit);
    btf_16_adds_subs_avx2(&x[8], &x[11]);
    btf_16_adds_subs_avx2(&x[9], &x[10]);
    btf_16_adds_subs_avx2(&x[15], &x[12]);
    btf_16_adds_subs_avx2(&x[14], &x[13]);
    idct64_stage7_high48_avx2(x, cospi, _r, cos_bit);

    // stage 8
    btf_16_adds_subs_avx2(&x[0], &x[7]);
    btf_16_adds_subs_avx2(&x[1], &x[6]);
    btf_16_adds_subs_avx2(&x[2], &x[5]);
    btf_16_adds_subs_avx2(&x[3], &x[4]);
    btf_16_w16_avx2(cospi_m32_p32, cospi_p32_p32, &x[10], &x[13], _r, cos_bit);
    btf_16_w16_avx2(cospi_m32_p32, cospi_p32_p32, &x[11], &x[12], _r, cos_bit);
    idct64_stage8_high48_avx2(x, cospi, _r, cos_bit);

    // stage 9~11
    idct64_stage9_avx2(x, cospi, _r, cos_bit);
    idct64_stage10_avx2(x, cospi, _r, cos_bit);
    idct64_stage11_avx2(output, x);
}

