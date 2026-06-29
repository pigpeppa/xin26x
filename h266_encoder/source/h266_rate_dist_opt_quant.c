/***************************************************************************//**
 *
 * @file          h266_rate_dist_opt_quant.c
 * @brief         h266 rate-distortion optimized quantization.
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
#include "xin_typedef.h"
#include "h266_constant.h"
#include "xin_video_common.h"
#include "h26x_definition.h"
#include "h266_definition.h"
#include "h266_picture_struct.h"
#include "h266_intra_pred_context.h"
#include "h266_pred_unit_struct.h"
#include "h266_seq_struct.h"
#include "h266_trans_unit_struct.h"
#include "h26x_me_struct.h"
#include "h266_cabac_struct.h"
#include "h266_md_buffer_struct.h"
#include "h266_coding_unit_struct.h"
#include "h266_alf_struct.h"
#include "h266_section_struct.h"
#include "h266_scan_order.h"
#include "video_macro.h"
#include "h266_quant_inv_quant.h"
#include "basic_macro.h"
#include "h266_alf_struct.h"
#include "h266_entropy_manipulate.h"
#include "h26x_compute_dist.h"
#include "memory.h"
#include "h266_func_struct.h"

typedef struct xin_rd_coeff_struct
{
    SINT64  codedLevelAndDist;
    SINT64  uncodedDist;
    SINT64  sigCost;
    SINT64  sigCost0;
} xin_rd_coeff_struct;

#define COEF_REMAIN_BIN_REDUCTION       5 ///< indicates the level at which the VLC transitions from Golomb-Rice to TU+EG(k)
#define XIN_COST_FRAC                   15
#define XIN_LAMBDA_FRAC                 16
#define MAX_TU_LEVEL_CTX_CODED_BIN      28

#define XIN_RDO_SHIFT                   (XIN_RATE_FRACTION + XIN_LAMBDA_FRAC - XIN_COST_FRAC)
#define XIN_RDO_OFFSET                  (1 << (XIN_RDO_SHIFT-1))
#define CALC_RDO_COST(lambda, bit)      ((((lambda)*(bit)) + XIN_RDO_OFFSET) >> XIN_RDO_SHIFT)

extern const UINT64 rgtGrpSigMask[6];
extern const UINT32 goRiceParsCoeff[32];

static const UINT32 cbfCtxOffset[3] =
{
    XIN_CO_QT_CBF_Y,
    XIN_CO_QT_CBF_U,
    XIN_CO_QT_CBF_V
};

extern const UINT32 lastSigXYGroupIdx[];
extern const UINT32 cabacbinFracBits[256][2];

static inline void Xin266EstimateBin (
    UINT32         binValue,
    xin_prob_model *context,
    UINT32         *bitNum)
{
    UINT32  state;

    state   = (context->state[0] + context->state[1]) >> 8;
    *bitNum = cabacbinFracBits[state][binValue];
}

void Xin266RdLastSigXYBit (
    xin_cabac_est  *cabacEst,
    UINT32         lastSigXPos,
    UINT32         lastSigYPos,
    const UINT32   lgWidth,
    const UINT32   lgHeight,
    UINT32         compType,
    UINT32         *bitNum)
{
    UINT32 xGroupIdx;
    UINT32 yGroupIdx;
    SINT32 groupCount;
    UINT32 totalBits;

    xGroupIdx  = lastSigXYGroupIdx[lastSigXPos];
    yGroupIdx  = lastSigXYGroupIdx[lastSigYPos];
    totalBits  = cabacEst->lastXBits[compType][lgWidth][xGroupIdx];
    totalBits += cabacEst->lastYBits[compType][lgHeight][yGroupIdx];

    if (xGroupIdx > 3)
    {
        groupCount  = (xGroupIdx - 2 ) >> 1;
        totalBits  += groupCount << XIN_RATE_FRACTION;
    }

    if (yGroupIdx > 3)
    {
        groupCount  = (yGroupIdx - 2) >> 1;
        totalBits  += groupCount << XIN_RATE_FRACTION;
    }

    *bitNum = totalBits;

}

void Xin266GetICRate (
    xin_cabac_est  *cabacEst,
    UINT32          compType,
    UINT32          gtxCtxIdx,
    UINT32          absLevel,
    SINT32          remRegBins,
    UINT32          pos0Par,
    UINT32          goRicePar,
    SINT32          maxLgTrRange,
    SINT32          *bitNum)
{

    SINT32  rate;
    SINT32  symbol;
    UINT32  length;
    SINT32  maxPreLength;
    SINT32  prefixLength;
    SINT32  suffixLength;
    SINT32  threshold;

    rate      = 1 << XIN_RATE_FRACTION;
    threshold = COEF_REMAIN_BIN_REDUCTION << goRicePar;

    if (remRegBins < 4)
    {
        symbol = (absLevel == 0 ? pos0Par : absLevel <= pos0Par ? absLevel - 1 : absLevel);

        if (symbol < threshold)
        {
            length = symbol >> goRicePar;
            rate  += (length + 1 + goRicePar) << XIN_RATE_FRACTION;
        }
        else
        {
            maxPreLength = 32 - COEF_REMAIN_BIN_REDUCTION - maxLgTrRange;
            symbol       = (symbol >> goRicePar) - COEF_REMAIN_BIN_REDUCTION;
            prefixLength = 0;

            while ((prefixLength < maxPreLength) && (symbol > ((2 << prefixLength) - 2)))
            {
                prefixLength++;
            }

            suffixLength = (prefixLength == maxPreLength) ? (maxLgTrRange - goRicePar) : ( prefixLength + 1);

            rate += (COEF_REMAIN_BIN_REDUCTION + prefixLength + suffixLength + goRicePar) << XIN_RATE_FRACTION;
        }

    }
    else
    {
        if (absLevel >= 4)
        {
            symbol = (absLevel - 4) >> 1;

            if (symbol < threshold)
            {
                length = symbol >> goRicePar;
                rate  += (length + 1 + goRicePar) << XIN_RATE_FRACTION;
            }
            else
            {
                maxPreLength = 32 - COEF_REMAIN_BIN_REDUCTION - maxLgTrRange;
                prefixLength = 0;
                symbol       = (symbol >> goRicePar) - COEF_REMAIN_BIN_REDUCTION;

                while ((prefixLength < maxPreLength) && (symbol > (( 2 << prefixLength) - 2)))
                {
                    prefixLength++;
                }

                suffixLength = (prefixLength == maxPreLength) ? (maxLgTrRange - goRicePar) : ( prefixLength + 1);

                rate += (COEF_REMAIN_BIN_REDUCTION + prefixLength + suffixLength + goRicePar) << XIN_RATE_FRACTION;
            }

            rate += cabacEst->greaterOneBits[gtxCtxIdx + compType*XIN_NUM_GT1_FLAG_LUMA_CTX][1];
            rate += cabacEst->parFlagBits[gtxCtxIdx + compType*XIN_NUM_PAR_FLAG_LUMA_CTX][(absLevel - 2) & 1];
            rate += cabacEst->greaterTwoBits[gtxCtxIdx + compType*XIN_NUM_PAR_FLAG_LUMA_CTX][1];

        }
        else if (absLevel == 1)
        {
            rate += cabacEst->greaterOneBits[gtxCtxIdx + compType*XIN_NUM_GT1_FLAG_LUMA_CTX][0];
        }
        else if (absLevel == 2)
        {
            rate += cabacEst->greaterOneBits[gtxCtxIdx + compType*XIN_NUM_GT1_FLAG_LUMA_CTX][1];
            rate += cabacEst->parFlagBits[gtxCtxIdx + compType*XIN_NUM_PAR_FLAG_LUMA_CTX][0];
            rate += cabacEst->greaterTwoBits[gtxCtxIdx + compType*XIN_NUM_PAR_FLAG_LUMA_CTX][0];
        }
        else if (absLevel == 3)
        {
            rate += cabacEst->greaterOneBits[gtxCtxIdx + compType*XIN_NUM_GT1_FLAG_LUMA_CTX][1];
            rate += cabacEst->parFlagBits[gtxCtxIdx + compType*XIN_NUM_PAR_FLAG_LUMA_CTX][1];
            rate += cabacEst->greaterTwoBits[gtxCtxIdx + compType*XIN_NUM_PAR_FLAG_LUMA_CTX][0];
        }
        else
        {
            rate = 0;
        }

    }

    *bitNum = rate;

}

void Xin266RdoQuant (
    xin_sec_struct  *secSet,
    xin_fast_md_buf *fastBuf,
    UINT32          mtsIdx,
    intptr_t        coefAddr,
    UINT32          partIdx,
    xin_tu_struct   *tu,
    UINT32          planeIdx)
{
    SINT32  lastBlockIdx;
    SINT32  lastCoeffIdx;
    SINT32  bestLastBlockIdx;
    SINT32  bestLastCoeffIdx;
    SINT32  scanIndex;
    SINT32  blockIdx;
    UINT32  tuIdx;
    UINT32  compType;
    SINT32  blockNum;
    UINT32  blockX;
    UINT32  blockY;
    UINT32  blockPos;
    SINT32  innerIdx;
    UINT32  innerX;
    UINT32  innerY;
    UINT32  innerPos;
    UINT32  coeffX;
    UINT32  coeffY;
    BOOL    isAdjust;
    UINT32  lgSize;
    UINT32  lgCGSize;
    UINT32  clipLgWidth;
    UINT32  clipWidth, clipHeight;
    UINT32  lgWidth, lgHeight;
    UINT32  cgWidth, cgHeight;
    UINT32  lgCGWidth, lgCGHeight;
    UINT32  cgSize;
    UINT32  width, height;
    UINT32  widthInCG;
    UINT32  sigGrpCtxIdx;
    COEFF   *rCoeffBase;
    COEFF   *tCoeffBase;
    COEFF   *qCoeffBase;
    COEFF   *tCoeff;
    COEFF   *rCoeff;
    COEFF   *qCoeff;
    SINT32  r1Coeff;
    SINT32  r0Coeff;
    SINT32  q0Coeff;
    SINT32  q1Coeff;
    UINT16  origGt0Map;
    UINT16  gt0CGMap;
    UINT16  signCoeff;
    UINT16  *gt0BitMapBase;
    UINT16  *signCoeffBase;
    SINT64  coeffCostBase[XIN_MAX_TU_SIZE*XIN_MAX_TU_SIZE];
    UINT32  coeff0DistBase[XIN_MAX_TU_SIZE*XIN_MAX_TU_SIZE];
    SINT64  sigCostBase[XIN_MAX_TU_SIZE*XIN_MAX_TU_SIZE];
    SINT64  sigGrpCostBase[64];
    SINT32  rateIncUpBase[XIN_MAX_TU_SIZE*XIN_MAX_TU_SIZE];
    SINT32  rateIncDownBase[XIN_MAX_TU_SIZE*XIN_MAX_TU_SIZE];
    SINT64  sigRateDeltaBase[XIN_MAX_TU_SIZE*XIN_MAX_TU_SIZE];
    SINT32  distNow[16];
    SINT32  distUp[16];
    SINT32  distDown[16];
    SINT64  *coeffCost;
    UINT32  *coeff0Dist;
    SINT64  *sigCost;
    SINT64  *sigGrpCost;
    UINT64  sigGrpMapRgt;
    UINT64  sigGrpMapDwn;
    UINT64  sigGrpMapRs;
    UINT64  sigGrpMapEs;
    UINT32  sigGrpRgt;
    UINT32  sigGrpDwn;
    UINT32  origLevel;
    UINT32  bestLevel;
    UINT64  level0Ssd;
    UINT64  level1Ssd;
    SINT64  level0Cost;
    SINT64  level1Cost;
    SINT64  totalBaseCost;
    SINT64  totalBestCost;
    SINT64  totalZeroCost;
    SINT64  tempBestCost;
    UINT64  blockDist;
    UINT32  qp;
    SINT32  iqShift;
    SINT32  iqAdd;
    UINT32  scaleBits;
    SINT64  lambda;
    SINT64  sigCoeffCost;
    BOOL    foundLast;
    UINT32  lastSigPosX;
    UINT32  lastSigPosY;
    SINT32  *rateIncUp;
    SINT32  *rateIncDown;
    SINT64  *sigRateDelta;
    SINT32  rateNow;
    SINT32  rateUp;
    SINT32  rateDown;
    SINT32  remRegBins;
    UINT32  sigCtxIdx;
    UINT32  gtxCtxIdx;
    SINT32  sumAbs1;
    UINT32  sumAbs;
    UINT32  ricePar;
    UINT32  pos0Par;
    UINT32  numOfBit;
    UINT32  cbfCtxInc;
    SINT32  firstNZIdxInCG;
    SINT32  lastNZIdxInCG;
    UINT32  coeffAbsSum;
    UINT32  firstSignBit;
    SINT32  minPos;
    SINT64  curCost;
    SINT32  finalChange;
    SINT64  costUp;
    SINT64  costDown;
    SINT64  minCostInc;
    SINT32  curChange;
    UINT32  thisSignBit;
    BOOL    lastCG;
    SINT32  qCoeffAbs;

    xin_scan_pos        *scanOrderCG;
    xin_scan_pos        *scanOrder;
    xin_func_struct     *funcSet;
    xin_full_md_buf     *fullBuf;
    intptr_t            coeffLoc;
    intptr_t            coeffStride;
    xin_seq_struct      *seqSet;
    xin_quant_param     *qParam;
    xin_rd_coeff_struct rdStats;
    xin_cabac_est       *cabacEst;

    seqSet        = secSet->seqSet;
    fullBuf       = fastBuf->fullBuf;
    compType      = (planeIdx != PLANE_LUMA);
    tuIdx         = tu->tuIdx;
    coeffStride   = fullBuf->coeffStride[compType];
    lgWidth       = tu->lgWidth[compType];
    lgHeight      = tu->lgHeight[compType];
    clipLgWidth   = XIN_MIN (lgWidth, 5);
    width         = 1 << lgWidth;
    height        = 1 << lgHeight;
    clipWidth     = XIN_MIN (width, 32);
    clipHeight    = XIN_MIN (height, 32);
    isAdjust      = (lgWidth + lgHeight) & 1;
    lgSize        = (lgWidth + lgHeight) >> 1;
    lgCGWidth     = tu->lgCGWidth[compType];
    lgCGHeight    = tu->lgCGHeight[compType];
    lgCGSize      = lgCGWidth + lgCGHeight;
    cgSize        = 1 << lgCGSize;
    cgWidth       = 1 << lgCGWidth;
    cgHeight      = 1 << lgCGHeight;
    blockNum      = (clipWidth*clipHeight) >> (lgCGWidth + lgCGHeight);
    widthInCG     = clipWidth >> lgCGWidth;
    rCoeffBase    = fullBuf->rCoefBuf[mtsIdx][planeIdx] + coefAddr;
    tCoeffBase    = fullBuf->tCoefBuf[mtsIdx][planeIdx] + coefAddr;
    qCoeffBase    = fullBuf->qCoefBuf[mtsIdx][planeIdx] + coefAddr;
    gt0BitMapBase = fullBuf->gt0BitMap[mtsIdx][planeIdx] + partIdx;
    signCoeffBase = fullBuf->coeffSign[mtsIdx][planeIdx] + partIdx;
    scanOrderCG   = tu->scanOrderCG[compType];
    scanOrder     = tu->scanOrder[compType];
    totalBaseCost = 0;
    totalZeroCost = 0;
    sigGrpMapEs   = fullBuf->nzCGMapEs[mtsIdx][tuIdx][planeIdx];
    sigGrpMapRs   = fullBuf->nzCGMapRs[mtsIdx][tuIdx][planeIdx];
    qp            = (compType == PLANE_LUMA) ? secSet->qp : secSet->uvQp;
#ifdef ENABLE_10BIT_ENCODER
    scaleBits     = XIN_COST_FRAC - 10 + lgWidth + lgHeight;
    qp           += XIN_QP_SHIFT;
#else
    scaleBits     = XIN_COST_FRAC - 14 + lgWidth + lgHeight;
#endif
    qParam        = seqSet->quantParam[isAdjust] + qp;
    iqShift       = qParam->iqShift + lgSize + isAdjust;
    iqAdd         = 1 << (iqShift - 1);
    lambda        = secSet->sseLambda[compType];
    remRegBins    = (clipHeight*clipWidth*MAX_TU_LEVEL_CTX_CODED_BIN) >> 4;
    sumAbs1       = -1;
    ricePar       = 0;
    pos0Par       = 0;
    funcSet       = secSet->funcSet;
    cabacEst      = &secSet->cabacEst;

    Xin266InitRdEstBit (
        secSet->cabacSet->context,
        cabacEst,
        lgWidth,
        lgHeight,
        compType);

    BIT_SCAN_REVERSE_64 (sigGrpMapEs,  lastBlockIdx);

    sigGrpMapRgt  = (sigGrpMapRs >> 1) & rgtGrpSigMask[clipLgWidth];
    sigGrpMapDwn  = (sigGrpMapRs >> widthInCG);

    BIT_SCAN_REVERSE_32 (gt0BitMapBase[lastBlockIdx], lastCoeffIdx);

    lastCoeffIdx = (lastBlockIdx << lgCGSize) + lastCoeffIdx;

    for (blockIdx = lastBlockIdx + 1; blockIdx < blockNum; blockIdx++)
    {
        blockY     = scanOrderCG[blockIdx].posY;
        blockX     = scanOrderCG[blockIdx].posX;
        blockPos   = scanOrderCG[blockIdx].posIdx;
        blockX     = blockX * cgWidth;
        blockY     = blockY * cgHeight;
        coeffLoc   = blockY * coeffStride + blockX;
        tCoeff     = tCoeffBase + coeffLoc;
        coeff0Dist = coeff0DistBase + blockPos*cgSize;

        funcSet->pfXinComputeBlockSsd (
            tCoeff,
            coeffStride,
            coeff0Dist,
            &blockDist);

        totalBaseCost += blockDist << scaleBits;
        totalZeroCost += blockDist << scaleBits;
        
    }

    for (blockIdx = lastBlockIdx; blockIdx >= 0; blockIdx--)
    {
        blockPos       = scanOrderCG[blockIdx].posIdx;
        blockY         = scanOrderCG[blockIdx].posY;
        blockX         = scanOrderCG[blockIdx].posX;
        blockX         = blockX * cgWidth;
        blockY         = blockY * cgHeight;
        coeffLoc       = blockY * coeffStride + blockX;
        tCoeff         = tCoeffBase + coeffLoc;
        rCoeff         = rCoeffBase + coeffLoc;
        qCoeff         = qCoeffBase + coeffLoc;
        coeff0Dist     = coeff0DistBase + (blockPos<<lgCGSize);
        origGt0Map     = gt0BitMapBase[blockIdx];
        gt0CGMap       = 0;
        sigGrpRgt      = (sigGrpMapRgt >> blockPos) & 1;
        sigGrpDwn      = (sigGrpMapDwn >> blockPos) & 1;
        sigGrpCtxIdx   = sigGrpRgt || sigGrpDwn;
        coeffCost      = coeffCostBase + (blockIdx<<lgCGSize);
        sigCost        = sigCostBase + (blockIdx<<lgCGSize);
        sigGrpCost     = sigGrpCostBase + blockIdx;
        rateIncUp      = rateIncUpBase + (blockIdx<<lgCGSize);
        rateIncDown    = rateIncDownBase + (blockIdx<<lgCGSize);
        sigRateDelta   = sigRateDeltaBase + (blockIdx<<lgCGSize);
        ricePar        = 0;

        funcSet->pfXinComputeBlockSsd (
            tCoeff,
            coeffStride,
            coeff0Dist,
            &blockDist);

        totalZeroCost += blockDist << scaleBits;

        if ((blockIdx) && (!origGt0Map))
        {
            sigGrpCost[0]  = CALC_RDO_COST(lambda, cabacEst->sigCoeffGrpBits[sigGrpCtxIdx + compType*XIN_NUM_SIG_COEFF_GROUP_CTX][0]);
            totalBaseCost += sigGrpCost[0];
            totalBaseCost += blockDist << scaleBits;

            continue;
        }

        memset (&rdStats,     0, sizeof(xin_rd_coeff_struct));
        memset (rateIncDown,  0, sizeof(SINT32)*cgSize);
        memset (rateIncUp,    0, sizeof(SINT32)*cgSize);
        memset (sigRateDelta, 0, sizeof(SINT64)*cgSize);

        for (innerIdx = (cgSize-1); innerIdx >= 0; innerIdx--)
        {
            innerX    = scanOrder[innerIdx].posX;
            innerY    = scanOrder[innerIdx].posY;
            innerPos  = scanOrder[innerIdx].posIdx;
            coeffX    = blockX + innerX;
            coeffY    = blockY + innerY;
            coeffLoc  = innerY*coeffStride + innerX;
            scanIndex = (blockIdx << lgCGSize) + innerIdx;

            coeffCost[innerIdx] = XIN_MAX_U64_COST;

            // Before find last non-zero coeff
            if (scanIndex > lastCoeffIdx)
            {
                coeffCost[innerIdx] = 0;
                sigCost[innerIdx]   = 0;
                totalBaseCost      += (UINT64)coeff0Dist[innerPos] << scaleBits;
            }
            else
            {
                if (scanIndex != lastCoeffIdx)
                {
                    Xin266GetSigCxtIdx (
                        qCoeffBase,
                        coeffStride,
                        compType,
                        coeffX,
                        coeffY,
                        clipWidth,
                        clipHeight,
                        &sumAbs1,
                        &sigCtxIdx,
                        0);

                    sigCost[innerIdx]   = CALC_RDO_COST(lambda, cabacEst->sigCoeffBits[XIN_NUM_SIG_FLAG_LUMA_CTX*compType + sigCtxIdx][0]);
                    coeffCost[innerIdx] = ((UINT64)coeff0Dist[innerPos] << scaleBits) + sigCost[innerIdx];

                    sigCoeffCost           = CALC_RDO_COST(lambda, cabacEst->sigCoeffBits[XIN_NUM_SIG_FLAG_LUMA_CTX*compType + sigCtxIdx][1]);
                    sigRateDelta[innerIdx] = (remRegBins < 4) ? 0 : sigCoeffCost - sigCost[innerIdx];

                }
                else
                {
                    sigCoeffCost = 0;
                }

                Xin266GetGtxCxtIdx (
                    compType,
                    (scanIndex == lastCoeffIdx) ? -1 : coeffX + coeffY,
                    sumAbs1,
                    &gtxCtxIdx);

                if (remRegBins < 4)
                {
                    Xin266GetAbsSum (
                        qCoeffBase,
                        coeffX,
                        coeffY,
                        clipWidth,
                        clipHeight,
                        coeffStride,
                        0,
                        &sumAbs);

                    ricePar = goRiceParsCoeff[sumAbs];
                    pos0Par = 1 << ricePar;

                }
                else
                {
                    pos0Par = 0;
                }

                bestLevel = 0;
                r0Coeff   = rCoeff[coeffLoc];
                q0Coeff   = qCoeff[coeffLoc];
                origLevel = XIN_ABS (q0Coeff);

                rCoeff[coeffLoc] = 0;
                qCoeff[coeffLoc] = 0;

                if (origLevel == 1)
                {
                    level0Ssd = (tCoeff[coeffLoc] - r0Coeff)*(tCoeff[coeffLoc] - r0Coeff);
                    level0Ssd = level0Ssd << scaleBits;

                    Xin266GetICRate (
                        cabacEst,
                        compType,
                        gtxCtxIdx,
                        1,
                        remRegBins,
                        pos0Par,
                        ricePar,
                        15,
                        &rateNow);

                    level0Cost  = CALC_RDO_COST(lambda, rateNow);
                    level0Cost += sigCoeffCost;
                    level0Cost += level0Ssd;

                    if (level0Cost < coeffCost[innerIdx])
                    {
                        bestLevel           = 1;
                        coeffCost[innerIdx] = level0Cost;
                        sigCost[innerIdx]   = sigCoeffCost;
                        rCoeff[coeffLoc]    = (COEFF)r0Coeff;
                        qCoeff[coeffLoc]    = (COEFF)q0Coeff;
                    }

                }
                else if (origLevel > 1)// origLevel > 1
                {
                    q1Coeff   = (tCoeff[coeffLoc] > 0) ? q0Coeff - 1 : q0Coeff + 1;
                    r1Coeff   = (q1Coeff*qParam->iqMult + iqAdd)>>iqShift;
                    level0Ssd = (tCoeff[coeffLoc] - r0Coeff)*(tCoeff[coeffLoc] - r0Coeff);
                    level0Ssd = level0Ssd << scaleBits;
                    level1Ssd = (tCoeff[coeffLoc] - r1Coeff)*(tCoeff[coeffLoc] - r1Coeff);
                    level1Ssd = level1Ssd << scaleBits;

                    Xin266GetICRate (
                        cabacEst,
                        compType,
                        gtxCtxIdx,
                        origLevel - 1,
                        remRegBins,
                        pos0Par,
                        ricePar,
                        15,
                        &rateDown);

                    Xin266GetICRate (
                        cabacEst,
                        compType,
                        gtxCtxIdx,
                        origLevel,
                        remRegBins,
                        pos0Par,
                        ricePar,
                        15,
                        &rateNow);

                    level0Cost = CALC_RDO_COST(lambda, rateNow);
                    level1Cost = CALC_RDO_COST(lambda, rateDown);

                    level0Cost += sigCoeffCost;
                    level1Cost += sigCoeffCost;
                    level0Cost  = level0Cost + level0Ssd;
                    level1Cost  = level1Cost + level1Ssd;

                    if (level0Cost < coeffCost[innerIdx])
                    {
                        bestLevel           = origLevel;
                        coeffCost[innerIdx] = level0Cost;
                        sigCost[innerIdx]   = sigCoeffCost;
                        rCoeff[coeffLoc]    = (COEFF)r0Coeff;
                        qCoeff[coeffLoc]    = (COEFF)q0Coeff;
                    }

                    if (level1Cost < coeffCost[innerIdx])
                    {
                        bestLevel           = origLevel - 1;
                        coeffCost[innerIdx] = level1Cost;
                        sigCost[innerIdx]   = sigCoeffCost;
                        rCoeff[coeffLoc]    = (COEFF)r1Coeff;
                        qCoeff[coeffLoc]    = (COEFF)q1Coeff;
                    }

                }

                totalBaseCost += coeffCost[innerIdx];

                if (bestLevel)
                {
                    Xin266GetICRate (
                        cabacEst,
                        compType,
                        gtxCtxIdx,
                        bestLevel,
                        remRegBins,
                        pos0Par,
                        ricePar,
                        15,
                        &rateNow);

                    Xin266GetICRate (
                        cabacEst,
                        compType,
                        gtxCtxIdx,
                        bestLevel + 1,
                        remRegBins,
                        pos0Par,
                        ricePar,
                        15,
                        &rateUp);

                    rateIncUp[innerIdx] = rateUp - rateNow;

                    Xin266GetICRate (
                        cabacEst,
                        compType,
                        gtxCtxIdx,
                        bestLevel - 1,
                        remRegBins,
                        pos0Par,
                        ricePar,
                        15,
                        &rateDown);

                    rateIncDown[innerIdx] = rateDown - rateNow;

                }
                else
                {
                    if (remRegBins < 4)
                    {
                        Xin266GetICRate (
                            cabacEst,
                            compType,
                            gtxCtxIdx,
                            0,
                            remRegBins,
                            pos0Par,
                            ricePar,
                            15,
                            &rateNow);

                        Xin266GetICRate (
                            cabacEst,
                            compType,
                            gtxCtxIdx,
                            1,
                            remRegBins,
                            pos0Par,
                            ricePar,
                            15,
                            &rateUp);

                        rateIncUp[innerIdx] = rateUp - rateNow;

                    }
                    else
                    {
                        rateIncUp[innerIdx] = cabacEst->greaterOneBits[gtxCtxIdx + compType*XIN_NUM_GT1_FLAG_LUMA_CTX][0];
                    }

                }

                if ((remRegBins >= 4) && (innerIdx > 0))
                {
                    Xin266GetAbsSum (
                        qCoeffBase,
                        coeffX,
                        coeffY,
                        clipWidth,
                        clipHeight,
                        coeffStride,
                        4,
                        &sumAbs);

                    ricePar     = goRiceParsCoeff[sumAbs];
                    remRegBins -= (bestLevel < 2 ? bestLevel : 3) + (scanIndex != lastCoeffIdx);
                }

                if (bestLevel)
                {
                    gt0CGMap                  |= 1<<innerIdx;
                    rdStats.codedLevelAndDist += coeffCost[innerIdx] - sigCost[innerIdx];
                    rdStats.uncodedDist       += (UINT64)coeff0Dist[innerPos] << scaleBits;
                }

                rdStats.sigCost += sigCost[innerIdx];

            }

        }

        rdStats.sigCost0 = sigCost[0];
        sigGrpCost[0]    = 0;

        if (blockIdx)
        {
            if (!gt0CGMap)
            {
                sigGrpCost[0]  = CALC_RDO_COST(lambda, cabacEst->sigCoeffGrpBits[sigGrpCtxIdx + compType*XIN_NUM_SIG_COEFF_GROUP_CTX][0]);
                totalBaseCost += sigGrpCost[0];
                totalBaseCost -= rdStats.sigCost;
            }
            else
            {
                if (blockIdx != lastBlockIdx)
                {
                    if (gt0CGMap == 1)
                    {
                        /* if only coeff 0 in this CG is coded, its significant coeff bit is implied */
                        totalBaseCost   -= rdStats.sigCost0;
                        rdStats.sigCost -= rdStats.sigCost0;
                    }

                    tempBestCost  = totalBaseCost + CALC_RDO_COST(lambda, cabacEst->sigCoeffGrpBits[sigGrpCtxIdx + compType*XIN_NUM_SIG_COEFF_GROUP_CTX][0]);
                    tempBestCost += rdStats.uncodedDist;
                    tempBestCost -= rdStats.codedLevelAndDist;
                    tempBestCost -= rdStats.sigCost;

                    sigGrpCost[0]  = CALC_RDO_COST(lambda, cabacEst->sigCoeffGrpBits[sigGrpCtxIdx + compType*XIN_NUM_SIG_COEFF_GROUP_CTX][1]);
                    totalBaseCost += sigGrpCost[0];

                    if (tempBestCost < totalBaseCost)
                    {
                        gt0CGMap      = 0;
                        totalBaseCost = tempBestCost;
                        sigGrpCost[0] = CALC_RDO_COST(lambda, cabacEst->sigCoeffGrpBits[sigGrpCtxIdx + compType*XIN_NUM_SIG_COEFF_GROUP_CTX][0]);

                        memset (rCoeff + coeffStride * 0, 0, sizeof(COEFF)*cgWidth);
                        memset (rCoeff + coeffStride * 1, 0, sizeof(COEFF)*cgWidth);
                        memset (rCoeff + coeffStride * 2, 0, sizeof(COEFF)*cgWidth);
                        memset (rCoeff + coeffStride * 3, 0, sizeof(COEFF)*cgWidth);

                        memset (qCoeff + coeffStride * 0, 0, sizeof(COEFF)*cgWidth);
                        memset (qCoeff + coeffStride * 1, 0, sizeof(COEFF)*cgWidth);
                        memset (qCoeff + coeffStride * 2, 0, sizeof(COEFF)*cgWidth);
                        memset (qCoeff + coeffStride * 3, 0, sizeof(COEFF)*cgWidth);

                    }

                }

            }

        }

        if (!gt0CGMap)
        {
            sigGrpMapEs &= ~((UINT64)1 << blockIdx);
            sigGrpMapRs &= ~((UINT64)1 << blockPos);
            sigGrpMapRgt = (sigGrpMapRs >> 1) & rgtGrpSigMask[clipLgWidth];
            sigGrpMapDwn = (sigGrpMapRs >> widthInCG);
        }

        gt0BitMapBase[blockIdx] = gt0CGMap;

    }

    if ((fastBuf->type != XIN_INTRA_MODE) && (compType == PLANE_LUMA) && (fullBuf->tuNum == 1))
    {
        totalBestCost  = totalZeroCost + CALC_RDO_COST(lambda, cabacEst->blockRootCbpBits[0][0]);
        totalBaseCost += CALC_RDO_COST (lambda, cabacEst->blockRootCbpBits[0][1]);
    }
    else
    {
        cbfCtxInc = ((planeIdx == PLANE_CHROMA_V) & (fullBuf->yuvCbf[mtsIdx][PLANE_CHROMA_U])) ? 1 : 0;

        totalBestCost  = totalZeroCost + CALC_RDO_COST(lambda, cabacEst->blockCbpBits[compType*XIN_NUM_QT_CBF_Y_CTX + (planeIdx == PLANE_CHROMA_V)*XIN_NUM_QT_CBF_U_CTX + cbfCtxInc][0]);
        totalBaseCost += CALC_RDO_COST(lambda, cabacEst->blockCbpBits[compType*XIN_NUM_QT_CBF_Y_CTX + (planeIdx == PLANE_CHROMA_V)*XIN_NUM_QT_CBF_U_CTX + cbfCtxInc][1]);
    }

    BIT_SCAN_REVERSE_64 (sigGrpMapEs, lastBlockIdx);
    BIT_SCAN_REVERSE_32 (gt0BitMapBase[lastBlockIdx], lastCoeffIdx);

    foundLast        = FALSE;
    bestLastBlockIdx = 0;
    bestLastCoeffIdx = 0;

    for (blockIdx = lastBlockIdx; blockIdx >= 0; blockIdx--)
    {
        totalBaseCost -= sigGrpCostBase[blockIdx];
        gt0CGMap       = gt0BitMapBase[blockIdx];

        if (!gt0CGMap)
        {
            continue;
        }

        blockPos   = scanOrderCG[blockIdx].posIdx;
        blockX     = scanOrderCG[blockIdx].posX;
        blockY     = scanOrderCG[blockIdx].posY;
        blockX     = blockX*cgWidth;
        blockY     = blockY*cgHeight;
        coeffLoc   = blockX + blockY*coeffStride;
        qCoeff     = qCoeffBase + coeffLoc;
        sigCost    = sigCostBase + blockIdx*cgSize;
        coeff0Dist = coeff0DistBase + blockPos*cgSize;
        coeffCost  = coeffCostBase + blockIdx*cgSize;

        for (innerIdx = cgSize - 1; innerIdx >= 0; innerIdx--)
        {
            if ((blockIdx >= lastBlockIdx) && (innerIdx > lastCoeffIdx))
            {
                continue;
            }

            if (gt0CGMap & (1 << innerIdx))
            {
                innerX      = scanOrder[innerIdx].posX;
                innerY      = scanOrder[innerIdx].posY;
                innerPos    = scanOrder[innerIdx].posIdx;
                lastSigPosX = blockX + innerX;
                lastSigPosY = blockY + innerY;

                Xin266RdLastSigXYBit (
                    cabacEst,
                    lastSigPosX,
                    lastSigPosY,
                    lgWidth,
                    lgHeight,
                    compType,
                    &numOfBit);

                tempBestCost = totalBaseCost + CALC_RDO_COST (lambda, numOfBit) - sigCost[innerIdx];

                if (tempBestCost < totalBestCost)
                {
                    bestLastBlockIdx = blockIdx;
                    bestLastCoeffIdx = innerIdx + 1;
                    totalBestCost    = tempBestCost;
                }

                if (XIN_ABS (qCoeff[innerX + innerY*coeffStride]) > 1)
                {
                    if (bestLastCoeffIdx == 0)
                    {
                        bestLastBlockIdx = blockIdx;
                        bestLastCoeffIdx = innerIdx + 1;
                    }

                    foundLast = TRUE;

                    break;
                }

                totalBaseCost -= coeffCost[innerIdx];
                totalBaseCost += (UINT64)coeff0Dist[innerPos] << scaleBits;

            }
            else
            {
                totalBaseCost -= sigCost[innerIdx];
            }

        }

        if (foundLast)
        {
            break;
        }

    }

    // Clear coefficient before last scan position
    if (bestLastCoeffIdx != 0)
    {
        bestLastCoeffIdx--;

        if ((bestLastBlockIdx != lastBlockIdx) || (bestLastCoeffIdx != lastCoeffIdx))
        {
            for (blockIdx = lastBlockIdx; blockIdx > bestLastBlockIdx; blockIdx--)
            {
                if (sigGrpMapEs & ((UINT64)1 << blockIdx))
                {
                    blockPos    = scanOrderCG[blockIdx].posIdx;
                    blockX      = scanOrderCG[blockIdx].posX;
                    blockY      = scanOrderCG[blockIdx].posY;
                    blockX      = blockX*cgWidth;
                    blockY      = blockY*cgHeight;
                    coeffLoc    = blockY * coeffStride + blockX;
                    rCoeff      = rCoeffBase + coeffLoc;
                    qCoeff      = qCoeffBase + coeffLoc;

                    sigGrpMapEs &= ~((UINT64)1 << blockIdx);
                    sigGrpMapRs &= ~((UINT64)1 << blockPos);

                    memset (rCoeff + coeffStride * 0, 0, sizeof(COEFF)*cgWidth);
                    memset (rCoeff + coeffStride * 1, 0, sizeof(COEFF)*cgWidth);
                    memset (rCoeff + coeffStride * 2, 0, sizeof(COEFF)*cgWidth);
                    memset (rCoeff + coeffStride * 3, 0, sizeof(COEFF)*cgWidth);

                    memset (qCoeff + coeffStride * 0, 0, sizeof(COEFF)*cgWidth);
                    memset (qCoeff + coeffStride * 1, 0, sizeof(COEFF)*cgWidth);
                    memset (qCoeff + coeffStride * 2, 0, sizeof(COEFF)*cgWidth);
                    memset (qCoeff + coeffStride * 3, 0, sizeof(COEFF)*cgWidth);

                    gt0BitMapBase[blockIdx] = 0;

                }

            }

            blockPos    = scanOrderCG[bestLastBlockIdx].posIdx;
            blockX      = scanOrderCG[bestLastBlockIdx].posX;
            blockY      = scanOrderCG[bestLastBlockIdx].posY;
            blockX      = blockX*cgWidth;
            blockY      = blockY*cgHeight;
            coeffLoc    = blockY * coeffStride + blockX;
            rCoeff      = rCoeffBase + coeffLoc;
            qCoeff      = qCoeffBase + coeffLoc;
            gt0CGMap    = gt0BitMapBase[bestLastBlockIdx];

            for (innerIdx = cgSize - 1; innerIdx > bestLastCoeffIdx; innerIdx--)
            {
                if (gt0CGMap & (1 << innerIdx))
                {
                    innerX    = scanOrder[innerIdx].posX;
                    innerY    = scanOrder[innerIdx].posY;
                    coeffX    = innerX + blockX;
                    coeffY    = innerY + blockY;
                    coeffLoc  = innerY*coeffStride + innerX;
                    gt0CGMap &= ~(1<<innerIdx);

                    rCoeff[coeffLoc] = 0;
                    qCoeff[coeffLoc] = 0;
                }
            }

            gt0BitMapBase[bestLastBlockIdx] = gt0CGMap;

        }

    }
    else
    {
        for (blockIdx = lastBlockIdx; blockIdx >= 0; blockIdx--)
        {
            if (sigGrpMapEs & ((UINT64)1 << blockIdx))
            {
                blockPos    = scanOrderCG[blockIdx].posIdx;
                blockX      = scanOrderCG[blockIdx].posX;
                blockY      = scanOrderCG[blockIdx].posY;
                blockX      = blockX*cgWidth;
                blockY      = blockY*cgHeight;
                coeffLoc    = blockY * coeffStride + blockX;
                rCoeff      = rCoeffBase + coeffLoc;
                qCoeff      = qCoeffBase + coeffLoc;

                sigGrpMapEs &= ~((UINT64)1 << blockIdx);
                sigGrpMapRs &= ~((UINT64)1 << blockPos);

                memset (rCoeff + coeffStride * 0, 0, sizeof(COEFF)*cgWidth);
                memset (rCoeff + coeffStride * 1, 0, sizeof(COEFF)*cgWidth);
                memset (rCoeff + coeffStride * 2, 0, sizeof(COEFF)*cgWidth);
                memset (rCoeff + coeffStride * 3, 0, sizeof(COEFF)*cgWidth);

                memset (qCoeff + coeffStride*0, 0, sizeof(COEFF)*cgWidth);
                memset (qCoeff + coeffStride*1, 0, sizeof(COEFF)*cgWidth);
                memset (qCoeff + coeffStride*2, 0, sizeof(COEFF)*cgWidth);
                memset (qCoeff + coeffStride*3, 0, sizeof(COEFF)*cgWidth);

                gt0BitMapBase[blockIdx] = 0;

            }

        }

    }

    // Update significant group map
    fullBuf->nzCGMapEs[mtsIdx][tuIdx][planeIdx] = sigGrpMapEs;
    fullBuf->nzCGMapRs[mtsIdx][tuIdx][planeIdx] = sigGrpMapRs;

    if (!sigGrpMapEs)
    {
        fullBuf->yuvCbf[mtsIdx][planeIdx] &= ~(1 << tuIdx);
    }

    if ((!seqSet->config.enableSignDataHiding) || (!sigGrpMapEs))
    {
        return;
    }

    lastCG  = TRUE;

    for (blockIdx = bestLastBlockIdx; blockIdx >= 0; blockIdx--)
    {
        if (sigGrpMapEs & ((UINT64)1 << blockIdx))
        {
            gt0CGMap  = *(gt0BitMapBase + blockIdx);
            signCoeff = *(signCoeffBase + blockIdx);

            BIT_SCAN_FORWARD_32 (gt0CGMap, firstNZIdxInCG);
            BIT_SCAN_REVERSE_32 (gt0CGMap, lastNZIdxInCG);

            if (lastNZIdxInCG - firstNZIdxInCG >= XIN_SBH_THRESHOLD)
            {
                coeffAbsSum  = 0;
                firstSignBit = (signCoeff >> firstNZIdxInCG) & 1;
                blockPos     = scanOrderCG[blockIdx].posIdx;
                blockX       = scanOrderCG[blockIdx].posX;
                blockY       = scanOrderCG[blockIdx].posY;
                blockX       = blockX*cgWidth;
                blockY       = blockY*cgHeight;
                coeffLoc     = blockY * coeffStride + blockX;
                qCoeff       = qCoeffBase + coeffLoc;

                for (innerIdx = firstNZIdxInCG; innerIdx <= lastNZIdxInCG; innerIdx++)
                {
                    innerX = scanOrder[innerIdx].posX;
                    innerY = scanOrder[innerIdx].posY;

                    coeffAbsSum += XIN_ABS (qCoeff[innerX + innerY*coeffStride]);
                }

                if (firstSignBit != (coeffAbsSum & 0x1))
                {
                    minCostInc   = XIN_MAX_U64_COST;
                    curCost      = XIN_MAX_U64_COST;
                    finalChange  = 0;
                    minPos       = -1;
                    sigRateDelta = sigRateDeltaBase + blockIdx*cgSize;
                    rateIncUp    = rateIncUpBase + blockIdx*cgSize;
                    rateIncDown  = rateIncDownBase + blockIdx*cgSize;

                    rCoeff  = rCoeffBase + coeffLoc;
                    qCoeff  = qCoeffBase + coeffLoc;
                    tCoeff  = tCoeffBase + coeffLoc;

                    funcSet->pfXinCalcBlockDeltaU[(cgWidth == 4) && (cgHeight == 4)] (
                        tCoeff,
                        rCoeff,
                        qCoeff,
                        coeffStride,
                        qParam->iqMult,
                        iqShift,
                        cgWidth,
                        cgHeight,
                        distNow,
                        distUp,
                        distDown,
                        &signCoeff);

                    for (innerIdx = (lastCG ? lastNZIdxInCG : (cgSize - 1)); innerIdx >= 0; innerIdx--)
                    {
                        innerPos  = scanOrder[innerIdx].posIdx;
                        innerX    = scanOrder[innerIdx].posX;
                        innerY    = scanOrder[innerIdx].posY;
                        qCoeffAbs = XIN_ABS (qCoeff[innerX + innerY*coeffStride]);

                        if (qCoeffAbs)
                        {
                            costUp   = distUp[innerPos] - distNow[innerPos];
                            costUp   = (costUp << scaleBits) + CALC_RDO_COST(lambda, rateIncUp[innerIdx]);
                            costDown = distDown[innerPos] - distNow[innerPos];
                            costDown = (costDown << scaleBits)  + CALC_RDO_COST(lambda, rateIncDown[innerIdx])
                                - ((qCoeffAbs == 1) ? sigRateDelta[innerIdx] : 0);

                            if ((lastCG == 1) && (lastNZIdxInCG == innerIdx) && (qCoeffAbs == 1))
                            {
                                costDown -= CALC_RDO_COST (lambda, 4 << XIN_RATE_FRACTION);
                            }

                            if (costUp < costDown)
                            {
                                curCost   = costUp;
                                curChange =  1;
                            }
                            else
                            {
                                curChange = -1;

                                if (innerIdx == firstNZIdxInCG && (qCoeffAbs == 1))
                                {
                                    curCost = XIN_MAX_U64_COST;
                                }
                                else
                                {
                                    curCost = costDown;
                                }
                            }

                        }
                        else
                        {
                            curCost   = distUp[innerPos] - distNow[innerPos];
                            curCost   = (curCost << scaleBits) + CALC_RDO_COST (lambda, (1 << XIN_RATE_FRACTION) + rateIncUp[innerIdx]) + sigRateDelta[innerIdx];
                            curChange = 1;

                            if (innerIdx < firstNZIdxInCG)
                            {
                                thisSignBit = (signCoeff >> innerPos) & 1;

                                if (thisSignBit != firstSignBit)
                                {
                                    curCost = XIN_MAX_U64_COST;
                                }
                            }
                        }

                        if (curCost < minCostInc)
                        {
                            minCostInc  = curCost;
                            finalChange = curChange;
                            minPos      = innerPos;
                        }

                    }

                    coeffX   = minPos & (cgWidth - 1);
                    coeffY   = minPos >> lgCGWidth;
                    coeffLoc = (coeffY * coeffStride + coeffX);

                    if (qCoeff[coeffLoc] == MAX_COEFF_VALUE || qCoeff[coeffLoc] == MIN_COEFF_VALUE)
                    {
                        finalChange = -1;
                    }

                    if ((signCoeff >> minPos) & 1)
                    {
                        qCoeff[coeffLoc] -= (COEFF)finalChange;
                    }
                    else
                    {
                        qCoeff[coeffLoc] += (COEFF)finalChange;
                    }

                    funcSet->pfXinCoeffScanCG[(cgWidth == 4) && (cgHeight == 4)] (
                        qCoeff,
                        coeffStride,
                        cgWidth,
                        cgHeight,
                        scanOrder,
                        signCoeffBase + blockIdx,
                        gt0BitMapBase + blockIdx);

                    // Dequant changed level
                    r0Coeff = (qCoeff[coeffLoc]*qParam->iqMult + iqAdd) >> iqShift;
                    r0Coeff = XIN_CLIP(r0Coeff, MIN_COEFF_VALUE, MAX_COEFF_VALUE);

                    rCoeff[coeffLoc] = (COEFF)r0Coeff;

                }

            }

        }

        lastCG  = FALSE;

    }

}

