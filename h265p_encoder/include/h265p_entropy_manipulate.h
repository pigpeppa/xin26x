/***************************************************************************//**
 *
 * @file          h265p_entropy_manipulate.h
 * @brief         This file declares av1 entropy estimation and write subroutines.
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
#ifndef _h265p_entropy_manipulate_h_
#define _h265p_entropy_manipulate_h_

void Xin265pWriteMbRec (
    xin_sec_struct *secSet,
    xin_mb_struct  *mb);

void Xin265pEstimateMbSynatax (
    xin_sec_struct  *secSet,
    xin_mb_struct   *mb,
    xin_fast_md_buf *fastBuf,
    UINT32          *bitNum);

void Xin265pInitModeRate (
    xin_pic_struct *picSet,
    xin_cdf_prob   *context,
    xin_cabac_est  *cabacEst);

void Xin265pInitCoeffRate (
    xin_cdf_prob   *context,
    xin_cabac_est  *cabacEst);

void Xin265pInitModeProb (
    xin_cabac_context *cabacSet);

void Xin265pInitCoeffProb (
    xin_cabac_context *cabacSet,
    SINT32            baseQIndex);

void Xin265pCabacContextInit (
    xin_cabac_context *cabacSet,
    UINT8             *context,
    SINT32            qpIdx,
    BOOL              resetContext);

void Xin265pGetEobPosToken (
    SINT32 eob,
    SINT32 *extra,
    SINT32 *eobPt);

void Xin265pGetExtTxSetType (
    UINT32 txSize,
    BOOL   isInter,
    BOOL   useReducedSet,
    UINT32 *txSetType);

void Xin265pTxInitLevel (
    COEFF    *coeff,
    intptr_t coeffStride,
    SINT32   width,
    SINT32   height,
    UINT8    *level,
    intptr_t levelStride);

void Xin265pGetNzMapContext (
    UINT8          *level,
    intptr_t       levelStride,
    const SINT16   *scanOrder,
    SINT32         eob,
    UINT32         txSize,
    UINT32         txClass,
    SINT8          *coeffContext);

SINT32 Xin265pGetBrCtx (
    UINT8    *levelBuf,
    intptr_t levelStride,
    SINT32   posX,
    SINT32   posY,
    UINT32   txClass);

void Xin265pEstimateMbCoeff (
    xin_sec_struct  *secSet,
    xin_mb_struct   *mb,
    xin_full_md_buf *fullBuf,
    UINT32          *lumaBits,
    UINT32          *chromaBits);

void Xin265pEstimateCoeff (
    xin_sec_struct  *secSet,
    xin_mb_struct   *mb,
    xin_full_md_buf *fullBuf,
    UINT32          *coefBits,
    UINT32          planeIdx);

void Xin265pEstimateCoeffSkipFlag (
    xin_sec_struct  *secSet,
    xin_full_md_buf *fullBuf,
    xin_mb_struct   *mb,
    UINT32          *bitNum);

void Xin265pFullBufLoadCtx (
    xin_full_md_buf *fullBuf,
    xin_mb_struct   *mb);

void Xin265pComputeEob (
    xin_full_md_buf *fullBuf,
    intptr_t        coefAddr,
    xin_tu_struct   *tu,
    UINT32          planeIdx);

void Xin265pWriteDone (
    xin_cabac_struct *cabac,
    xin_bs_struct    *bitstream);

#endif
