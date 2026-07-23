/***************************************************************************//**
 *
 * @file          h266_dep_quant_struct.h
 * @brief         This file contains data structures related to dependent quantization.
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
#ifndef _h266_dep_quant_struct_h_
#define _h266_dep_quant_struct_h_

#define XIN_SCAN_ISCSBB                 0
#define XIN_SCAN_SOCSBB                 1
#define XIN_SCAN_EOCSBB                 2
#define MAX_TU_LEVEL_CTX_CODED_BIN      28
#define XIN_MAX_DEP_COST                ((((UINT64)1 << 63) - 1) >> 1)
#define XIN_OUT_UPDATE(k)               {COEFF t=absLevels[nbOut->outPos[k]]; sumAbs+=t; sumAbs1+=XIN_MIN(4+(t&1),t); sumNum+=!!t;}
#define XIN_LASTOFFSET(scanInfo)        (depQuant->lastBitsX[scanInfo->posX] + depQuant->lastBitsY[scanInfo->posY])

typedef struct xin_func_struct xin_func_struct;

typedef struct xin_sbb_ctx
{
    UINT8    *sbbFlags;
    UINT8    *levels;
} xin_sbb_ctx;

typedef struct bin_frac_bits
{
    UINT32  intBits[2];
} bin_frac_bits;

typedef struct coeff_frac_bits
{
    UINT32  intBits[6];
} coeff_frac_bits;

typedef struct ctx_acc
{
    UINT8   tplAcc;
    UINT8   sumAbs;
} ctx_acc;

typedef struct abs_ctx
{
    UINT8   absLevels[16];
    ctx_acc ctx[16];
}abs_ctx;

typedef union xin_abs_ctx
{
    UINT8   states[48];
    abs_ctx absCtx;
}xin_abs_ctx;

typedef struct xin_dp_state
{
    SINT64          rdCost;
    xin_abs_ctx     absAndCtx; 
    SINT8           numSigSbb;
    SINT32          remRegBins;
    SINT8           refSbbCtxId;
    UINT32          effWidth;
    UINT32          effHeight;
    bin_frac_bits   sbbFracBits;
    bin_frac_bits   sigFracBits;
    coeff_frac_bits coeffFracBits;
    SINT8           goRicePar;
    SINT8           goRiceZero;
    SINT8           stateId;
    bin_frac_bits   *sigFracBitsArray;
    coeff_frac_bits *gtxFracBitsArray;
} xin_dp_state;

typedef struct simd_coeff_ctx 
{
    UINT8   sig[4];
    UINT8   coeff[4];
}simd_coeff_ctx;

typedef struct xin_dp_simd_state
{
    SINT64          rdCost[4];
    SINT16          remRegBins[4];
    SINT32          sbbBits0[4];
    SINT32          sbbBits1[4];

    UINT8           tplAcc[64];
    UINT8           sum1st[64];
    UINT8           absVal[64];
    simd_coeff_ctx  coeffCtx;

    UINT8           numSig[4];
    SINT8           refSbbCtxId[4];

    SINT32          coeffBits1[XIN_NUM_PAR_FLAG_LUMA_CTX + 3];

    SINT8           goRicePar[4];
    SINT8           goRiceZero[4];
    bin_frac_bits   *sigFracBitsArray[4];
    coeff_frac_bits *gtxFracBitsArray;

    SINT32          coeffBitsCtxOffset;
    BOOL            anyRemRegBinsLt4;
    SINT32          initRemRegBins;
}xin_dp_simd_state;

typedef struct xin_pq_data
{
    SINT32  absLevel;
    SINT64  deltaDist;
} xin_pq_data;

typedef struct xin_decision
{
    SINT64    rdCost[4];
    SINT16    absLevel[4];
    SINT8     prevId[4];
} xin_decision;

typedef struct xin_dep_quant
{
    xin_func_struct     *funcSet;
    xin_prob_model      *context;
    xin_dp_param        *dpParam[XIN_MAX_LG_TU_SIZE + XIN_MAX_LG_TU_SIZE + 1];
    xin_dp_state        allStates[12];
    xin_dp_state        *currStates;
    xin_dp_state        *prevStates;
    xin_dp_state        *skipStates;
    xin_dp_state        startState;
    xin_decision        trellis[XIN_MAX_TU_SIZE*XIN_MAX_TU_SIZE][2];
    xin_scan_info       *scanInfo;
    xin_nb_info_out     *nbInfo;

    xin_tu_param        *tuParam[XIN_MAX_LG_TU_SIZE+1][XIN_MAX_LG_TU_SIZE+1][2];

    bin_frac_bits       sbbFlagBits[2];
    xin_sbb_ctx         allSbbCtx[8];
    xin_sbb_ctx         *currSbbCtx;
    xin_sbb_ctx         *prevSbbCtx;

    SINT32              lastBitsX[XIN_MAX_TU_SIZE];
    SINT32              lastBitsY[XIN_MAX_TU_SIZE];
    xin_scan_pos        *scanId2Pos;
    bin_frac_bits       sigSbbFracBits[XIN_NUM_SIG_COEFF_GROUP_CTX];
    bin_frac_bits       sigFracBits[3][XIN_NUM_SIG_FLAG_STATE_LUMA_CTX];
    coeff_frac_bits     gtxFracBits[XIN_NUM_PAR_FLAG_LUMA_CTX];

    xin_dp_simd_state   simdStateCurr;
    xin_dp_simd_state   simdStateSkip;

    UINT8               memory[8*(XIN_MAX_TU_SIZE*XIN_MAX_TU_SIZE + 256)];
} xin_dep_quant;

#endif

