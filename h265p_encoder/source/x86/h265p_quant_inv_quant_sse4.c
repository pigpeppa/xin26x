/***************************************************************************//**
*
* @file          h265p_quant_inv_quant_sse4.c
* @brief         av1 forward quantization and inverse quantization.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#include "smmintrin.h"
#include "xin_typedef.h"
#include "basic_macro.h"
#include "h26x_definition.h"
#include "h265p_definition.h"

void Xin265pQuantInvQuantB4xH_SSE4 (
    COEFF    *qCoeff,
    SINT32   *tCoeff,
    SINT32   *rCoeff,
    intptr_t coeffStride,
    SINT32   width,
    SINT32   height,
    SINT32   logScale,
    SINT32   *qAdd,
    SINT32   *qMult,
    SINT32   *qzBin,
    SINT32   *qShift,
    SINT32   *iqMult,
    UINT32   *nonZeroCount)
{
    SINT32  rowIdx;
    SINT32  qCoef;
    SINT32  rCoef;
    SINT32  absCoef;
    SINT32  zBin[2];
    SINT32  add[2];
    __m128i thrx8;
    __m128i addx8;
    __m128i qMultx8;
    __m128i iqMultx8;
    __m128i shiftx8;
    __m128i coeffx4;
    __m128i coeffx8;
    __m128i magx8;
    __m128i levx8;
    __m128i recx8;
    __m128i recx4;
    __m128i maskx8;
    __m128i allZero;
    __m128i nzCx8;
    SINT32  dcCoeff;
    BOOL    zeroFlag;

    (void)width;
    dcCoeff   = tCoeff[0];
    tCoeff[0] = 0;
    zBin[0]   =  XIN_ROUND_POWER2 (qzBin[0], logScale);
    zBin[1]   =  XIN_ROUND_POWER2 (qzBin[1], logScale);
    add[0]    =  XIN_ROUND_POWER2 (qAdd[0],  logScale);
    add[1]    =  XIN_ROUND_POWER2 (qAdd[1],  logScale);
    thrx8     = _mm_set1_epi16 ((SINT16)zBin[1]);
    addx8     = _mm_set1_epi16 ((SINT16)add[1]);
    qMultx8   = _mm_set1_epi16 ((SINT16)qMult[1]);
    shiftx8   = _mm_set1_epi16 ((SINT16)(qShift[1] << logScale));
    iqMultx8  = _mm_set1_epi16 ((SINT16)(iqMult[1]));
    allZero   = _mm_setzero_si128 ();
    nzCx8     = _mm_setzero_si128 ();

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        coeffx4 = _mm_loadu_si128 ((__m128i *)(tCoeff));
        coeffx8  = _mm_packs_epi32 (coeffx4, allZero);

        magx8  = _mm_abs_epi16 (coeffx8);
        maskx8 = _mm_cmpgt_epi16(magx8, thrx8);
        maskx8 = _mm_or_si128 (maskx8, _mm_cmpeq_epi16 (magx8, thrx8));

        zeroFlag = _mm_testz_si128 (maskx8, maskx8);

        if (!zeroFlag)
        {
            magx8 = _mm_adds_epi16 (magx8, addx8);
            levx8 = _mm_add_epi16 (_mm_mulhi_epi16 (magx8, qMultx8), magx8);
            levx8 = _mm_mulhi_epi16 (levx8, shiftx8);
            levx8 = _mm_and_si128 (levx8, maskx8);
            recx8 = _mm_mullo_epi16 (levx8, iqMultx8);
            recx8 = _mm_mullo_epi16 (levx8, iqMultx8);
            recx8 = _mm_srai_epi16 (recx8, logScale);
            nzCx8 = _mm_sub_epi16(nzCx8, _mm_cmpgt_epi16 (levx8, allZero));
            levx8 = _mm_sign_epi16 (levx8, coeffx8);
            recx8 = _mm_sign_epi16 (recx8, coeffx8);
            recx4 = _mm_cvtepi16_epi32 (recx8);

            _mm_storel_epi64 ((__m128i *)(qCoeff), levx8);
            _mm_storeu_si128 ((__m128i *)(rCoeff), recx4);;

        }
        else
        {
            _mm_storel_epi64 ((__m128i *)(qCoeff), allZero);
            _mm_storeu_si128 ((__m128i *)(rCoeff), allZero);
        }

        qCoeff += coeffStride;
        rCoeff += coeffStride;
        tCoeff += coeffStride;

    }

    qCoeff -= height*coeffStride;
    rCoeff -= height*coeffStride;
    tCoeff -= height*coeffStride;

    nzCx8 = _mm_sad_epu8 (nzCx8, allZero);

    *nonZeroCount = _mm_cvtsi128_si32 (nzCx8);

    absCoef = XIN_ABS (dcCoeff);
    qCoef   = 0;
    rCoef   = 0;

    if (absCoef >= zBin[0])
    {
        absCoef = XIN_CLIP (absCoef + add[0], XIN_MIN_S16, XIN_MAX_S16);
        absCoef = ((absCoef * qMult[0]) >> 16) + absCoef;
        absCoef = (absCoef * qShift[0] * (1<<logScale)) >> 16;
        qCoef   = (dcCoeff > 0) ? absCoef : -absCoef;
        rCoef   = (absCoef*iqMult[0]) >> logScale;
        rCoef   = (dcCoeff > 0) ? rCoef : -rCoef;
    }

    qCoeff[0] = (SINT16)qCoef;
    rCoeff[0] = rCoef;
    tCoeff[0] = dcCoeff;

    *nonZeroCount += (qCoef != 0);

}


void Xin265pQuantInvQuantB8xH_SSE4 (
    COEFF    *qCoeff,
    SINT32   *tCoeff,
    SINT32   *rCoeff,
    intptr_t coeffStride,
    SINT32   width,
    SINT32   height,
    SINT32   logScale,
    SINT32   *qAdd,
    SINT32   *qMult,
    SINT32   *qzBin,
    SINT32   *qShift,
    SINT32   *iqMult,
    UINT32   *nonZeroCount)
{
    SINT32  rowIdx;
    SINT32  qCoef;
    SINT32  rCoef;
    SINT32  absCoef;
    SINT32  zBin[2];
    SINT32  add[2];
    __m128i thrx8;
    __m128i addx8;
    __m128i qMultx8;
    __m128i iqMultx8;
    __m128i shiftx8;
    __m128i coeff0x4;
    __m128i coeff1x4;
    __m128i coeffx8;
    __m128i magx8;
    __m128i levx8;
    __m128i recx8;
    __m128i rec0x4;
    __m128i rec1x4;
    __m128i maskx8;
    __m128i allZero;
    __m128i nzCx8;
    SINT32  dcCoeff;
    BOOL    zeroFlag;

    (void)width;
    dcCoeff   = tCoeff[0];
    tCoeff[0] = 0;
    zBin[0]   =  XIN_ROUND_POWER2 (qzBin[0], logScale);
    zBin[1]   =  XIN_ROUND_POWER2 (qzBin[1], logScale);
    add[0]    =  XIN_ROUND_POWER2 (qAdd[0],  logScale);
    add[1]    =  XIN_ROUND_POWER2 (qAdd[1],  logScale);
    thrx8     = _mm_set1_epi16 ((SINT16)zBin[1]);
    addx8     = _mm_set1_epi16 ((SINT16)add[1]);
    qMultx8   = _mm_set1_epi16 ((SINT16)qMult[1]);
    shiftx8   = _mm_set1_epi16 ((SINT16)(qShift[1] << logScale));
    iqMultx8  = _mm_set1_epi16 ((SINT16)(iqMult[1]));
    allZero   = _mm_setzero_si128 ();
    nzCx8     = _mm_setzero_si128 ();

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        coeff0x4 = _mm_loadu_si128 ((__m128i *)(tCoeff));
        coeff1x4 = _mm_loadu_si128 ((__m128i *)(tCoeff + 4));

        coeffx8 = _mm_packs_epi32 (coeff0x4, coeff1x4);

        magx8  = _mm_abs_epi16 (coeffx8);
        maskx8 = _mm_cmpgt_epi16(magx8, thrx8);
        maskx8 = _mm_or_si128 (maskx8, _mm_cmpeq_epi16 (magx8, thrx8));

        zeroFlag = _mm_testz_si128 (maskx8, maskx8);

        if (!zeroFlag)
        {
            magx8  = _mm_adds_epi16 (magx8, addx8);
            levx8  = _mm_add_epi16 (_mm_mulhi_epi16 (magx8, qMultx8), magx8);
            levx8  = _mm_mulhi_epi16 (levx8, shiftx8);
            levx8  = _mm_and_si128 (levx8, maskx8);
            recx8  = _mm_mullo_epi16 (levx8, iqMultx8);
            recx8  = _mm_mullo_epi16 (levx8, iqMultx8);
            recx8  = _mm_srai_epi16 (recx8, logScale);
            nzCx8  = _mm_sub_epi16(nzCx8, _mm_cmpgt_epi16 (levx8, allZero));
            levx8  = _mm_sign_epi16 (levx8, coeffx8);
            recx8  = _mm_sign_epi16 (recx8, coeffx8);
            rec0x4 = _mm_cvtepi16_epi32 (recx8);
            rec1x4 = _mm_cvtepi16_epi32 (_mm_unpackhi_epi64 (recx8, recx8));

            _mm_storeu_si128 ((__m128i *)(qCoeff),     levx8);
            _mm_storeu_si128 ((__m128i *)(rCoeff),     rec0x4);
            _mm_storeu_si128 ((__m128i *)(rCoeff + 4), rec1x4);

        }
        else
        {
            _mm_storeu_si128 ((__m128i *)(qCoeff),     allZero);
            _mm_storeu_si128 ((__m128i *)(rCoeff),     allZero);
            _mm_storeu_si128 ((__m128i *)(rCoeff + 4), allZero);
        }

        qCoeff += coeffStride;
        rCoeff += coeffStride;
        tCoeff += coeffStride;

    }

    qCoeff -= height*coeffStride;
    rCoeff -= height*coeffStride;
    tCoeff -= height*coeffStride;

    nzCx8 = _mm_sad_epu8 (nzCx8, _mm_srli_si128 (nzCx8, 7));

    *nonZeroCount = _mm_cvtsi128_si32 (nzCx8);

    absCoef = XIN_ABS (dcCoeff);
    qCoef   = 0;
    rCoef   = 0;

    if (absCoef >= zBin[0])
    {
        absCoef = XIN_CLIP (absCoef + add[0], XIN_MIN_S16, XIN_MAX_S16);
        absCoef = ((absCoef * qMult[0]) >> 16) + absCoef;
        absCoef = (absCoef * qShift[0] * (1<<logScale)) >> 16;
        qCoef   = (dcCoeff > 0) ? absCoef : -absCoef;
        rCoef   = (absCoef*iqMult[0]) >> logScale;
        rCoef   = (dcCoeff > 0) ? rCoef : -rCoef;
    }

    qCoeff[0] = (SINT16)qCoef;
    rCoeff[0] = rCoef;
    tCoeff[0] = dcCoeff;

    *nonZeroCount += (qCoef != 0);

}

