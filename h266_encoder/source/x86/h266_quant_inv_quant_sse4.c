/***************************************************************************//**
 *
 * @file          h266_quant_inv_quant_sse4.c
 * @brief         h266 forward quantization and inverse quantization (SSE4).
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
#include "basic_macro.h"
#include "h266_definition.h"
#include "xin_video_common.h"
#include "h266_trans_unit_struct.h"

static const UINT8 qAddLoShuf[16] =
{
    0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1
};

static const UINT8 qAddHiShuf[16] =
{
    0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 0x2, 0x3
};

static const UINT8 s_lowMask[16] =
{
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
};

static const UINT8 s_highMask[16] =
{
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

void Xin266GetBlockDeltaU_SSE4 (
    COEFF    *qCoeff,
    COEFF    *tCoeff,
    COEFF    *deltaU,
    UINT32   cgWidth,
    UINT32   cgHeight,
    UINT16   *coeffSign,
    intptr_t coeffStride,
    SINT32   qMult,
    SINT32   qShift)
{
    SINT32  signBitMap;
    SINT32  qShift8;
    __m128i coef0x4, coef1x4, coef2x4, coef3x4;
    __m128i coef10x4, coef32x4, coef3210;
    __m128i level0x4, level1x4, level2x4, level3x4;
    __m128i coef0Mag, coef1Mag, coef2Mag, coef3Mag;
    __m128i level0Mag, level1Mag, level2Mag, level3Mag;
    __m128i deltaU0x4, deltaU1x4, deltaU2x4, deltaU3x4;
    __m128i deltaU10, deltaU32;
    __m128i allZero;
    __m128i qMultx8;

    (void)cgWidth;
    (void)cgHeight;
    signBitMap = 0;
    qShift8    = qShift - 8;
    allZero    = _mm_setzero_si128 ();
    qMultx8    = _mm_set1_epi16 ((UINT16)qMult);

    coef0x4 = _mm_loadl_epi64 ((__m128i *)(tCoeff));
    coef1x4 = _mm_loadl_epi64 ((__m128i *)(tCoeff + coeffStride));
    coef2x4 = _mm_loadl_epi64 ((__m128i *)(tCoeff + coeffStride*2));
    coef3x4 = _mm_loadl_epi64 ((__m128i *)(tCoeff + coeffStride*3));

    level0x4 = _mm_loadl_epi64 ((__m128i *)(qCoeff));
    level1x4 = _mm_loadl_epi64 ((__m128i *)(qCoeff + coeffStride));
    level2x4 = _mm_loadl_epi64 ((__m128i *)(qCoeff + coeffStride*2));
    level3x4 = _mm_loadl_epi64 ((__m128i *)(qCoeff + coeffStride*3));

    coef0Mag = _mm_abs_epi16 (coef0x4);
    coef1Mag = _mm_abs_epi16 (coef1x4);
    coef2Mag = _mm_abs_epi16 (coef2x4);
    coef3Mag = _mm_abs_epi16 (coef3x4);

    level0Mag = _mm_abs_epi16 (level0x4);
    level1Mag = _mm_abs_epi16 (level1x4);
    level2Mag = _mm_abs_epi16 (level2x4);
    level3Mag = _mm_abs_epi16 (level3x4);

    coef0Mag = _mm_unpacklo_epi16 (coef0Mag, allZero);
    coef1Mag = _mm_unpacklo_epi16 (coef1Mag, allZero);
    coef2Mag = _mm_unpacklo_epi16 (coef2Mag, allZero);
    coef3Mag = _mm_unpacklo_epi16 (coef3Mag, allZero);

    level0Mag = _mm_unpacklo_epi16 (level0Mag, allZero);
    level1Mag = _mm_unpacklo_epi16 (level1Mag, allZero);
    level2Mag = _mm_unpacklo_epi16 (level2Mag, allZero);
    level3Mag = _mm_unpacklo_epi16 (level3Mag, allZero);

    coef0Mag = _mm_madd_epi16 (coef0Mag, qMultx8);
    coef1Mag = _mm_madd_epi16 (coef1Mag, qMultx8);
    coef2Mag = _mm_madd_epi16 (coef2Mag, qMultx8);
    coef3Mag = _mm_madd_epi16 (coef3Mag, qMultx8);

    level0Mag = _mm_slli_epi32 (level0Mag, qShift);
    level1Mag = _mm_slli_epi32 (level1Mag, qShift);
    level2Mag = _mm_slli_epi32 (level2Mag, qShift);
    level3Mag = _mm_slli_epi32 (level3Mag, qShift);

    deltaU0x4 = _mm_sub_epi32 (coef0Mag, level0Mag);
    deltaU1x4 = _mm_sub_epi32 (coef1Mag, level1Mag);
    deltaU2x4 = _mm_sub_epi32 (coef2Mag, level2Mag);
    deltaU3x4 = _mm_sub_epi32 (coef3Mag, level3Mag);

    deltaU0x4 = _mm_srai_epi32 (deltaU0x4, qShift8);
    deltaU1x4 = _mm_srai_epi32 (deltaU1x4, qShift8);
    deltaU2x4 = _mm_srai_epi32 (deltaU2x4, qShift8);
    deltaU3x4 = _mm_srai_epi32 (deltaU3x4, qShift8);

    deltaU10 = _mm_packs_epi32 (deltaU0x4, deltaU1x4);
    deltaU32 = _mm_packs_epi32 (deltaU2x4, deltaU3x4);

    _mm_storeu_si128 ((__m128i *)(deltaU + 0), deltaU10);
    _mm_storeu_si128 ((__m128i *)(deltaU + 8), deltaU32);

    coef10x4 = _mm_unpacklo_epi64 (coef0x4, coef1x4);
    coef32x4 = _mm_unpacklo_epi64 (coef2x4, coef3x4);
    coef3210 = _mm_packs_epi16 (coef10x4, coef32x4);

    signBitMap = _mm_movemask_epi8 (coef3210);
    *coeffSign = (UINT16)signBitMap;

}

void Xin266ComputeBlockDeltaU_SSE4 (
    SINT16   *tCoef,
    SINT16   *rCoef,
    SINT16   *qCoef,
    intptr_t coefStride,
    SINT32   iqMult,
    SINT32   iqShift,
    UINT32   cgWidth,
    UINT32   cgHeight,
    SINT32   *distNow,
    SINT32   *distUp,
    SINT32   *distDown,
    UINT16   *coeffSign)
{
    UINT32  rowIdx;
    SINT32  iqAdd;
    UINT32  signBitMap;
    __m128i tCoefx4;
    __m128i rCoefx4;
    __m128i qCoefx4;
    __m128i allZero;
    __m128i onex4;
    __m128i iqMultx4;
    __m128i rP1Coefx4;
    __m128i rM1Coefx4;
    __m128i iqAddx4;
    __m128i distNowx4;
    __m128i distDownx4;
    __m128i distUpx4;
    __m128i coef3210;

    (void)cgWidth;
    (void)cgHeight;
    iqAdd      = 1 << (iqShift - 1);
    signBitMap = 0;
    allZero    = _mm_setzero_si128 ();
    onex4      = _mm_set1_epi32 (1);
    iqMultx4   = _mm_set1_epi32 (iqMult);
    iqAddx4    = _mm_set1_epi32 (iqAdd);
    coef3210   = _mm_setzero_si128 ();

    for (rowIdx = 0; rowIdx < 4; rowIdx++)
    {
        tCoefx4 = _mm_loadl_epi64 ((__m128i *)tCoef);
        rCoefx4 = _mm_loadl_epi64 ((__m128i *)rCoef);
        qCoefx4 = _mm_loadl_epi64 ((__m128i *)qCoef);

        coef3210 = _mm_slli_si128 (coef3210, 4);
        coef3210 = _mm_or_si128 (coef3210, _mm_packs_epi16 (tCoefx4, allZero));

        tCoefx4 = _mm_abs_epi16 (tCoefx4);
        rCoefx4 = _mm_abs_epi16 (rCoefx4);
        qCoefx4 = _mm_abs_epi16 (qCoefx4);

        tCoefx4 = _mm_unpacklo_epi16 (tCoefx4, allZero);
        rCoefx4 = _mm_unpacklo_epi16 (rCoefx4, allZero);
        qCoefx4 = _mm_unpacklo_epi16 (qCoefx4, allZero);

        rP1Coefx4 = _mm_add_epi32 (qCoefx4, onex4);
        rP1Coefx4 = _mm_mullo_epi32 (rP1Coefx4, iqMultx4);
        rP1Coefx4 = _mm_add_epi32 (rP1Coefx4, iqAddx4);
        rP1Coefx4 = _mm_srai_epi32 (rP1Coefx4, iqShift);

        rM1Coefx4 = _mm_sub_epi32 (qCoefx4, onex4);
        rM1Coefx4 = _mm_mullo_epi32 (rM1Coefx4, iqMultx4);
        rM1Coefx4 = _mm_add_epi32 (rM1Coefx4, iqAddx4);
        rM1Coefx4 = _mm_srai_epi32 (rM1Coefx4, iqShift);

        distNowx4  = _mm_sub_epi32 (tCoefx4, rCoefx4);
        distNowx4  = _mm_mullo_epi32 (distNowx4, distNowx4);
        distDownx4 = _mm_sub_epi32 (tCoefx4, rM1Coefx4);
        distDownx4 = _mm_mullo_epi32 (distDownx4, distDownx4);
        distUpx4   = _mm_sub_epi32 (tCoefx4, rP1Coefx4);
        distUpx4   = _mm_mullo_epi32 (distUpx4, distUpx4);

        _mm_storeu_si128 ((__m128i *)distNow,  distNowx4);
        _mm_storeu_si128 ((__m128i *)distDown, distDownx4);
        _mm_storeu_si128 ((__m128i *)distUp,   distUpx4);

        tCoef += coefStride;
        rCoef += coefStride;
        qCoef += coefStride;

        distNow  += 4;
        distDown += 4;
        distUp   += 4;

    }

    coef3210   = _mm_shuffle_epi32 (coef3210, 0x1B);
    signBitMap = _mm_movemask_epi8 (coef3210);
    *coeffSign = (UINT16)signBitMap;

}

void Xin266QuantInvQuant4xH_SSE4 (
    COEFF    *qCoeff,
    COEFF    *tCoeff,
    COEFF    *rCoeff,
    intptr_t coeffStride,
    UINT32   width,
    UINT32   height,
    UINT32   cGWidth,
    UINT32   cGHeight,
    SINT32   qMult,
    SINT32   qAdd,
    SINT32   qShift,
    SINT32   iqMult,
    SINT32   iqAdd,
    SINT32   iqShift,
    UINT64   *nzCGBitMapRs)
{
    __m128i qMultx8, iqMultx8;
    __m128i qAddx4, iqAddx4;
    __m128i loShuf, hiShuf;
    __m128i qAddLox8, qAddHix8;
    __m128i coef0x4, coef1x4, coef2x4, coef3x4;
    __m128i coef10, coef32;
    __m128i coef10Mag, coef32Mag;
    __m128i coefMagL, coefMagH;
    __m128i levl10Mag, levl32Mag;
    __m128i level10, level32;
    __m128i scaledL10, scaledH10;
    __m128i scaledL32, scaledH32;
    __m128i scaled0, scaled1;
    __m128i scaled2, scaled3;
    __m128i recon0, recon1;
    __m128i recon2, recon3;
    __m128i level3210;
    __m128i allOnes;
    UINT32  nZBlock;
    UINT64  nZBlockRs;
    UINT32  rowIdx;

    (void)width;
    (void)cGWidth;
    (void)cGHeight;

    qMultx8   = _mm_set1_epi16 ((UINT16)qMult);
    iqMultx8  = _mm_set1_epi16 ((UINT16)iqMult);
    iqAddx4   = _mm_set1_epi32 ((UINT16)iqAdd);
    qAddx4    = _mm_set1_epi32 ((UINT32)qAdd);
    loShuf    = _mm_lddqu_si128 ((__m128i *)qAddLoShuf);
    hiShuf    = _mm_lddqu_si128 ((__m128i *)qAddHiShuf);
    qAddLox8  = _mm_shuffle_epi8 (qAddx4, loShuf);
    qAddHix8  = _mm_shuffle_epi8 (qAddx4, hiShuf);
    allOnes   = _mm_cmpeq_epi16 (qAddLox8, qAddLox8);
    qShift    = qShift - 16;
    nZBlockRs = 0;

    for (rowIdx = 0; rowIdx < height; rowIdx += 4)
    {
        coef0x4 = _mm_loadl_epi64 ((__m128i *)(tCoeff));
        coef1x4 = _mm_loadl_epi64 ((__m128i *)(tCoeff + coeffStride));
        coef2x4 = _mm_loadl_epi64 ((__m128i *)(tCoeff + coeffStride*2));
        coef3x4 = _mm_loadl_epi64 ((__m128i *)(tCoeff + coeffStride*3));

        coef10  = _mm_unpacklo_epi64 (coef0x4, coef1x4);
        coef32  = _mm_unpacklo_epi64 (coef2x4, coef3x4);

        coef10Mag = _mm_abs_epi16 (coef10);
        coefMagL  = _mm_mullo_epi16 (coef10Mag, qMultx8);
        coefMagH  = _mm_mulhi_epi16 (coef10Mag, qMultx8);
        coefMagL  = _mm_avg_epu16 (coefMagL, qAddLox8);
        coefMagH  = _mm_adds_epu16 (coefMagH, qAddHix8);
        coefMagL  = _mm_srli_epi16 (coefMagL, 15);
        levl10Mag = _mm_adds_epu16 (coefMagL, coefMagH);
        levl10Mag = _mm_srli_epi16 (levl10Mag, qShift);

        coef32Mag = _mm_abs_epi16 (coef32);
        coefMagL  = _mm_mullo_epi16 (coef32Mag, qMultx8);
        coefMagH  = _mm_mulhi_epi16 (coef32Mag, qMultx8);
        coefMagL  = _mm_avg_epu16 (coefMagL, qAddLox8);
        coefMagH  = _mm_adds_epu16 (coefMagH, qAddHix8);
        coefMagL  = _mm_srli_epi16 (coefMagL, 15);
        levl32Mag = _mm_adds_epu16 (coefMagL, coefMagH);
        levl32Mag = _mm_srli_epi16 (levl32Mag, qShift);

        level3210 = _mm_packs_epi16 (levl10Mag, levl32Mag);
        level10   = _mm_sign_epi16 (levl10Mag, coef10);
        level32   = _mm_sign_epi16 (levl32Mag, coef32);

        _mm_storel_epi64 ((__m128i *)(qCoeff + coeffStride*0), level10);
        _mm_storel_epi64 ((__m128i *)(qCoeff + coeffStride*1), _mm_shuffle_epi32 (level10, 0x4E));
        _mm_storel_epi64 ((__m128i *)(qCoeff + coeffStride*2), level32);
        _mm_storel_epi64 ((__m128i *)(qCoeff + coeffStride*3), _mm_shuffle_epi32 (level32, 0x4E));

        scaledL10 = _mm_mullo_epi16 (level10, iqMultx8);
        scaledH10 = _mm_mulhi_epi16 (level10, iqMultx8);
        scaledL32 = _mm_mullo_epi16 (level32, iqMultx8);
        scaledH32 = _mm_mulhi_epi16 (level32, iqMultx8);

        scaled0 = _mm_unpacklo_epi16 (scaledL10, scaledH10);
        scaled1 = _mm_unpackhi_epi16 (scaledL10, scaledH10);
        scaled2 = _mm_unpacklo_epi16 (scaledL32, scaledH32);
        scaled3 = _mm_unpackhi_epi16 (scaledL32, scaledH32);

        scaled0 = _mm_add_epi32 (scaled0, iqAddx4);
        scaled1 = _mm_add_epi32 (scaled1, iqAddx4);
        scaled2 = _mm_add_epi32 (scaled2, iqAddx4);
        scaled3 = _mm_add_epi32 (scaled3, iqAddx4);

        recon0  = _mm_srai_epi32 (scaled0, iqShift);
        recon1  = _mm_srai_epi32 (scaled1, iqShift);
        recon2  = _mm_srai_epi32 (scaled2, iqShift);
        recon3  = _mm_srai_epi32 (scaled3, iqShift);

        recon0  = _mm_packs_epi32 (recon0, recon0);
        recon1  = _mm_packs_epi32 (recon1, recon1);
        recon2  = _mm_packs_epi32 (recon2, recon2);
        recon3  = _mm_packs_epi32 (recon3, recon3);

        _mm_storel_epi64 ((__m128i *)(rCoeff + coeffStride*0), recon0);
        _mm_storel_epi64 ((__m128i *)(rCoeff + coeffStride*1), recon1);
        _mm_storel_epi64 ((__m128i *)(rCoeff + coeffStride*2), recon2);
        _mm_storel_epi64 ((__m128i *)(rCoeff + coeffStride*3), recon3);

        nZBlock    = _mm_test_all_zeros (level3210, allOnes);
        nZBlockRs |= ((!nZBlock) << (rowIdx >> 2));

        tCoeff += 4*coeffStride;
        rCoeff += 4*coeffStride;
        qCoeff += 4*coeffStride;

    }

    *nzCGBitMapRs = nZBlockRs;

}

void Xin266QuantInvQuantGt4x4_SSE4 (
    COEFF    *qCoeff,
    COEFF    *tCoeff,
    COEFF    *rCoeff,
    intptr_t coeffStride,
    UINT32   width,
    UINT32   height,
    UINT32   cGWidth,
    UINT32   cGHeight,
    SINT32   qMult,
    SINT32   qAdd,
    SINT32   qShift,
    SINT32   iqMult,
    SINT32   iqAdd,
    SINT32   iqShift,
    UINT64   *nzCGBitMapRs)
{
    __m128i qMultx8, iqMultx8;
    __m128i qAddx4, iqAddx4;
    __m128i loShuf, hiShuf;
    __m128i qAddLox8, qAddHix8;
    __m128i coef0x8, coef1x8, coef2x8, coef3x8;
    __m128i coef0Mag, coef1Mag, coef2Mag, coef3Mag;
    __m128i coef0MagL, coef1MagL, coef2MagL, coef3MagL;
    __m128i coef0MagH, coef1MagH, coef2MagH, coef3MagH;
    __m128i levl0Mag, levl1Mag, levl2Mag, levl3Mag;
    __m128i level0, level1, level2, level3;
    __m128i scaledL0, scaledL1, scaledL2, scaledL3;
    __m128i scaledH0, scaledH1, scaledH2, scaledH3;
    __m128i recon0, recon1;
    __m128i recon2, recon3;
    __m128i nZBitMap;
    __m128i allZero;
    __m128i lowMask;
    __m128i highMask;
    UINT32  nZBlockL;
    UINT32  nZBlockH;
    UINT32  nZBlock;
    UINT64  nZBlockRs;
    UINT32  colIdx, rowIdx;
    UINT32  blockIdx;

    (void)cGWidth;
    (void)cGHeight;

    qMultx8   = _mm_set1_epi16 ((UINT16)qMult);
    iqMultx8  = _mm_set1_epi16 ((UINT16)iqMult);
    iqAddx4   = _mm_set1_epi32 ((UINT16)iqAdd);
    qAddx4    = _mm_set1_epi32 ((UINT32)qAdd);
    loShuf    = _mm_lddqu_si128 ((__m128i *)qAddLoShuf);
    hiShuf    = _mm_lddqu_si128 ((__m128i *)qAddHiShuf);
    qAddLox8  = _mm_shuffle_epi8 (qAddx4, loShuf);
    qAddHix8  = _mm_shuffle_epi8 (qAddx4, hiShuf);
    lowMask   = _mm_lddqu_si128 ((__m128i *)s_lowMask);
    highMask  = _mm_lddqu_si128 ((__m128i *)s_highMask);
    allZero   = _mm_setzero_si128 ();
    qShift    = qShift - 16;
    nZBlockRs = 0;
    blockIdx  = 0;

    for (rowIdx = 0; rowIdx < height; rowIdx += 4)
    {
        for (colIdx = 0; colIdx < width; colIdx += 8)
        {
            coef0x8 = _mm_loadu_si128 ((__m128i *)(tCoeff + colIdx + coeffStride*0));
            coef1x8 = _mm_loadu_si128 ((__m128i *)(tCoeff + colIdx + coeffStride*1));
            coef2x8 = _mm_loadu_si128 ((__m128i *)(tCoeff + colIdx + coeffStride*2));
            coef3x8 = _mm_loadu_si128 ((__m128i *)(tCoeff + colIdx + coeffStride*3));

            coef0Mag = _mm_abs_epi16 (coef0x8);
            coef1Mag = _mm_abs_epi16 (coef1x8);
            coef2Mag = _mm_abs_epi16 (coef2x8);
            coef3Mag = _mm_abs_epi16 (coef3x8);

            coef0MagL = _mm_mullo_epi16 (coef0Mag, qMultx8);
            coef1MagL = _mm_mullo_epi16 (coef1Mag, qMultx8);
            coef2MagL = _mm_mullo_epi16 (coef2Mag, qMultx8);
            coef3MagL = _mm_mullo_epi16 (coef3Mag, qMultx8);

            coef0MagH = _mm_mulhi_epi16 (coef0Mag, qMultx8);
            coef1MagH = _mm_mulhi_epi16 (coef1Mag, qMultx8);
            coef2MagH = _mm_mulhi_epi16 (coef2Mag, qMultx8);
            coef3MagH = _mm_mulhi_epi16 (coef3Mag, qMultx8);

            coef0MagL = _mm_avg_epu16 (coef0MagL, qAddLox8);
            coef1MagL = _mm_avg_epu16 (coef1MagL, qAddLox8);
            coef2MagL = _mm_avg_epu16 (coef2MagL, qAddLox8);
            coef3MagL = _mm_avg_epu16 (coef3MagL, qAddLox8);

            coef0MagH = _mm_adds_epu16 (coef0MagH, qAddHix8);
            coef1MagH = _mm_adds_epu16 (coef1MagH, qAddHix8);
            coef2MagH = _mm_adds_epu16 (coef2MagH, qAddHix8);
            coef3MagH = _mm_adds_epu16 (coef3MagH, qAddHix8);

            coef0MagL = _mm_srli_epi16 (coef0MagL, 15);
            coef1MagL = _mm_srli_epi16 (coef1MagL, 15);
            coef2MagL = _mm_srli_epi16 (coef2MagL, 15);
            coef3MagL = _mm_srli_epi16 (coef3MagL, 15);

            levl0Mag = _mm_adds_epu16 (coef0MagL, coef0MagH);
            levl1Mag = _mm_adds_epu16 (coef1MagL, coef1MagH);
            levl2Mag = _mm_adds_epu16 (coef2MagL, coef2MagH);
            levl3Mag = _mm_adds_epu16 (coef3MagL, coef3MagH);

            levl0Mag = _mm_srli_epi16 (levl0Mag, qShift);
            levl1Mag = _mm_srli_epi16 (levl1Mag, qShift);
            levl2Mag = _mm_srli_epi16 (levl2Mag, qShift);
            levl3Mag = _mm_srli_epi16 (levl3Mag, qShift);

            level0   = _mm_sign_epi16 (levl0Mag, coef0x8);
            level1   = _mm_sign_epi16 (levl1Mag, coef1x8);
            level2   = _mm_sign_epi16 (levl2Mag, coef2x8);
            level3   = _mm_sign_epi16 (levl3Mag, coef3x8);

            nZBitMap = _mm_or_si128 (level0,   level1);
            nZBitMap = _mm_or_si128 (nZBitMap, level2);
            nZBitMap = _mm_or_si128 (nZBitMap, level3);
            nZBlockL = _mm_test_all_zeros (nZBitMap, lowMask);
            nZBlockH = _mm_test_all_zeros (nZBitMap, highMask);
            nZBlock  = (!nZBlockL) | ((!nZBlockH) << 1);

            _mm_storeu_si128 ((__m128i *)(qCoeff + colIdx + coeffStride*0), level0);
            _mm_storeu_si128 ((__m128i *)(qCoeff + colIdx + coeffStride*1), level1);
            _mm_storeu_si128 ((__m128i *)(qCoeff + colIdx + coeffStride*2), level2);
            _mm_storeu_si128 ((__m128i *)(qCoeff + colIdx + coeffStride*3), level3);

            _mm_storeu_si128 ((__m128i *)(rCoeff + colIdx + coeffStride*0), allZero);
            _mm_storeu_si128 ((__m128i *)(rCoeff + colIdx + coeffStride*1), allZero);
            _mm_storeu_si128 ((__m128i *)(rCoeff + colIdx + coeffStride*2), allZero);
            _mm_storeu_si128 ((__m128i *)(rCoeff + colIdx + coeffStride*3), allZero);

            nZBlockRs |= ((UINT64)nZBlock << blockIdx);

            if (nZBlock)
            {
                scaledL0  = _mm_unpacklo_epi16 (level0, allZero);
                scaledH0  = _mm_unpackhi_epi16 (level0, allZero);
                scaledL0  = _mm_madd_epi16 (scaledL0, iqMultx8);
                scaledH0  = _mm_madd_epi16 (scaledH0, iqMultx8);
                scaledL0  = _mm_add_epi32 (scaledL0, iqAddx4);
                scaledH0  = _mm_add_epi32 (scaledH0, iqAddx4);
                scaledL0  = _mm_srai_epi32 (scaledL0, iqShift);
                scaledH0  = _mm_srai_epi32 (scaledH0, iqShift);
                recon0    = _mm_packs_epi32 (scaledL0, scaledH0);

                scaledL1  = _mm_unpacklo_epi16 (level1, allZero);
                scaledH1  = _mm_unpackhi_epi16 (level1, allZero);
                scaledL1  = _mm_madd_epi16 (scaledL1, iqMultx8);
                scaledH1  = _mm_madd_epi16 (scaledH1, iqMultx8);
                scaledL1  = _mm_add_epi32 (scaledL1, iqAddx4);
                scaledH1  = _mm_add_epi32 (scaledH1, iqAddx4);
                scaledL1  = _mm_srai_epi32 (scaledL1, iqShift);
                scaledH1  = _mm_srai_epi32 (scaledH1, iqShift);
                recon1    = _mm_packs_epi32 (scaledL1, scaledH1);

                scaledL2  = _mm_unpacklo_epi16 (level2, allZero);
                scaledH2  = _mm_unpackhi_epi16 (level2, allZero);
                scaledL2  = _mm_madd_epi16 (scaledL2, iqMultx8);
                scaledH2  = _mm_madd_epi16 (scaledH2, iqMultx8);
                scaledL2  = _mm_add_epi32 (scaledL2, iqAddx4);
                scaledH2  = _mm_add_epi32 (scaledH2, iqAddx4);
                scaledL2  = _mm_srai_epi32 (scaledL2, iqShift);
                scaledH2  = _mm_srai_epi32 (scaledH2, iqShift);
                recon2    = _mm_packs_epi32 (scaledL2, scaledH2);

                scaledL3  = _mm_unpacklo_epi16 (level3, allZero);
                scaledH3  = _mm_unpackhi_epi16 (level3, allZero);
                scaledL3  = _mm_madd_epi16 (scaledL3, iqMultx8);
                scaledH3  = _mm_madd_epi16 (scaledH3, iqMultx8);
                scaledL3  = _mm_add_epi32 (scaledL3, iqAddx4);
                scaledH3  = _mm_add_epi32 (scaledH3, iqAddx4);
                scaledL3  = _mm_srai_epi32 (scaledL3, iqShift);
                scaledH3  = _mm_srai_epi32 (scaledH3, iqShift);
                recon3    = _mm_packs_epi32 (scaledL3, scaledH3);

                _mm_storeu_si128 ((__m128i *)(rCoeff + colIdx + coeffStride*0), recon0);
                _mm_storeu_si128 ((__m128i *)(rCoeff + colIdx + coeffStride*1), recon1);
                _mm_storeu_si128 ((__m128i *)(rCoeff + colIdx + coeffStride*2), recon2);
                _mm_storeu_si128 ((__m128i *)(rCoeff + colIdx + coeffStride*3), recon3);

            }

            blockIdx += 2;

        }

        rCoeff += 4*coeffStride;
        tCoeff += 4*coeffStride;
        qCoeff += 4*coeffStride;

    }

    *nzCGBitMapRs = nZBlockRs;

}

void Xin266PreRdoq_SSE4 (
    COEFF       *tCoeff,
    COEFF       *rCoeff,
    COEFF       *qCoeff,
    intptr_t    coeffStride,
    UINT32       width,
    UINT32       height,
    UINT32       cGWidth,
    UINT32       cGHeight,
    SINT32       rdoqThrVal,
    xin_scan_pos *scanOrderCG,
    UINT64       *nzCGBitMapRs)
{
    SINT32  blockIdx;
    SINT32  blockPos;
    SINT32  blockNum;
    SINT32  blockX;
    SINT32  blockY;
    SINT32  widthInCG;
    COEFF   *tCoeffBlock;
    COEFF   *rCoeffBlock;
    COEFF   *qCoeffBlock;
    UINT64  nzCGBitMap;
    BOOL    smallBlock;
    __m128i coef0;
    __m128i coef1;
    __m128i coef2;
    __m128i coef3;
    __m128i coef01, coef23;
    __m128i coef0123;
    __m128i thrValx16;
    __m128i allOnes;
    __m128i allZeros;

    blockNum   = width*height / (cGWidth*cGHeight);
    widthInCG  = width / cGWidth;
    nzCGBitMap = *nzCGBitMapRs;
    thrValx16  = _mm_set1_epi16 ((SINT16)rdoqThrVal);
    allOnes    = _mm_cmpeq_epi16 (thrValx16, thrValx16);
    allZeros   = _mm_setzero_si128 ();

    for (blockIdx = blockNum - 1; blockIdx >= 1; blockIdx--)
    {
        blockPos = scanOrderCG[blockIdx].posIdx;

        if (nzCGBitMap & (UINT64)1 << blockPos)
        {
            blockY      = blockPos / widthInCG;
            blockX      = blockPos - blockY*widthInCG;
            tCoeffBlock = tCoeff + blockY*cGHeight*coeffStride + blockX*cGWidth;

            coef0 = _mm_loadl_epi64 ((__m128i *)(tCoeffBlock + coeffStride*0));
            coef1 = _mm_loadl_epi64 ((__m128i *)(tCoeffBlock + coeffStride*1));
            coef2 = _mm_loadl_epi64 ((__m128i *)(tCoeffBlock + coeffStride*2));
            coef3 = _mm_loadl_epi64 ((__m128i *)(tCoeffBlock + coeffStride*3));

            coef01 = _mm_unpacklo_epi64 (coef0, coef1);
            coef23 = _mm_unpacklo_epi64 (coef2, coef3);

            coef01 = _mm_abs_epi16 (coef01);
            coef23 = _mm_abs_epi16 (coef23);

            coef01 = _mm_cmpgt_epi16 (coef01, thrValx16);
            coef23 = _mm_cmpgt_epi16 (coef23, thrValx16);

            coef0123   = _mm_or_si128 (coef01, coef23);
            smallBlock = _mm_test_all_zeros (coef0123, allOnes);

            if (smallBlock)
            {
                rCoeffBlock = rCoeff + blockY*cGHeight*coeffStride + blockX*cGWidth;
                qCoeffBlock = qCoeff + blockY*cGHeight*coeffStride + blockX*cGWidth;

                _mm_storel_epi64 ((__m128i *)(rCoeffBlock + 0*coeffStride), allZeros);
                _mm_storel_epi64 ((__m128i *)(rCoeffBlock + 1*coeffStride), allZeros);
                _mm_storel_epi64 ((__m128i *)(rCoeffBlock + 2*coeffStride), allZeros);
                _mm_storel_epi64 ((__m128i *)(rCoeffBlock + 3*coeffStride), allZeros);

                _mm_storel_epi64 ((__m128i *)(qCoeffBlock + 0*coeffStride), allZeros);
                _mm_storel_epi64 ((__m128i *)(qCoeffBlock + 1*coeffStride), allZeros);
                _mm_storel_epi64 ((__m128i *)(qCoeffBlock + 2*coeffStride), allZeros);
                _mm_storel_epi64 ((__m128i *)(qCoeffBlock + 3*coeffStride), allZeros);

                nzCGBitMap &= ~((UINT64)1 << blockPos);
            }
            else
            {
                break;
            }

        }

    }

    *nzCGBitMapRs = nzCGBitMap;

}

void Xin266PreDepQuant_SSE4 (
    COEFF       *tCoeff,
    COEFF       *rCoeff,
    COEFF       *qCoeff,
    intptr_t    coeffStride,
    UINT32       width,
    UINT32       height,
    UINT32       cGWidth,
    UINT32       cGHeight,
    SINT32       rdoqThrVal,
    xin_scan_pos *scanOrderCG,
    xin_scan_pos *scanOrder,
    SINT32       *nzCoeffIdx)
{
    SINT32  blockIdx;
    SINT32  blockPos;
    SINT32  blockNum;
    SINT32  blockX;
    SINT32  blockY;
    SINT32  widthInCG;
    UINT32  rowIdx;
    SINT32  coeffIdx;
    SINT32  innerPos;
    SINT32  innerX;
    SINT32  innerY;
    COEFF   *tCoeffBlock;
    BOOL    smallBlock;
    __m128i coef0;
    __m128i coef1;
    __m128i coef2;
    __m128i coef3;
    __m128i coef01, coef23;
    __m128i coef0123;
    __m128i thrValx16;
    __m128i allOnes;
    __m128i allZeros;

    blockNum    = width*height / (cGWidth*cGHeight);
    widthInCG   = width / cGWidth;
    thrValx16   = _mm_set1_epi16 ((SINT16)rdoqThrVal);
    allOnes     = _mm_cmpeq_epi16 (thrValx16, thrValx16);
    allZeros    = _mm_setzero_si128 ();
    blockY      = 0;
    blockX      = 0;
    *nzCoeffIdx = -1;

    if (width == 4)
    {
        for (rowIdx = 0; rowIdx < height; rowIdx++)
        {
            _mm_storel_epi64 ((__m128i *)(rCoeff + rowIdx*coeffStride), allZeros);
            _mm_storel_epi64 ((__m128i *)(qCoeff + rowIdx*coeffStride), allZeros);
        }
    }
    else if (width == 8)
    {
        for (rowIdx = 0; rowIdx < height; rowIdx++)
        {
            _mm_storeu_si128 ((__m128i *)(rCoeff + rowIdx*coeffStride), allZeros);
            _mm_storeu_si128 ((__m128i *)(qCoeff + rowIdx*coeffStride), allZeros);
        }
    }
    else if (width == 16)
    {
        for (rowIdx = 0; rowIdx < height; rowIdx++)
        {
            _mm_storeu_si128 ((__m128i *)(rCoeff + rowIdx*coeffStride),     allZeros);
            _mm_storeu_si128 ((__m128i *)(rCoeff + rowIdx*coeffStride + 8), allZeros);

            _mm_storeu_si128 ((__m128i *)(qCoeff + rowIdx*coeffStride),     allZeros);
            _mm_storeu_si128 ((__m128i *)(qCoeff + rowIdx*coeffStride + 8), allZeros);
        }
    }
    else if (width == 32)
    {
        for (rowIdx = 0; rowIdx < height; rowIdx++)
        {
            _mm_storeu_si128 ((__m128i *)(rCoeff + rowIdx*coeffStride),      allZeros);
            _mm_storeu_si128 ((__m128i *)(rCoeff + rowIdx*coeffStride + 8),  allZeros);
            _mm_storeu_si128 ((__m128i *)(rCoeff + rowIdx*coeffStride + 16), allZeros);
            _mm_storeu_si128 ((__m128i *)(rCoeff + rowIdx*coeffStride + 24), allZeros);

            _mm_storeu_si128 ((__m128i *)(qCoeff + rowIdx*coeffStride),      allZeros);
            _mm_storeu_si128 ((__m128i *)(qCoeff + rowIdx*coeffStride + 8),  allZeros);
            _mm_storeu_si128 ((__m128i *)(qCoeff + rowIdx*coeffStride + 16), allZeros);
            _mm_storeu_si128 ((__m128i *)(qCoeff + rowIdx*coeffStride + 24), allZeros);
        }
    }

    for (blockIdx = blockNum - 1; blockIdx >= 0; blockIdx--)
    {
        blockPos = scanOrderCG[blockIdx].posIdx;

        blockY      = blockPos / widthInCG;
        blockX      = blockPos - blockY*widthInCG;
        tCoeffBlock = tCoeff + blockY*cGHeight*coeffStride + blockX*cGWidth;

        coef0 = _mm_loadl_epi64 ((__m128i *)(tCoeffBlock + coeffStride*0));
        coef1 = _mm_loadl_epi64 ((__m128i *)(tCoeffBlock + coeffStride*1));
        coef2 = _mm_loadl_epi64 ((__m128i *)(tCoeffBlock + coeffStride*2));
        coef3 = _mm_loadl_epi64 ((__m128i *)(tCoeffBlock + coeffStride*3));

        coef01 = _mm_unpacklo_epi64 (coef0, coef1);
        coef23 = _mm_unpacklo_epi64 (coef2, coef3);

        coef01 = _mm_abs_epi16 (coef01);
        coef23 = _mm_abs_epi16 (coef23);

        coef01 = _mm_cmpgt_epi16 (coef01, thrValx16);
        coef23 = _mm_cmpgt_epi16 (coef23, thrValx16);

        coef0123   = _mm_or_si128 (coef01, coef23);
        smallBlock = _mm_test_all_zeros (coef0123, allOnes);

        if (!smallBlock)
        {
            break;
        }

    }

    // Find last significant coefficient scan index
    if (blockIdx >= 0)
    {
        tCoeffBlock = tCoeff + blockY*cGHeight*coeffStride + blockX*cGWidth;

        for (coeffIdx = cGWidth*cGHeight - 1; coeffIdx >= 0 ; coeffIdx--)
        {
            innerPos    = scanOrder[coeffIdx].posIdx;
            innerY      = innerPos / cGWidth;
            innerX      = innerPos - innerY*cGWidth;

            if (XIN_ABS (tCoeffBlock[innerX + innerY*coeffStride]) > rdoqThrVal)
            {
                break;
            }
        }

        *nzCoeffIdx = blockIdx*(cGWidth*cGHeight) + coeffIdx;
    }

}


