/***************************************************************************//**
*
* @file          h265p_compute_dist_avx2.c
* @brief         Compute sad or sse in frequence domain.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#include "immintrin.h"
#include "xin_typedef.h"
#include "h26x_definition.h"
#include "h265p_definition.h"
#include "h265p_constant.h"
#include "h265p_trans_context.h"
#include "h265p_common_data.h"
#include "basic_macro.h"

void Xin265pComputeSsdFdGt16xH_AVX2 (
    SINT32   *tCoeff,
    intptr_t tCoeffStride,
    SINT32   *rCoeff,
    intptr_t rCoeffStride,
    UINT32   width,
    UINT32   height,
    SINT32   txSize,
    UINT64   *ssd)
{
    UINT32  rowIdx, colIdx;
    __m256i tCoef0x8, tCoef1x8;
    __m256i tCoef2x8, tCoef3x8;
    __m256i tCoef0x16, tCoef1x16;
    __m256i rCoef0x8, rCoef1x8;
    __m256i rCoef2x8, rCoef3x8;
    __m256i rCoef0x16, rCoef1x16;
    __m256i diff0x16, diff1x16;
    __m256i sseLx8, sseRx8;
    __m256i sse0x8;
    __m256i sse1x8;
    __m128i ssd128;
    SINT32  txShift;

    sse0x8  = _mm256_setzero_si256 ();
    sse1x8  = _mm256_setzero_si256 ();
    txShift = (1 - txSize2LogScale[txSize])*2;

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        for (colIdx = 0; colIdx < width; colIdx += 32)
        {
            tCoef0x8 = _mm256_loadu_si256 ((__m256i *)(tCoeff + colIdx));
            tCoef1x8 = _mm256_loadu_si256 ((__m256i *)(tCoeff + colIdx + 8));
            tCoef2x8 = _mm256_loadu_si256 ((__m256i *)(tCoeff + colIdx + 16));
            tCoef3x8 = _mm256_loadu_si256 ((__m256i *)(tCoeff + colIdx + 24));

            tCoef0x16 = _mm256_packs_epi32 (tCoef0x8, tCoef1x8);
            tCoef0x16 = _mm256_permute4x64_epi64 (tCoef0x16, 0xD8);

            tCoef1x16 = _mm256_packs_epi32 (tCoef2x8, tCoef3x8);
            tCoef1x16 = _mm256_permute4x64_epi64 (tCoef1x16, 0xD8);
            
            rCoef0x8 = _mm256_loadu_si256 ((__m256i *)(rCoeff + colIdx));
            rCoef1x8 = _mm256_loadu_si256 ((__m256i *)(rCoeff + colIdx + 8));
            rCoef2x8 = _mm256_loadu_si256 ((__m256i *)(rCoeff + colIdx + 16));
            rCoef3x8 = _mm256_loadu_si256 ((__m256i *)(rCoeff + colIdx + 24));

            rCoef0x16 = _mm256_packs_epi32 (rCoef0x8, rCoef1x8);
            rCoef0x16 = _mm256_permute4x64_epi64 (rCoef0x16, 0xD8);

            rCoef1x16 = _mm256_packs_epi32 (rCoef2x8, rCoef3x8);
            rCoef1x16 = _mm256_permute4x64_epi64 (rCoef1x16, 0xD8);
            
            diff0x16 = _mm256_sub_epi16 (tCoef0x16, rCoef0x16);
            diff1x16 = _mm256_sub_epi16 (tCoef1x16, rCoef1x16);
            
            sseLx8  = _mm256_madd_epi16 (diff0x16, diff0x16);
            sseRx8  = _mm256_madd_epi16 (diff1x16, diff1x16);
            sse0x8  = _mm256_add_epi32 (sse0x8, _mm256_add_epi32 (sseLx8, sseRx8));

            sseLx8  = _mm256_madd_epi16 (tCoef0x16, tCoef0x16);
            sseRx8  = _mm256_madd_epi16 (tCoef1x16, tCoef1x16);
            sse1x8  = _mm256_add_epi32 (sse1x8, _mm256_add_epi32 (sseLx8, sseRx8));
            
        }

        tCoeff += tCoeffStride;
        rCoeff += rCoeffStride;
        
    }

    sse0x8 = _mm256_add_epi32 (sse0x8,  _mm256_permute4x64_epi64 (sse0x8, 0x4E));
    sse0x8 = _mm256_add_epi32 (sse0x8,  _mm256_shuffle_epi32 (sse0x8, 0x4E));
    ssd128 = _mm256_castsi256_si128 (sse0x8); 
    ssd128 = _mm_add_epi32 (ssd128, _mm_shuffle_epi32 (ssd128, 0xB1));
    ssd[0] = _mm_cvtsi128_si32 (ssd128);

    sse1x8 = _mm256_add_epi32 (sse1x8,  _mm256_permute4x64_epi64 (sse1x8, 0x4E));
    sse1x8 = _mm256_add_epi32 (sse1x8,  _mm256_shuffle_epi32 (sse1x8, 0x4E));
    ssd128 = _mm256_castsi256_si128 (sse1x8); 
    ssd128 = _mm_add_epi32 (ssd128, _mm_shuffle_epi32 (ssd128, 0xB1));
    ssd[1] = _mm_cvtsi128_si32 (ssd128);

    ssd[0] = XIN_SIGNED_SHIFT(ssd[0], txShift);
    ssd[1] = XIN_SIGNED_SHIFT(ssd[1], txShift);

}


