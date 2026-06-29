/***************************************************************************//**
 *
 * @file          h266_sign_bit_hiding_hdq.c
 * @brief         h266 sign bit hiding decision (HDQ).
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
#include "h266_alf_struct.h"
#include "h266_entropy_manipulate.h"
#include "basic_macro.h"
#include "h266_func_struct.h"

void Xin266GetBlockDeltaU (
    COEFF    *qCoeff,
    COEFF    *tCoeff,
    COEFF    *deltaU,
    UINT32   cgWidth,
    UINT32   cgHeight,
    UINT16   *coeffSign,
    intptr_t coeffStride,
    SINT32   qMult,
    SINT32   qShift)
{
    UINT32  coeffX;
    UINT32  coeffY;
    SINT32  tCoefficient;
    SINT32  scaledCoeff;
    UINT16  signBitMap;
    UINT32  coeffPos;
    SINT32  qShift8;

    signBitMap = 0;
    coeffPos   = 0;
    qShift8    = qShift - 8;
    
    for (coeffY = 0; coeffY < cgHeight; coeffY++)
    {
        for (coeffX = 0; coeffX < cgWidth; coeffX++)
        {
            tCoefficient   = tCoeff[coeffX];
            scaledCoeff    = XIN_ABS(tCoefficient)*qMult;
            deltaU[coeffX] = (COEFF)((scaledCoeff - (XIN_ABS(qCoeff[coeffX]) << qShift)) >> qShift8);
            signBitMap    |= (tCoeff[coeffX] < 0) << coeffPos;

            coeffPos++;
        }

        qCoeff += coeffStride;
        tCoeff += coeffStride;
        deltaU += cgWidth;
    }

    *coeffSign = signBitMap;

}

// To minimize the distortion only. No rate is considered.
void Xin266SignBitHidingHdq (
    xin_sec_struct  *secSet,
    xin_full_md_buf *fullBuf,
    UINT32          mtsIdx,
    intptr_t        coefAddr,
    UINT32          partIdx,
    xin_tu_struct   *tu,
    UINT32          compIdx)
{
    UINT64  sigGrpMapEs;
    UINT32  tuIdx;
    UINT32  compType;
    SINT32  blockIdx;
    UINT32  lastBlockIdx;
    SINT32  lastNZPosInCG;
    SINT32  firstNZPosInCG;
    UINT16  *gt0BitMapBase;
    UINT16  *signCoeffBase;
    UINT16  sigCoeff;
    UINT16  coeffSign;
    COEFF   *qCoeffBase;
    COEFF   *rCoeffBase;
    COEFF   *tCoeffBase;
    COEFF   *qCoeff;
    COEFF   *tCoeff;
    COEFF   *rCoeff;
    SINT32  reconCoeff;
    UINT32  cgWidth;
    UINT32  lgCGWidth;
    UINT32  cgHeight;
    UINT32  cgSize;
    UINT32  lgWidth;
    UINT32  lgHeight;
    UINT32  lgSize;
    BOOL    isAdjust;
    UINT32  firstSignBit;
    UINT32  thisSignBit;
    SINT32  coeffIdx;
    UINT32  coeffPos;
    UINT32  blockX;
    UINT32  blockY;
    UINT32  coeffX;
    UINT32  coeffY;
    UINT32  innerX;
    UINT32  innerY;
    UINT32  coeffAbsSum;
    COEFF   deltaU[64];
    UINT32  qp;
    SINT32  qShift;
    SINT32  iqShift;
    SINT32  iqAdd;
    SINT32  qMult;
    SINT32  iqMult;
    SINT32  curCost;
    SINT32  minCostInc;
    SINT32  minPos;
    SINT32  finalChange;
    SINT32  curChange;
    BOOL    lastCG;
    intptr_t        coeffStride;
    intptr_t        coeffLoc;
    xin_quant_param *qParam;
    xin_seq_struct  *seqSet;
    xin_scan_pos    *scanOrderCG;
    xin_scan_pos    *scanOrder;
    xin_func_struct *funcSet;

    seqSet      = secSet->seqSet;
    funcSet     = secSet->funcSet;
    tuIdx       = tu->tuIdx;
    sigGrpMapEs = fullBuf->nzCGMapEs[mtsIdx][tuIdx][compIdx];

    if ((!seqSet->config.enableSignDataHiding) || (!sigGrpMapEs) 
        || (seqSet->config.enableRdoq) || (mtsIdx == XIN_MTS_SKIP))
    {
        return;
    }

    compType      = (compIdx != PLANE_LUMA);
    qp            = (compType == PLANE_LUMA) ? secSet->qp : secSet->uvQp;
    qp           += (XIN_INTERNAL_BIT_DEPTH == XIN_10_BIT_DEPTH) ? XIN_QP_SHIFT : 0;
    lgWidth       = tu->lgWidth[compType];
    lgHeight      = tu->lgHeight[compType];
    coeffStride   = fullBuf->coeffStride[compType];
    lgCGWidth     = tu->lgCGWidth[compType];
    cgWidth       = 1 << lgCGWidth;
    cgHeight      = 1 << tu->lgCGHeight[compType];
    cgSize        = cgWidth*cgHeight;
    isAdjust      = (lgWidth + lgHeight) & 1;
    lgSize        = (lgWidth + lgHeight) >> 1;
    qParam        = seqSet->quantParam[isAdjust] + qp;
    qShift        = qParam->qShift - lgSize - isAdjust;
    iqShift       = qParam->iqShift + lgSize + isAdjust;
    iqAdd         = 1 << (iqShift - 1);
    qMult         = qParam->qMult;
    iqMult        = qParam->iqMult;
    gt0BitMapBase = fullBuf->gt0BitMap[mtsIdx][compIdx] + partIdx;
    signCoeffBase = fullBuf->coeffSign[mtsIdx][compIdx] + partIdx;
    qCoeffBase    = fullBuf->qCoefBuf[mtsIdx][compIdx] + coefAddr;
    rCoeffBase    = fullBuf->rCoefBuf[mtsIdx][compIdx] + coefAddr;
    tCoeffBase    = fullBuf->tCoefBuf[mtsIdx][compIdx] + coefAddr;
    scanOrderCG   = tu->scanOrderCG[compType];
    scanOrder     = tu->scanOrder[compType];
    lastCG        = TRUE;
    
    BIT_SCAN_REVERSE_64 (sigGrpMapEs, lastBlockIdx);

    for (blockIdx = lastBlockIdx; blockIdx >= 0; blockIdx--)
    {
        if (sigGrpMapEs & ((UINT64)1 << blockIdx))
        {
            sigCoeff = *(gt0BitMapBase + blockIdx);

            BIT_SCAN_FORWARD_32 (sigCoeff, firstNZPosInCG);
            BIT_SCAN_REVERSE_32 (sigCoeff, lastNZPosInCG);

            if (lastNZPosInCG - firstNZPosInCG >= XIN_SBH_THRESHOLD)
            {
                coeffSign    = *(signCoeffBase + blockIdx);  
                firstSignBit = (coeffSign >> firstNZPosInCG) & 1;
                coeffAbsSum  = 0;
                blockX       = scanOrderCG[blockIdx].posX;
                blockY       = scanOrderCG[blockIdx].posY;
                blockX       = blockX*cgWidth;
                blockY       = blockY*cgHeight;
                coeffLoc     = blockY*coeffStride + blockX;
                tCoeff       = tCoeffBase + coeffLoc;
                rCoeff       = rCoeffBase + coeffLoc;
                qCoeff       = qCoeffBase + coeffLoc;
                
                for (coeffIdx = firstNZPosInCG; coeffIdx <= lastNZPosInCG; coeffIdx++)
                {
                    innerX = scanOrder[coeffIdx].posX;
                    innerY = scanOrder[coeffIdx].posY;

                    coeffAbsSum += XIN_ABS (qCoeff[innerX + innerY*coeffStride]);
                }

                if (firstSignBit != (coeffAbsSum & 0x1))
                {
                    curCost     = XIN_MAX_U32_COST;
                    minCostInc  = XIN_MAX_U32_COST;
                    minPos      = -1;
                    finalChange = 0; 
                    curChange   = 0;
                
                    funcSet->pfXinGetBlockDeltaU[(cgWidth == 4) && (cgHeight == 4)] (
                        qCoeff,
                        tCoeff,
                        deltaU,
                        cgWidth,
                        cgHeight,
                        &coeffSign,
                        coeffStride,
                        qMult,
                        qShift);

                    for (coeffIdx = (lastCG ? lastNZPosInCG : (cgSize - 1)); coeffIdx >= 0; coeffIdx--)
                    {
                        coeffPos = scanOrder[coeffIdx].posIdx;
                        innerX   = scanOrder[coeffIdx].posX;
                        innerY   = scanOrder[coeffIdx].posY;

                        if (qCoeff[innerX + innerY*coeffStride])
                        {
                            if (deltaU[coeffPos] > 0)
                            {
                                curCost   = -deltaU[coeffPos];
                                curChange = 1;
                            }
                            else
                            {
                                // curChange = -1;
                                if ((coeffIdx == firstNZPosInCG) && (XIN_ABS(qCoeff[innerX + innerY*coeffStride]) == 1))
                                {
                                    curCost = XIN_MAX_U32_COST;
                                }
                                else
                                {
                                    curCost   = deltaU[coeffPos];
                                    curChange = -1;
                                }
                            }
                            
                        }
                        else
                        {
                            if (coeffIdx < firstNZPosInCG)
                            {
                                thisSignBit = (coeffSign >> coeffPos) & 1;
                                
                                if (thisSignBit != firstSignBit)
                                {
                                    curCost = XIN_MAX_U32_COST;
                                }
                                else
                                {
                                    curCost   = -deltaU[coeffPos];
                                    curChange = 1;
                                }
                            }
                            else
                            {
                                curCost   = -deltaU[coeffPos];
                                curChange = 1 ;
                            }
                            
                        }

                        if (curCost < minCostInc)
                        {
                            minCostInc  = curCost;
                            finalChange = curChange;
                            minPos      = coeffPos;
                        }
                        
                    }

                    coeffX   = minPos & (cgWidth - 1);
                    coeffY   = minPos >> lgCGWidth;
                    coeffLoc = (coeffY * coeffStride + coeffX);
                    
                    if (qCoeff[coeffLoc] == MAX_COEFF_VALUE || qCoeff[coeffLoc] == MIN_COEFF_VALUE)
                    {
                        finalChange = -1;
                    }

                    if ((coeffSign>>minPos) & 1)
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
                    reconCoeff = (qCoeff[coeffLoc]*iqMult + iqAdd) >> iqShift;
                    reconCoeff = XIN_CLIP(reconCoeff, MIN_COEFF_VALUE, MAX_COEFF_VALUE);
                    rCoeff[coeffLoc] = (COEFF)reconCoeff;
        
                }
                
            }
            
        }

        lastCG = FALSE;

    }

}