void Xin266GetICRateTs (
    xin_prob_model  *context,
    UINT32          compType,
    UINT32          gtxCtxIdx,
    UINT32          absLevel,
    SINT32          remRegBins,
    UINT32          pos0Par,
    UINT32          goRicePar,
    SINT32          maxLgTrRange,
    SINT32          *bitNum)
{

    SINT32  rate;
    SINT32  symbol;
    UINT32  length;
    SINT32  maxPreLength;
    SINT32  prefixLength;
    SINT32  suffixLength;
    SINT32  threshold;
    UINT32  numOfBit;

    rate      = 1 << XIN_RATE_FRACTION;
    threshold = COEF_REMAIN_BIN_REDUCTION << goRicePar;

    if (remRegBins < 4)
    {
        symbol = (absLevel == 0 ? pos0Par : absLevel <= pos0Par ? absLevel - 1 : absLevel);

        if (symbol < threshold)
        {
            length = symbol >> goRicePar;
            rate  += (length + 1 + goRicePar) << XIN_RATE_FRACTION;
        }
        else
        {
            maxPreLength = 32 - COEF_REMAIN_BIN_REDUCTION - maxLgTrRange;
            symbol       = (symbol >> goRicePar) - COEF_REMAIN_BIN_REDUCTION;
            prefixLength = 0;

            while ((prefixLength < maxPreLength) && (symbol > ((2 << prefixLength) - 2)))
            {
                prefixLength++;
            }

            suffixLength = (prefixLength == maxPreLength) ? (maxLgTrRange - goRicePar) : ( prefixLength + 1);

            rate += (COEF_REMAIN_BIN_REDUCTION + prefixLength + suffixLength + goRicePar) << XIN_RATE_FRACTION;
        }

    }
    else
    {
        if (absLevel >= 4)
        {
            symbol = (absLevel - 4) >> 1;

            if (symbol < threshold)
            {
                length = symbol >> goRicePar;
                rate  += (length + 1 + goRicePar) << XIN_RATE_FRACTION;
            }
            else
            {
                maxPreLength = 32 - COEF_REMAIN_BIN_REDUCTION - maxLgTrRange;
                prefixLength = 0;
                symbol       = (symbol >> goRicePar) - COEF_REMAIN_BIN_REDUCTION;

                while ((prefixLength < maxPreLength) && (symbol > (( 2 << prefixLength) - 2)))
                {
                    prefixLength++;
                }

                suffixLength = (prefixLength == maxPreLength) ? (maxLgTrRange - goRicePar) : ( prefixLength + 1);

                rate += (COEF_REMAIN_BIN_REDUCTION + prefixLength + suffixLength + goRicePar) << XIN_RATE_FRACTION;
            }

            Xin266EstimateBin (
                TRUE,
                context + XIN_CO_GT1_FLAG_LUMA + XIN_NUM_GT1_FLAG_LUMA_CTX*compType + gtxCtxIdx,
                &numOfBit);

            rate += numOfBit;

            Xin266EstimateBin (
                (absLevel - 2) & 1,
                context + XIN_CO_PAR_FLAG_LUMA + XIN_NUM_PAR_FLAG_LUMA_CTX*compType + gtxCtxIdx,
                &numOfBit);

            rate += numOfBit;

            Xin266EstimateBin (
                TRUE,
                context + XIN_CO_GT2_FLAG_LUMA + XIN_NUM_GT2_FLAG_LUMA_CTX*compType + gtxCtxIdx,
                &numOfBit);

            rate += numOfBit;

        }
        else if (absLevel == 1)
        {
            Xin266EstimateBin (
                FALSE,
                context + XIN_CO_GT1_FLAG_LUMA + XIN_NUM_GT1_FLAG_LUMA_CTX*compType + gtxCtxIdx,
                &numOfBit);

            rate += numOfBit;
        }
        else if (absLevel == 2)
        {
            Xin266EstimateBin (
                TRUE,
                context + XIN_CO_GT1_FLAG_LUMA + XIN_NUM_GT1_FLAG_LUMA_CTX*compType + gtxCtxIdx,
                &numOfBit);

            rate += numOfBit;

            Xin266EstimateBin (
                FALSE,
                context + XIN_CO_PAR_FLAG_LUMA + XIN_NUM_PAR_FLAG_LUMA_CTX*compType + gtxCtxIdx,
                &numOfBit);

            rate += numOfBit;

            Xin266EstimateBin (
                FALSE,
                context + XIN_CO_GT2_FLAG_LUMA + XIN_NUM_GT2_FLAG_LUMA_CTX*compType + gtxCtxIdx,
                &numOfBit);

            rate += numOfBit;

        }
        else if (absLevel == 3)
        {
            Xin266EstimateBin (
                TRUE,
                context + XIN_CO_GT1_FLAG_LUMA + XIN_NUM_GT1_FLAG_LUMA_CTX*compType + gtxCtxIdx,
                &numOfBit);

            rate += numOfBit;

            Xin266EstimateBin (
                TRUE,
                context + XIN_CO_PAR_FLAG_LUMA + XIN_NUM_PAR_FLAG_LUMA_CTX*compType + gtxCtxIdx,
                &numOfBit);

            rate += numOfBit;

            Xin266EstimateBin (
                FALSE,
                context + XIN_CO_GT2_FLAG_LUMA + XIN_NUM_GT2_FLAG_LUMA_CTX*compType + gtxCtxIdx,
                &numOfBit);

            rate += numOfBit;

        }
        else
        {
            rate = 0;
        }

    }

    *bitNum = rate;

}


