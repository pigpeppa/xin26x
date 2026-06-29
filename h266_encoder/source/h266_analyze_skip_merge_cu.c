/***************************************************************************//**
 *
 * @file          h266_analyze_skip_merge_cu.c
 * @brief         Analyze skip/merge coding unit.
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
#include "string.h"
#include "xin_video_common.h"
#include "h26x_definition.h"
#include "h266_constant.h"
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
#include "basic_macro.h"
#include "h266_md_buf_manipulate.h"
#include "h266_intra_prediction.h"
#include "h26x_compute_dist.h"
#include "h266_entropy_manipulate.h"
#include "h266_trans_recon.h"
#include "h266_compute_dist.h"
#include "h26x_block_utility.h"
#include "h266_get_neighbour_mv.h"
#include "h266_inter_prediction.h"
#include "h266_analyze_cu.h"
#include "h266_lmcs.h"
#include "h266_func_struct.h"

static const UINT32 earlyStopThr[XIN_QP_NUM] =
{
    10,   11,   12,   14,   16,   17,
    20,   22,   25,   28,   32,   35,
    40,   44,   51,   57,   64,   71,
    80,   89,   102,  114,  128,  143,
    160,  179,  204,  228,  256,  287,
    320,  359,  408,  456,  512,  575,
    640,  719,  816,  912,  1024, 1151,
    1280, 1439, 1632, 1824, 2048, 2303,
    2560, 2879, 3264, 3648,
};

static void Xin266KickOutSameMv (
    xin_neighbour_mv *mergeMv,
    UINT32           *mergeIdx,
    UINT32           *numMv)
{
    UINT32  mvIdx;
    UINT32  idx;
    UINT32  numValidMv;
    BOOL    isSameMv;

    numValidMv  = 1;
    mergeIdx[0] = 0;

    for (mvIdx = 1;  mvIdx < *numMv; mvIdx++)
    {
        isSameMv = FALSE;

        for (idx = 0; idx < numValidMv; idx++)
        {
            if ((mergeMv[idx].mv[XIN_LIST_0].s64x1 == mergeMv[mvIdx].mv[XIN_LIST_0].s64x1)
                    && (mergeMv[idx].refIdx[XIN_LIST_0] == mergeMv[mvIdx].refIdx[XIN_LIST_0])
                    && (mergeMv[idx].mv[XIN_LIST_1].s64x1 == mergeMv[mvIdx].mv[XIN_LIST_1].s64x1)
                    && (mergeMv[idx].refIdx[XIN_LIST_1] == mergeMv[mvIdx].refIdx[XIN_LIST_1])
                    && (mergeMv[idx].useAltHpelIf == mergeMv[mvIdx].useAltHpelIf))
            {
                isSameMv = TRUE;

                break;
            }
        }

        if (!isSameMv)
        {
            mergeIdx[numValidMv]  = mvIdx;
            mergeMv[numValidMv++] = mergeMv[mvIdx];
        }

    }

    *numMv = numValidMv;

}

static void Xin266KickOutSameAffineMv (
    xin_affine_cpmv *mergeMv,
    UINT32          *mergeIdx,
    UINT32          *numMv)
{
    UINT32  mvIdx;
    UINT32  idx;
    UINT32  numValidMv;
    BOOL    isSameMv;

    numValidMv  = 1;
    mergeIdx[0] = 0;

    for (mvIdx = 1;  mvIdx < *numMv; mvIdx++)
    {
        isSameMv = FALSE;

        for (idx = 0; idx < numValidMv; idx++)
        {
            if (mergeMv[idx].affineType == mergeMv[mvIdx].affineType)
            {
                if (mergeMv[mvIdx].affineType == XIN_AFFINE_MODEL_4PARAM)
                {
                    if (((mergeMv[idx].mv[XIN_LIST_0][0].s64x1 == mergeMv[mvIdx].mv[XIN_LIST_0][0].s64x1)
                            && (mergeMv[idx].mv[XIN_LIST_0][1].s64x1 == mergeMv[mvIdx].mv[XIN_LIST_0][1].s64x1)
                            && (mergeMv[idx].refIdx[XIN_LIST_0] == mergeMv[mvIdx].refIdx[XIN_LIST_0])
                            && (mergeMv[idx].mv[XIN_LIST_1][0].s64x1 == mergeMv[mvIdx].mv[XIN_LIST_1][0].s64x1)
                            && (mergeMv[idx].mv[XIN_LIST_1][1].s64x1 == mergeMv[mvIdx].mv[XIN_LIST_1][1].s64x1)
                            && (mergeMv[idx].refIdx[XIN_LIST_1] == mergeMv[mvIdx].refIdx[XIN_LIST_1])))
                    {

                        isSameMv = TRUE;

                        break;
                    }
                }
                else if (mergeMv[mvIdx].affineType == XIN_AFFINE_MODEL_6PARAM)
                {
                    if (((mergeMv[idx].mv[XIN_LIST_0][0].s64x1 == mergeMv[mvIdx].mv[XIN_LIST_0][0].s64x1)
                            && (mergeMv[idx].mv[XIN_LIST_0][1].s64x1 == mergeMv[mvIdx].mv[XIN_LIST_0][1].s64x1)
                            && (mergeMv[idx].mv[XIN_LIST_0][2].s64x1 == mergeMv[mvIdx].mv[XIN_LIST_0][2].s64x1)
                            && (mergeMv[idx].refIdx[XIN_LIST_0] == mergeMv[mvIdx].refIdx[XIN_LIST_0])
                            && (mergeMv[idx].mv[XIN_LIST_1][0].s64x1 == mergeMv[mvIdx].mv[XIN_LIST_1][0].s64x1)
                            && (mergeMv[idx].mv[XIN_LIST_1][1].s64x1 == mergeMv[mvIdx].mv[XIN_LIST_1][1].s64x1)
                            && (mergeMv[idx].mv[XIN_LIST_1][2].s64x1 == mergeMv[mvIdx].mv[XIN_LIST_1][2].s64x1)
                            && (mergeMv[idx].refIdx[XIN_LIST_1] == mergeMv[mvIdx].refIdx[XIN_LIST_1])))
                    {
                        isSameMv = TRUE;

                        break;
                    }
                }

            }

        }

        if (!isSameMv)
        {
            mergeIdx[numValidMv]  = mvIdx;
            mergeMv[numValidMv++] = mergeMv[mvIdx];
        }

    }

    *numMv = numValidMv;

}

void Xin266CheckCoeffSkip (
    xin_sec_struct  *secSet,
    PIXEL           *input,
    intptr_t        inputStride,
    PIXEL           *recon,
    intptr_t        reconStride,
    UINT32          width,
    UINT32          height,
    SINT32          coeffThr,
    BOOL            *skipCoeff)
{
    xin_func_struct *funcSet;
    UINT32          rowIdx;
    UINT32          colIdx;
    SINT32          sseVal;
    SINT32          varVal;

    funcSet    = secSet->funcSet;
    *skipCoeff = TRUE;

    for (rowIdx = 0; rowIdx < height; rowIdx += 8)
    {
        for (colIdx = 0; colIdx < width; colIdx += 8)
        {
            funcSet->pfXinComputeVar8x8 (
                input + colIdx,
                inputStride,
                recon + colIdx,
                reconStride,
                &sseVal,
                &varVal);

            if (!(varVal < coeffThr))
            {
                *skipCoeff = FALSE;

                return;
            }

            if (!(sseVal - varVal < coeffThr))
            {
                *skipCoeff = FALSE;

                return;
            }

        }

        input += inputStride*8;
        recon += reconStride*8;

    }

}

void Xin266ComputeSkipThresh (
    xin_sec_struct  *secSet,
    xin_cu_struct   *cu,
    UINT64          *skipThr)
{
    xin_ctu_struct   *lftCtu;
    xin_ctu_struct   *topCtu;
    xin_ctu_struct   *topLftCtu;
    xin_ctu_struct   *topRgtCtu;
    UINT32           skipCount;
    UINT64           skipSse;
    UINT32           rowIdx, colIdx;

    lftCtu    = secSet->lftCtu;
    topCtu    = secSet->topCtu;
    topLftCtu = secSet->topLftCtu;
    topRgtCtu = secSet->topRgtCtu;
    skipCount = 0;
    skipSse   = 0;
    rowIdx    = cu->lgHeight - 2;
    colIdx    = cu->lgWidth - 2;

    if (lftCtu)
    {
        skipCount += lftCtu->skipCount[rowIdx][colIdx];
        skipSse   += lftCtu->skipSse[rowIdx][colIdx];
    }

    if (topCtu)
    {
        skipCount += topCtu->skipCount[rowIdx][colIdx];
        skipSse   += topCtu->skipSse[rowIdx][colIdx];
    }

    if (topLftCtu)
    {
        skipCount += topLftCtu->skipCount[rowIdx][colIdx];
        skipSse   += topLftCtu->skipSse[rowIdx][colIdx];
    }

    if (topRgtCtu)
    {
        skipCount += topRgtCtu->skipCount[rowIdx][colIdx];
        skipSse   += topRgtCtu->skipSse[rowIdx][colIdx];
    }

    *skipThr = XIN_UNSIGNED_ROUND_DIV (skipSse, skipCount);

}

static void Xin266EarlySkipDetection (
    xin_sec_struct  *secSet,
    xin_cu_struct   *cu,
    xin_fast_md_buf *fastBuf,
    UINT32          skipSyntax)
{
    UINT64           skipThr;
    xin_ctu_struct   *ctu;
    UINT32           rowIdx, colIdx;
    BOOL             coeffSkip;
    intptr_t         predYPos;

    ctu      = secSet->ctu;
    rowIdx   = cu->lgHeight - 2;
    colIdx   = cu->lgWidth - 2;
    predYPos = cu->offX + cu->offY*fastBuf->lumaStride;

    Xin266ComputeSkipThresh (
        secSet,
        cu,
        &skipThr);

    if (fastBuf->sse < skipThr)
    {
        ctu->skipSse[rowIdx][colIdx]   += fastBuf->sse;
        ctu->skipCount[rowIdx][colIdx] += 1;

        fastBuf->type       = XIN_SKIP_MODE;
        fastBuf->syntaxRate = skipSyntax;
    }

    if ((fastBuf->type != XIN_SKIP_MODE) && (fastBuf->sse < 3*skipThr))
    {
        Xin266CheckCoeffSkip (
            secSet,
            secSet->reshapeCu[PLANE_LUMA],
            secSet->inputYStride,
            fastBuf->predBuf[0] + predYPos,
            fastBuf->lumaStride,
            cu->width,
            cu->height,
            earlyStopThr[secSet->qp],
            &coeffSkip);

        if (coeffSkip)
        {
            ctu->skipSse[rowIdx][colIdx]   += fastBuf->sse;
            ctu->skipCount[rowIdx][colIdx] += 1;

            fastBuf->type       = XIN_SKIP_MODE;
            fastBuf->syntaxRate = skipSyntax;
        }

    }

}

static BOOL Xin266CheckAffineMvValid (
    xin_affine_cpmv *affineMv)
{
    UINT32  mvNum;
    UINT32  mvIdx;
    BOOL    difMv;

    mvNum = affineMv->affineType == XIN_AFFINE_MODEL_4PARAM ? 2 : 3;
    difMv = FALSE;

    if (affineMv->refIdx[0] >= 0)
    {
        for (mvIdx = 1; mvIdx < mvNum; mvIdx++)
        {
            if (affineMv->mv[0][0].s64x1 != affineMv->mv[0][mvIdx].s64x1)
            {
                difMv = TRUE;

                break;
            }
        }
    }

    if ((affineMv->refIdx[1] >= 0) && (difMv == FALSE))
    {
        for (mvIdx = 1; mvIdx < mvNum; mvIdx++)
        {
            if (affineMv->mv[1][0].s64x1 != affineMv->mv[1][mvIdx].s64x1)
            {
                difMv = TRUE;

                break;
            }
        }
    }

    return difMv;

}

static void Xin266ComputeSkipCuCost (
    xin_sec_struct  *secSet,
    xin_fast_md_buf *fastBuf,
    xin_cu_struct   *cu,
    UINT32          skipSyntax)
{
    PIXEL           *input[PLANE_NUM];
    intptr_t        inputYStride;
    intptr_t        inputUvStride;
    PIXEL           *recon[PLANE_NUM];
    intptr_t        predYPos;
    intptr_t        predUvPos;
    intptr_t        reconYStride;
    intptr_t        reconUvStride;
    UINT64          ySse, uSse, vSse;
    UINT64          weightUvSse;
    xin_func_struct *funcSet;
    xin_prob_model  *context;

    funcSet   = secSet->funcSet;
    predUvPos = (cu->offX + cu->offY*fastBuf->chromaStride) >> 1;
    predYPos  = cu->offX + cu->offY*fastBuf->lumaStride;
    context   = secSet->cabacSet->context;

    recon[0]      = fastBuf->predBuf[0] + predYPos;
    recon[1]      = fastBuf->predBuf[1] + predUvPos;
    recon[2]      = fastBuf->predBuf[2] + predUvPos;
    reconYStride  = fastBuf->lumaStride;
    reconUvStride = fastBuf->chromaStride;
    input[0]      = secSet->reshapeCu[0];
    input[1]      = secSet->reshapeCu[1];
    input[2]      = secSet->reshapeCu[2];
    inputYStride  = secSet->inputYStride;
    inputUvStride = secSet->inputUvStride;

    funcSet->pfXinComputeSsd[cu->lgWidth] (
        input[PLANE_LUMA],
        inputYStride,
        recon[PLANE_LUMA],
        reconYStride,
        cu->width,
        cu->height,
        &ySse);

    funcSet->pfXinComputeSsd[cu->lgWidth-1] (
        input[PLANE_CHROMA_U],
        inputUvStride,
        recon[PLANE_CHROMA_U],
        reconUvStride,
        cu->width/2,
        cu->height/2,
        &uSse);

    funcSet->pfXinComputeSsd[cu->lgWidth-1] (
        input[PLANE_CHROMA_V],
        inputUvStride,
        recon[PLANE_CHROMA_V],
        reconUvStride,
        cu->width/2,
        cu->height/2,
        &vSse);

    weightUvSse  = (uSse*secSet->chromaWeight) >> XIN_UV_WEIGHT_FRAC;
    weightUvSse += (vSse*secSet->chromaWeight) >> XIN_UV_WEIGHT_FRAC;

    fastBuf->sse      = ySse + weightUvSse;
    fastBuf->sseCost  = fastBuf->sse << XIN_COST_FRACTION;
    fastBuf->sseCost += CALC_SSE_COST(secSet->sseLambda[PLANE_LUMA], skipSyntax);

}

void Xin266AnalyzeSkipCu (
    xin_sec_struct  *secSet,
    xin_cu_struct   *cu)
{
    xin_pu_struct      *pu;
    xin_seq_struct     *seqSet;
    xin_prob_model     *context;
    xin_mode_struct    *modeCtrl;
    xin_func_struct    *funcSet;
    xin_neighbour_mv   *mergeCand;
    xin_neighbour_mv   *curMerge;
    xin_fast_md_buf    *curBuf;
    xin_lmcs_struct    *lmcsSet;
    xin_ref_picture    *pictureWrite;
    xin_ref_picture    *pictureRead0;
    xin_ref_picture    *pictureRead1;
    xin_pic_struct     *picSet;
    intptr_t           predYPos;
    UINT32             bufIdx;
    UINT32             mergeIdes[XIN_MAX_MERGE_MV_NUM];
    UINT32             numMv;
    UINT32             mvIdx;
    UINT32             mergeIdx;
    UINT32             sad;
    PIXEL              *input[PLANE_NUM];
    intptr_t           inputYStride;
    intptr_t           inputUvStride;
    UINT32             lambda;
    SINT16             refIdx0;
    SINT16             refIdx1;
    BOOL               affineAvail;
    UINT32             skipSyntax;
    xin_affine_cpmv    *affineMv;

    mvIdx   = 0;
    bufIdx  = 0;
    pu      = &cu->pu;
    seqSet  = secSet->seqSet;
    funcSet = secSet->funcSet;
    picSet  = secSet->picSet;
    lmcsSet = picSet->lmcsSet;
    lambda  = secSet->sadLambda[PLANE_LUMA];
    curBuf  = NULL;
    context = secSet->cabacSet->context;

    affineAvail  = (seqSet->config.enableSbtMvp || seqSet->config.enableAffine);
    mergeCand    = secSet->mergeCand;
    pictureWrite = picSet->pictureWrite;
    modeCtrl     = cu->modeCtrl;

    memset (
        secSet->mergeCand,
        0xFF,
        sizeof(xin_neighbour_mv)*XIN_MAX_MERGE_MV_NUM);

    if (pictureWrite->frameType == XIN_B_FRAME)
    {
        Xin266FillMergeCandB (
            secSet,
            pu,
            secSet->mergeCand,
            &numMv);
    }
    else
    {
        Xin266FillMergeCandP (
            secSet,
            pu,
            secSet->mergeCand,
            &numMv);
    }

    numMv = XIN_MIN (numMv, seqSet->config.maxMergeCand);

    Xin266KickOutSameMv (
        secSet->mergeCand,
        mergeIdes,
        &numMv);

    input[0]      = secSet->reshapeCu[0];
    input[1]      = secSet->reshapeCu[1];
    input[2]      = secSet->reshapeCu[2];
    inputYStride  = secSet->inputYStride;
    inputUvStride = secSet->inputUvStride;

    for (mvIdx = 0; mvIdx < numMv; mvIdx++)
    {
        Xin266FindHighestSadBuf (
            secSet->interBuf,
            secSet->interBufNum,
            &curBuf);

        mergeIdx = mergeIdes[mvIdx];
        curMerge = mergeCand + mvIdx;
        refIdx0  = mergeCand[mvIdx].refIdx[0];
        refIdx1  = mergeCand[mvIdx].refIdx[1];
        predYPos = cu->offX + cu->offY*curBuf->lumaStride;

        if ((seqSet->config.enableDmvr) && (refIdx0 >= 0) && (refIdx1 >= 0) && (cu->width*cu->height > 64))
        {
            pictureRead0 = picSet->pictureRead[XIN_LIST_0][refIdx0];
            pictureRead1 = picSet->pictureRead[XIN_LIST_1][refIdx1];

            curBuf->mvRefine = (pictureRead0->framePoc != pictureRead1->framePoc) && ((pictureWrite->framePoc - pictureRead0->framePoc) == (pictureRead1->framePoc - pictureWrite->framePoc)) && (curMerge->bcwIdx == XIN_BCW_DEFAULT);
        }
        else
        {
            curBuf->mvRefine = FALSE;
        }

        if (curBuf->mvRefine)
        {
            Xin266LumaMotionCompDmvr (
                secSet,
                pu,
                curBuf->predBuf[PLANE_LUMA] + predYPos,
                curBuf->lumaStride,
                mergeCand[mvIdx].mv,
                mergeCand[mvIdx].refIdx,
                curBuf->mvdL0SubPu,
                mergeCand[mvIdx].useAltHpelIf);
        }
        else
        {
            Xin266LumaMotionComp (
                secSet,
                pu->width,
                pu->height,
                curBuf->predBuf[PLANE_LUMA] + predYPos,
                curBuf->lumaStride,
                mergeCand[mvIdx].mv,
                mergeCand[mvIdx].refIdx,
                (UINT8)mergeCand[mvIdx].bcwIdx,
                mergeCand[mvIdx].useAltHpelIf);
        }

        if (seqSet->config.enableLmcs && lmcsSet->lmcsParam.sliceReshaperEnabled)
        {
            Xin266ReshapeSignal (
                curBuf->predBuf[PLANE_LUMA] + predYPos,
                curBuf->lumaStride,
                curBuf->predBuf[PLANE_LUMA] + predYPos,
                curBuf->lumaStride,
                pu->width,
                pu->height,
                lmcsSet->fwdLUT);
        }

        funcSet->pfXinComputeDist[cu->lgWidth] (
            input[PLANE_LUMA],
            inputYStride,
            curBuf->predBuf[PLANE_LUMA] + predYPos,
            curBuf->lumaStride,
            cu->width,
            cu->height,
            &sad);

        curBuf->fullBuf     = NULL;
        curBuf->sad         = sad;
        curBuf->mergeIndex  = mergeIdx;
        curBuf->affine      = FALSE;
        curBuf->imvIdx      = curMerge->useAltHpelIf ? XIN_IMV_HPEL : XIN_IMV_OFF;
        curBuf->bcwIdx      = curMerge->bcwIdx;
        curBuf->mergeFlag   = TRUE;
        curBuf->type        = XIN_MERGE_MODE;
        curBuf->didChromaMc = FALSE;

        curBuf->mv[XIN_LIST_0].s64x1 = curMerge->mv[XIN_LIST_0].s64x1;
        curBuf->refIdx[XIN_LIST_0]   = curMerge->refIdx[XIN_LIST_0];
        curBuf->mv[XIN_LIST_1].s64x1 = curMerge->mv[XIN_LIST_1].s64x1;
        curBuf->refIdx[XIN_LIST_1]   = curMerge->refIdx[XIN_LIST_1];

        curBuf->predMv[XIN_LIST_0].s64x1 = curBuf->mv[XIN_LIST_0].s64x1;
        curBuf->predMv[XIN_LIST_1].s64x1 = curBuf->mv[XIN_LIST_1].s64x1;

        // Estimate syntax rate for merge mode
        Xin266EstimateCuSynatax (
            secSet,
            context,
            FALSE,
            cu,
            curBuf,
            &curBuf->syntaxRate);

        curBuf->sadCost  = curBuf->sad << XIN_COST_FRACTION;
        curBuf->sadCost += CALC_SAD_COST(lambda, curBuf->syntaxRate);

    }

    if (affineAvail)
    {
        Xin266GetAffineMergeCand (
            secSet,
            pu,
            secSet->affMergeCand,
            &numMv);

        for (mvIdx = 0; mvIdx < numMv; mvIdx++)
        {
            Xin266FindHighestSadBuf (
                secSet->interBuf,
                secSet->interBufNum,
                &curBuf);

            affineMv = secSet->affMergeCand + mvIdx;
            predYPos = cu->offX + cu->offY*curBuf->lumaStride;

            if (affineMv->affineType != XIN_AFFINE_SBTMVP)
            {
                if (!Xin266CheckAffineMvValid (affineMv))
                {
                    continue;
                }

                if (affineMv->refIdx[0] >= 0)
                {
                    Xin266FillAffineMotionVector (
                        pu,
                        affineMv->mv,
                        affineMv->refIdx,
                        affineMv->affineType,
                        XIN_LIST_0,
                        secSet->affMvBuf[mvIdx][XIN_LIST_0]);

                }

                if (affineMv->refIdx[1] >= 0)
                {
                    Xin266FillAffineMotionVector (
                        pu,
                        affineMv->mv,
                        affineMv->refIdx,
                        affineMv->affineType,
                        XIN_LIST_1,
                        secSet->affMvBuf[mvIdx][XIN_LIST_1]);

                }

                Xin266LumaMotionCompAffine (
                    secSet,
                    pu,
                    curBuf->predBuf[PLANE_LUMA] + predYPos,
                    curBuf->lumaStride,
                    affineMv->refIdx,
                    secSet->affMvBuf[mvIdx],
                    affineMv->bcwIdx);

            }
            else
            {
                Xin266LumaMotionCompSubBlock (
                    secSet,
                    pu,
                    curBuf->predBuf[PLANE_LUMA] + predYPos,
                    curBuf->lumaStride,
                    modeCtrl->subMv,
                    modeCtrl->subRefIdx,
                    affineMv->bcwIdx);
            }

            if (seqSet->config.enableLmcs && lmcsSet->lmcsParam.sliceReshaperEnabled)
            {
                Xin266ReshapeSignal (
                    curBuf->predBuf[PLANE_LUMA] + predYPos,
                    curBuf->lumaStride,
                    curBuf->predBuf[PLANE_LUMA] + predYPos,
                    curBuf->lumaStride,
                    pu->width,
                    pu->height,
                    lmcsSet->fwdLUT);

            }

            funcSet->pfXinComputeDist[cu->lgWidth] (
                input[PLANE_LUMA],
                inputYStride,
                curBuf->predBuf[PLANE_LUMA] + predYPos,
                curBuf->lumaStride,
                cu->width,
                cu->height,
                &sad);

            curBuf->fullBuf     = NULL;
            curBuf->affine      = TRUE;
            curBuf->mvRefine    = FALSE;
            curBuf->imvIdx      = 0;
            curBuf->sad         = sad;
            curBuf->mergeIndex  = mvIdx;
            curBuf->affineType  = affineMv->affineType;
            curBuf->refIdx[0]   = affineMv->refIdx[0];
            curBuf->refIdx[1]   = affineMv->refIdx[1];
            curBuf->bcwIdx      = affineMv->bcwIdx;
            curBuf->mergeFlag   = TRUE;
            curBuf->type        = XIN_MERGE_MODE;
            curBuf->didChromaMc = FALSE;

            curBuf->affineMv[0][0] = affineMv->mv[0][0];
            curBuf->affineMv[0][1] = affineMv->mv[0][1];
            curBuf->affineMv[0][2] = affineMv->mv[0][2];
            curBuf->affineMv[1][0] = affineMv->mv[1][0];
            curBuf->affineMv[1][1] = affineMv->mv[1][1];
            curBuf->affineMv[1][2] = affineMv->mv[1][2];

            // Estimate syntax rate for merge mode
            Xin266EstimateCuSynatax (
                secSet,
                context,
                FALSE,
                cu,
                curBuf,
                &curBuf->syntaxRate);

            curBuf->sadCost  = curBuf->sad << XIN_COST_FRACTION;
            curBuf->sadCost += CALC_SAD_COST(lambda, curBuf->syntaxRate);

        }

    }

    Xin266SortMdBufSad (
        secSet->interBuf,
        secSet->interBufNum);

    if (seqSet->config.cuEarlySkip)
    {
        curBuf = secSet->interBuf[0];

        Xin266ChromaCompensation (
            secSet,
            curBuf,
            cu);

        // Estimate syntax rate for skip mode
        curBuf->type = XIN_SKIP_MODE;
        Xin266EstimateCuSynatax (
            secSet,
            context,
            FALSE,
            cu,
            curBuf,
            &skipSyntax);
        curBuf->type = XIN_MERGE_MODE;

        Xin266ComputeSkipCuCost (
            secSet,
            curBuf,
            cu,
            skipSyntax);

        Xin266EarlySkipDetection (
            secSet,
            cu,
            curBuf,
            skipSyntax);
        
    }

}

void Xin266EncodeMergeCu (
    xin_sec_struct  *secSet,
    xin_fast_md_buf *fastBuf,
    xin_cu_struct   *cu)
{
    xin_ctu_struct  *ctu;
    xin_seq_struct  *seqSet;
    xin_func_struct *funcSet;
    xin_prob_model  *context;
    intptr_t        yCoefPos;
    intptr_t        uvCoefPos;
    UINT32          cbfBits;
    UINT32          rate;
    UINT64          mergeCost;
    UINT64          lambda;
    xin_full_md_buf *fullBuf;
    UINT32          mtsIdx;
    UINT32          mtsNumL;
    UINT32          mtsNumC;
    UINT32          coefBits;
    UINT64          ssd[2];
    UINT64          testCost;
    UINT64          bestCost[3];
    UINT32          bestIdx[3];
    UINT64          ssdYuv[3];
    UINT32          coefYuv[3];
    UINT32          planeIdx;
    UINT32          rowIdx, colIdx;
    UINT32          skipSyntax;

    if (fastBuf->fullBuf)
    {
        return;
    }

    Xin266FindFreeSseBuf (
        secSet->fullBuf,
        secSet->fullBufNum,
        &fullBuf);

    funcSet   = secSet->funcSet;
    seqSet    = secSet->seqSet;
    context   = secSet->cabacSet->context;
    lambda    = secSet->sseLambda[PLANE_LUMA];
    ctu       = secSet->ctu;
    yCoefPos  = cu->offX + cu->offY*fullBuf->coeffStride[0];
    uvCoefPos = (cu->offX + cu->offY*fullBuf->coeffStride[1]) >> 1;
    rowIdx    = cu->lgWidth - 2;
    colIdx    = cu->lgWidth - 2;
    mtsNumL   = 1;
    mtsNumC   = 1;

    bestCost[0] = XIN_MAX_U64_COST;
    bestCost[1] = XIN_MAX_U64_COST;
    bestCost[2] = XIN_MAX_U64_COST;
    ssdYuv[0]   = XIN_MAX_U64_COST;
    coefYuv[0]  = XIN_MAX_U32_COST;

    // Estimate syntax rate for skip mode
    fastBuf->type = XIN_SKIP_MODE;
    Xin266EstimateCuSynatax (
        secSet,
        context,
        FALSE,
        cu,
        fastBuf,
        &skipSyntax);
    fastBuf->type = XIN_MERGE_MODE;

    fastBuf->fullBuf = fullBuf;
    fullBuf->tuNum   = XIN_MAX (1, cu->width / seqSet->config.maxTrSize) * XIN_MAX (1, cu->height / seqSet->config.maxTrSize);

    for (mtsIdx = 0; mtsIdx < mtsNumL; mtsIdx++)
    {
        Xin266Transform (
            secSet,
            cu,
            fastBuf,
            mtsIdx,
            PLANE_LUMA);

        Xin266EstimateCoeff (
            secSet,
            context,
            FALSE,
            cu,
            fastBuf,
            mtsIdx,
            &coefBits,
            PLANE_LUMA);

#ifdef XIN266_COMPUTE_DIST_PIXEL_DOMAIN
        Xin266ReconTu (
            secSet,
            cu,
            fastBuf,
            mtsIdx,
            PLANE_LUMA);

        funcSet->pfXinComputeSsd[cu->lgWidth] (
            secSet->inputCu[PLANE_LUMA],
            secSet->inputYStride,
            fullBuf->reconBuf[mtsIdx][PLANE_LUMA] + yCoefPos,
            fullBuf->coeffStride[PLANE_LUMA],
            cu->width,
            cu->height,
            ssd);
#else
        funcSet->pfXinComputeSsdFd[cu->lgWidth] (
            fullBuf->tCoefBuf[mtsIdx][PLANE_LUMA] + yCoefPos,
            fullBuf->coeffStride[mtsIdx],
            fullBuf->rCoefBuf[mtsIdx][PLANE_LUMA] + yCoefPos,
            fullBuf->coeffStride[PLANE_LUMA],
            cu->lgTuWidth[PLANE_LUMA] + cu->lgTuHeight[PLANE_LUMA],
            cu->width,
            cu->height,
            ssd);
#endif

        testCost  = ssd[0] << XIN_COST_FRACTION;
        testCost += CALC_SSE_COST (lambda, coefBits);

        if (testCost < bestCost[0])
        {
            bestCost[0] = testCost;
            bestIdx[0]  = mtsIdx;
            ssdYuv[0]   = ssd[0];
            coefYuv[0]  = coefBits;
        }

        if (coefBits == 0)
        {
            break;
        }

    }

    if (fastBuf->didChromaMc == FALSE)
    {
        Xin266ChromaCompensation (
            secSet,
            fastBuf,
            cu);
    }

    for (planeIdx = PLANE_CHROMA_U; planeIdx <= PLANE_CHROMA_V; planeIdx++)
    {
        for (mtsIdx = 0; mtsIdx < mtsNumC; mtsIdx++)
        {
            Xin266Transform (
                secSet,
                cu,
                fastBuf,
                mtsIdx,
                planeIdx);

            Xin266EstimateCoeff (
                secSet,
                context,
                FALSE,
                cu,
                fastBuf,
                mtsIdx,
                &coefBits,
                planeIdx);

#ifdef XIN266_COMPUTE_DIST_PIXEL_DOMAIN
            Xin266ReconTu (
                secSet,
                cu,
                fastBuf,
                mtsIdx,
                planeIdx);

            funcSet->pfXinComputeSsd[cu->lgWidth - 1] (
                secSet->inputCu[planeIdx],
                secSet->inputUvStride,
                fullBuf->reconBuf[mtsIdx][planeIdx] + uvCoefPos,
                fullBuf->coeffStride[PLANE_CHROMA],
                cu->width>>1,
                cu->height>>1,
                ssd);
#else
            funcSet->pfXinComputeSsdFd[cu->lgWidth-1] (
                fullBuf->tCoefBuf[mtsIdx][planeIdx] + uvCoefPos,
                fullBuf->coeffStride[PLANE_CHROMA],
                fullBuf->rCoefBuf[mtsIdx][planeIdx] + uvCoefPos,
                fullBuf->coeffStride[PLANE_CHROMA],
                cu->lgTuWidth[PLANE_CHROMA] + cu->lgTuHeight[PLANE_CHROMA],
                cu->width>>1,
                cu->height>>1,
                ssd);
#endif

            testCost  = ssd[0] << XIN_COST_FRACTION;
            testCost += CALC_SSE_COST (lambda, coefBits);

            if (testCost < bestCost[planeIdx])
            {
                bestCost[planeIdx] = testCost;
                bestIdx[planeIdx]  = mtsIdx;
                ssdYuv[planeIdx]   = ssd[0];
                coefYuv[planeIdx]  = coefBits;
            }

        }
    }

    fullBuf->mtsIdx[0] = (UINT8)bestIdx[0];
    fullBuf->mtsIdx[1] = (UINT8)bestIdx[1];
    fullBuf->mtsIdx[2] = (UINT8)bestIdx[2];

    fullBuf->rootCbf = fullBuf->yuvCbf[bestIdx[0]][0] || fullBuf->yuvCbf[bestIdx[1]][1] || fullBuf->yuvCbf[bestIdx[2]][2];
    fastBuf->type    = fullBuf->rootCbf ? XIN_MERGE_MODE : XIN_SKIP_MODE;

    ssdYuv[1] = (ssdYuv[1]*secSet->chromaWeight) >> XIN_UV_WEIGHT_FRAC;
    ssdYuv[2] = (ssdYuv[2]*secSet->chromaWeight) >> XIN_UV_WEIGHT_FRAC;

    if (fastBuf->type == XIN_SKIP_MODE)
    {
        fastBuf->syntaxRate = skipSyntax;
        fastBuf->sse        = ssdYuv[0] + ssdYuv[1] + ssdYuv[2];
        fastBuf->sseCost    = (fastBuf->sse << XIN_COST_FRACTION) + CALC_SSE_COST(lambda, fastBuf->syntaxRate);
        fullBuf->sseCost    = fastBuf->sseCost;
        fullBuf->sse        = fastBuf->sse;
        fullBuf->rate       = fastBuf->syntaxRate;

        ctu->skipSse[rowIdx][colIdx]   += fastBuf->sse;
        ctu->skipCount[rowIdx][colIdx] += 1;

        return;

    }

    Xin266EstimateCuCbf (
        context,
        FALSE,
        cu,
        fastBuf,
        &cbfBits);

    rate       = coefYuv[0] + coefYuv[1] + coefYuv[2] + cbfBits + fastBuf->syntaxRate;
    mergeCost  = (ssdYuv[0] + ssdYuv[1] + ssdYuv[2]) << XIN_COST_FRACTION;
    mergeCost += CALC_SSE_COST(lambda, rate);

    Xin266ComputeSkipCuCost (
        secSet,
        fastBuf,
        cu,
        skipSyntax);

    // Change the cu inter mode to skip mode,
    // if skip cost is lower.
    if (fastBuf->sseCost < mergeCost)
    {
        fullBuf->yuvCbf[0][0] = 0;
        fullBuf->yuvCbf[0][1] = 0;
        fullBuf->yuvCbf[0][2] = 0;
        fastBuf->type         = XIN_SKIP_MODE;
        fastBuf->syntaxRate   = skipSyntax;
        fullBuf->rootCbf      = 0;
        fullBuf->sseCost      = fastBuf->sseCost;
        fullBuf->sse          = fastBuf->sse;
        fullBuf->rate         = fastBuf->syntaxRate;

    }
    else
    {
        fullBuf->sseCost = mergeCost;
        fastBuf->type    = XIN_INTER_MODE;
        fastBuf->sseCost = mergeCost;
        fullBuf->sse     = ssdYuv[0] + ssdYuv[1] + ssdYuv[2];
        fullBuf->rate    = rate;
    }

}

void Xin266EncodeSkipCu (
    xin_sec_struct  *secSet,
    xin_fast_md_buf *fastBuf,
    xin_cu_struct   *cu)
{
    xin_seq_struct  *seqSet;
    xin_full_md_buf *fullBuf;

    if (fastBuf->fullBuf)
    {
        return;
    }

    Xin266FindFreeSseBuf (
        secSet->fullBuf,
        secSet->fullBufNum,
        &fullBuf);

    seqSet = secSet->seqSet;

    fullBuf->rootCbf      = 0;
    fullBuf->mtsIdx[0]    = 0;
    fullBuf->mtsIdx[1]    = 0;
    fullBuf->mtsIdx[2]    = 0;
    fullBuf->yuvCbf[0][0] = 0;
    fullBuf->yuvCbf[0][1] = 0;
    fullBuf->yuvCbf[0][2] = 0;
    fullBuf->tuNum        = XIN_MAX (1, cu->width / seqSet->config.maxTrSize) * XIN_MAX (1, cu->height / seqSet->config.maxTrSize);
    fullBuf->sseCost      = fastBuf->sseCost;
    fullBuf->sse          = fastBuf->sse;
    fullBuf->rate         = fastBuf->syntaxRate;
    fastBuf->fullBuf      = fullBuf;

}

