/***************************************************************************//**
 *
 * @file          h266_quant_inv_quant_avx2.c
 * @brief         h266 forward quantization and inverse quantization (AVX2).
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
#ifdef __linux__
#include "xin_avx2_linux_patch.h"
#endif

static inline __m256i xQuant16 (
    __m256i coef,
    __m256i qMult,
    __m256i qAddLo,
    __m256i qAddHi,
    SINT32  qShift)
{
    __m256i mag;
    __m256i magL;
    __m256i magH;
    __m256i level;

    mag   = _mm256_abs_epi16 (coef);
    magL  = _mm256_mullo_epi16 (mag, qMult);
    magH  = _mm256_mulhi_epi16 (mag, qMult);
    magL  = _mm256_avg_epu16 (magL, qAddLo);
    magH  = _mm256_adds_epu16 (magH, qAddHi);
    magL  = _mm256_srli_epi16 (magL, 15);
    level = _mm256_adds_epu16 (magL, magH);
    level = _mm256_srli_epi16 (level, qShift);

    return _mm256_sign_epi16 (level, coef);
}

static inline __m256i xDequant16 (
    __m256i level,
    __m256i iqMult,
    __m256i iqAdd,
    SINT32  iqShift)
{
    __m256i lo;
    __m256i hi;
    __m256i zero = _mm256_setzero_si256 ();

    lo = _mm256_unpacklo_epi16 (level, zero);
    hi = _mm256_unpackhi_epi16 (level, zero);
    lo = _mm256_madd_epi16 (lo, iqMult);
    hi = _mm256_madd_epi16 (hi, iqMult);
    lo = _mm256_add_epi32 (lo, iqAdd);
    hi = _mm256_add_epi32 (hi, iqAdd);
    lo = _mm256_srai_epi32 (lo, iqShift);
    hi = _mm256_srai_epi32 (hi, iqShift);

    return _mm256_packs_epi32 (lo, hi);
}

void Xin266QuantInvQuantGt8xH_AVX2 (
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
    const __m256i qMultx16  = _mm256_set1_epi16 ((SINT16)qMult);
    const __m256i iqMultx16 = _mm256_set1_epi16 ((SINT16)iqMult);
    const __m256i iqAddx8   = _mm256_set1_epi32 (iqAdd);
    /* Low/high 16 bits of qAdd, broadcast as epi16 (replaces shuffle tables). */
    const __m256i qAddLox16 = _mm256_set1_epi16 ((SINT16)(UINT16)qAdd);
    const __m256i qAddHix16 = _mm256_set1_epi16 ((SINT16)(UINT16)((UINT32)qAdd >> 16));
    const __m256i allZero   = _mm256_setzero_si256 ();
    /* 4×4 CG masks: 4 groups × 4×int16 (= 2×int32) inside the 16-coeff vector. */
    const __m256i nZMask0   = _mm256_setr_epi32 (-1, -1,  0,  0,  0,  0,  0,  0);
    const __m256i nZMask1   = _mm256_setr_epi32 ( 0,  0, -1, -1,  0,  0,  0,  0);
    const __m256i nZMask2   = _mm256_setr_epi32 ( 0,  0,  0,  0, -1, -1,  0,  0);
    const __m256i nZMask3   = _mm256_setr_epi32 ( 0,  0,  0,  0,  0,  0, -1, -1);
    UINT64        nZBlockRs = 0;
    UINT32        blockIdx  = 0;
    UINT32        colIdx;
    UINT32        rowIdx;
    SINT32        qShiftAdj = qShift - 16;

    (void)cGWidth;
    (void)cGHeight;

    for (rowIdx = 0; rowIdx < height; rowIdx += 4)
    {
        for (colIdx = 0; colIdx < width; colIdx += 16)
        {
            __m256i coef0 = _mm256_loadu_si256 ((const __m256i *)(tCoeff + colIdx + coeffStride * 0));
            __m256i coef1 = _mm256_loadu_si256 ((const __m256i *)(tCoeff + colIdx + coeffStride * 1));
            __m256i coef2 = _mm256_loadu_si256 ((const __m256i *)(tCoeff + colIdx + coeffStride * 2));
            __m256i coef3 = _mm256_loadu_si256 ((const __m256i *)(tCoeff + colIdx + coeffStride * 3));
            __m256i level0;
            __m256i level1;
            __m256i level2;
            __m256i level3;
            __m256i nZBitMap;
            UINT32  nZBlock;

            level0 = xQuant16 (coef0, qMultx16, qAddLox16, qAddHix16, qShiftAdj);
            level1 = xQuant16 (coef1, qMultx16, qAddLox16, qAddHix16, qShiftAdj);
            level2 = xQuant16 (coef2, qMultx16, qAddLox16, qAddHix16, qShiftAdj);
            level3 = xQuant16 (coef3, qMultx16, qAddLox16, qAddHix16, qShiftAdj);

            _mm256_storeu_si256 ((__m256i *)(qCoeff + colIdx + coeffStride * 0), level0);
            _mm256_storeu_si256 ((__m256i *)(qCoeff + colIdx + coeffStride * 1), level1);
            _mm256_storeu_si256 ((__m256i *)(qCoeff + colIdx + coeffStride * 2), level2);
            _mm256_storeu_si256 ((__m256i *)(qCoeff + colIdx + coeffStride * 3), level3);

            nZBitMap = _mm256_or_si256 (_mm256_or_si256 (level0, level1),
                                        _mm256_or_si256 (level2, level3));
            nZBlock  = (!_mm256_testz_si256 (nZBitMap, nZMask0))
                     | ((!_mm256_testz_si256 (nZBitMap, nZMask1)) << 1)
                     | ((!_mm256_testz_si256 (nZBitMap, nZMask2)) << 2)
                     | ((!_mm256_testz_si256 (nZBitMap, nZMask3)) << 3);

            nZBlockRs |= ((UINT64)nZBlock << blockIdx);

            if (nZBlock)
            {
                /* Zero levels naturally dequant to 0 — no need to pre-clear rCoeff. */
                _mm256_storeu_si256 ((__m256i *)(rCoeff + colIdx + coeffStride * 0),
                                     xDequant16 (level0, iqMultx16, iqAddx8, iqShift));
                _mm256_storeu_si256 ((__m256i *)(rCoeff + colIdx + coeffStride * 1),
                                     xDequant16 (level1, iqMultx16, iqAddx8, iqShift));
                _mm256_storeu_si256 ((__m256i *)(rCoeff + colIdx + coeffStride * 2),
                                     xDequant16 (level2, iqMultx16, iqAddx8, iqShift));
                _mm256_storeu_si256 ((__m256i *)(rCoeff + colIdx + coeffStride * 3),
                                     xDequant16 (level3, iqMultx16, iqAddx8, iqShift));
            }
            else
            {
                _mm256_storeu_si256 ((__m256i *)(rCoeff + colIdx + coeffStride * 0), allZero);
                _mm256_storeu_si256 ((__m256i *)(rCoeff + colIdx + coeffStride * 1), allZero);
                _mm256_storeu_si256 ((__m256i *)(rCoeff + colIdx + coeffStride * 2), allZero);
                _mm256_storeu_si256 ((__m256i *)(rCoeff + colIdx + coeffStride * 3), allZero);
            }

            blockIdx += 4;
        }

        rCoeff += 4 * coeffStride;
        tCoeff += 4 * coeffStride;
        qCoeff += 4 * coeffStride;
    }

    *nzCGBitMapRs = nZBlockRs;
}
