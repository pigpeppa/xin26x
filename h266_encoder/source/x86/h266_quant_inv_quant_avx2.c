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

static const UINT8 s_nZMask0[32] =
{
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const UINT8 s_nZMask2[32] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const UINT8 qAddLoShuf[32] =
{
    0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1,
    0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1
};

static const UINT8 qAddHiShuf[32] =
{
    0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 0x2, 0x3,
    0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 0x2, 0x3
};

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
    __m256i qMultx16, iqMultx16;
    __m256i qAddx8, iqAddx8;
    __m256i loShuf, hiShuf;
    __m256i qAddLox16, qAddHix16;
    __m256i coef0x16, coef1x16, coef2x16, coef3x16;
    __m256i coef0Mag, coef1Mag, coef2Mag, coef3Mag;
    __m256i coef0MagL, coef1MagL, coef2MagL, coef3MagL;
    __m256i coef0MagH, coef1MagH, coef2MagH, coef3MagH;
    __m256i levl0Mag, levl1Mag, levl2Mag, levl3Mag;
    __m256i level0, level1, level2, level3;
    __m256i scaledL0, scaledL1, scaledL2, scaledL3;
    __m256i scaledH0, scaledH1, scaledH2, scaledH3;
    __m256i recon0, recon1;
    __m256i recon2, recon3;
    __m256i nZBitMap;
    __m256i allZero;
    __m256i nZMask0, nZMask1, nZMask2, nZMask3;
    UINT32  nZBlock0, nZBlock1, nZBlock2, nZBlock3;
    UINT32  nZBlock;
    UINT64  nZBlockRs;
    UINT32  colIdx, rowIdx;
    UINT32  blockIdx;

    (void)cGWidth;
    (void)cGHeight;
    
    qMultx16  = _mm256_set1_epi16 ((UINT16)qMult);
    iqMultx16 = _mm256_set1_epi16 ((UINT16)iqMult);
    iqAddx8   = _mm256_set1_epi32 ((UINT32)iqAdd);
    qAddx8    = _mm256_set1_epi32 ((UINT32)qAdd);
    loShuf    = _mm256_lddqu_si256 ((__m256i *)qAddLoShuf);
    hiShuf    = _mm256_lddqu_si256 ((__m256i *)qAddHiShuf);
    qAddLox16 = _mm256_shuffle_epi8 (qAddx8, loShuf);
    qAddHix16 = _mm256_shuffle_epi8 (qAddx8, hiShuf);
    nZMask0   = _mm256_lddqu_si256 ((__m256i *)s_nZMask0);
    nZMask1   = _mm256_slli_si256 (nZMask0, 8);
    nZMask2   = _mm256_lddqu_si256 ((__m256i *)s_nZMask2);
    nZMask3   = _mm256_slli_si256 (nZMask2, 8);
    allZero   = _mm256_setzero_si256 ();
    qShift    = qShift - 16;
    nZBlockRs = 0;
    blockIdx  = 0;

    for (rowIdx = 0; rowIdx < height; rowIdx += 4)
    {
        for (colIdx = 0; colIdx < width; colIdx += 16)
        {
            coef0x16 = _mm256_lddqu_si256 ((__m256i *)(tCoeff + colIdx + coeffStride*0));
            coef1x16 = _mm256_lddqu_si256 ((__m256i *)(tCoeff + colIdx + coeffStride*1));
            coef2x16 = _mm256_lddqu_si256 ((__m256i *)(tCoeff + colIdx + coeffStride*2));
            coef3x16 = _mm256_lddqu_si256 ((__m256i *)(tCoeff + colIdx + coeffStride*3));

            coef0Mag = _mm256_abs_epi16 (coef0x16);
            coef1Mag = _mm256_abs_epi16 (coef1x16);
            coef2Mag = _mm256_abs_epi16 (coef2x16);
            coef3Mag = _mm256_abs_epi16 (coef3x16);

            coef0MagL = _mm256_mullo_epi16 (coef0Mag, qMultx16);
            coef1MagL = _mm256_mullo_epi16 (coef1Mag, qMultx16);
            coef2MagL = _mm256_mullo_epi16 (coef2Mag, qMultx16);
            coef3MagL = _mm256_mullo_epi16 (coef3Mag, qMultx16);

            coef0MagH = _mm256_mulhi_epi16 (coef0Mag, qMultx16);
            coef1MagH = _mm256_mulhi_epi16 (coef1Mag, qMultx16);
            coef2MagH = _mm256_mulhi_epi16 (coef2Mag, qMultx16);
            coef3MagH = _mm256_mulhi_epi16 (coef3Mag, qMultx16);

            coef0MagL = _mm256_avg_epu16 (coef0MagL, qAddLox16);
            coef1MagL = _mm256_avg_epu16 (coef1MagL, qAddLox16);
            coef2MagL = _mm256_avg_epu16 (coef2MagL, qAddLox16);
            coef3MagL = _mm256_avg_epu16 (coef3MagL, qAddLox16);

            coef0MagH = _mm256_adds_epu16 (coef0MagH, qAddHix16);
            coef1MagH = _mm256_adds_epu16 (coef1MagH, qAddHix16);
            coef2MagH = _mm256_adds_epu16 (coef2MagH, qAddHix16);
            coef3MagH = _mm256_adds_epu16 (coef3MagH, qAddHix16);

            coef0MagL = _mm256_srli_epi16 (coef0MagL, 15);
            coef1MagL = _mm256_srli_epi16 (coef1MagL, 15);
            coef2MagL = _mm256_srli_epi16 (coef2MagL, 15);
            coef3MagL = _mm256_srli_epi16 (coef3MagL, 15);

            levl0Mag = _mm256_adds_epu16 (coef0MagL, coef0MagH);
            levl1Mag = _mm256_adds_epu16 (coef1MagL, coef1MagH);
            levl2Mag = _mm256_adds_epu16 (coef2MagL, coef2MagH);
            levl3Mag = _mm256_adds_epu16 (coef3MagL, coef3MagH);

            levl0Mag = _mm256_srli_epi16 (levl0Mag, qShift);
            levl1Mag = _mm256_srli_epi16 (levl1Mag, qShift);
            levl2Mag = _mm256_srli_epi16 (levl2Mag, qShift);
            levl3Mag = _mm256_srli_epi16 (levl3Mag, qShift);

            level0   = _mm256_sign_epi16 (levl0Mag, coef0x16);
            level1   = _mm256_sign_epi16 (levl1Mag, coef1x16);
            level2   = _mm256_sign_epi16 (levl2Mag, coef2x16);
            level3   = _mm256_sign_epi16 (levl3Mag, coef3x16);

            nZBitMap = _mm256_or_si256 (level0,   level1);
            nZBitMap = _mm256_or_si256 (nZBitMap, level2);
            nZBitMap = _mm256_or_si256 (nZBitMap, level3);
            nZBlock0 = _mm256_testz_si256 (nZBitMap, nZMask0);
            nZBlock1 = _mm256_testz_si256 (nZBitMap, nZMask1);
            nZBlock2 = _mm256_testz_si256 (nZBitMap, nZMask2);
            nZBlock3 = _mm256_testz_si256 (nZBitMap, nZMask3);
            nZBlock  = (!nZBlock0) | ((!nZBlock1) << 1) | ((!nZBlock2) << 2) | ((!nZBlock3) << 3);

            _mm256_storeu_si256 ((__m256i *)(qCoeff + colIdx + coeffStride*0), level0);
            _mm256_storeu_si256 ((__m256i *)(qCoeff + colIdx + coeffStride*1), level1);
            _mm256_storeu_si256 ((__m256i *)(qCoeff + colIdx + coeffStride*2), level2);
            _mm256_storeu_si256 ((__m256i *)(qCoeff + colIdx + coeffStride*3), level3);

            _mm256_storeu_si256 ((__m256i *)(rCoeff + colIdx + coeffStride*0), allZero);
            _mm256_storeu_si256 ((__m256i *)(rCoeff + colIdx + coeffStride*1), allZero);
            _mm256_storeu_si256 ((__m256i *)(rCoeff + colIdx + coeffStride*2), allZero);
            _mm256_storeu_si256 ((__m256i *)(rCoeff + colIdx + coeffStride*3), allZero);

            nZBlockRs |= ((UINT64)nZBlock << blockIdx);

            if (nZBlock)
            {
                scaledL0  = _mm256_unpacklo_epi16 (level0, allZero);
                scaledH0  = _mm256_unpackhi_epi16 (level0, allZero);
                scaledL0  = _mm256_madd_epi16 (scaledL0, iqMultx16);
                scaledH0  = _mm256_madd_epi16 (scaledH0, iqMultx16);
                scaledL0  = _mm256_add_epi32 (scaledL0, iqAddx8);
                scaledH0  = _mm256_add_epi32 (scaledH0, iqAddx8);
                scaledL0  = _mm256_srai_epi32 (scaledL0, iqShift);
                scaledH0  = _mm256_srai_epi32 (scaledH0, iqShift);
                recon0    = _mm256_packs_epi32 (scaledL0, scaledH0);

                scaledL1  = _mm256_unpacklo_epi16 (level1, allZero);
                scaledH1  = _mm256_unpackhi_epi16 (level1, allZero);
                scaledL1  = _mm256_madd_epi16 (scaledL1, iqMultx16);
                scaledH1  = _mm256_madd_epi16 (scaledH1, iqMultx16);
                scaledL1  = _mm256_add_epi32 (scaledL1, iqAddx8);
                scaledH1  = _mm256_add_epi32 (scaledH1, iqAddx8);
                scaledL1  = _mm256_srai_epi32 (scaledL1, iqShift);
                scaledH1  = _mm256_srai_epi32 (scaledH1, iqShift);
                recon1    = _mm256_packs_epi32 (scaledL1, scaledH1);

                scaledL2  = _mm256_unpacklo_epi16 (level2, allZero);
                scaledH2  = _mm256_unpackhi_epi16 (level2, allZero);
                scaledL2  = _mm256_madd_epi16 (scaledL2, iqMultx16);
                scaledH2  = _mm256_madd_epi16 (scaledH2, iqMultx16);
                scaledL2  = _mm256_add_epi32 (scaledL2, iqAddx8);
                scaledH2  = _mm256_add_epi32 (scaledH2, iqAddx8);
                scaledL2  = _mm256_srai_epi32 (scaledL2, iqShift);
                scaledH2  = _mm256_srai_epi32 (scaledH2, iqShift);
                recon2    = _mm256_packs_epi32 (scaledL2, scaledH2);

                scaledL3  = _mm256_unpacklo_epi16 (level3, allZero);
                scaledH3  = _mm256_unpackhi_epi16 (level3, allZero);
                scaledL3  = _mm256_madd_epi16 (scaledL3, iqMultx16);
                scaledH3  = _mm256_madd_epi16 (scaledH3, iqMultx16);
                scaledL3  = _mm256_add_epi32 (scaledL3, iqAddx8);
                scaledH3  = _mm256_add_epi32 (scaledH3, iqAddx8);
                scaledL3  = _mm256_srai_epi32 (scaledL3, iqShift);
                scaledH3  = _mm256_srai_epi32 (scaledH3, iqShift);
                recon3    = _mm256_packs_epi32 (scaledL3, scaledH3);

                _mm256_storeu_si256 ((__m256i *)(rCoeff + colIdx + coeffStride*0), recon0);
                _mm256_storeu_si256 ((__m256i *)(rCoeff + colIdx + coeffStride*1), recon1);
                _mm256_storeu_si256 ((__m256i *)(rCoeff + colIdx + coeffStride*2), recon2);
                _mm256_storeu_si256 ((__m256i *)(rCoeff + colIdx + coeffStride*3), recon3);

            }

            blockIdx += 4;

        }

        rCoeff += 4*coeffStride;
        tCoeff += 4*coeffStride;
        qCoeff += 4*coeffStride;

    }

    *nzCGBitMapRs = nZBlockRs;

}

