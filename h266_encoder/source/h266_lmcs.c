/***************************************************************************//**
 *
 * @file          h266_lmcs.c
 * @brief         This file implements LMCS.
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
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "h266_constant.h"
#include "xin_video_common.h"
#include "h26x_definition.h"
#include "h266_definition.h"
#include "basic_macro.h"
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
#include "h266_section_struct.h"
#include "h266_coding_unit_struct.h"
#include "h266_bit_stream.h"
#include "h266_cabac_context.h"
#include "h26x_calc_psnr_ssim.h"
#include "h266_write_header.h"
#include "h266_get_neighbour_mv.h"
#include "h266_intra_prediction.h"
#include "h26x_rate_control_struct.h"
#include "h26x_thread_wrapper.h"
#include "h26x_common_data.h"
#include "h266_common_data.h"
#include "h266_alf_struct.h"
#include "h266_entropy_manipulate.h"
#include "h266_enc_init.h"
#include "h26x_extend_picture.h"
#include "h266_alf_rdo.h"
#include "assert.h"
#include "h26x_frame_operation.h"
#include "h266_inter_pred_context.h"
#include "video_macro.h"

static void Xin266InitLmcsSeq (
    xin_lmcs_seq *lmcsSeq,
    SINT32       binNum)
{
    SINT32 idx;

    for (idx = 0; idx < binNum; idx++)
    {
        lmcsSeq->binVar[idx]  = 0.0;
        lmcsSeq->binHist[idx] = 0.0;
        lmcsSeq->normVar[idx] = 0.0;
    }

    lmcsSeq->nonZeroCnt = 0;
    lmcsSeq->weightVar  = 0.0;
    lmcsSeq->weightNorm = 0.0;
    lmcsSeq->minBinVar  = 0.0;
    lmcsSeq->maxBinVar  = 0.0;
    lmcsSeq->meanBinVar = 0.0;
    lmcsSeq->ratioStdU  = 0.0;
    lmcsSeq->ratioStdV  = 0.0;

}

void Xin266CalcLmcsSeqStat (
    xin_pic_struct *picSet,
    xin_lmcs_seq   *lmcsSeq)
{
    xin_input_picture *inputPicture;
    SINT32            width;
    SINT32            height;
    SINT32            x, y;
    intptr_t          strideY;
    intptr_t          strideUv;
    SINT32            winLens;
    xin_lmcs_struct   *lmcsSet;
    xin_reshape_cw    *reshapeCW;
    PIXEL             *inputY;
    PIXEL             *inputU;
    PIXEL             *inputV;
    PIXEL             *winY;
    SINT32            y1, y2, x1, x2;
    SINT32            bx, by;
    SINT64            sum, sumSq;
    UINT32            pixelNum;
    PIXEL             pixelVal;
    SINT64            tempSq;
    SINT64            topSum, topSumSq;
    SINT64            leftSum, leftSumSq;
    SINT32            idx;
    double            avgY, avgU, avgV;
    double            varY, varU, varV;
    double            average;
    double            variance;
    SINT32            binLen;
    UINT32            binIdx;
    SINT64            *leftColSum;
    SINT64            *leftColSumSq;
    SINT64            *topRowSum;
    SINT64            *topRowSumSq;
    SINT64            *topColSum;
    SINT64            *topColSumSq;
    UINT32            *binCnt;

    lmcsSet      = picSet->lmcsSet;
    reshapeCW    = &lmcsSet->reshapeCW;
    inputPicture = picSet->inputPicture;
    width        = inputPicture->inputWidth;
    height       = inputPicture->inputHeight;
    strideY      = inputPicture->inputStride[0];
    strideUv     = inputPicture->inputStride[1];
    winLens      = (lmcsSet->binNum == XIN_LMCS_ENCODE_CW_BINS) ? (XIN_MIN(height, width) / 240) : 2;
    winLens      = winLens > 0 ? winLens : 1;
    inputY       = inputPicture->inputBuf[0];
    inputU       = inputPicture->inputBuf[1];
    inputV       = inputPicture->inputBuf[2];
    leftSum      = 0;
    leftSumSq    = 0;
    topSum       = 0;
    topSumSq     = 0;
    leftColSum   = (SINT64 *)(lmcsSet->tempBuffer);
    leftColSumSq = leftColSum + width;
    topRowSum    = leftColSumSq + width;
    topRowSumSq  = topRowSum + height;
    topColSum    = topRowSumSq + height;
    topColSumSq  = topColSum + width;
    binCnt       = (UINT32 *)(topColSumSq + width);

    memset (leftColSum,   0, width * sizeof(SINT64));
    memset (leftColSumSq, 0, width * sizeof(SINT64));
    memset (topRowSum,    0, height * sizeof(SINT64));
    memset (topRowSumSq,  0, height * sizeof(SINT64));
    memset (topColSum,    0, width * sizeof(SINT64));
    memset (topColSumSq,  0, width * sizeof(SINT64));
    memset (binCnt,       0, lmcsSet->binNum * sizeof(UINT32));

    Xin266InitLmcsSeq (
        lmcsSeq,
        lmcsSet->binNum);

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pixelVal = inputY[x];
            sum      = 0;
            sumSq    = 0;
            y1       = XIN_MAX (y - winLens, 0);
            y2       = XIN_MIN (y + winLens, height - 1);
            x1       = XIN_MAX (x - winLens, 0);
            x2       = XIN_MIN (x + winLens, width - 1);
            winY     = inputY;

            pixelNum = (x2 - x1 + 1) * (y2 - y1 + 1);

            if (x == 0 && y == 0)
            {
                for (by = y1; by <= y2; by++)
                {
                    for (bx = x1; bx <= x2; bx++)
                    {
                        tempSq            = (SINT64)winY[bx] * (SINT64)winY[bx];
                        leftSum          += winY[bx];
                        leftSumSq        += tempSq;
                        leftColSum[bx]   += winY[bx];
                        leftColSumSq[bx] += tempSq;
                        topColSum[bx]    += winY[bx];
                        topColSumSq[bx]  += tempSq;
                        topRowSum[by]    += winY[bx];
                        topRowSumSq[by]  += tempSq;
                    }

                    winY += strideY;
                }

                topSum   = leftSum;
                topSumSq = leftSumSq;
                sum      = leftSum;
                sumSq    = leftSumSq;

            }
            else if (x == 0 && y > 0)
            {
                if (y < height - winLens)
                {
                    winY += winLens * strideY;

                    topRowSum[y + winLens]   = 0;
                    topRowSumSq[y + winLens] = 0;

                    for (bx = x1; bx <= x2; bx++)
                    {
                        topRowSum[y + winLens]   += winY[bx];
                        topRowSumSq[y + winLens] += (SINT64)winY[bx] * (SINT64)winY[bx];
                    }

                    topSum   += topRowSum[y + winLens];
                    topSumSq += topRowSumSq[y + winLens];
                }

                if (y > winLens)
                {
                    topSum   -= topRowSum[y - 1 - winLens];
                    topSumSq -= topRowSumSq[y - 1 - winLens];
                }

                memset (leftColSum,   0, width * sizeof(SINT64));
                memset (leftColSumSq, 0, width * sizeof(SINT64));

                winY = inputY;
                winY -= (y <= winLens ? y : winLens) * strideY;

                for (by = y1; by <= y2; by++)
                {
                    for (bx = x1; bx <= x2; bx++)
                    {
                        leftColSum[bx]   += winY[bx];
                        leftColSumSq[bx] += (SINT64)winY[bx] * (SINT64)winY[bx];
                    }

                    winY += strideY;
                }

                leftSum   = topSum;
                leftSumSq = topSumSq;
                sum       = topSum;
                sumSq     = topSumSq;

            }
            else if (x > 0)
            {
                if (x < width - winLens)
                {
                    winY -= (y <= winLens ? y : winLens) * strideY;

                    if (y == 0)
                    {
                        leftColSum[x + winLens]   = 0;
                        leftColSumSq[x + winLens] = 0;

                        for (by = y1; by <= y2; by++)
                        {
                            leftColSum[x + winLens]   += winY[x + winLens];
                            leftColSumSq[x + winLens] += (SINT64)winY[x + winLens] * (SINT64)winY[x + winLens];

                            winY += strideY;
                        }
                    }
                    else
                    {
                        leftColSum[x + winLens]   = topColSum[x + winLens];
                        leftColSumSq[x + winLens] = topColSumSq[x + winLens];

                        if (y < height - winLens)
                        {
                            winY  = inputY;
                            winY += winLens * strideY;

                            leftColSum[x + winLens]   += winY[x + winLens];
                            leftColSumSq[x + winLens] += (SINT64)winY[x + winLens] * (SINT64)winY[x + winLens];
                        }

                        if (y > winLens)
                        {
                            winY  = inputY;
                            winY -= (winLens + 1) * strideY;

                            leftColSum[x + winLens]   -= winY[x + winLens];
                            leftColSumSq[x + winLens] -= (SINT64)winY[x + winLens] * (SINT64)winY[x + winLens];
                        }

                    }

                    topColSum[x + winLens]   = leftColSum[x + winLens];
                    topColSumSq[x + winLens] = leftColSumSq[x + winLens];

                    leftSum   += leftColSum[x + winLens];
                    leftSumSq += leftColSumSq[x + winLens];

                }

                if (x > winLens)
                {
                    leftSum   -= leftColSum[x - 1 - winLens];
                    leftSumSq -= leftColSumSq[x - 1 - winLens];
                }

                sum   = leftSum;
                sumSq = leftSumSq;

            }

            average  = (double)(sum) / pixelNum;
            variance = (double)(sumSq) / pixelNum - average * average;
            binLen   = lmcsSet->reshapeLUTSize / lmcsSet->binNum;
            binIdx   = (UINT32)(pixelVal / binLen);

            if (lmcsSet->lumaBD > 10)
            {
                average  = average / (double)(1 << (lmcsSet->lumaBD - 10));
                variance = variance / (double)(1 << (2 * lmcsSet->lumaBD - 20));
            }
            else if (lmcsSet->lumaBD < 10)
            {
                average  = average * (double)(1 << (10 - lmcsSet->lumaBD));
                variance = variance * (double)(1 << (20 - 2 * lmcsSet->lumaBD));
            }

            lmcsSeq->binVar[binIdx] += log10(variance + 1.0);

            binCnt[binIdx]++;

        }

        inputY += strideY;

    }

    for (idx = 0; idx < lmcsSet->binNum; idx++)
    {
        lmcsSeq->binHist[idx] = (double)binCnt[idx] / (double)(reshapeCW->rspPicSize);
        lmcsSeq->binVar[idx]  = (binCnt[idx] > 0) ? (lmcsSeq->binVar[idx] / binCnt[idx]) : 0.0;
    }

    lmcsSeq->minBinVar  = 5.0;
    lmcsSeq->maxBinVar  = 0.0;
    lmcsSeq->meanBinVar = 0.0;
    lmcsSeq->nonZeroCnt = 0;

    for (idx = 0; idx < lmcsSet->binNum; idx++)
    {
        if (lmcsSeq->binHist[idx] > 0.001)
        {
            lmcsSeq->nonZeroCnt++;
            lmcsSeq->meanBinVar += lmcsSeq->binVar[idx];

            if (lmcsSeq->binVar[idx] > lmcsSeq->maxBinVar)
            {
                lmcsSeq->maxBinVar = lmcsSeq->binVar[idx];
            }

            if (lmcsSeq->binVar[idx] < lmcsSeq->minBinVar)
            {
                lmcsSeq->minBinVar = lmcsSeq->binVar[idx];
            }
        }
    }

    lmcsSeq->meanBinVar /= (double)lmcsSeq->nonZeroCnt;

    for (idx = 0; idx < lmcsSet->binNum; idx++)
    {
        if (lmcsSeq->meanBinVar > 0.0)
        {
            lmcsSeq->normVar[idx] = lmcsSeq->binVar[idx] / lmcsSeq->meanBinVar;
        }

        lmcsSeq->weightVar  += lmcsSeq->binHist[idx] * lmcsSeq->binVar[idx];
        lmcsSeq->weightNorm += lmcsSeq->binHist[idx] * lmcsSeq->normVar[idx];
    }

    inputY = inputPicture->inputBuf[0];
    avgY   = 0.0;
    varY   = 0.0;

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            avgY += inputY[x];
            varY += (double)inputY[x] * (double)inputY[x];
        }

        inputY += strideY;
    }

    avgY = avgY / (width * height);
    varY = varY / (width * height) - avgY * avgY;

    height = height / 2;
    width  = width / 2;
    avgU   = 0.0;
    avgV   = 0.0;
    varU   = 0.0;
    varV   = 0.0;

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            avgU += inputU[x];
            avgV += inputV[x];
            varU += (SINT64)inputU[x] * (int64_t)inputU[x];
            varV += (SINT64)inputV[x] * (int64_t)inputV[x];
        }

        inputU += strideUv;
        inputV += strideUv;
    }

    avgU = avgU / (width * height);
    avgV = avgV / (width * height);
    varU = varU / (width * height) - avgU * avgU;
    varV = varV / (width * height) - avgV * avgV;

    if (varY > 0)
    {
        lmcsSeq->ratioStdU = sqrt(varU) / sqrt(varY);
        lmcsSeq->ratioStdV = sqrt(varV) / sqrt(varY);
    }

}

static void BubbleSortDsd (
    double  *array,
    SINT32  *idx,
    SINT32  n)
{
    SINT32 i, j;
    BOOL   swapped;

    for (i = 0; i < n - 1; i++)
    {
        swapped = FALSE;

        for (j = 0; j < n - i - 1; j++)
        {
            if (array[j] < array[j + 1])
            {
                XIN_SWAP(double, array[j], array[j + 1]);
                XIN_SWAP(SINT32, idx[j], idx[j + 1]);
                swapped = TRUE;
            }
        }

        if (swapped == FALSE)
        {
            break;
        }
    }
}

static void Xin266CWPerturbation (
    xin_lmcs_struct *lmcsSet,
    SINT32          startBinIdx,
    SINT32          endBinIdx,
    UINT16          maxCW)
{
    xin_lmcs_seq *lmcsSeq;
    SINT32       idx;
    double       hist;
    UINT16       delta1, delta2;

    lmcsSeq = &lmcsSet->srcSeq;

    for (idx = 0; idx < lmcsSet->binNum; idx++)
    {
        if (idx >= startBinIdx && idx <= endBinIdx)
        {
            lmcsSet->binCW[idx] = (UINT16)round((double)maxCW / (endBinIdx - startBinIdx + 1));
        }
        else
        {
            lmcsSet->binCW[idx] = 0;
        }
    }

    for (idx = 0; idx < lmcsSet->binNum; idx++)
    {
        if (lmcsSeq->binHist[idx] > 0.001)
        {
            hist   = lmcsSeq->binHist[idx] > 0.4 ? 0.4 : lmcsSeq->binHist[idx];
            delta1 = (uint16_t)(10.0 * hist + 0.5);
            delta2 = (uint16_t)(20.0 * hist + 0.5);

            if (lmcsSeq->normVar[idx] < 0.8)
            {
                lmcsSet->binCW[idx] = lmcsSet->binCW[idx] + delta2;
            }
            else if (lmcsSeq->normVar[idx] < 0.9)
            {
                lmcsSet->binCW[idx] = lmcsSet->binCW[idx] + delta1;
            }

            if (lmcsSeq->normVar[idx] > 1.2)
            {
                lmcsSet->binCW[idx] = lmcsSet->binCW[idx] - delta2;
            }
            else if (lmcsSeq->normVar[idx] > 1.1)
            {
                lmcsSet->binCW[idx] = lmcsSet->binCW[idx] - delta1;
            }

        }

    }

}

static void Xin266CWReduction (
    xin_lmcs_struct *lmcsSet,
    SINT32          startBinIdx,
    SINT32          endBinIdx)
{
    SINT32 bdShift;
    SINT32 totCW;
    SINT32 maxAllowedCW;
    SINT32 usedCW;
    SINT32 idx;
    SINT32 deltaCW;
    SINT32 divCW;
    SINT32 modCW;

    bdShift = lmcsSet->lumaBD - 10;
    totCW   = bdShift != 0 ? (bdShift > 0 ? lmcsSet->reshapeLUTSize / (1 << bdShift) : lmcsSet->reshapeLUTSize * (1 << (-bdShift))) : lmcsSet->reshapeLUTSize;
    usedCW  = 0;

    maxAllowedCW = totCW - 1;

    for (idx = 0; idx < lmcsSet->binNum; idx++)
    {
        usedCW += lmcsSet->binCW[idx];
    }

    if (usedCW > maxAllowedCW)
    {
        deltaCW = usedCW - maxAllowedCW;
        divCW   = deltaCW / (endBinIdx - startBinIdx + 1);
        modCW   = deltaCW - divCW * (endBinIdx - startBinIdx + 1);

        if (divCW > 0)
        {
            for (idx = startBinIdx; idx <= endBinIdx; idx++)
            {
                lmcsSet->binCW[idx] -= (UINT16)divCW;
            }
        }

        for (idx = startBinIdx; idx <= endBinIdx; idx++)
        {
            if (modCW == 0)
            {
                break;
            }

            if (lmcsSet->binCW[idx] > 0)
            {
                lmcsSet->binCW[idx]--;
                modCW--;
            }
        }
    }

}

static void Xin266DeriveReshapeParam (
    double         *binVar,
    SINT32         startBinIdx,
    SINT32         endBinIdx,
    xin_reshape_cw *reshapeCW,
    double         *alpha,
    double         *beta)
{
    SINT32 idx;
    double minVar, maxVar;
    double maxCW;
    double minCW;

    minVar = 10.0;
    maxVar = 0.0;

    for (idx = startBinIdx; idx <= endBinIdx; idx++)
    {
        if (binVar[idx] < minVar)
        {
            minVar = binVar[idx];
        }

        if (binVar[idx] > maxVar)
        {
            maxVar = binVar[idx];
        }
    }

    maxCW  = (double)reshapeCW->binCW[0];
    minCW  = (double)reshapeCW->binCW[1];
    *alpha = (minCW - maxCW) / (maxVar - minVar);
    *beta  = (maxCW * maxVar - minCW * minVar) / (maxVar - minVar);

}

static void Xin266DeriveReshapeParamSDR (
    xin_lmcs_struct *lmcsSet,
    BOOL            *intraAdp,
    BOOL            *interAdp)
{
    xin_lmcs_seq   *srcSeq;
    xin_lmcs_seq   *rspSeq;
    xin_reshape_cw *reshapeCW;
    SINT32         idx;
    BOOL           isSkipCase;
    BOOL           isLowCase;
    SINT32         firstBinVarLessThanVal1;
    SINT32         firstBinVarLessThanVal2;
    SINT32         firstBinVarLessThanVal3;
    double         percBinVarLessThenVal1;
    double         percBinVarLessThenVal2;
    double         percBinVarLessThenVal3;
    SINT32         binIdxSortDsd[XIN_LMCS_ANALYZE_CW_BINS];
    double         binVarSortDsd[XIN_LMCS_ANALYZE_CW_BINS];
    double         binVarSortDsdCDF[XIN_LMCS_ANALYZE_CW_BINS];
    double         ratioWeiVar, ratioWeiVarNorm;
    SINT32         startBinIdx;
    SINT32         endBinIdx;

    rspSeq      = &lmcsSet->rspSeq;
    srcSeq      = &lmcsSet->srcSeq;
    reshapeCW   = &lmcsSet->reshapeCW;
    isSkipCase  = FALSE;
    isLowCase   = FALSE;

    firstBinVarLessThanVal1 = 0;
    firstBinVarLessThanVal2 = 0;
    firstBinVarLessThanVal3 = 0;
    percBinVarLessThenVal1  = 0.0;
    percBinVarLessThenVal2  = 0.0;
    percBinVarLessThenVal3  = 0.0;

    ratioWeiVar     = 0.0;
    ratioWeiVarNorm = 0.0;
    startBinIdx     = lmcsSet->lmcsParam.reshaperModelMinBinIdx;
    endBinIdx       = lmcsSet->lmcsParam.reshaperModelMaxBinIdx;

    for (idx = 0; idx < lmcsSet->binNum; idx++)
    {
        binVarSortDsd[idx] = srcSeq->binVar[idx];
        binIdxSortDsd[idx] = idx;
    }

    BubbleSortDsd (
        binVarSortDsd,
        binIdxSortDsd,
        lmcsSet->binNum);

    binVarSortDsdCDF[0] = srcSeq->binHist[binIdxSortDsd[0]];

    for (idx = 1; idx < lmcsSet->binNum; idx++)
    {
        binVarSortDsdCDF[idx] = binVarSortDsdCDF[idx - 1] + srcSeq->binHist[binIdxSortDsd[idx]];
    }

    for (idx = 0; idx < lmcsSet->binNum - 1; idx++)
    {
        if (binVarSortDsd[idx] > 3.4)
        {
            firstBinVarLessThanVal1 = idx + 1;
        }

        if (binVarSortDsd[idx] > 2.8)
        {
            firstBinVarLessThanVal2 = idx + 1;
        }

        if (binVarSortDsd[idx] > 2.5)
        {
            firstBinVarLessThanVal3 = idx + 1;
        }
    }

    percBinVarLessThenVal1 = binVarSortDsdCDF[firstBinVarLessThanVal1];
    percBinVarLessThenVal2 = binVarSortDsdCDF[firstBinVarLessThanVal2];
    percBinVarLessThenVal3 = binVarSortDsdCDF[firstBinVarLessThanVal3];

    Xin266CWPerturbation (
        lmcsSet,
        startBinIdx,
        endBinIdx,
        (UINT16)reshapeCW->binCW[1]);

    Xin266CWReduction (
        lmcsSet,
        startBinIdx,
        endBinIdx);

    Xin266InitLmcsSeq (
        rspSeq,
        lmcsSet->binNum);

    for (idx = 0; idx < lmcsSet->binNum; idx++)
    {
        rspSeq->binHist[idx] = srcSeq->binHist[idx];
        rspSeq->binVar[idx]  = srcSeq->binVar[idx] + 2.0 * log10(lmcsSet->binCW[idx] > 0 ? ((double)lmcsSet->binCW[idx] / (double)lmcsSet->initCWAnalyze) : 1.0);
    }

    rspSeq->minBinVar  = 5.0;
    rspSeq->maxBinVar  = 0.0;
    rspSeq->meanBinVar = 0.0;
    rspSeq->nonZeroCnt = 0;

    for (idx = 0; idx < lmcsSet->binNum; idx++)
    {
        if (rspSeq->binHist[idx] > 0.001)
        {
            rspSeq->nonZeroCnt++;
            rspSeq->meanBinVar += rspSeq->binVar[idx];

            if (rspSeq->binVar[idx] > rspSeq->maxBinVar)
            {
                rspSeq->maxBinVar = rspSeq->binVar[idx];
            }

            if (rspSeq->binVar[idx] < rspSeq->minBinVar)
            {
                rspSeq->minBinVar = rspSeq->binVar[idx];
            }
        }
    }

    rspSeq->meanBinVar /= (double)rspSeq->nonZeroCnt;

    for (idx = 0; idx < lmcsSet->binNum; idx++)
    {
        if (rspSeq->meanBinVar > 0.0)
        {
            rspSeq->normVar[idx] = rspSeq->binVar[idx] / rspSeq->meanBinVar;
        }

        rspSeq->weightVar  += rspSeq->binHist[idx] * rspSeq->binVar[idx];
        rspSeq->weightNorm += rspSeq->binHist[idx] * rspSeq->normVar[idx];
    }

    ratioWeiVar     = rspSeq->weightVar / srcSeq->weightVar;
    ratioWeiVarNorm = rspSeq->weightNorm / srcSeq->weightNorm;

    if ((srcSeq->binHist[0] + srcSeq->binHist[lmcsSet->binNum - 1]) > 0.0001 && srcSeq->binHist[lmcsSet->binNum - 2] < 0.001)
    {
        if (percBinVarLessThenVal3 > 0.8 && percBinVarLessThenVal2 > 0.4 && srcSeq->binVar[lmcsSet->binNum - 2] > 4.8)
        {
            isSkipCase = TRUE;
        }
        else if (percBinVarLessThenVal3 < 0.1 && percBinVarLessThenVal1 < 0.05 && srcSeq->binVar[lmcsSet->binNum - 2] < 4.0)
        {
            isSkipCase = TRUE;
        }
    }

    if (isSkipCase)
    {
        *intraAdp = FALSE;
        *interAdp = FALSE;

        return;
    }

    if (reshapeCW->rspPicSize > 5184000)
    {
        isLowCase = TRUE;
    }
    else if (srcSeq->binVar[1] > 4.0)
    {
        isLowCase = TRUE;
    }
    else if (rspSeq->meanBinVar > 3.4 && ratioWeiVarNorm > 1.005 && ratioWeiVar > 1.02)
    {
        isLowCase = TRUE;
    }
    else if (rspSeq->meanBinVar > 3.1 && ratioWeiVarNorm > 1.005 && ratioWeiVar > 1.04)
    {
        isLowCase = TRUE;
    }
    else if (rspSeq->meanBinVar > 2.8 && ratioWeiVarNorm > 1.01 && ratioWeiVar > 1.04)
    {
        isLowCase = TRUE;
    }

    if (reshapeCW->updateCtrl == 0)
    {
        reshapeCW->binCW[1] = 1022;

        if (isLowCase)
        {
            *intraAdp = FALSE;

            lmcsSet->rateAdpMode = 1;
            reshapeCW->binCW[1]  = 980;

            if (srcSeq->binHist[lmcsSet->binNum - 2] > 0.05)
            {
                lmcsSet->reshapeCW.binCW[1] = 896;

                if (srcSeq->binVar[lmcsSet->binNum - 2] < 1.2)
                {
                    reshapeCW->binCW[1] = 938;
                }
            }
            else if (percBinVarLessThenVal2 < 0.8 && percBinVarLessThenVal3 == 1.0)
            {
                lmcsSet->rateAdpMode = 1;
                reshapeCW->binCW[1]  = 938;
            }

        }

        if (srcSeq->binHist[lmcsSet->binNum - 2] < 0.001)
        {
            if (srcSeq->binHist[1] > 0.05 && srcSeq->binVar[1] > 3.0)
            {
                *intraAdp = TRUE;

                lmcsSet->rateAdpMode = 1;
                reshapeCW->binCW[1]  = 784;
            }
            else if (srcSeq->binHist[1] < 0.006)
            {
                *intraAdp = FALSE;

                lmcsSet->rateAdpMode = 0;
                reshapeCW->binCW[1]  = 1008;
            }
            else if (percBinVarLessThenVal3 < 0.5)
            {
                *intraAdp = TRUE;

                lmcsSet->rateAdpMode = 0;
                reshapeCW->binCW[1]  = 1022;
            }
        }
        else if ((srcSeq->maxBinVar > 4.0 && rspSeq->meanBinVar > 3.2 && percBinVarLessThenVal2 < 0.25) || ratioWeiVar < 1.03)
        {
            *intraAdp = TRUE;

            lmcsSet->rateAdpMode = 0;
            reshapeCW->binCW[1]  = 1022;
        }

        if (*intraAdp == TRUE && lmcsSet->rateAdpMode == 0)
        {
            lmcsSet->tcase = 9;
        }

    }
    else if (reshapeCW->updateCtrl == 1)
    {
        reshapeCW->binCW[1] = 952;

        if (isLowCase)
        {
            if (reshapeCW->rspPicSize > 5184000)
            {
                lmcsSet->rateAdpMode = 1;
                reshapeCW->binCW[1]  = 812;
            }
            if (srcSeq->binHist[lmcsSet->binNum - 2] > 0.05)
            {
                lmcsSet->rateAdpMode = 1;
                reshapeCW->binCW[1]  = 812;

                if (srcSeq->binHist[lmcsSet->binNum - 2] > 0.1 || srcSeq->binHist[1] > 0.1)
                {
                    lmcsSet->rateAdpMode = 0;
                    reshapeCW->binCW[1]  = 924;
                }
            }
            else if (percBinVarLessThenVal2 < 0.8 && percBinVarLessThenVal3 == 1.0)
            {
                lmcsSet->rateAdpMode = 1;
                reshapeCW->binCW[1]  = 896;
            }
            else if (percBinVarLessThenVal2 > 0.98 && srcSeq->binHist[1] > 0.05)
            {
                lmcsSet->rateAdpMode = 0;
                reshapeCW->binCW[1]  = 784;
            }
            else if (percBinVarLessThenVal2 < 0.1)
            {
                lmcsSet->rateAdpMode = 0;
                reshapeCW->binCW[1]  = 1022;
            }

        }
        if (srcSeq->binHist[1] > 0.1 && (srcSeq->binVar[1] > 1.8 && srcSeq->binVar[1] < 3.0))
        {
            lmcsSet->rateAdpMode = 1;

            if (srcSeq->binVar[lmcsSet->binNum - 2] > 1.2 && srcSeq->binVar[lmcsSet->binNum - 2] < 4.0)
            {
                reshapeCW->binCW[1] = 784;
            }
        }
        else if (srcSeq->binHist[lmcsSet->binNum - 2] < 0.001)
        {
            if (srcSeq->binHist[1] > 0.05 && srcSeq->binVar[1] > 3.0)
            {
                lmcsSet->rateAdpMode = 1;
                reshapeCW->binCW[1]  = 784;
            }
            else if (srcSeq->binHist[1] < 0.006)
            {
                lmcsSet->rateAdpMode = 0;
                reshapeCW->binCW[1]  = 980;
            }
            else if (percBinVarLessThenVal3 < 0.5)
            {
                lmcsSet->rateAdpMode = 0;
                reshapeCW->binCW[1]  = 924;
            }
        }
        else if ((srcSeq->maxBinVar > 4.0 && rspSeq->meanBinVar > 3.2 && percBinVarLessThenVal2 < 0.25) || ratioWeiVar < 1.03)
        {
            lmcsSet->rateAdpMode = 0;
            reshapeCW->binCW[1]  = 980;
        }
    }
    else
    {
        lmcsSet->useAdpCW   = TRUE;
        reshapeCW->binCW[0] = 36;
        reshapeCW->binCW[1] = 30;

        if (isLowCase)
        {
            if (srcSeq->binHist[lmcsSet->binNum - 2] > 0.05)
            {
                lmcsSet->useAdpCW    = FALSE;
                lmcsSet->rateAdpMode = 1;

                reshapeCW->binCW[1] = 896;

                if (srcSeq->binHist[1] > 0.005)
                {
                    lmcsSet->rateAdpMode = 0;
                }
            }
            else if (percBinVarLessThenVal2 < 0.8 && percBinVarLessThenVal3 == 1.0)
            {
                lmcsSet->reshapeCW.binCW[1] = 28;
            }
        }

        if (srcSeq->binHist[1] > 0.1 && srcSeq->binVar[1] > 1.8 && srcSeq->binVar[1] < 3.0)
        {
            lmcsSet->useAdpCW    = FALSE;
            lmcsSet->rateAdpMode = 1;
            reshapeCW->binCW[1]  = 952;
        }
        else if (srcSeq->binHist[1] > 0.05 && srcSeq->binHist[lmcsSet->binNum - 2] < 0.001 && srcSeq->binVar[1] > 3.0)
        {
            lmcsSet->useAdpCW    = FALSE;
            lmcsSet->rateAdpMode = 1;
            reshapeCW->binCW[1]  = 784;
        }
        else if (srcSeq->binHist[1] > 0.05 && srcSeq->binHist[lmcsSet->binNum - 2] < 0.005 && srcSeq->binVar[1] > 1.0 && srcSeq->binVar[1] < 1.5)
        {
            lmcsSet->rateAdpMode = 2;
            reshapeCW->binCW[0]  = 38;
        }
        else if (srcSeq->binHist[1] < 0.005 && srcSeq->binHist[lmcsSet->binNum - 2] > 0.05 && srcSeq->binVar[lmcsSet->binNum - 2] > 1.0 && srcSeq->binVar[lmcsSet->binNum - 2] < 1.5)
        {
            lmcsSet->rateAdpMode = 2;
            reshapeCW->binCW[0]  = 36;
        }
        else if (srcSeq->binHist[1] > 0.02 && srcSeq->binHist[lmcsSet->binNum - 2] > 0.04 && srcSeq->binVar[1] < 2.0 && srcSeq->binVar[lmcsSet->binNum - 2] < 1.5)
        {
            lmcsSet->rateAdpMode = 2;
            reshapeCW->binCW[0]  = 34;
        }
        else if ((srcSeq->binHist[1] > 0.05 && srcSeq->binHist[lmcsSet->binNum - 2] > 0.2 && srcSeq->binVar[1] > 3.0 && srcSeq->binVar[1] < 4.0) || ratioWeiVar < 1.03)
        {
            lmcsSet->rateAdpMode = 1;
            lmcsSet->reshapeCW.binCW[0] = 34;
        }
        else if (srcSeq->binVar[1] < 4.0 && percBinVarLessThenVal2 == 1.0 && percBinVarLessThenVal3 == 1.0)
        {
            lmcsSet->rateAdpMode = 0;
            reshapeCW->binCW[0]  = 34;
        }

        if (lmcsSet->useAdpCW && !isLowCase)
        {
            reshapeCW->binCW[1] = 66 - reshapeCW->binCW[0];
        }
    }
}

static void Xin266AdjustLmcsPivot (
    xin_lmcs_struct *lmcsSet)
{
    SINT32         bdShift;
    SINT32         totCW;
    SINT32         orgCW;
    SINT32         log2SegSize;
    SINT32         idx;
    SINT32         binIdx;
    SINT32         segIdxMax;
    SINT32         segIdxCurr;
    SINT32         segIdxNext;
    SINT32         adjustVal;
    xin_lmcs_param *lmcsParam;

    bdShift     = lmcsSet->lumaBD - 10;
    totCW       = bdShift != 0 ? (bdShift > 0 ? lmcsSet->reshapeLUTSize / (1 << bdShift) : lmcsSet->reshapeLUTSize * (1 << (-bdShift))) : lmcsSet->reshapeLUTSize;
    orgCW       = totCW / XIN_LMCS_ENCODE_CW_BINS;
    log2SegSize = lmcsSet->lumaBD - 5;
    lmcsParam   = &lmcsSet->lmcsParam;

    lmcsSet->reshapePivot[0] = 0;

    for (idx = 0; idx < XIN_LMCS_ENCODE_CW_BINS; idx++)
    {
        lmcsSet->reshapePivot[idx + 1] = lmcsSet->reshapePivot[idx] + (UINT8)lmcsSet->binCW[idx];
    }

    segIdxMax = (lmcsSet->reshapePivot[lmcsParam->reshaperModelMaxBinIdx + 1] >> log2SegSize);

    for (idx = lmcsParam->reshaperModelMinBinIdx; idx <= lmcsParam->reshaperModelMaxBinIdx; idx++)
    {
        lmcsSet->reshapePivot[idx + 1] = lmcsSet->reshapePivot[idx] + (UINT8)lmcsSet->binCW[idx];

        segIdxCurr = lmcsSet->reshapePivot[idx] >> log2SegSize;
        segIdxNext = lmcsSet->reshapePivot[idx + 1] >> log2SegSize;

        if ((segIdxCurr == segIdxNext) && (lmcsSet->reshapePivot[idx] != (segIdxCurr << log2SegSize)))
        {
            if (segIdxCurr == segIdxMax)
            {
                lmcsSet->reshapePivot[idx] = lmcsSet->reshapePivot[lmcsParam->reshaperModelMaxBinIdx + 1];

                for (binIdx = idx; binIdx <= lmcsParam->reshaperModelMaxBinIdx; binIdx++)
                {
                    lmcsSet->reshapePivot[binIdx + 1] = lmcsSet->reshapePivot[idx];

                    lmcsSet->binCW[binIdx] = 0;
                }

                lmcsSet->binCW[idx - 1] = lmcsSet->reshapePivot[idx] - lmcsSet->reshapePivot[idx - 1];

                break;
            }
            else
            {
                adjustVal = ((segIdxCurr + 1) << log2SegSize) - lmcsSet->reshapePivot[idx + 1];

                lmcsSet->reshapePivot[idx + 1] += (UINT8)adjustVal;
                lmcsSet->binCW[idx]            += (UINT8)adjustVal;

                for (binIdx = idx + 1; binIdx <= lmcsParam->reshaperModelMaxBinIdx; binIdx++)
                {
                    if (lmcsSet->binCW[binIdx] < (adjustVal + (orgCW >> 3)))
                    {
                        adjustVal -= (lmcsSet->binCW[binIdx] - (orgCW >> 3));

                        lmcsSet->binCW[binIdx] = (UINT16)(orgCW >> 3);
                    }
                    else
                    {
                        lmcsSet->binCW[binIdx] -= (UINT16)adjustVal;

                        adjustVal = 0;
                    }

                    if (adjustVal == 0)
                    {
                        break;
                    }

                }

            }

        }

    }

    for (idx = XIN_LMCS_ENCODE_CW_BINS - 1; idx >= 0; idx--)
    {
        if (lmcsSet->binCW[idx] > 0)
        {
            lmcsParam->reshaperModelMaxBinIdx = idx;

            break;
        }
    }

}

static SINT32 Xin266GetReshapeInvIdx (
    xin_lmcs_struct *lmcsSet,
    SINT32          lumaVal)
{
    SINT32         idx;
    xin_lmcs_param *lmcsParam;

    lmcsParam = &lmcsSet->lmcsParam;

    for (idx = lmcsParam->reshaperModelMinBinIdx; idx <= lmcsParam->reshaperModelMaxBinIdx; idx++)
    {
        if (lumaVal < lmcsSet->reshapePivot[idx + 1])
        {
            break;
        }
    }

    return XIN_MIN (idx, XIN_LMCS_ENCODE_CW_BINS - 1);
}

void Xin266ConstructReshaperLmcs (
    xin_lmcs_struct *lmcsSet)
{
    SINT32         bdShift;
    SINT32         totCW;
    SINT32         histLenth;
    SINT32         log2Value;
    SINT32         idx;
    xin_lmcs_param *lmcsParam;
    SINT32         maxAbsDeltaCW;
    SINT32         absDeltaCW;
    SINT32         deltaCW;
    SINT32         sumBins;
    SINT32         startBinIdx;
    SINT32         endBinIdx;
    SINT32         pixelVal;
    SINT32         fwdIdx;
    SINT32         invIdx;

    bdShift   = lmcsSet->lumaBD - 10;
    totCW     = bdShift != 0 ? (bdShift > 0 ? lmcsSet->reshapeLUTSize / (1 << bdShift) : lmcsSet->reshapeLUTSize * (1 << (-bdShift))) : lmcsSet->reshapeLUTSize;
    histLenth = totCW / lmcsSet->binNum;
    lmcsParam = &lmcsSet->lmcsParam;

    if (lmcsSet->binNum == XIN_LMCS_ANALYZE_CW_BINS)
    {
        for (idx = 0; idx < XIN_LMCS_ENCODE_CW_BINS; idx++)
        {
            lmcsSet->binCW[idx] = lmcsSet->binCW[2 * idx] + lmcsSet->binCW[2 * idx + 1];
        }
    }

    for (idx = 0; idx <= XIN_LMCS_ENCODE_CW_BINS; idx++)
    {
        lmcsSet->inputPivot[idx] = (UINT8)(lmcsSet->initCW * idx);
    }

    lmcsParam->reshaperModelMinBinIdx = 0;
    lmcsParam->reshaperModelMaxBinIdx = XIN_LMCS_ENCODE_CW_BINS - 1;

    for (idx = 0; idx < XIN_LMCS_ENCODE_CW_BINS; idx++)
    {
        if (lmcsSet->binCW[idx] > 0)
        {
            lmcsParam->reshaperModelMinBinIdx = idx;

            break;
        }
    }

    for (idx = XIN_LMCS_ENCODE_CW_BINS - 1; idx >= 0; idx--)
    {
        if (lmcsSet->binCW[idx] > 0)
        {
            lmcsParam->reshaperModelMaxBinIdx = idx;

            break;
        }
    }

    if (bdShift != 0)
    {
        for (idx = 0; idx < XIN_LMCS_ANALYZE_CW_BINS; idx++)
        {
            lmcsSet->binCW[idx] = bdShift > 0 ? lmcsSet->binCW[idx] * (1 << bdShift) : lmcsSet->binCW[idx] / (1 << (-bdShift));
        }
    }

    Xin266AdjustLmcsPivot (
        lmcsSet);

    maxAbsDeltaCW = 0;
    absDeltaCW    = 0;
    deltaCW       = 0;

    for (idx = lmcsParam->reshaperModelMinBinIdx; idx <= lmcsParam->reshaperModelMaxBinIdx; idx++)
    {
        deltaCW    = (SINT32)lmcsSet->binCW[idx] - (SINT32)lmcsSet->initCW;
        absDeltaCW = (deltaCW < 0) ? (-deltaCW) : deltaCW;

        if (absDeltaCW > maxAbsDeltaCW)
        {
            maxAbsDeltaCW = absDeltaCW;
        }

        lmcsParam->reshaperModelBinCWDelta[idx] = deltaCW;
    }

    if (maxAbsDeltaCW == 0)
    {
        lmcsParam->maxNbitsNeededDeltaCW = 1;
    }
    else
    {
        BIT_SCAN_REVERSE_32 (maxAbsDeltaCW, log2Value);

        lmcsParam->maxNbitsNeededDeltaCW = XIN_MAX (1, 1 + log2Value);
    }

    sumBins   = 0;
    histLenth = lmcsSet->initCW;

    BIT_SCAN_REVERSE_32 (histLenth, log2Value);

    for (idx = 0; idx < XIN_LMCS_ENCODE_CW_BINS; idx++)
    {
        sumBins += lmcsSet->binCW[idx];
    }

    assert (sumBins < lmcsSet->reshapeLUTSize && "SDR CW assignment is wrong!!");

    for (idx = 0; idx < XIN_LMCS_ENCODE_CW_BINS; idx++)
    {
        lmcsSet->reshapePivot[idx + 1] = lmcsSet->reshapePivot[idx] + (UINT8)lmcsSet->binCW[idx];
        lmcsSet->fwdScaleCoef[idx]     = ((SINT32)lmcsSet->binCW[idx] * (1 << 11) + (1 << (log2Value - 1))) >> log2Value;

        if (lmcsSet->binCW[idx] == 0)
        {
            lmcsSet->invScaleCoef[idx]     = 0;
            lmcsSet->chromaAdjHelpLUT[idx] = 1 << 11;
        }
        else
        {
            lmcsSet->invScaleCoef[idx]     = (SINT32)(lmcsSet->initCW * (1 << 11) / lmcsSet->binCW[idx]);
            lmcsSet->chromaAdjHelpLUT[idx] = (SINT32)(lmcsSet->initCW * (1 << 11) / (lmcsSet->binCW[idx] + lmcsParam->chrResScalingOffset));
        }
    }

    for (idx = 0; idx < lmcsSet->reshapeLUTSize; idx++)
    {
        fwdIdx   = idx / lmcsSet->initCW;
        pixelVal = lmcsSet->reshapePivot[fwdIdx] + ((lmcsSet->fwdScaleCoef[fwdIdx] * (idx - lmcsSet->inputPivot[fwdIdx]) + (1 << (11 - 1))) >> 11);

        lmcsSet->fwdLUT[idx] = (PIXEL)XIN_CLIP(pixelVal, MIN_PIXEL_VALUE, MAX_PIXEL_VALUE);

        invIdx   = Xin266GetReshapeInvIdx (lmcsSet, idx);
        pixelVal = lmcsSet->inputPivot[invIdx] + ((lmcsSet->invScaleCoef[invIdx] * (idx - lmcsSet->reshapePivot[invIdx]) + (1 << (11 - 1))) >> 11);

        lmcsSet->invLUT[idx] = (PIXEL)XIN_CLIP(pixelVal, MIN_PIXEL_VALUE, MAX_PIXEL_VALUE);
    }

    for (idx = 0; idx < XIN_LMCS_ENCODE_CW_BINS; idx++)
    {
        startBinIdx = idx * histLenth;
        endBinIdx   = (idx + 1) * histLenth - 1;

        lmcsSet->cwLumaWeight[idx] = lmcsSet->fwdLUT[endBinIdx] - lmcsSet->fwdLUT[startBinIdx];
    }

}

void Xin266LmcsPreAnalyzer (
    xin_pic_struct *picSet,
    UINT32         signalType)
{
    xin_input_picture *inputPicture;
    xin_lmcs_struct   *lmcsSet;
    xin_reshape_cw    *reshapeCW;
    xin_lmcs_param    *lmcsParam;
    xin_lmcs_seq      *srcSeq;
    SINT32            framePoc;
    UINT32            frameType;
    SINT32            modIp;
    SINT32            pixelMax;
    SINT32            pixelMin;
    SINT32            startBinIdx;
    SINT32            endBinIdx;
    SINT32            idx;
    BOOL              intraAdp;
    BOOL              interAdp;
    BOOL              isFlat;
    double            alpha, beta;
    PIXEL             *inputY;
    PIXEL             *inputU;
    PIXEL             *inputV;
    PIXEL             pixelVal;
    intptr_t          strideY, strideUv;
    SINT32            x, y;
    SINT32            width, height;
    UINT32            binCnt[XIN_LMCS_ENCODE_CW_BINS];
    SINT32            binLen;
    UINT32            binIdx;
    double            avgY, avgU, avgV;
    double            varY, varU, varV;

    inputPicture = picSet->inputPicture;
    framePoc     = inputPicture->inputNumber;
    frameType    = inputPicture->frameType;
    lmcsSet      = picSet->lmcsSet;
    srcSeq       = &lmcsSet->srcSeq;
    reshapeCW    = &lmcsSet->reshapeCW;
    lmcsParam    = &lmcsSet->lmcsParam;
    modIp        = framePoc - framePoc / reshapeCW->rspFpsToIp*reshapeCW->rspFpsToIp;

    lmcsParam->sliceReshaperModelPresent = TRUE;
    lmcsParam->sliceReshaperEnabled      = TRUE;
    reshapeCW->rspBaseQp                 = picSet->picQp;

    if ((frameType >= XIN_I_FRAME) || (reshapeCW->updateCtrl == 2 && modIp == 0))
    {
        if (lmcsParam->sliceReshaperModelPresent == TRUE)
        {
            lmcsSet->binNum = XIN_LMCS_ENCODE_CW_BINS;
            binLen          = lmcsSet->reshapeLUTSize / lmcsSet->binNum;
            pixelMin        = 16 << (lmcsSet->lumaBD - 8);
            pixelMax        = 235 << (lmcsSet->lumaBD - 8);
            startBinIdx     = pixelMin / binLen;
            endBinIdx       = pixelMax / binLen;
            intraAdp        = TRUE;
            interAdp        = TRUE;
            isFlat          = TRUE;

            lmcsParam->reshaperModelMinBinIdx = startBinIdx;
            lmcsParam->reshaperModelMaxBinIdx = endBinIdx;
            lmcsParam->enableChromaAdj        = 1;
            lmcsSet->initCWAnalyze            = lmcsSet->lumaBD > 10 ? (binLen >> (lmcsSet->lumaBD - 10)) : lmcsSet->lumaBD < 10 ? (binLen << (10 - lmcsSet->lumaBD)) : binLen;

            for (idx = 0; idx < lmcsSet->binNum; idx++)
            {
                lmcsSet->binCW[idx] = (UINT16)lmcsSet->initCWAnalyze;
            }

            lmcsSet->reshape      = TRUE;
            lmcsSet->useAdpCW     = FALSE;
            lmcsSet->exceedSTD    = FALSE;
            lmcsSet->chromaWeight = 1.0;
            lmcsSet->rateAdpMode  = 0;
            lmcsSet->tcase        = 0;

            Xin266CalcLmcsSeqStat (
                picSet,
                &lmcsSet->srcSeq);

            for (idx = 0; idx < lmcsSet->binNum; idx++)
            {
                if (srcSeq->binVar[idx] > 0)
                {
                    isFlat = FALSE;
                }
            }

            if (isFlat)
            {
                intraAdp = FALSE;
                interAdp = FALSE;
            }

            if (lmcsSet->binNum == XIN_LMCS_ENCODE_CW_BINS)
            {
                if ((srcSeq->binHist[0] + srcSeq->binHist[lmcsSet->binNum - 1]) > 0.005)
                {
                    lmcsSet->exceedSTD = TRUE;
                }

                if (srcSeq->binHist[lmcsSet->binNum - 1] > 0.0003)
                {
                    intraAdp = FALSE;
                    interAdp = FALSE;
                }

                if (srcSeq->binHist[0] > 0.03)
                {
                    intraAdp = FALSE;
                    interAdp = FALSE;
                }

            }
            else if (lmcsSet->binNum == XIN_LMCS_ANALYZE_CW_BINS)
            {
                if ((srcSeq->binHist[0] + srcSeq->binHist[1] + srcSeq->binHist[lmcsSet->binNum - 2] + srcSeq->binHist[lmcsSet->binNum - 1]) > 0.01)
                {
                    lmcsSet->exceedSTD = TRUE;
                }

                if ((srcSeq->binHist[lmcsSet->binNum - 2] + srcSeq->binHist[lmcsSet->binNum - 1]) > 0.0003)
                {
                    intraAdp = FALSE;
                    interAdp = FALSE;
                }

                if ((srcSeq->binHist[0] + srcSeq->binHist[1]) > 0.03)
                {
                    intraAdp = FALSE;
                    interAdp = FALSE;
                }
            }

            if (lmcsSet->exceedSTD)
            {
                for (idx = 0; idx < lmcsSet->binNum; idx++)
                {
                    if (srcSeq->binHist[idx] > 0 && idx < startBinIdx)
                    {
                        startBinIdx = idx;
                    }

                    if (srcSeq->binHist[idx] > 0 && idx > endBinIdx)
                    {
                        endBinIdx = idx;
                    }

                }

                lmcsParam->reshaperModelMaxBinIdx = startBinIdx;
                lmcsParam->reshaperModelMaxBinIdx = endBinIdx;
            }

            if ((srcSeq->ratioStdU + srcSeq->ratioStdV) > 1.5 && srcSeq->binHist[1] > 0.5)
            {
                intraAdp = FALSE;
                interAdp = FALSE;
            }

            if (srcSeq->ratioStdU > 0.36 && srcSeq->ratioStdV > 0.2 && reshapeCW->rspPicSize > 5184000)
            {
                lmcsParam->enableChromaAdj = 0;
                lmcsSet->chromaWeight      = 1.05;

                if ((srcSeq->ratioStdU + srcSeq->ratioStdV) < 0.69)
                {
                    lmcsSet->chromaWeight = 0.95;
                }
            }

            if (interAdp)
            {
                if (reshapeCW->adpOption)
                {
                    reshapeCW->binCW[0]  = 0;
                    reshapeCW->binCW[1]  = (UINT16)reshapeCW->initialCW;
                    lmcsSet->rateAdpMode = reshapeCW->adpOption - 2 * (reshapeCW->adpOption / 2);

                    if (reshapeCW->adpOption == 2)
                    {
                        lmcsSet->tcase = 9;
                    }
                    else if (reshapeCW->adpOption > 2)
                    {
                        intraAdp = FALSE;
                    }
                }
                else if (signalType == XIN_RESHAPE_SIGNAL_SDR)
                {
                    reshapeCW->binCW[0] = 0;
                    reshapeCW->binCW[1] = 1022;

                    Xin266DeriveReshapeParamSDR (
                        lmcsSet,
                        &intraAdp,
                        &interAdp);
                }
                else if (signalType == XIN_RESHAPE_SIGNAL_HLG)
                {
                    if (reshapeCW->updateCtrl == 0)
                    {
                        lmcsSet->rateAdpMode = 0;
                        lmcsSet->tcase       = 9;
                        reshapeCW->binCW[1]  = 952;

                        if (srcSeq->meanBinVar < 2.5)
                        {
                            reshapeCW->binCW[1] = 840;
                        }
                    }
                    else
                    {
                        lmcsSet->useAdpCW    = TRUE;
                        lmcsSet->rateAdpMode = 2;

                        if (lmcsSet->binNum == XIN_LMCS_ENCODE_CW_BINS)
                        {
                            reshapeCW->binCW[0] = 72;
                            reshapeCW->binCW[1] = 58;
                        }
                        else if (lmcsSet->binNum == XIN_LMCS_ANALYZE_CW_BINS)
                        {
                            reshapeCW->binCW[0] = 36;
                            reshapeCW->binCW[1] = 30;
                        }
                        if (srcSeq->meanBinVar < 2.5)
                        {
                            intraAdp = FALSE;
                            interAdp = FALSE;
                        }
                    }

                }

            }

            if ((lmcsSet->rateAdpMode == 2) && (reshapeCW->rspBaseQp <= 22))
            {
                intraAdp = FALSE;
                interAdp = FALSE;
            }

            lmcsParam->sliceReshaperEnabled = intraAdp;

            if (!intraAdp && !interAdp)
            {
                lmcsParam->sliceReshaperModelPresent = FALSE;

                lmcsSet->reshape = FALSE;

                return;
            }

            if (lmcsSet->rateAdpMode == 1 && reshapeCW->rspBaseQp <= 22)
            {
                for (idx = 0; idx < lmcsSet->binNum; idx++)
                {
                    if (idx >= startBinIdx && idx <= endBinIdx)
                    {
                        lmcsSet->binCW[idx] = (UINT16)lmcsSet->initCWAnalyze + 2;
                    }
                    else
                    {
                        lmcsSet->binCW[idx] = 0;
                    }
                }
            }
            else if (lmcsSet->useAdpCW)
            {
                if (signalType == XIN_RESHAPE_SIGNAL_SDR && reshapeCW->updateCtrl == 2)
                {
                    lmcsSet->binNum = XIN_LMCS_ANALYZE_CW_BINS;

                    startBinIdx = startBinIdx * 2;
                    endBinIdx   = endBinIdx * 2 + 1;

                    Xin266CalcLmcsSeqStat (
                        picSet,
                        &lmcsSet->srcSeq);
                }

                alpha = 1.0;
                beta  = 0.0;

                Xin266DeriveReshapeParam (
                    srcSeq->binVar,
                    startBinIdx,
                    endBinIdx,
                    reshapeCW,
                    &alpha,
                    &beta);

                for (idx = 0; idx < lmcsSet->binNum; idx++)
                {
                    if (idx >= startBinIdx && idx <= endBinIdx)
                    {
                        lmcsSet->binCW[idx] = (UINT16)round(alpha * srcSeq->binVar[idx] + beta);
                    }
                    else
                    {
                        lmcsSet->binCW[idx] = 0;
                    }
                }

            }
            else
            {
                Xin266CWPerturbation (
                    lmcsSet,
                    startBinIdx,
                    endBinIdx,
                    reshapeCW->binCW[1]);
            }

            Xin266CWReduction (
                lmcsSet,
                startBinIdx,
                endBinIdx);

        }

        lmcsSet->chromaAdj          = lmcsParam->enableChromaAdj;
        lmcsParam->lmcsParamChanged = TRUE;

        Xin266ConstructReshaperLmcs (
            lmcsSet);

    }
    else
    {
        lmcsParam->sliceReshaperModelPresent = FALSE;
        lmcsParam->enableChromaAdj           = lmcsSet->chromaAdj;

        if (!lmcsSet->reshape)
        {
            lmcsParam->sliceReshaperEnabled = FALSE;
        }
        else
        {
            if (lmcsSet->tcase == 5)
            {
                lmcsParam->sliceReshaperEnabled = FALSE;
            }
            else if (lmcsSet->tcase < 5)
            {
                lmcsParam->sliceReshaperEnabled = reshapeCW->rspTid < lmcsSet->tcase + 1 ? FALSE : TRUE;
            }
            else
            {
                lmcsParam->sliceReshaperEnabled = reshapeCW->rspTid <= 10 - lmcsSet->tcase ? TRUE : FALSE;
            }

            if (lmcsParam->sliceReshaperEnabled)
            {
                lmcsSet->binNum = XIN_LMCS_ENCODE_CW_BINS;

                inputY  = inputPicture->inputBuf[0];
                strideY = inputPicture->inputStride[0];
                width   = inputPicture->inputWidth;
                height  = inputPicture->inputHeight;

                memset (binCnt, 0, sizeof(UINT32)*XIN_LMCS_ENCODE_CW_BINS);

                Xin266InitLmcsSeq (
                    srcSeq,
                    lmcsSet->binNum);

                for (y = 0; y < height; y++)
                {
                    for (x = 0; x < width; x++)
                    {
                        pixelVal = inputY[x];
                        binLen   = lmcsSet->reshapeLUTSize / lmcsSet->binNum;
                        binIdx   = (UINT32)(pixelVal / binLen);

                        binCnt[binIdx]++;
                    }

                    inputY += strideY;
                }

                for (idx = 0; idx < lmcsSet->binNum; idx++)
                {
                    srcSeq->binHist[idx] = (double)binCnt[idx] / (double)(reshapeCW->rspPicSize);
                }

                avgY   = 0.0;
                varY   = 0.0;
                inputY = inputPicture->inputBuf[0];

                for (y = 0; y < height; y++)
                {
                    for (x = 0; x < width; x++)
                    {
                        avgY += inputY[x];
                        varY += (double)inputY[x] * (double)inputY[x];
                    }

                    inputY += strideY;
                }

                avgY = avgY / (width * height);
                varY = varY / (width * height) - avgY * avgY;

                inputU   = inputPicture->inputBuf[1];
                inputV   = inputPicture->inputBuf[2];
                width    = width >> 1;
                height   = height >> 1;
                strideUv = inputPicture->inputStride[1];
                avgU     = 0.0;
                avgV     = 0.0;
                varU     = 0.0;
                varV     = 0.0;

                for (y = 0; y < height; y++)
                {
                    for (x = 0; x < width; x++)
                    {
                        avgU += inputU[x];
                        avgV += inputV[x];
                        varU += (SINT64)inputU[x] * (SINT64)inputU[x];
                        varV += (SINT64)inputV[x] * (SINT64)inputV[x];
                    }

                    inputU += strideUv;
                    inputV += strideUv;
                }

                avgU = avgU / (width * height);
                avgV = avgV / (width * height);
                varU = varU / (width * height) - avgU * avgU;
                varV = varV / (width * height) - avgV * avgV;

                if (varY > 0)
                {
                    srcSeq->ratioStdU = sqrt(varU) / sqrt(varY);
                    srcSeq->ratioStdV = sqrt(varV) / sqrt(varY);
                }

                if (srcSeq->binHist[lmcsSet->binNum - 1] > 0.0003)
                {
                    lmcsParam->sliceReshaperEnabled = FALSE;
                }
                if (srcSeq->binHist[0] > 0.03)
                {
                    lmcsParam->sliceReshaperEnabled = FALSE;
                }

                if ((srcSeq->ratioStdU + srcSeq->ratioStdV) > 1.5 && srcSeq->binHist[1] > 0.5)
                {
                    lmcsParam->sliceReshaperEnabled = FALSE;
                }

            }
        }
    }

}

SINT32 Xin266CreateLmcs (
    xin_pic_struct *picSet)
{
    xin_seq_struct  *seqSet;
    xin_lmcs_struct *lmcsSet;
    xin_reshape_cw  *reshapeCW;
    SINT32          frameWidth;
    SINT32          frameHeight;

    seqSet      = picSet->seqSet;
    frameWidth  = seqSet->config.inputWidth;
    frameHeight = seqSet->config.inputHeight;

    if (!seqSet->config.enableLmcs)
    {
        return XIN_SUCCESS;
    }

    XIN_MALLOC_CHECK (lmcsSet, sizeof(xin_lmcs_struct));
    memset(lmcsSet, 0, sizeof(xin_lmcs_struct));

    XIN_MALLOC_CHECK (lmcsSet->tempBuffer, frameWidth*sizeof(SINT64)*8);

    lmcsSet->lumaBD         = XIN_INTERNAL_BIT_DEPTH;
    lmcsSet->reshapeLUTSize = 1 << lmcsSet->lumaBD;
    lmcsSet->initCWAnalyze  = lmcsSet->reshapeLUTSize / XIN_LMCS_ANALYZE_CW_BINS;
    lmcsSet->initCW         = (UINT16)(lmcsSet->reshapeLUTSize / XIN_LMCS_ENCODE_CW_BINS);

    reshapeCW             = &lmcsSet->reshapeCW;
    reshapeCW->rspBaseQp  = 25;
    reshapeCW->rspFps     = (SINT32)seqSet->config.frameRate;
    reshapeCW->rspPicSize = frameWidth*frameHeight;
    reshapeCW->updateCtrl = seqSet->config.bFrameNum ? 0 : 2;
    reshapeCW->rspFpsToIp = XIN_MAX (16, 16 * (SINT32)(round((double)reshapeCW->rspFps/16.0)));
    picSet->lmcsSet       = lmcsSet;

    return XIN_SUCCESS;

}

SINT32 Xin266GetPWLIdxInv (
    xin_lmcs_struct *lmcsSet,
    SINT32          lumaVal)
{
    SINT32 idxS;

    for (idxS = lmcsSet->lmcsParam.reshaperModelMinBinIdx; (idxS <= lmcsSet->lmcsParam.reshaperModelMaxBinIdx); idxS++)
    {
        if (lumaVal < lmcsSet->reshapePivot[idxS + 1])
        {
            break;
        }
    }

    return XIN_MIN (idxS, XIN_LMCS_ENCODE_CW_BINS-1);
}

void Xin266CalcChromaAdjVpdu (
    xin_sec_struct *secSet,
    SINT32         *ouputScale)
{
    xin_seq_struct  *seqSet;
    xin_pic_struct  *picSet;
    xin_lmcs_struct *lmcsSet;
    xin_ref_picture *pictureWrite;
    SINT32          ctuSize;
    SINT32          pixelX;
    SINT32          pixelY;
    SINT32          numNeigh;
    SINT32          numNeighLog;
    SINT32          pixelNum;
    SINT32          idx;
    SINT32          frameWidth;
    SINT32          frameHeight;
    PIXEL           *reconY;
    intptr_t        reconStride;
    SINT32          reconLuma;
    SINT32          offset;
    SINT32          lumaValue;
    SINT32          chromaScale;

    seqSet       = secSet->seqSet;
    picSet       = secSet->picSet;
    lmcsSet      = picSet->lmcsSet;
    pictureWrite = picSet->pictureWrite;
    reconStride  = pictureWrite->refStride[0];
    ctuSize      = seqSet->ctuSize == 128 ? 64 : seqSet->ctuSize;
    pixelX       = secSet->cu->cuPelX;
    pixelY       = secSet->cu->cuPelY;
    pixelX       = pixelX / ctuSize * ctuSize;
    pixelY       = pixelY / ctuSize * ctuSize;
    numNeigh     = ctuSize;
    numNeighLog  = calcLog2[numNeigh];
    frameWidth   = seqSet->frameWidth;
    frameHeight  = seqSet->frameHeight;
    reconY       = pictureWrite->refBuf[0] + reconStride*pixelY + pixelX;
    pixelNum     = 0;
    reconLuma    = 0;

    // Left
    if (pixelX)
    {
        for (idx = 0; idx < numNeigh; idx++)
        {
            offset     = (pixelY + idx) >= frameHeight ? (frameHeight - pixelY - 1) : idx;
            reconLuma += reconY[-1 + offset * reconStride];

            pixelNum++;
        }
    }

    // Above
    if (pixelY)
    {
        for (idx = 0; idx < numNeigh; idx++)
        {
            offset     = (pixelX + idx) >= frameWidth ? (frameWidth - pixelX - 1) : idx;
            reconLuma += reconY[-reconStride + offset];

            pixelNum++;
        }
    }

    if (pixelNum == numNeigh)
    {
        lumaValue = (reconLuma + (1 << (numNeighLog - 1))) >> numNeighLog;
    }
    else if (pixelNum == (numNeigh << 1))
    {
        lumaValue = (reconLuma + (1 << numNeighLog)) >> (numNeighLog + 1);
    }
    else
    {
        lumaValue = XIN_DEFAULT_PIXEL;
    }

    chromaScale = lmcsSet->chromaAdjHelpLUT[Xin266GetPWLIdxInv(lmcsSet, lumaValue)];

    *ouputScale = chromaScale;

}

void Xin266ReshapeSignal (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *output,
    intptr_t outputStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *fwdLUT)
{
    SINT32 rowIdx, colIdx;

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        for (colIdx = 0; colIdx < width; colIdx++)
        {
            output[colIdx] = fwdLUT[input[colIdx]];
        }

        input  += inputStride;
        output += outputStride;
    }

}

void Xin266FwdScaleSignal (
    SINT16   *residual,
    intptr_t residualStride,
    SINT32   width,
    SINT32   height,
    SINT32   chromaAdj)
{
    SINT32 colIdx, rowIdx;
    SINT32 sign;
    SINT32 absVal;
    SINT32 maxVal;

    maxVal = MAX_PIXEL_VALUE;

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        for (colIdx = 0; colIdx < width; colIdx++)
        {
            sign   = residual[colIdx] >= 0 ? 1 : -1;
            absVal = sign * residual[colIdx];

            residual[colIdx] = (SINT16)XIN_CLIP (sign * (((absVal << 11) + (chromaAdj >> 1)) / chromaAdj), -maxVal, +maxVal);

        }

        residual += residualStride;
    }

}

void Xin266InvScaleSignal (
    SINT16   *residual,
    intptr_t residualStride,
    SINT32   width,
    SINT32   height,
    SINT32   chromaAdj)
{
    SINT32 colIdx, rowIdx;
    SINT32 sign;
    SINT32 absVal;
    SINT32 maxVal;

    maxVal = MAX_PIXEL_VALUE;

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        for (colIdx = 0; colIdx < width; colIdx++)
        {
            residual[colIdx] = (SINT16)XIN_CLIP(residual[colIdx], -maxVal, +maxVal);

            sign   = residual[colIdx] >= 0 ? 1 : -1;
            absVal = sign * residual[colIdx];

            residual[colIdx] = (SINT16)(sign*((absVal * chromaAdj + (1 << (11 - 1))) >> 11));

        }

        residual += residualStride;
    }

}

void Xin266InvReshapeSignal (
    xin_pic_struct *picSet,
    xin_ctu_struct *ctu)
{
    xin_ref_picture *pictureWrite;
    xin_lmcs_struct *lmcsSet;
    intptr_t        reconStride;
    PIXEL           *reconY;
    SINT32          width;
    SINT32          height;

    pictureWrite = picSet->pictureWrite;
    lmcsSet      = picSet->lmcsSet;
    reconStride  = pictureWrite->refStride[0];
    reconY       = pictureWrite->refBuf[PLANE_LUMA] + ctu->ctuPelY*reconStride + ctu->ctuPelX;
    width        = ctu->width;
    height       = ctu->height;

    Xin266ReshapeSignal (
        reconY,
        reconStride,
        reconY,
        reconStride,
        width,
        height,
        lmcsSet->invLUT);

}