void Xin266RdoQuantTs (
    xin_sec_struct  *secSet,
    xin_fast_md_buf *fastBuf,
    UINT32          mtsIdx,
    intptr_t        coefAddr,
    UINT32          partIdx,
    xin_tu_struct   *tu,
    UINT32          planeIdx)
{
    SINT32  lastBlockIdx;
    SINT32  lastCoeffIdx;
    SINT32  bestLastBlockIdx;
    SINT32  bestLastCoeffIdx;
    SINT32  scanIndex;
    SINT32  blockIdx;
    UINT32  tuIdx;
    UINT32  compType;
    SINT32  blockNum;
    UINT32  blockX;
    UINT32  blockY;
    UINT32  blockPos;
    SINT32  innerIdx;
    UINT32  innerX;
    UINT32  innerY;
    UINT32  innerPos;
    UINT32  coeffX;
    UINT32  coeffY;
    BOOL    isAdjust;
    UINT32  lgSize;
    UINT32  lgCGSize;
    UINT32  lgWidth, lgHeight;
    UINT32  cgWidth, cgHeight;
    UINT32  lgCGWidth, lgCGHeight;
    UINT32  cgSize;
    UINT32  width, height;
    UINT32  widthInCG;
    UINT32  sigGrpCtxIdx;
    COEFF   *rCoeffBase;
    COEFF   *tCoeffBase;
    COEFF   *qCoeffBase;
    COEFF   *tCoeff;
    COEFF   *rCoeff;
    COEFF   *qCoeff;
    SINT32  r1Coeff;
    SINT32  r0Coeff;
    SINT32  q0Coeff;
    SINT32  q1Coeff;
    UINT16  origGt0Map;
    UINT16  gt0CGMap;
    UINT16  *gt0BitMapBase;
    SINT64  coeffCostBase[XIN_MAX_TU_SIZE*XIN_MAX_TU_SIZE];
    UINT32  coeff0DistBase[XIN_MAX_TU_SIZE*XIN_MAX_TU_SIZE];
    SINT64  sigCostBase[XIN_MAX_TU_SIZE*XIN_MAX_TU_SIZE];
    SINT64  sigGrpCostBase[64];
    SINT32  rateIncUpBase[XIN_MAX_TU_SIZE*XIN_MAX_TU_SIZE];
    SINT32  rateIncDownBase[XIN_MAX_TU_SIZE*XIN_MAX_TU_SIZE];
    SINT64  sigRateDeltaBase[XIN_MAX_TU_SIZE*XIN_MAX_TU_SIZE];
    SINT64  *coeffCost;
    UINT32  *coeff0Dist;
    SINT64  *sigCost;
    SINT64  *sigGrpCost;
    UINT64  sigGrpMapRgt;
    UINT64  sigGrpMapDwn;
    UINT64  sigGrpMapRs;
    UINT64  sigGrpMapEs;
    UINT32  sigGrpRgt;
    UINT32  sigGrpDwn;
    UINT32  origLevel;
    UINT32  bestLevel;
    UINT64  level0Ssd;
    UINT64  level1Ssd;
    SINT64  level0Cost;
    SINT64  level1Cost;
    SINT64  totalBaseCost;
    SINT64  totalBestCost;
    SINT64  totalZeroCost;
    SINT64  tempBestCost;
    UINT64  blockDist;
    UINT32  qp;
    SINT32  iqShift;
    SINT32  iqAdd;
    UINT32  scaleBits;
    SINT64  lambda;
    UINT64  sigCoeffCost;
    BOOL    foundLast;
    UINT32  lastSigPosX;
    UINT32  lastSigPosY;
    SINT32  *rateIncUp;
    SINT32  *rateIncDown;
    SINT64  *sigRateDelta;
    SINT32  rateNow;
    SINT32  rateUp;
    SINT32  rateDown;
    SINT32  remRegBins;
    UINT32  sigCtxIdx;
    UINT32  gtxCtxIdx;
    UINT32  sumAbs;
    UINT32  ricePar;
    UINT32  pos0Par;
    UINT32  numOfBit;
    UINT32  cbfCtxInc;

    xin_scan_pos    *scanOrderCG;
    xin_scan_pos    *scanOrder;
    intptr_t        coeffLoc;
    intptr_t        coeffStride;
    xin_seq_struct      *seqSet;
    xin_quant_param     *qParam;
    xin_rd_coeff_struct rdStats;
    xin_func_struct     *funcSet;
    xin_full_md_buf     *fullBuf;

    seqSet = secSet->seqSet;

    if (!seqSet->config.enableRdoq)
    {
        return;
    }

    fullBuf       = fastBuf->fullBuf;
    compType      = (planeIdx != PLANE_LUMA);
    tuIdx         = tu->tuIdx;
    coeffStride   = fullBuf->coeffStride[compType];
    lgWidth       = tu->lgWidth[compType];
    lgHeight      = tu->lgHeight[compType];
    width         = 1 << lgWidth;
    height        = 1 << lgHeight;
    isAdjust      = (lgWidth + lgHeight) & 1;
    lgSize        = (lgWidth + lgHeight) >> 1;
    lgCGWidth     = tu->lgCGWidth[compType];
    lgCGHeight    = tu->lgCGHeight[compType];
    lgCGSize      = lgCGWidth + lgCGHeight;
    cgSize        = 1 << lgCGSize;
    cgWidth       = 1 << lgCGWidth;
    cgHeight      = 1 << lgCGHeight;
    blockNum      = (width*height) >> (lgCGWidth + lgCGHeight);
    widthInCG     = width >> lgCGWidth;
    rCoeffBase    = fullBuf->rCoefBuf[mtsIdx][planeIdx] + coefAddr;
    tCoeffBase    = fullBuf->tCoefBuf[mtsIdx][planeIdx] + coefAddr;
    qCoeffBase    = fullBuf->qCoefBuf[mtsIdx][planeIdx] + coefAddr;
    gt0BitMapBase = fullBuf->gt0BitMap[mtsIdx][planeIdx] + partIdx;
    scanOrderCG   = tu->scanOrderCG[compType];
    scanOrder     = tu->scanOrder[compType];
    totalBaseCost = 0;
    totalZeroCost = 0;
    sigGrpMapEs   = fullBuf->nzCGMapEs[mtsIdx][tuIdx][planeIdx];
    sigGrpMapRs   = fullBuf->nzCGMapRs[mtsIdx][tuIdx][planeIdx];
    sigGrpMapRgt  = (sigGrpMapRs >> 1) & rgtGrpSigMask[lgWidth];
    sigGrpMapDwn  = (sigGrpMapRs >> widthInCG);
    qp            = (compType == PLANE_LUMA) ? secSet->qp : secSet->uvQp;
    qParam        = seqSet->quantParam[isAdjust] + qp;
    iqShift       = qParam->iqShift + lgSize + isAdjust;
    iqAdd         = 1 << (iqShift - 1);
    lambda        = secSet->sseLambda[compType];
    scaleBits     = XIN_COST_FRAC - (14 - lgWidth - lgHeight);
    remRegBins    = (height*width*7) >> 2;
    ricePar       = 0;
    pos0Par       = 0;
    funcSet       = secSet->funcSet;

    BIT_SCAN_REVERSE_64 (sigGrpMapEs, lastBlockIdx);
    BIT_SCAN_REVERSE_32 (gt0BitMapBase[lastBlockIdx], lastCoeffIdx);

    lastCoeffIdx = (lastBlockIdx << lgCGSize) + lastCoeffIdx;

    for (blockIdx = lastBlockIdx + 1; blockIdx < blockNum; blockIdx++)
    {
        blockY     = scanOrderCG[blockIdx].posY;
        blockX     = scanOrderCG[blockIdx].posX;
        blockPos   = scanOrderCG[blockIdx].posIdx;
        blockX     = blockX * cgWidth;
        blockY     = blockY * cgHeight;
        coeffLoc   = blockY * coeffStride + blockX;
        tCoeff     = tCoeffBase + coeffLoc;
        coeff0Dist = coeff0DistBase + blockPos*cgSize;

        funcSet->pfXinComputeBlockSsd (
            tCoeff,
            coeffStride,
            coeff0Dist,
            &blockDist);

        totalBaseCost += blockDist << scaleBits;
        totalZeroCost += blockDist << scaleBits;
    }

    for (blockIdx = 0; blockIdx <= lastBlockIdx; blockIdx++)
    {
        blockPos       = scanOrderCG[blockIdx].posIdx;
        blockY         = scanOrderCG[blockIdx].posY;
        blockX         = scanOrderCG[blockIdx].posX;
        blockX         = blockX * cgWidth;
        blockY         = blockY * cgHeight;
        coeffLoc       = blockY * coeffStride + blockX;
        tCoeff         = tCoeffBase + coeffLoc;
        rCoeff         = rCoeffBase + coeffLoc;
        qCoeff         = qCoeffBase + coeffLoc;
        coeff0Dist     = coeff0DistBase + (blockPos<<lgCGSize);
        origGt0Map     = gt0BitMapBase[blockIdx];
        gt0CGMap       = 0;
        sigGrpRgt      = (sigGrpMapRgt >> blockPos) & 1;
        sigGrpDwn      = (sigGrpMapDwn >> blockPos) & 1;
        sigGrpCtxIdx   = sigGrpRgt || sigGrpDwn;
        coeffCost      = coeffCostBase + (blockIdx<<lgCGSize);
        sigCost        = sigCostBase + (blockIdx<<lgCGSize);
        sigGrpCost     = sigGrpCostBase + blockIdx;
        rateIncUp      = rateIncUpBase + (blockIdx<<lgCGSize);
        rateIncDown    = rateIncDownBase + (blockIdx<<lgCGSize);
        sigRateDelta   = sigRateDeltaBase + (blockIdx<<lgCGSize);
        ricePar        = 0;

        funcSet->pfXinComputeBlockSsd (
            tCoeff,
            coeffStride,
            coeff0Dist,
            &blockDist);

        totalZeroCost += blockDist << scaleBits;

        if ((blockIdx) && (!origGt0Map))
        {
            Xin266EstimateBin (
                FALSE,
                secSet->cabacSet->context + sigGrpCtxIdx + compType*XIN_NUM_SIG_COEFF_GROUP_CTX + XIN_CO_SIG_COEFF_GROUP_LUMA,
                &numOfBit);

            sigGrpCost[0]  = CALC_RDO_COST(lambda, numOfBit);
            totalBaseCost += sigGrpCost[0];
            totalBaseCost += blockDist << scaleBits;

            continue;
        }

        memset (&rdStats,     0, sizeof(xin_rd_coeff_struct));
        memset (rateIncDown,  0, sizeof(SINT32)*cgSize);
        memset (rateIncUp,    0, sizeof(SINT32)*cgSize);
        memset (sigRateDelta, 0, sizeof(SINT64)*cgSize);

        for (innerIdx = (cgSize-1); innerIdx >= 0; innerIdx--)
        {
            innerX    = scanOrder[innerIdx].posX;
            innerY    = scanOrder[innerIdx].posY;
            innerPos  = scanOrder[innerIdx].posIdx;
            coeffX    = blockX + innerX;
            coeffY    = blockY + innerY;
            coeffLoc  = innerY*coeffStride + innerX;
            scanIndex = (blockIdx << lgCGSize) + innerIdx;

            coeffCost[innerIdx] = XIN_MAX_U64_COST;

            // Before find last non-zero coeff
            if (scanIndex > lastCoeffIdx)
            {
                coeffCost[innerIdx] = 0;
                sigCost[innerIdx]   = 0;
                totalBaseCost      += (UINT64)coeff0Dist[innerPos] << scaleBits;
            }
            else
            {
                if (scanIndex != lastCoeffIdx)
                {
                    Xin266GetSigCxtIdxTs (
                        qCoeffBase,
                        coeffStride,
                        coeffX,
                        coeffY,
                        &sigCtxIdx);

                    Xin266EstimateBin (
                        FALSE,
                        secSet->cabacSet->context + XIN_CO_TS_SIG_FLAG + sigCtxIdx,
                        &numOfBit);

                    sigCost[innerIdx]   = CALC_RDO_COST(lambda, numOfBit);
                    coeffCost[innerIdx] = ((UINT64)coeff0Dist[innerPos] << scaleBits) + sigCost[innerIdx];

                    Xin266EstimateBin (
                        TRUE,
                        secSet->cabacSet->context + XIN_CO_TS_SIG_FLAG + sigCtxIdx,
                        &numOfBit);

                    sigCoeffCost           = CALC_RDO_COST(lambda, numOfBit);
                    sigRateDelta[innerIdx] = sigCoeffCost - sigCost[innerIdx];

                }
                else
                {
                    sigCoeffCost = 0;
                }

                Xin266GetSigCxtIdxTs (
                    qCoeffBase,
                    coeffStride,
                    coeffX,
                    coeffY,
                    &gtxCtxIdx);

                if (remRegBins < 4)
                {
                    Xin266GetAbsSum (
                        qCoeffBase,
                        coeffX,
                        coeffY,
                        width,
                        height,
                        coeffStride,
                        0,
                        &sumAbs);

                    ricePar = goRiceParsCoeff[sumAbs];
                    pos0Par = 1 << ricePar;

                }
                else
                {
                    pos0Par = 0;
                }

                bestLevel = 0;
                r0Coeff   = rCoeff[coeffLoc];
                q0Coeff   = qCoeff[coeffLoc];
                origLevel = XIN_ABS(q0Coeff);

                rCoeff[coeffLoc] = 0;
                qCoeff[coeffLoc] = 0;

                if (origLevel == 1)
                {
                    level0Ssd = (tCoeff[coeffLoc] - r0Coeff)*(tCoeff[coeffLoc] - r0Coeff);
                    level0Ssd = level0Ssd << scaleBits;

                    Xin266GetICRateTs (
                        secSet->cabacSet->context,
                        compType,
                        gtxCtxIdx,
                        1,
                        remRegBins,
                        pos0Par,
                        ricePar,
                        15,
                        &rateNow);

                    level0Cost  = CALC_RDO_COST(lambda, rateNow);
                    level0Cost += sigCoeffCost;
                    level0Cost  = level0Cost + level0Ssd;

                    if (level0Cost < coeffCost[innerIdx])
                    {
                        bestLevel           = 1;
                        coeffCost[innerIdx] = level0Cost;
                        sigCost[innerIdx]   = sigCoeffCost;
                        rCoeff[coeffLoc]    = (SINT16)r0Coeff;
                        qCoeff[coeffLoc]    = (SINT16)q0Coeff;
                    }

                }
                else if (origLevel > 1)// origLevel > 1
                {
                    q1Coeff   = (tCoeff[coeffLoc] > 0) ? q0Coeff - 1 : q0Coeff + 1;
                    r1Coeff   = (q1Coeff*qParam->iqMult + iqAdd)>>iqShift;
                    level0Ssd = (tCoeff[coeffLoc] - r0Coeff)*(tCoeff[coeffLoc] - r0Coeff);
                    level0Ssd = level0Ssd << scaleBits;
                    level1Ssd = (tCoeff[coeffLoc] - r1Coeff)*(tCoeff[coeffLoc] - r1Coeff);
                    level1Ssd = level1Ssd << scaleBits;

                    Xin266GetICRateTs (
                        secSet->cabacSet->context,
                        compType,
                        gtxCtxIdx,
                        origLevel - 1,
                        remRegBins,
                        pos0Par,
                        ricePar,
                        15,
                        &rateDown);

                    Xin266GetICRateTs (
                        secSet->cabacSet->context,
                        compType,
                        gtxCtxIdx,
                        origLevel,
                        remRegBins,
                        pos0Par,
                        ricePar,
                        15,
                        &rateNow);

                    level0Cost = CALC_RDO_COST(lambda, rateNow);
                    level1Cost = CALC_RDO_COST(lambda, rateDown);

                    level0Cost += sigCoeffCost;
                    level1Cost += sigCoeffCost;
                    level0Cost  = level0Cost + level0Ssd;
                    level1Cost  = level1Cost + level1Ssd;

                    if (level0Cost < coeffCost[innerIdx])
                    {
                        bestLevel           = origLevel;
                        coeffCost[innerIdx] = level0Cost;
                        sigCost[innerIdx]   = sigCoeffCost;
                        rCoeff[coeffLoc]    = (SINT16)r0Coeff;
                        qCoeff[coeffLoc]    = (SINT16)q0Coeff;
                    }

                    if (level1Cost < coeffCost[innerIdx])
                    {
                        bestLevel           = origLevel - 1;
                        coeffCost[innerIdx] = level1Cost;
                        sigCost[innerIdx]   = sigCoeffCost;
                        rCoeff[coeffLoc]    = (SINT16)r1Coeff;
                        qCoeff[coeffLoc]    = (SINT16)q1Coeff;
                    }

                }

                totalBaseCost += coeffCost[innerIdx];

                if (bestLevel)
                {
                    Xin266GetICRateTs (
                        secSet->cabacSet->context,
                        compType,
                        gtxCtxIdx,
                        bestLevel,
                        remRegBins,
                        pos0Par,
                        ricePar,
                        15,
                        &rateNow);

                    Xin266GetICRateTs (
                        secSet->cabacSet->context,
                        compType,
                        gtxCtxIdx,
                        bestLevel + 1,
                        remRegBins,
                        pos0Par,
                        ricePar,
                        15,
                        &rateUp);

                    rateIncUp[innerIdx] = rateUp - rateNow;

                    Xin266GetICRateTs (
                        secSet->cabacSet->context,
                        compType,
                        gtxCtxIdx,
                        bestLevel - 1,
                        remRegBins,
                        pos0Par,
                        ricePar,
                        15,
                        &rateDown);

                    rateIncDown[innerIdx] = rateDown - rateNow;

                }
                else
                {
                    if (remRegBins < 4)
                    {
                        Xin266GetICRateTs (
                            secSet->cabacSet->context,
                            compType,
                            gtxCtxIdx,
                            0,
                            remRegBins,
                            pos0Par,
                            ricePar,
                            15,
                            &rateNow);

                        Xin266GetICRateTs (
                            secSet->cabacSet->context,
                            compType,
                            gtxCtxIdx,
                            1,
                            remRegBins,
                            pos0Par,
                            ricePar,
                            15,
                            &rateUp);

                        rateIncUp[innerIdx] = rateUp - rateNow;

                    }
                    else
                    {
                        Xin266EstimateBin (
                            FALSE,
                            secSet->cabacSet->context + XIN_CO_GT1_FLAG_LUMA + XIN_NUM_GT1_FLAG_LUMA_CTX*compType + gtxCtxIdx,
                            &numOfBit);

                        rateIncUp[innerIdx] = numOfBit;
                    }

                }

                if ((remRegBins >= 4) && (innerIdx > 0))
                {
                    Xin266GetAbsSum (
                        qCoeffBase,
                        coeffX,
                        coeffY,
                        width,
                        height,
                        coeffStride,
                        4,
                        &sumAbs);

                    ricePar     = goRiceParsCoeff[sumAbs];
                    remRegBins -= (bestLevel < 2 ? bestLevel : 3) + (scanIndex != lastCoeffIdx);
                }

                if (bestLevel)
                {
                    gt0CGMap                  |= 1<<innerIdx;
                    rdStats.codedLevelAndDist += coeffCost[innerIdx] - sigCost[innerIdx];
                    rdStats.uncodedDist       += (UINT64)coeff0Dist[innerPos] << scaleBits;
                }

                rdStats.sigCost += sigCost[innerIdx];

            }

        }

        rdStats.sigCost0 = sigCost[0];
        sigGrpCost[0]    = 0;

        if (blockIdx)
        {
            if (!gt0CGMap)
            {
                Xin266EstimateBin (
                    FALSE,
                    secSet->cabacSet->context + sigGrpCtxIdx + compType*XIN_NUM_SIG_COEFF_GROUP_CTX + XIN_CO_SIG_COEFF_GROUP_LUMA,
                    &numOfBit);

                sigGrpCost[0]  = CALC_RDO_COST(lambda, numOfBit);
                totalBaseCost += sigGrpCost[0];
                totalBaseCost -= rdStats.sigCost;
            }
            else
            {
                if (blockIdx != lastBlockIdx)
                {
                    if (gt0CGMap == 1)
                    {
                        /* if only coeff 0 in this CG is coded, its significant coeff bit is implied */
                        totalBaseCost   -= rdStats.sigCost0;
                        rdStats.sigCost -= rdStats.sigCost0;
                    }

                    Xin266EstimateBin (
                        FALSE,
                        secSet->cabacSet->context + sigGrpCtxIdx + compType*XIN_NUM_SIG_COEFF_GROUP_CTX + XIN_CO_SIG_COEFF_GROUP_LUMA,
                        &numOfBit);

                    tempBestCost  = totalBaseCost + CALC_RDO_COST(lambda, numOfBit);
                    tempBestCost += rdStats.uncodedDist;
                    tempBestCost -= rdStats.codedLevelAndDist;
                    tempBestCost -= rdStats.sigCost;

                    Xin266EstimateBin (
                        TRUE,
                        secSet->cabacSet->context + sigGrpCtxIdx + compType*XIN_NUM_SIG_COEFF_GROUP_CTX + XIN_CO_SIG_COEFF_GROUP_LUMA,
                        &numOfBit);

                    sigGrpCost[0]  = CALC_RDO_COST(lambda, numOfBit);
                    totalBaseCost += sigGrpCost[0];

                    if (tempBestCost < totalBaseCost)
                    {
                        gt0CGMap      = 0;
                        totalBaseCost = tempBestCost;

                        Xin266EstimateBin (
                            FALSE,
                            secSet->cabacSet->context + sigGrpCtxIdx + compType*XIN_NUM_SIG_COEFF_GROUP_CTX + XIN_CO_SIG_COEFF_GROUP_LUMA,
                            &numOfBit);

                        sigGrpCost[0] = CALC_RDO_COST(lambda, numOfBit);

                        memset (rCoeff + coeffStride*0, 0, sizeof(COEFF)*cgWidth);
                        memset (rCoeff + coeffStride*1, 0, sizeof(COEFF)*cgWidth);
                        memset (rCoeff + coeffStride*2, 0, sizeof(COEFF)*cgWidth);
                        memset (rCoeff + coeffStride*3, 0, sizeof(COEFF)*cgWidth);

                        memset (qCoeff + coeffStride*0, 0, sizeof(COEFF)*cgWidth);
                        memset (qCoeff + coeffStride*1, 0, sizeof(COEFF)*cgWidth);
                        memset (qCoeff + coeffStride*2, 0, sizeof(COEFF)*cgWidth);
                        memset (qCoeff + coeffStride*3, 0, sizeof(COEFF)*cgWidth);

                    }

                }

            }

        }

        if (!gt0CGMap)
        {
            sigGrpMapEs &= ~((UINT64)1 << blockIdx);
            sigGrpMapRs &= ~((UINT64)1 << blockPos);
            sigGrpMapRgt = (sigGrpMapRs >> 1) & rgtGrpSigMask[lgWidth];
            sigGrpMapDwn = (sigGrpMapRs >> widthInCG);
        }

        gt0BitMapBase[blockIdx] = gt0CGMap;

    }

    if ((fastBuf->type != XIN_INTRA_MODE) && (compType == PLANE_LUMA) && (fullBuf->tuNum == 1))
    {
        Xin266EstimateBin (
            FALSE,
            secSet->cabacSet->context + XIN_CO_QT_ROOT_CBF,
            &numOfBit);

        totalBestCost  = totalZeroCost + CALC_RDO_COST(lambda, numOfBit);

        Xin266EstimateBin (
            TRUE,
            secSet->cabacSet->context + XIN_CO_QT_ROOT_CBF,
            &numOfBit);

        totalBaseCost += CALC_RDO_COST (lambda, numOfBit);
    }
    else
    {
        cbfCtxInc = ((planeIdx == PLANE_CHROMA_V) & (fullBuf->yuvCbf[mtsIdx][PLANE_CHROMA_U])) ? 1 : 0;

        Xin266EstimateBin (
            FALSE,
            secSet->cabacSet->context + cbfCtxOffset[planeIdx] + cbfCtxInc,
            &numOfBit);

        totalBestCost  = totalZeroCost + CALC_RDO_COST(lambda, numOfBit);

        Xin266EstimateBin (
            TRUE,
            secSet->cabacSet->context + cbfCtxOffset[planeIdx] + cbfCtxInc,
            &numOfBit);

        totalBaseCost += CALC_RDO_COST(lambda, numOfBit);
    }

    BIT_SCAN_REVERSE_64 (sigGrpMapEs, lastBlockIdx);
    BIT_SCAN_REVERSE_32 (gt0BitMapBase[lastBlockIdx], lastCoeffIdx);

    foundLast        = FALSE;
    bestLastBlockIdx = 0;
    bestLastCoeffIdx = 0;

    for (blockIdx = lastBlockIdx; blockIdx >= 0; blockIdx--)
    {
        totalBaseCost -= sigGrpCostBase[blockIdx];
        gt0CGMap       = gt0BitMapBase[blockIdx];

        if (!gt0CGMap)
        {
            continue;
        }

        blockPos   = scanOrderCG[blockIdx].posIdx;
        blockX     = scanOrderCG[blockIdx].posX;
        blockY     = scanOrderCG[blockIdx].posY;
        blockX     = blockX*cgWidth;
        blockY     = blockY*cgHeight;
        coeffLoc   = blockX + blockY*coeffStride;
        qCoeff     = qCoeffBase + coeffLoc;
        sigCost    = sigCostBase + blockIdx*cgSize;
        coeff0Dist = coeff0DistBase + blockPos*cgSize;
        coeffCost  = coeffCostBase + blockIdx*cgSize;

        for (innerIdx = cgSize - 1; innerIdx >= 0; innerIdx--)
        {
            if ((blockIdx >= lastBlockIdx) && (innerIdx > lastCoeffIdx))
            {
                continue;
            }

            if (gt0CGMap & (1 << innerIdx))
            {
                innerX      = scanOrder[innerIdx].posX;
                innerY      = scanOrder[innerIdx].posY;
                innerPos    = scanOrder[innerIdx].posIdx;
                lastSigPosX = blockX + innerX;
                lastSigPosY = blockY + innerY;

                Xin266EstimateLastSigXY (
                    secSet->cabacSet->context,
                    FALSE,
                    lastSigPosX,
                    lastSigPosY,
                    lgWidth,
                    lgHeight,
                    compType,
                    &numOfBit);

                tempBestCost = totalBaseCost + CALC_RDO_COST (lambda, numOfBit) - sigCost[innerIdx];

                if (tempBestCost < totalBestCost)
                {
                    bestLastBlockIdx = blockIdx;
                    bestLastCoeffIdx = innerIdx + 1;
                    totalBestCost    = tempBestCost;
                }

                if (XIN_ABS(qCoeff[innerX + innerY*coeffStride]) > 1)
                {
                    if (bestLastCoeffIdx == 0)
                    {
                        bestLastBlockIdx = blockIdx;
                        bestLastCoeffIdx = innerIdx + 1;
                    }

                    foundLast = TRUE;

                    break;
                }

                totalBaseCost -= coeffCost[innerIdx];
                totalBaseCost += (UINT64)coeff0Dist[innerPos] << scaleBits;

            }
            else
            {
                totalBaseCost -= sigCost[innerIdx];
            }

        }

        if (foundLast)
        {
            break;
        }

    }

    // Clear coefficient before last scan position
    if (bestLastCoeffIdx != 0)
    {
        bestLastCoeffIdx--;

        if ((bestLastBlockIdx != lastBlockIdx) || (bestLastCoeffIdx != lastCoeffIdx))
        {
            for (blockIdx = lastBlockIdx; blockIdx > bestLastBlockIdx; blockIdx--)
            {
                if (sigGrpMapEs & ((UINT64)1 << blockIdx))
                {
                    blockPos    = scanOrderCG[blockIdx].posIdx;
                    blockX      = scanOrderCG[blockIdx].posX;
                    blockY      = scanOrderCG[blockIdx].posY;
                    blockX      = blockX*cgWidth;
                    blockY      = blockY*cgHeight;
                    coeffLoc    = blockY * coeffStride + blockX;
                    rCoeff      = rCoeffBase + coeffLoc;
                    qCoeff      = qCoeffBase + coeffLoc;

                    sigGrpMapEs &= ~((UINT64)1 << blockIdx);
                    sigGrpMapRs &= ~((UINT64)1 << blockPos);

                    memset (rCoeff + coeffStride*0, 0, sizeof(COEFF)*cgWidth);
                    memset (rCoeff + coeffStride*1, 0, sizeof(COEFF)*cgWidth);
                    memset (rCoeff + coeffStride*2, 0, sizeof(COEFF)*cgWidth);
                    memset (rCoeff + coeffStride*3, 0, sizeof(COEFF)*cgWidth);

                    memset (qCoeff + coeffStride*0, 0, sizeof(COEFF)*cgWidth);
                    memset (qCoeff + coeffStride*1, 0, sizeof(COEFF)*cgWidth);
                    memset (qCoeff + coeffStride*2, 0, sizeof(COEFF)*cgWidth);
                    memset (qCoeff + coeffStride*3, 0, sizeof(COEFF)*cgWidth);

                    gt0BitMapBase[blockIdx] = 0;

                }

            }

            blockPos    = scanOrderCG[bestLastBlockIdx].posIdx;
            blockX      = scanOrderCG[bestLastBlockIdx].posX;
            blockY      = scanOrderCG[bestLastBlockIdx].posY;
            blockX      = blockX*cgWidth;
            blockY      = blockY*cgHeight;
            coeffLoc    = blockY * coeffStride + blockX;
            rCoeff      = rCoeffBase + coeffLoc;
            qCoeff      = qCoeffBase + coeffLoc;
            gt0CGMap    = gt0BitMapBase[bestLastBlockIdx];

            for (innerIdx = cgSize - 1; innerIdx > bestLastCoeffIdx; innerIdx--)
            {
                if (gt0CGMap & (1 << innerIdx))
                {
                    innerX    = scanOrder[innerIdx].posX;
                    innerY    = scanOrder[innerIdx].posY;
                    coeffX    = innerX + blockX;
                    coeffY    = innerY + blockY;
                    coeffLoc  = innerY*coeffStride + innerX;
                    gt0CGMap &= ~(1<<innerIdx);

                    rCoeff[coeffLoc] = 0;
                    qCoeff[coeffLoc] = 0;
                }
            }

            gt0BitMapBase[bestLastBlockIdx] = gt0CGMap;

        }

    }
    else
    {
        for (blockIdx = lastBlockIdx; blockIdx >= 0; blockIdx--)
        {
            if (sigGrpMapEs & ((UINT64)1 << blockIdx))
            {
                blockPos    = scanOrderCG[blockIdx].posIdx;
                blockX      = scanOrderCG[blockIdx].posX;
                blockY      = scanOrderCG[blockIdx].posY;
                blockX      = blockX*cgWidth;
                blockY      = blockY*cgHeight;
                coeffLoc    = blockY * coeffStride + blockX;
                rCoeff      = rCoeffBase + coeffLoc;
                qCoeff      = qCoeffBase + coeffLoc;

                sigGrpMapEs &= ~((UINT64)1 << blockIdx);
                sigGrpMapRs &= ~((UINT64)1 << blockPos);

                memset (rCoeff + coeffStride*0, 0, sizeof(COEFF)*cgWidth);
                memset (rCoeff + coeffStride*1, 0, sizeof(COEFF)*cgWidth);
                memset (rCoeff + coeffStride*2, 0, sizeof(COEFF)*cgWidth);
                memset (rCoeff + coeffStride*3, 0, sizeof(COEFF)*cgWidth);

                memset (qCoeff + coeffStride*0, 0, sizeof(COEFF)*cgWidth);
                memset (qCoeff + coeffStride*1, 0, sizeof(COEFF)*cgWidth);
                memset (qCoeff + coeffStride*2, 0, sizeof(COEFF)*cgWidth);
                memset (qCoeff + coeffStride*3, 0, sizeof(COEFF)*cgWidth);

                gt0BitMapBase[blockIdx] = 0;

            }

        }

    }

    // Update significant group map
    fullBuf->nzCGMapEs[mtsIdx][tuIdx][planeIdx] = sigGrpMapEs;
    fullBuf->nzCGMapRs[mtsIdx][tuIdx][planeIdx] = sigGrpMapRs;

}

