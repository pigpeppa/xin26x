/***************************************************************************//**
*
* @file          h265p_quant_inv_quant.h
* @brief         This file declare h265 qiq subroutines.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _h265p_quant_inv_quant_h_
#define _h265p_quant_inv_quant_h_

void Xin265pQuantInvQuantB (
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
    UINT32   *nonZeroCount);

void Xin265pQuantInvQuantFp (
    COEFF    *qCoeff,
	SINT32   *tCoeff,
	SINT32   *rCoeff,
    intptr_t coeffStride,
    SINT32   width,
    SINT32   height,
    SINT32   logScale,
    SINT32   *qAdd,
    SINT32   *qMult,
    SINT32   *qShift,
    SINT32   *iqMult,
    UINT32   *nonZeroCount);

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
    UINT32   *nonZeroCount);

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
    UINT32   *nonZeroCount);

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
    UINT32   *nonZeroCount);

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
    UINT32   *nonZeroCount);

#endif
