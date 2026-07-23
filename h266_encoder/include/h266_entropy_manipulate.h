/***************************************************************************//**
 *
 * @file          h266_entropy_manipulate.h
 * @brief         h266 entropy estimation and write subroutines declaration.
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
#ifndef _h266_entropy_manipulate_h_
#define _h266_entropy_manipulate_h_

extern const UINT8 ExpGolombBits[16383*2+1];

void Xin266EncodeTerminate (
    xin_cabac_struct *cabac,
    UINT32           binValue);

void Xin266EncodeFinish (
    xin_cabac_struct *cabac);

void Xin266EstiamteSaoBoOffsets (
    SINT8   offset[4],
    UINT32  *bitNum);

void Xin266EstimateSaoMerge (
    xin_prob_model *context,
    BOOL           mergeFlag,
    UINT32         *bitNum);

void Xin266EstimateSaoType (
    xin_prob_model *context,
    SINT32         saoType,
    UINT32         *bitNum);

void Xin266EstimateSaoParam (
    xin_prob_model *context,
    xin_ctu_struct *ctu,
    UINT32         planeIdx,
    UINT32         *bitNum);

void Xin266EstiamteSaoEoOffsets (
    SINT8   offset[5],
    UINT32  *bitNum);

void Xin266EstimateCuCbf (
    xin_prob_model  *context,
    BOOL            updateContext,
    xin_cu_struct   *cu,
    xin_fast_md_buf *fastBuf,
    UINT32          *cbfBits);

void Xin266EstimateCuSynatax (
    xin_sec_struct  *secSet,
    xin_prob_model  *context,
    BOOL            updateContext,
    xin_cu_struct   *cu,
    xin_fast_md_buf *fastBuf,
    UINT32          *bitNum);

void Xin266EstimateSplitType (
    xin_prob_model *context,
    BOOL           updateCabacState,
    xin_cu_struct  *cu,
    UINT32         splitType,
    UINT32         *bitNum);

void Xin266EstimateInterPredIdc (
    xin_prob_model *context,
    BOOL           updateContext,
    SINT32         interPredIdc,
    UINT32         lgWidth,
    UINT32         lgHeight,
    UINT32         *bitNum);

void Xin266EstimateNormalMvd (
    xin_prob_model *context,
    BOOL           updateContext,
    xin_mv32_u     *mv,
    xin_mv32_u     *predMv,
    SINT32         imvIdx,
    UINT32         *bitNum);

void Xin266EstimateAffineMvd (
    xin_prob_model *context,
    BOOL           updateContext,
    xin_mv32_u     *mv,
    xin_mv32_u     *predMv,
    SINT32         imvIdx,
    UINT32         affineType,
    UINT32         *bitNum);

void Xin266EstimateRefIdx (
    xin_prob_model *context,
    BOOL           updateContext,
    UINT32         refIdx,
    UINT32         refNum,
    UINT32         *bitNum);

void Xin266EstimateMvpIdx (
    xin_prob_model *context,
    BOOL           updateContext,
    UINT32         mvpIdx,
    UINT32         *bitNum);

void Xin266EstimateBcw (
    xin_prob_model  *context,
    BOOL            updateContext,
    UINT32          bcwIdx,
    BOOL            checkLdc,
    UINT32          *bitNum);

void Xin266EstimateIntraPredMode (
    xin_prob_model *context,
    BOOL           updateContext,
    SINT32         lumaMode,
    xin_pu_struct  *pu,
    UINT32         *bitNum);

void Xin266EstimateMergeIndex (
    xin_prob_model *context,
    BOOL           updateContext,
    UINT32         mergeIdx,
    UINT32         maxMergeCand,
    UINT32         *bitNum);

void Xin266EstimateMergeFlag (
    xin_prob_model *context,
    BOOL           updateContext,
    UINT32         mergeFlag,
    UINT32         *bitNum);

void Xin266EstimateSubblockMergeFlag (
    xin_prob_model *context,
    xin_cu_struct  *cu,
    BOOL           updateContext,
    UINT32         mergeFlag,
    UINT32         *bitNum);

void Xin266EstimateAffineMergeIndex (
    xin_prob_model *context,
    BOOL           updateContext,
    UINT32         mergeIdx,
    UINT32         maxMergeCand,
    UINT32         *bitNum);

void Xin266EstimateChromaIntraPredMode (
    xin_prob_model *context,
    BOOL           updateContext,
    UINT32         chromaMode,
    BOOL           enableCclm,
    UINT32         *bitNum);

void Xin266EstimateLastSigXY (
    xin_prob_model *context,
    BOOL           updateContext,
    UINT32         lastSigXPos,
    UINT32         lastSigYPos,
    const UINT32   lgWidth,
    const UINT32   lgHeight,
    UINT32         compType,
    UINT32         *bitNum);

void Xin266GetSigCxtIdx (
    COEFF    *qCoeff,
    intptr_t stride,
    UINT32   compType,
    SINT32   posX,
    SINT32   posY,
    SINT32   width,
    SINT32   height,
    SINT32   *sumAbs1,
    UINT32   *sigCxtIdx,
    SINT32   state);

void Xin266GetGtxCxtIdx (
    SINT32        planeIdx,
    SINT32        coeffDiag,
    UINT32        sumAbs1,
    UINT32        *gtxCxtIdx);

void Xin266GetAbsSum (
    COEFF    *qCoeff,
    SINT32   posX,
    SINT32   posY,
    SINT32   width,
    SINT32   height,
    intptr_t stride,
    UINT32   baseLevel,
    UINT32   *absSum);

void Xin266GetSigCxtIdxTs (
    COEFF    *qCoeff,
    intptr_t stride,
    SINT32   posX,
    SINT32   posY,
    UINT32   *sigCxtIdx);

void Xin266GetNeighCoeff (
    COEFF    *qCoeff,
    intptr_t stride,
    SINT32   posX,
    SINT32   posY,
    COEFF    *coeffRgt,
    COEFF    *coeffDwn);

void Xin266DeriveModCoeff (
    COEFF  rgtCoeff,
    COEFF  dwnCoeff,
    COEFF  absCoeff,
    BOOL   bdPcm,
    SINT32 *modAbsCoeff);

void Xin266SignCtxIdTs (
    COEFF    *qCoeff,
    intptr_t stride,
    SINT32   posX,
    SINT32   posY,
    UINT32   *signCxtIdx);

void Xin266EstimateFullTuCoeff (
    xin_full_md_buf *fullBuf,
    UINT32          mtsIdx,
    UINT32          partIdx,
    intptr_t        coefPos,
    xin_prob_model  *context,
    BOOL            updateContext,
    xin_tu_struct   *tu,
    BOOL            sbhOn,
    BOOL            depQuant,
    UINT32          maxTrSkipLgSize,
    UINT32          planeIdx,
    UINT32          *bitNum);

void Xin266EstimateFastTuCoeff (
    xin_full_md_buf *fullBuf,
    UINT32          mtsIdx,
    UINT32          partIdx,
    intptr_t        coefPos,
    xin_prob_model  *context,
    BOOL            updateContext,
    xin_tu_struct   *tu,
    BOOL            sbhOn,
    BOOL            depQuant,
    UINT32          maxTrSkipLgSize,
    UINT32          planeIdx,
    UINT32          *bitNum);

void Xin266EstimateFullTuCoeffTs (
    xin_full_md_buf *fullBuf,
    UINT32          mtsIdx,
    UINT32          partIdx,
    intptr_t        coefPos,
    xin_prob_model  *context,
    BOOL            updateContext,
    xin_tu_struct   *tu,
    BOOL            sbhOn,
    UINT32          maxTrSkipLgSize,
    UINT32          planeIdx,
    UINT32          *bitNum);

void Xin266EstimateCuCoeff (
    xin_sec_struct  *secSet,
    xin_prob_model  *context,
    xin_cu_struct   *cu,
    xin_full_md_buf *fullBuf,
    UINT64          *lumaBits,
    UINT64          *chromaBits);

void Xin266EstimateCoeff (
    xin_sec_struct  *secSet,
    xin_prob_model  *context,
    BOOL            updateContext,
    xin_cu_struct   *cu,
    xin_fast_md_buf *fastBuf,
    UINT32          mtsIdx,
    UINT32          *coeffBits,
    UINT32          planeIdx);

void Xin266CoeffScanCG (
    COEFF        *coefBuffer,
    intptr_t     coefStride,
    SINT32       cgWidth,
    SINT32       cgHeight,
    xin_scan_pos *scanOrder,
    UINT16       *coefSign,
    UINT16       *gt0Buf);

void Xin266CoeffScan4x4_SSE4 (
    SINT16       *coefBuffer,
    intptr_t     coefStride,
    SINT32       cgWidth,
    SINT32       cgHeight,
    xin_scan_pos *scanOrder,
    UINT16       *coefSign,
    UINT16       *gt0Buf);

void Xin266ReorderCoeff (
    xin_sec_struct  *secSet,
    xin_full_md_buf *fullBuf,
    UINT32          mtsIdx,
    intptr_t        coefAddr,
    UINT32          partIdx,
    xin_tu_struct   *tu,
    UINT32          compIdx);

void Xin266RdoQuant (
    xin_sec_struct  *secSet,
    xin_fast_md_buf *fastBuf,
    UINT32          mtsIdx,
    intptr_t        coefAddr,
    UINT32          partIdx,
    xin_tu_struct   *tu,
    UINT32          planeIdx);

void Xin266SignBitHidingHdq (
    xin_sec_struct  *secSet,
    xin_full_md_buf *fullBuf,
    UINT32          mtsIdx,
    intptr_t        coefAddr,
    UINT32          partIdx,
    xin_tu_struct   *tu,
    UINT32          compIdx);

void Xin266WriteSaoParam (
    xin_cabac_context *cabacSet,
    BOOL              saoEnabled,
    UINT32            bitDepth,
    xin_ctu_struct    *ctu);

void Xin266WriteCuRec (
    xin_sec_struct *secSet,
    xin_cu_struct  *cu,
    BOOL           realEntropy);

void Xin266InitRdEstBit (
    xin_prob_model *context,
    xin_cabac_est  *cabacEst,
    UINT32         lgWidth,
    UINT32         lgHeight,
    UINT32         compType);

void Xin266EstimateAlfCtuFlag (
    xin_prob_model *context,
    UINT32         ctxInc,
    UINT32         planeIdx,
    BOOL           ctuAlfEnabled,
    BOOL           updateContext,
    UINT32         *bitNum);

void Xin266EstimateAlfCtuFilterIdx (
    xin_prob_model *context,
    xin_alf_aps    *alfAps,
    UINT32          apsNum,
    UINT32         filterIdx,
    BOOL           updateContext,
    UINT32         *bitNum);

void Xin266EstimateAlfCtuAlt (
    xin_prob_model *context,
    UINT32         ctbAlfAlt,
    UINT32         altNum,
    UINT32         planeIdx,
    BOOL           updateContext,
    UINT32         *bitNum);

void Xin266EstimateCcAlfFltCtrlIdc (
    xin_prob_model *context,
    SINT32         ctxInc,
    UINT32         idcVal,
    UINT32         filterCount,
    BOOL           updateContext,
    UINT32         *bitNum);

void Xin266WriteAlfParam (
    xin_cabac_context *cabacSet,
    xin_alf_struct    *alfSet,
    BOOL              alfEnabled,
    UINT32            ctuIdx);

void Xin266GetBitCount (
    xin_cabac_struct *cabac,
    UINT32           *bitNum);

BOOL Xin266NonZeroMvd (
    xin_fast_md_buf *mdBuf,
    BOOL            mvdL1Zero);


#endif
