/***************************************************************************//**
 *
 * @file          h266_quant_inv_quant.h
 * @brief         This file declares h266 quant/inv-quant subroutines.
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
#ifndef _h266_quant_inv_quant_h_
#define _h266_quant_inv_quant_h_

void Xin266QuantInvQuant (
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
    UINT64   *nzCGBitMapRs);

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
    UINT64   *nzCGBitMapRs);

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
    UINT64   *nzCGBitMapRs);

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
    UINT64   *nzCGBitMapRs);

void Xin266QuantInvQuant4x4_Neon (
    COEFF    *qCoeff,
    COEFF    *tCoeff,
    COEFF    *rCoeff,
    intptr_t coeffStride,
    UINT32   size,
    SINT32   qMult,
    SINT32   qAdd,
    SINT32   qShift,
    SINT32   iqMult,
    SINT32   iqAdd,
    SINT32   iqShift,
    UINT64   *nz4x4BitMapRs);

void Xin266QuantInvQuantGt4x4_Neon (
    COEFF    *qCoeff,
    COEFF    *tCoeff,
    COEFF    *rCoeff,
    intptr_t coeffStride,
    UINT32   size,
    SINT32   qMult,
    SINT32   qAdd,
    SINT32   qShift,
    SINT32   iqMult,
    SINT32   iqAdd,
    SINT32   iqShift,
    UINT64   *nz4x4BitMapRs);

void Xin266QuantInvQuantAround (
    COEFF    *qCoeff,
    COEFF    *tCoeff,
    COEFF    *rCoeff,
    SINT16   *adjBias,
    intptr_t coeffStride,
    UINT32   size,
    SINT32   qMult,
    SINT32   *qOffset,
    SINT32   qShift,
    SINT32   iqMult,
    SINT32   iqAdd,
    SINT32   iqShift,
    UINT64   *nz4x4BitMapRs);

void Xin266QuantInvQuantAround4x4_SSE4 (
    COEFF    *qCoeff,
    COEFF    *tCoeff,
    COEFF    *rCoeff,
    SINT16   *adjBias,
    intptr_t coeffStride,
    UINT32   size,
    SINT32   qMult,
    SINT32   *qOffset,
    SINT32   qShift,
    SINT32   iqMult,
    SINT32   iqAdd,
    SINT32   iqShift,
    UINT64   *nz4x4BitMapRs);

void Xin266QuantInvQuantAroundGt8x8_AVX2 (
    COEFF    *qCoeff,
    COEFF    *tCoeff,
    COEFF    *rCoeff,
    SINT16   *adjBias,
    intptr_t coeffStride,
    UINT32   size,
    SINT32   qMult,
    SINT32   *qOffset,
    SINT32   qShift,
    SINT32   iqMult,
    SINT32   iqAdd,
    SINT32   iqShift,
    UINT64   *nz4x4BitMapRs);

void Xin266QuantInvQuantAround8x8_AVX2 (
    COEFF    *qCoeff,
    COEFF    *tCoeff,
    COEFF    *rCoeff,
    SINT16   *adjBias,
    intptr_t coeffStride,
    UINT32   size,
    SINT32   qMult,
    SINT32   *qOffset,
    SINT32   qShift,
    SINT32   iqMult,
    SINT32   iqAdd,
    SINT32   iqShift,
    UINT64   *nz4x4BitMapRs);

void Xin266GetBlockDeltaU (
    COEFF    *qCoeff,
    COEFF    *tCoeff,
    COEFF    *deltaU,
    UINT32   cgWidth,
    UINT32   cgHeight,
    UINT16   *coeffSign,
    intptr_t coeffStride,
    SINT32   qMult,
    SINT32   qShift);

void Xin266GetBlockDeltaU_SSE4 (
    COEFF    *qCoeff,
    COEFF    *tCoeff,
    COEFF    *deltaU,
    UINT32   cgWidth,
    UINT32   cgHeight,
    UINT16   *coeffSign,
    intptr_t coeffStride,
    SINT32   qMult,
    SINT32   qShift);

void Xin266ComputeBlockDeltaU (
    COEFF    *tCoef,
    COEFF    *rCoef,
    COEFF    *qCoef,
    intptr_t coefStride,
    SINT32   iqMult,
    SINT32   iqShift,
    UINT32   cgWidth,
    UINT32   cgHeight,
    SINT32   *distNow,
    SINT32   *distUp,
    SINT32   *distDown,
    UINT16   *coeffSign);

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
    UINT16   *coeffSign);

void Xin266PreRdoq (
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
    UINT64       *nzCGBitMapRs);

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
    UINT64       *nzCGBitMapRs);

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
    SINT32       *nzCoeffIdx);

void Xin266PreDepQuant (
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
    SINT32       *nzCoeffIdx);

#endif

