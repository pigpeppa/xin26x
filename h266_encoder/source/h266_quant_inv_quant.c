/***************************************************************************//**
 *
 * @file          h266_quant_inv_quant.c
 * @brief         h266 forward quantization and inverse quantization.
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
#include "basic_macro.h"
#include "h266_definition.h"
#include "string.h"
#include "xin_video_common.h"
#include "h266_trans_unit_struct.h"

static void Xin266QIQCodingGroup (
    COEFF    *qCoeff,
    COEFF    *tCoeff,
    COEFF    *rCoeff,
    intptr_t coeffStride,
    SINT32   qMult,
    SINT32   qAdd,
    SINT32   qShift,
    SINT32   iqMult,
    SINT32   iqAdd,
    SINT32   iqShift,
    SINT32   cGWidth,
    SINT32   cGHeight,
    BOOL     *isNonZeroBlock)
{
    SINT32  pelX;
    SINT32  pelY;
    SINT32  tCoefficient;
    SINT32  qCoefficient;
    SINT32  rCoefficient;
    SINT32  absCoeff;
    SINT32  sign;
    BOOL    isNZBlock;

    isNZBlock  = FALSE;

    for (pelY = 0; pelY < cGHeight; pelY++)
    {
        for (pelX = 0; pelX < cGWidth; pelX++)
        {
            tCoefficient = tCoeff[pelX];
            sign         = (tCoefficient < 0) ? -1 : 1;
            absCoeff     = XIN_ABS(tCoefficient);
            qCoefficient = ((absCoeff*qMult + qAdd) >> qShift)*sign;
            qCoefficient = XIN_CLIP(qCoefficient, MIN_COEFF_VALUE, MAX_COEFF_VALUE);
            qCoeff[pelX] = (COEFF)qCoefficient;
            rCoefficient = (qCoefficient*iqMult + iqAdd)>>iqShift;
            rCoefficient = XIN_CLIP(rCoefficient, MIN_COEFF_VALUE, MAX_COEFF_VALUE);
            rCoeff[pelX] = (COEFF)rCoefficient;

            isNZBlock  |= (qCoefficient != 0);
        }

        qCoeff += coeffStride;
        rCoeff += coeffStride;
        tCoeff += coeffStride;

    }

    *isNonZeroBlock = isNZBlock;

}

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
    UINT64   *nzCGBitMapRs)
{
    SINT32  widthInCG;
    SINT32  heightInCG;
    SINT32  blockX;
    SINT32  blockY;
    BOOL    isNZBlock;
    UINT64  nzCGBitMap;

    widthInCG  = width / cGWidth;
    heightInCG = height / cGHeight;
    nzCGBitMap = 0;
    qCoeff    += coeffStride*(heightInCG - 1)*cGHeight + (widthInCG - 1)*cGWidth;
    rCoeff    += coeffStride*(heightInCG - 1)*cGHeight + (widthInCG - 1)*cGWidth;
    tCoeff    += coeffStride*(heightInCG - 1)*cGHeight + (widthInCG - 1)*cGWidth;

    for (blockY = 0; blockY < heightInCG; blockY++)
    {
        for (blockX = 0; blockX < widthInCG; blockX++)
        {
            Xin266QIQCodingGroup (
                qCoeff - blockX*cGWidth,
                tCoeff - blockX*cGWidth,
                rCoeff - blockX*cGWidth,
                coeffStride,
                qMult,
                qAdd,
                qShift,
                iqMult,
                iqAdd,
                iqShift,
                cGWidth,
                cGHeight,
                &isNZBlock);

            nzCGBitMap <<= 1;
            nzCGBitMap  |= isNZBlock;

        }

        qCoeff -= cGHeight*coeffStride;
        tCoeff -= cGHeight*coeffStride;
        rCoeff -= cGHeight*coeffStride;

    }

    *nzCGBitMapRs = nzCGBitMap;

}

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
    UINT16   *coeffSign)
{
    UINT32  rowIdx;
    UINT32  colIdx;
    SINT32  tCoeff;
    SINT32  qCoeff;
    SINT32  r0Coeff;
    SINT32  rM1Coeff;
    SINT32  rP1Coeff;
    SINT32  iqAdd;
    UINT16  signBitMap;
    UINT32  coeffPos;

    iqAdd      = 1 << (iqShift - 1);
    signBitMap = 0;
    coeffPos   = 0;

    for (rowIdx = 0; rowIdx < cgHeight; rowIdx++)
    {
        for (colIdx = 0; colIdx < cgWidth; colIdx++)
        {
            tCoeff   = tCoef[colIdx];
            r0Coeff  = rCoef[colIdx];
            qCoeff   = XIN_ABS(qCoef[colIdx]);
            rP1Coeff = ((qCoeff+1)*iqMult + iqAdd)>>iqShift;
            rM1Coeff = ((qCoeff-1)*iqMult + iqAdd)>>iqShift;
            rP1Coeff = (tCoeff > 0) ? rP1Coeff : -rP1Coeff;
            rM1Coeff = (tCoeff > 0) ? rM1Coeff : -rM1Coeff;

            distNow[colIdx]  = (tCoeff - r0Coeff)*(tCoeff - r0Coeff);
            distDown[colIdx] = (tCoeff - rM1Coeff)*(tCoeff - rM1Coeff);
            distUp[colIdx]   = (tCoeff - rP1Coeff)*(tCoeff - rP1Coeff);

            signBitMap |= (tCoeff < 0) << coeffPos;
            coeffPos++;
        }

        tCoef += coefStride;
        rCoef += coefStride;
        qCoef += coefStride;

        distNow  += cgWidth;
        distDown += cgWidth;
        distUp   += cgWidth;

    }

    *coeffSign = signBitMap;

}

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
    UINT64       *nzCGBitMapRs)
{
    SINT32  blockIdx;
    SINT32  blockPos;
    SINT32  blockNum;
    SINT32  blockX;
    SINT32  blockY;
    SINT32  coeffIdx;
    SINT32  widthInCG;
    COEFF   *tCoeffBlock;
    COEFF   *rCoeffBlock;
    COEFF   *qCoeffBlock;
    SINT32  coeffNum;
    UINT64  nzCGBitMap;
    BOOL    isBigCoeff;

    blockNum   = width*height / (cGWidth*cGHeight);
    widthInCG  = width / cGWidth;
    coeffNum   = cGWidth*cGHeight;
    nzCGBitMap = *nzCGBitMapRs;

    for (blockIdx = blockNum - 1; blockIdx >= 1; blockIdx--)
    {
        blockPos = scanOrderCG[blockIdx].posIdx;

        if (nzCGBitMap & (UINT64) 1 << blockPos)
        {
            blockY      = blockPos / widthInCG;
            blockX      = blockPos - blockY*widthInCG;
            tCoeffBlock = tCoeff + blockY*cGHeight*coeffStride + blockX*cGWidth;
            isBigCoeff  = FALSE;

            for (coeffIdx = 0; coeffIdx < coeffNum; coeffIdx += cGWidth)
            {
                isBigCoeff |= XIN_ABS (tCoeffBlock[0]) > rdoqThrVal;
                isBigCoeff |= XIN_ABS (tCoeffBlock[1]) > rdoqThrVal;
                isBigCoeff |= XIN_ABS (tCoeffBlock[2]) > rdoqThrVal;
                isBigCoeff |= XIN_ABS (tCoeffBlock[3]) > rdoqThrVal;

                tCoeffBlock += coeffStride;
            }

            if (!isBigCoeff)
            {
                rCoeffBlock = rCoeff + blockY*cGHeight*coeffStride + blockX*cGWidth;
                qCoeffBlock = qCoeff + blockY*cGHeight*coeffStride + blockX*cGWidth;

                memset (rCoeffBlock + 0*coeffStride, 0, sizeof(COEFF)*cGWidth);
                memset (rCoeffBlock + 1*coeffStride, 0, sizeof(COEFF)*cGWidth);
                memset (rCoeffBlock + 2*coeffStride, 0, sizeof(COEFF)*cGWidth);
                memset (rCoeffBlock + 3*coeffStride, 0, sizeof(COEFF)*cGWidth);

                memset (qCoeffBlock + 0*coeffStride, 0, sizeof(COEFF)*cGWidth);
                memset (qCoeffBlock + 1*coeffStride, 0, sizeof(COEFF)*cGWidth);
                memset (qCoeffBlock + 2*coeffStride, 0, sizeof(COEFF)*cGWidth);
                memset (qCoeffBlock + 3*coeffStride, 0, sizeof(COEFF)*cGWidth);

                nzCGBitMap &= ~((UINT64)1 << blockPos);
            }
            else
            {
                break;
            }

        }

    }

    *nzCGBitMapRs = nzCGBitMap;

}