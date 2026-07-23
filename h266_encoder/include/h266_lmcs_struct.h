/***************************************************************************//**
 *
 * @file          h266_lmcs_struct.h
 * @brief         This file contains data structures related to LMCS.
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
#ifndef _h266_lmcs_struct_h_
#define _h266_lmcs_struct_h_

typedef struct xin_reshape_cw
{
    UINT16  binCW[3];
    SINT32  updateCtrl;
    SINT32  adpOption;
    UINT32  initialCW;
    SINT32  rspPicSize;
    SINT32  rspFps;
    SINT32  rspBaseQp;
    SINT32  rspTid;
    SINT32  rspFpsToIp;
} xin_reshape_cw;

typedef struct xin_lmcs_param
{
    BOOL    lmcsParamChanged;
    BOOL    sliceReshaperEnabled;
    BOOL    sliceReshaperModelPresent;
    UINT32  enableChromaAdj;
    SINT32  reshaperModelMinBinIdx;
    SINT32  reshaperModelMaxBinIdx;
    SINT32  reshaperModelBinCWDelta[XIN_LMCS_ENCODE_CW_BINS];
    SINT32  maxNbitsNeededDeltaCW;
    SINT32  chrResScalingOffset;
} xin_lmcs_param;

typedef struct xin_lmcs_seq
{
  double    binVar[32];
  double    binHist[32];
  double    normVar[32];
  SINT32    nonZeroCnt;
  double    weightVar;
  double    weightNorm;
  double    minBinVar;
  double    maxBinVar;
  double    meanBinVar;
  double    ratioStdU;
  double    ratioStdV;
} xin_lmcs_seq;

typedef struct xin_lmcs_struct
{
    xin_reshape_cw  reshapeCW;
    xin_lmcs_param  lmcsParam;
    xin_lmcs_seq    srcSeq;
    xin_lmcs_seq    rspSeq;
    SINT32          binNum;
    SINT32          lumaBD;
    SINT32          reshapeLUTSize;
    SINT32          initCWAnalyze;
    UINT16          initCW;
    UINT16          binCW[XIN_LMCS_ANALYZE_CW_BINS];
    PIXEL           inputPivot[XIN_LMCS_ENCODE_CW_BINS + 1];
    PIXEL           reshapePivot[XIN_LMCS_ENCODE_CW_BINS + 1];
    SINT32          fwdScaleCoef[XIN_LMCS_ENCODE_CW_BINS];
    SINT32          invScaleCoef[XIN_LMCS_ENCODE_CW_BINS];
    PIXEL           invLUT[1 << XIN_INTERNAL_BIT_DEPTH];
    PIXEL           fwdLUT[1 << XIN_INTERNAL_BIT_DEPTH];
    SINT32          chromaAdjHelpLUT[XIN_LMCS_ENCODE_CW_BINS];
    BOOL            reshape;
    BOOL            useAdpCW;
    BOOL            exceedSTD;
    double          chromaWeight;
    SINT32          tcase;
    SINT32          rateAdpMode;
    SINT32          chromaAdj;
    PIXEL           cwLumaWeight[XIN_LMCS_ENCODE_CW_BINS];

    void            *tempBuffer;
} xin_lmcs_struct;

#endif

