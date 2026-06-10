/***************************************************************************//**
 *
 * @file          h265p_quant_inv_quant_avx2.c
 * @brief         av1 forward quantization and inverse quantization (AVX2).
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
#include "basic_macro.h"
#include "h26x_definition.h"
#include "h265p_definition.h"
#ifdef __linux__
#include "xin_avx2_linux_patch.h"
#endif

void Xin265pQuantInvQuantB16xH_AVX2 (
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
    __m256i thrx16;
    __m256i addx16;
    __m256i qMultx16;
    __m256i iqMultx16;
    __m256i shiftx16;
    __m256i coeff0x8;
    __m256i coeff1x8;
    __m256i coeffx16;
    __m256i magx16;
    __m256i levx16;
    __m256i recx16;
    __m256i rec0x8;
    __m256i rec1x8;
    __m256i maskx16;
    __m256i allZero;
    __m256i nzCx16;
    SINT32  dcCoeff;
    BOOL    zeroFlag;

    (void)width;
    dcCoeff   = tCoeff[0];
    tCoeff[0] = 0;
    zBin[0]   =  XIN_ROUND_POWER2 (qzBin[0], logScale);
    zBin[1]   =  XIN_ROUND_POWER2 (qzBin[1], logScale);
    add[0]    =  XIN_ROUND_POWER2 (qAdd[0],  logScale);
    add[1]    =  XIN_ROUND_POWER2 (qAdd[1],  logScale);
    thrx16    = _mm256_set1_epi16 ((SINT16)zBin[1]);
    addx16    = _mm256_set1_epi16 ((SINT16)add[1]);
    qMultx16  = _mm256_set1_epi16 ((SINT16)qMult[1]);
    shiftx16  = _mm256_set1_epi16 ((SINT16)(qShift[1] << logScale));
    iqMultx16 = _mm256_set1_epi16 ((SINT16)(iqMult[1]));
    allZero   = _mm256_setzero_si256 ();
    nzCx16    = _mm256_setzero_si256 ();

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        coeff0x8 = _mm256_loadu_si256 ((__m256i *)(tCoeff));
        coeff1x8 = _mm256_loadu_si256 ((__m256i *)(tCoeff + 8));

        coeffx16 = _mm256_packs_epi32 (coeff0x8, coeff1x8);
        coeffx16 = _mm256_permute4x64_epi64 (coeffx16, 0xD8);

        magx16  = _mm256_abs_epi16 (coeffx16);
        maskx16 = _mm256_cmpgt_epi16(magx16, thrx16);
        maskx16 = _mm256_or_si256 (maskx16, _mm256_cmpeq_epi16 (magx16, thrx16));
        
        zeroFlag = _mm256_testz_si256 (maskx16, maskx16);

        if (!zeroFlag)
        {
            magx16 = _mm256_adds_epi16 (magx16, addx16);
            levx16 = _mm256_add_epi16 (_mm256_mulhi_epi16 (magx16, qMultx16), magx16);
            levx16 = _mm256_mulhi_epi16 (levx16, shiftx16);
            levx16 = _mm256_and_si256 (levx16, maskx16);
            recx16 = _mm256_mullo_epi16 (levx16, iqMultx16);
            recx16 = _mm256_mullo_epi16 (levx16, iqMultx16);
            recx16 = _mm256_srai_epi16 (recx16, logScale);
            nzCx16 = _mm256_sub_epi16(nzCx16, _mm256_cmpgt_epi16(levx16, allZero));
            levx16 = _mm256_sign_epi16 (levx16, coeffx16);
            recx16 = _mm256_sign_epi16 (recx16, coeffx16);
            rec0x8 = _mm256_cvtepi16_epi32 (_mm256_castsi256_si128 (recx16));
            rec1x8 = _mm256_cvtepi16_epi32 (_mm256_castsi256_si128 (_mm256_permute4x64_epi64(recx16, 0xEE)));
            
            _mm256_storeu_si256 ((__m256i *)(qCoeff),     levx16);
            _mm256_storeu_si256 ((__m256i *)(rCoeff),     rec0x8);
            _mm256_storeu_si256 ((__m256i *)(rCoeff + 8), rec1x8);

        }
        else
        {
            _mm256_storeu_si256 ((__m256i *)(qCoeff),     allZero);
            _mm256_storeu_si256 ((__m256i *)(rCoeff),     allZero);
            _mm256_storeu_si256 ((__m256i *)(rCoeff + 8), allZero);
        }

        qCoeff += coeffStride;
        rCoeff += coeffStride;
        tCoeff += coeffStride;

    }

    qCoeff -= height*coeffStride;
    rCoeff -= height*coeffStride;
    tCoeff -= height*coeffStride;

    nzCx16 = _mm256_sad_epu8 (nzCx16, _mm256_srli_si256 (nzCx16, 7));

    *nonZeroCount = _mm_cvtsi128_si32 (_mm_add_epi32 (_mm256_extracti128_si256 (nzCx16, 0), _mm256_extracti128_si256 (nzCx16, 1)));

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

void Xin265pQuantInvQuantB32xH_AVX2 (
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
    __m256i thrx16;
    __m256i addx16;
    __m256i qMultx16;
    __m256i iqMultx16;
    __m256i shiftx16;
    __m256i coeff0x8;
    __m256i coeff1x8;
    __m256i coeff2x8;
    __m256i coeff3x8;
    __m256i coeff0x16;
    __m256i coeff1x16;
    __m256i mag0x16, mag1x16;
    __m256i lev0x16, lev1x16;
    __m256i rec0x16, rec1x16;
    __m256i rec0x8, rec1x8;
    __m256i rec2x8, rec3x8;
    __m256i mask0x16, mask1x16;
    __m256i allZero;
    __m256i nzCx16;
    SINT32  dcCoeff;
    BOOL    zero0Flag, zero1Flag;

    (void)width;
    dcCoeff   = tCoeff[0];
    tCoeff[0] = 0;
    zBin[0]   =  XIN_ROUND_POWER2 (qzBin[0], logScale);
    zBin[1]   =  XIN_ROUND_POWER2 (qzBin[1], logScale);
    add[0]    =  XIN_ROUND_POWER2 (qAdd[0],  logScale);
    add[1]    =  XIN_ROUND_POWER2 (qAdd[1],  logScale);
    thrx16    = _mm256_set1_epi16 ((SINT16)zBin[1]);
    addx16    = _mm256_set1_epi16 ((SINT16)add[1]);
    qMultx16  = _mm256_set1_epi16 ((SINT16)qMult[1]);
    shiftx16  = _mm256_set1_epi16 ((SINT16)(qShift[1] << logScale));
    iqMultx16 = _mm256_set1_epi16 ((SINT16)(iqMult[1]));
    allZero   = _mm256_setzero_si256 ();
    nzCx16    = _mm256_setzero_si256 ();

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        coeff0x8 = _mm256_loadu_si256 ((__m256i *)(tCoeff));
        coeff1x8 = _mm256_loadu_si256 ((__m256i *)(tCoeff + 8));
        coeff2x8 = _mm256_loadu_si256 ((__m256i *)(tCoeff + 16));
        coeff3x8 = _mm256_loadu_si256 ((__m256i *)(tCoeff + 24));

        coeff0x16 = _mm256_packs_epi32 (coeff0x8, coeff1x8);
        coeff0x16 = _mm256_permute4x64_epi64 (coeff0x16, 0xD8);
        coeff1x16 = _mm256_packs_epi32 (coeff2x8, coeff3x8);
        coeff1x16 = _mm256_permute4x64_epi64 (coeff1x16, 0xD8);

        mag0x16  = _mm256_abs_epi16 (coeff0x16);
        mask0x16 = _mm256_cmpgt_epi16(mag0x16, thrx16);
        mask0x16 = _mm256_or_si256 (mask0x16, _mm256_cmpeq_epi16 (mag0x16, thrx16));

        mag1x16  = _mm256_abs_epi16 (coeff1x16);
        mask1x16 = _mm256_cmpgt_epi16(mag1x16, thrx16);
        mask1x16 = _mm256_or_si256 (mask1x16, _mm256_cmpeq_epi16 (mag1x16, thrx16));
        
        zero0Flag = _mm256_testz_si256 (mask0x16, mask0x16);
        zero1Flag = _mm256_testz_si256 (mask1x16, mask1x16);

        if (!zero0Flag)
        {
            mag0x16 = _mm256_adds_epi16 (mag0x16, addx16);
            lev0x16 = _mm256_add_epi16 (_mm256_mulhi_epi16 (mag0x16, qMultx16), mag0x16);
            lev0x16 = _mm256_mulhi_epi16 (lev0x16, shiftx16);
            lev0x16 = _mm256_and_si256 (lev0x16, mask0x16);
            rec0x16 = _mm256_mullo_epi16 (lev0x16, iqMultx16);
            rec0x16 = _mm256_mullo_epi16 (lev0x16, iqMultx16);
            rec0x16 = _mm256_srai_epi16 (rec0x16, logScale);
            nzCx16  = _mm256_sub_epi16(nzCx16, _mm256_cmpgt_epi16(lev0x16, allZero));
            lev0x16 = _mm256_sign_epi16 (lev0x16, coeff0x16);
            rec0x16 = _mm256_sign_epi16 (rec0x16, coeff0x16);
            rec0x8  = _mm256_cvtepi16_epi32 (_mm256_castsi256_si128 (rec0x16));
            rec1x8  = _mm256_cvtepi16_epi32 (_mm256_castsi256_si128 (_mm256_permute4x64_epi64(rec0x16, 0xEE)));
            
            _mm256_storeu_si256 ((__m256i *)(qCoeff),     lev0x16);
            _mm256_storeu_si256 ((__m256i *)(rCoeff),     rec0x8);
            _mm256_storeu_si256 ((__m256i *)(rCoeff + 8), rec1x8);

        }
        else
        {
            _mm256_storeu_si256 ((__m256i *)(qCoeff),     allZero);
            _mm256_storeu_si256 ((__m256i *)(rCoeff),     allZero);
            _mm256_storeu_si256 ((__m256i *)(rCoeff + 8), allZero);
        }

        if (!zero1Flag)
        {
            mag1x16 = _mm256_adds_epi16 (mag1x16, addx16);
            lev1x16 = _mm256_add_epi16 (_mm256_mulhi_epi16 (mag1x16, qMultx16), mag1x16);
            lev1x16 = _mm256_mulhi_epi16 (lev1x16, shiftx16);
            lev1x16 = _mm256_and_si256 (lev1x16, mask1x16);
            rec1x16 = _mm256_mullo_epi16 (lev1x16, iqMultx16);
            rec1x16 = _mm256_mullo_epi16 (lev1x16, iqMultx16);
            rec1x16 = _mm256_srai_epi16 (rec1x16, logScale);
            nzCx16  = _mm256_sub_epi16(nzCx16, _mm256_cmpgt_epi16(lev1x16, allZero));
            lev1x16 = _mm256_sign_epi16 (lev1x16, coeff1x16);
            rec1x16 = _mm256_sign_epi16 (rec1x16, coeff1x16);
            rec2x8  = _mm256_cvtepi16_epi32 (_mm256_castsi256_si128 (rec1x16));
            rec3x8  = _mm256_cvtepi16_epi32 (_mm256_castsi256_si128 (_mm256_permute4x64_epi64(rec1x16, 0xEE)));
            
            _mm256_storeu_si256 ((__m256i *)(qCoeff + 16), lev1x16);
            _mm256_storeu_si256 ((__m256i *)(rCoeff + 16), rec2x8);
            _mm256_storeu_si256 ((__m256i *)(rCoeff + 24), rec3x8);

        }
        else
        {
            _mm256_storeu_si256 ((__m256i *)(qCoeff + 16), allZero);
            _mm256_storeu_si256 ((__m256i *)(rCoeff + 16), allZero);
            _mm256_storeu_si256 ((__m256i *)(rCoeff + 24), allZero);
        }

        qCoeff += coeffStride;
        rCoeff += coeffStride;
        tCoeff += coeffStride;

    }

    qCoeff -= height*coeffStride;
    rCoeff -= height*coeffStride;
    tCoeff -= height*coeffStride;

    nzCx16 = _mm256_sad_epu8 (nzCx16, _mm256_srli_si256 (nzCx16, 7));

    *nonZeroCount = _mm_cvtsi128_si32 (_mm_add_epi32 (_mm256_extracti128_si256 (nzCx16, 0), _mm256_extracti128_si256 (nzCx16, 1)));

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

