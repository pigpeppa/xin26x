/***************************************************************************//**
 *
 * @file          h266_analyze_inter_cu.c
 * @brief         Analyze inter coding unit.
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
#include "h266_motion_est.h"
#include "h26x_motion_est.h"
#include "h26x_construct_bi_me_input.h"
#include "h266_lmcs.h"
#include "assert.h"
#include "h26x_thread_wrapper.h"
#include "h266_common_data.h"
#include "h266_construct_weight_input.h"
#include "h266_func_struct.h"
#include "h266_analyze_cu.h"

extern const SINT32 amvrPrecision[4];

static void Xin266SelectMvp (
    xin_sec_struct *secSet,
    xin_pu_struct  *pu,
    SINT32         imvIdx,
    PIXEL          *predBuf[2],
    UINT32         *mvpIndex,
    UINT32         *mvpSad,
    SINT32         listIdx,
    SINT32         refIdx)
{
    UINT32          validMv;
    UINT32          mvIdx;
    PIXEL           *input;
    intptr_t        inputStride;
    UINT32          sad;
    UINT32          bestSad;
    UINT32          mvpIdx;
    xin_func_struct *funcSet;

    funcSet     = secSet->funcSet;
    inputStride = secSet->inputYStride;
    input       = secSet->inputCu[PLANE_LUMA];
    bestSad     = XIN_MAX_U32_COST;
    mvpIdx      = 0;

    assert(imvIdx <= XIN_IMV_HPEL);
    assert(imvIdx >= XIN_IMV_OFF);

    Xin266FillAmvpCand (
        secSet,
        pu,
        listIdx,
        refIdx,
        imvIdx,
        secSet->amvpCand,
        &validMv);

    secSet->amvpNum = validMv;

    for (mvIdx = 0; mvIdx < validMv; mvIdx++)
    {
        Xin266LumaInterPred (
            secSet,
            pu,
            predBuf[mvIdx],
            PRED_BUF_STRIDE,
            secSet->amvpCand + mvIdx,
            refIdx,
            listIdx,
            imvIdx == XIN_IMV_HPEL);

        funcSet->pfXinComputeDist[pu->lgWidth] (
            input,
            inputStride,
            predBuf[mvIdx],
            PRED_BUF_STRIDE,
            pu->width,
            pu->height,
            &sad);

        if (sad < bestSad)
        {
            bestSad = sad;
            mvpIdx  = mvIdx;
        }

    }

    *mvpSad   = bestSad;
    *mvpIndex = mvpIdx;

}

static void Xin266FillMeCand (
    xin_sec_struct  *secSet,
    xin_me_struct   *meSet)
{
    SINT32            listIdx;
    SINT32            refIdx;
    UINT32            mvIdx;
    UINT32            candNum;
    xin_pic_struct    *picSet;
    xin_seq_struct    *seqSet;
    xin_input_picture *input;
    xin_motion_data   *meData;
    xin_mv_u          *meCand;
    xin_mv_u          curMv;
    BOOL              isSameMv;
    UINT32            validMvIdx;
    UINT32            validCandNum;
    SINT32            unitX;
    SINT32            unitY;
    SINT32            unitIdx;
    SINT32            outputPrec;

    picSet  = secSet->picSet;
    seqSet  = secSet->seqSet;
    input   = picSet->inputPicture;
    listIdx = meSet->listIdx;
    refIdx  = meSet->refIdx;
    candNum = 2;
    meCand  = meSet->meCand;
    meData  = &secSet->meData;

    assert(meSet->iMvMode>=XIN_IMV_OFF);
    assert(meSet->iMvMode<=XIN_IMV_HPEL);

    meCand[0].s32x1 = 0;
    meCand[1].s32x1 = meSet->predMv.s32x1;

    if (meSet->iMvMode)
    {
        outputPrec = amvrPrecision[meSet->iMvMode];

        Xin266ChangeMv2Me (
            &meData->tempMv[listIdx][refIdx],
            &meCand[candNum]);

        Xin266ChangeMvPrec (
            &meCand[candNum],
            XIN_ME_PREC_INTERNAL,
            outputPrec);

        Xin266ChangeMvPrec (
            &meCand[candNum],
            outputPrec,
            XIN_ME_PREC_INTERNAL);

        candNum++;

    }
    else
    {
        if (secSet->parentRefIdx[listIdx] == refIdx)
        {
            Xin266ChangeMv2Me (
                &secSet->parentMv[listIdx],
                &meCand[candNum]);

            candNum++;
        }

        if ((secSet->lftBBlock->refIdx[listIdx] == refIdx) && (secSet->lftBBlock->type != XIN_INVALID_MODE))
        {
            Xin266ChangeMv2Me (
                &(secSet->lftBBlock->mv[listIdx]),
                &meCand[candNum]);

            candNum++;
        }

        if ((secSet->topRBlock->refIdx[listIdx] == refIdx) && (secSet->topRBlock->type != XIN_INVALID_MODE))
        {
            Xin266ChangeMv2Me (
                &(secSet->topRBlock->mv[listIdx]),
                &meCand[candNum]);

            candNum++;
        }

        if ((secSet->topLftBlock->refIdx[listIdx] == refIdx) && (secSet->topLftBlock->type != XIN_INVALID_MODE))
        {
            Xin266ChangeMv2Me (
                &(secSet->topLftBlock->mv[listIdx]),
                &meCand[candNum]);

            candNum++;
        }

        if ((secSet->lftBotBlock->refIdx[listIdx] == refIdx) && (secSet->lftBotBlock->type != XIN_INVALID_MODE))
        {
            Xin266ChangeMv2Me (
                &(secSet->lftBotBlock->mv[listIdx]),
                &meCand[candNum]);

            candNum++;
        }

        if ((secSet->topRgtBlock->refIdx[listIdx] == refIdx) && (secSet->topRgtBlock->type != XIN_INVALID_MODE))
        {
            Xin266ChangeMv2Me (
                &(secSet->topRgtBlock->mv[listIdx]),
                &meCand[candNum]);

            candNum++;
        }

        if ((refIdx == 0) && (seqSet->config.lookAhead))
        {
            unitX        = secSet->cu->cuPelX;
            unitY        = secSet->cu->cuPelY;
            unitX        = unitX >> 1;
            unitY        = unitY >> 1;
            unitX        = unitX / seqSet->laUnitSize;
            unitY        = unitY / seqSet->laUnitSize;
            unitIdx      = input->laWdtInUnit*unitY + unitX;

            meCand[candNum].s16x2[0] = input->laMv[listIdx][unitIdx].s16x2[0] << 1;
            meCand[candNum].s16x2[1] = input->laMv[listIdx][unitIdx].s16x2[1] << 1;

            candNum++;
        }

    }

    validCandNum = 1;

    for (mvIdx = 1;  mvIdx < candNum; mvIdx++)
    {
        isSameMv = FALSE;

        curMv.mv.mvX = (meCand[mvIdx].mv.mvX + 2) >> 2;
        curMv.mv.mvY = (meCand[mvIdx].mv.mvY + 2) >> 2;

        for (validMvIdx = 0; validMvIdx < validCandNum; validMvIdx++)
        {
            if (meCand[validMvIdx].s32x1 == curMv.s32x1)
            {
                isSameMv = TRUE;

                break;
            }
        }

        if (!isSameMv)
        {
            meCand[validCandNum++] = curMv;
        }

    }

    meSet->candNum = validCandNum;

}

static inline UINT32 CalcRefIdxBits (
    UINT32  refIdx,
    UINT32  refNum)
{
    return refIdx + (refIdx < refNum - 1);
}

#if 0
static void Xin266SelectAffineMvp (
    xin_sec_struct *secSet,
    xin_pu_struct  *pu,
    UINT32         affineType,
    SINT32         imvIdx,
    PIXEL          *predBuf[2],
    UINT32         *mvpIndex,
    UINT32         *mvpSad,
    SINT32         listIdx,
    SINT32         refIdx)
{
    UINT32          mvIdx;
    UINT32          validMv;
    xin_func_struct *funcSet;
    intptr_t        inputStride;
    PIXEL           *input;
    UINT32          bestSad;
    UINT32          sad;
    UINT32          mvpIdx;

    funcSet     = secSet->funcSet;
    inputStride = secSet->inputYStride;
    input       = secSet->inputCu[PLANE_LUMA];
    bestSad     = XIN_MAX_U32_COST;
    mvpIdx      = 0;

    Xin266FillAffineAmvpCand (
        secSet,
        pu,
        affineType,
        listIdx,
        refIdx,
        imvIdx,
        secSet->aAmvpCand,
        &validMv);

    for (mvIdx = 0; mvIdx < validMv; mvIdx++)
    {
        Xin266LumaInterPred (
            secSet,
            pu,
            predBuf[mvIdx],
            PRED_BUF_STRIDE,
            secSet->amvpCand + mvIdx,
            refIdx,
            listIdx,
            imvIdx == XIN_IMV_HPEL);

        funcSet->pfXinComputeDist[pu->lgWidth] (
            input,
            inputStride,
            predBuf[mvIdx],
            PRED_BUF_STRIDE,
            pu->width,
            pu->height,
            &sad);

        if (sad < bestSad)
        {
            bestSad = sad;
            mvpIdx  = mvIdx;
        }

    }

    *mvpSad   = bestSad;
    *mvpIndex = mvpIdx;

}

static void Xin266InterAffineSearch (
    xin_sec_struct *secSet,
    xin_pu_struct   *pu,
    xin_fast_md_buf *fastBuf,
    UINT32          affineType)
{
    UINT32          frameIdx;
    UINT32          validRefFrame;
    UINT32          listIdx;
    UINT32          refIdx;
    UINT32          refNum;
    UINT32          predBufIdx;
    PIXEL           *predBuf[2];
    UINT32          mvpIndex;
    UINT32          mvpSad;
    xin_pic_struct  *picSet;
    xin_ref_picture *pictureWrite;

    picSet       = secSet->picSet;
    pictureWrite = picSet->pictureWrite;

    for (frameIdx = 0; frameIdx < validRefFrame; frameIdx++)
    {
        listIdx = (frameIdx < pictureWrite->numOfRefs[XIN_LIST_0]) ? XIN_LIST_0 : XIN_LIST_1;
        refIdx  = (frameIdx < pictureWrite->numOfRefs[XIN_LIST_0]) ? frameIdx : (frameIdx - pictureWrite->numOfRefs[XIN_LIST_0]);
        refNum  = pictureWrite->numOfRefs[listIdx];

        predBufIdx  = (listIdx*XIN_MAX_REF_FRAMES + refIdx)*2;
        predBuf[0]  = secSet->predBuffer + predBufIdx*PRED_BUF_SIZE;
        predBuf[1]  = secSet->predBuffer + (predBufIdx+1)*PRED_BUF_SIZE;

        Xin266SelectAffineMvp (
            secSet,
            pu,
            affineType,
            0,
            predBuf,
            &mvpIndex,
            &mvpSad,
            listIdx,
            refIdx);

        // set ME result as start search position when it is best than mvp



    }

}
#endif

static SINT32 GetInterSadThr (
    xin_sec_struct *secSet,
    xin_cu_struct  *cu)
{
    xin_ctu_struct   *lftCtu;
    xin_ctu_struct   *topCtu;
    xin_ctu_struct   *topLftCtu;
    xin_ctu_struct   *topRgtCtu;
    xin_ctu_struct   *ctu;
    UINT32           interCount;
    UINT64           interSad;
    UINT32           rowIdx, colIdx;
    SINT32           interThr;

    lftCtu      = secSet->lftCtu;
    topCtu      = secSet->topCtu;
    topLftCtu   = secSet->topLftCtu;
    topRgtCtu   = secSet->topRgtCtu;
    interCount  = 0;
    interSad    = 0;
    ctu         = secSet->ctu;
    rowIdx      = cu->lgHeight - 2;
    colIdx      = cu->lgWidth - 2;

    interSad   = ctu->interSad[rowIdx][colIdx];
    interCount = ctu->interCount[rowIdx][colIdx];

    if (lftCtu)
    {
        interCount += lftCtu->interCount[rowIdx][colIdx];
        interSad   += lftCtu->interSad[rowIdx][colIdx];
    }

    if (topCtu)
    {
        interCount  += topCtu->interCount[rowIdx][colIdx];
        interSad    += topCtu->interSad[rowIdx][colIdx];
    }

    if (topLftCtu)
    {
        interCount  += topLftCtu->interCount[rowIdx][colIdx];
        interSad    += topLftCtu->interSad[rowIdx][colIdx];
    }

    if (topRgtCtu)
    {
        interCount  += topRgtCtu->interCount[rowIdx][colIdx];
        interSad    += topRgtCtu->interSad[rowIdx][colIdx];
    }

    interThr = (SINT32)XIN_UNSIGNED_ROUND_DIV (interSad, interCount);

    return interThr;

}

static UINT32 Xin266GetBcwWeight (
    UINT32 bcwIdx,
    UINT32 listIdx)
{
    UINT32  bcwWeight;

    bcwWeight = listIdx == XIN_LIST_0 ? XIN_BCW_WGT_BASE - bcwWeights[bcwIdx] : bcwWeights[bcwIdx];

    return bcwWeight;
}

static double Xin266GetMeDistWeight (
    UINT32 bcwIdx,
    UINT32 listIdx)
{

    if (bcwIdx != XIN_BCW_DEFAULT)
    {
        return fabs((double)Xin266GetBcwWeight (bcwIdx, listIdx) / (double)XIN_BCW_WGT_BASE);
    }
    else
    {
        return 0.5;
    }
}

static void Xin266InterBiSearch (
    xin_sec_struct  *secSet,
    xin_pu_struct   *pu,
    xin_fast_md_buf *fastBuf,
    UINT64          *minCost,
    UINT32          *minSad)
{
    xin_pic_struct  *picSet;
    xin_ref_picture *pictureWrite;
    xin_ref_picture *pictureRead;
    UINT32          iterNum;
    UINT32          iterIdx;
    SINT32          refIdx;
    SINT32          refNum;
    UINT32          listIdx;
    UINT32          validRefFrame;
    xin_motion_data *meData;
    xin_me_struct   *meSet;
    UINT64          bestCost;
    UINT64          sadCost;
    UINT64          biCost;
    UINT32          biSad;
    xin_func_struct *funcSet;
    xin_seq_struct  *seqSet;
    xin_mv32_u      meOutMv;
    xin_prob_model  *context;
    UINT32          mvdBit;
    UINT32          refIdxBit;
    UINT32          mvpIdxBit;
    UINT32          predIdcBit;
    UINT32          bcwIdxBit;
    xin_cu_struct   *cu;
    UINT32          lambda;
    intptr_t        predPos;
    UINT32          bcwLoopNum;
    UINT32          bcwLoopIdx;
    UINT32          bcwIdx;

    picSet        = secSet->picSet;
    seqSet        = secSet->seqSet;
    pictureWrite  = picSet->pictureWrite;
    validRefFrame = picSet->validRefFrame;
    iterNum       = pictureWrite->mvdL1Zero ? 1 : 2;
    meData        = &fastBuf->meData;
    meSet         = secSet->meSet;
    funcSet       = secSet->funcSet;
    context       = secSet->cabacSet->context;
    cu            = secSet->cu;
    lambda        = secSet->sadLambda[PLANE_LUMA];
    predPos       = cu->offX + cu->offY*fastBuf->lumaStride;
    bcwLoopNum    = seqSet->config.enableBcw ? XIN_BCW_NUM : 1;
    bcwLoopNum    = pictureWrite->frameType == XIN_B_FRAME ? bcwLoopNum : 1;
    bcwLoopNum    = pu->width*pu->height >= XIN_BCW_SIZE_CONSTRAINT ? bcwLoopNum : 1;

    for (bcwLoopIdx = 0; bcwLoopIdx < bcwLoopNum; bcwLoopIdx++)
    {
        if (!pictureWrite->checkLDC)
        {
            if (bcwLoopIdx != 0 && bcwLoopIdx != 3 && bcwLoopIdx != 4)
            {
                continue;
            }
        }

        bcwIdx = bcwSearchOrder[bcwLoopIdx];

        if (pictureWrite->mvdL1Zero)
        {
            listIdx = XIN_LIST_0;
            refNum  = validRefFrame - pictureWrite->numOfRefs[XIN_LIST_1];
        }
        else if (bcwIdx == XIN_BCW_DEFAULT)
        {
            if (meData->bestCost[XIN_LIST_0] <= meData->bestCost[XIN_LIST_1])
            {
                listIdx = XIN_LIST_1;
                refNum  = validRefFrame - pictureWrite->numOfRefs[XIN_LIST_0];
            }
            else
            {
                listIdx = XIN_LIST_0;
                refNum  = pictureWrite->numOfRefs[XIN_LIST_0];
            }
        }
        else
        {
            if (Xin266GetBcwWeight(bcwIdx, XIN_LIST_0) < Xin266GetBcwWeight(bcwIdx, XIN_LIST_1))
            {
                listIdx = XIN_LIST_1;
                refNum  = validRefFrame - pictureWrite->numOfRefs[XIN_LIST_0];
            }
            else
            {
                listIdx = XIN_LIST_0;
                refNum  = pictureWrite->numOfRefs[XIN_LIST_0];
            }
        }

        meSet->input        = secSet->biMeInput;
        meSet->inputStride  = XIN_MAX_CTU_SIZE;
        meSet->searchWidth  = 4;
        meSet->searchHeight = 4;
        meSet->biMe         = TRUE;
        meSet->sadLambda    = secSet->sadLambda[PLANE_LUMA];
        meSet->sseLambda    = secSet->sseLambda[PLANE_LUMA];
        meSet->iMvMode      = fastBuf->imvIdx;
        bestCost            = XIN_MAX_U64_COST;

        for (iterIdx = 0; iterIdx < iterNum; iterIdx++)
        {
            if (bcwIdx == XIN_BCW_DEFAULT)
            {
                funcSet->pfXinConstructBiMeInput[pu->lgWidth] (
                    secSet->inputCu[PLANE_LUMA],
                    secSet->inputYStride,
                    meData->predBuf[1-listIdx],
                    PRED_BUF_STRIDE,
                    meSet->input,
                    meSet->inputStride,
                    pu->width,
                    pu->height);
            }
            else
            {
                funcSet->pfXinConstructWeightBiMeInput[pu->lgWidth] (
                    secSet->inputCu[PLANE_LUMA],
                    secSet->inputYStride,
                    meData->predBuf[1-listIdx],
                    PRED_BUF_STRIDE,
                    meSet->input,
                    meSet->inputStride,
                    pu->width,
                    pu->height,
                    Xin266GetBcwWeight(bcwIdx, listIdx));

            }

            for (refIdx = 0; refIdx < refNum; refIdx++)
            {
                if ((refIdx != meData->refIdx[listIdx]) && (bcwLoopIdx != 0))
                {
                    continue;
                }

                if ((fastBuf->imvIdx != XIN_IMV_OFF) && (refIdx != meData->refIdx[listIdx]))
                {
                    continue;
                }

                if ((cu->partType != XIN_CU_QUAD_PART) && (refIdx != meData->refIdx[listIdx]))
                {
                    continue;
                }
                
                Xin266ChangeMv2Me (
                    &meData->tempPredMv[listIdx][refIdx],
                    &meSet->predMv);

                Xin266ChangeMv2Me (
                    &meData->tempMv[listIdx][refIdx],
                    &meSet->bestMv);

                meSet->refIdx       = refIdx;
                meSet->listIdx      = listIdx;
                meSet->bestMv.s32x1 = meSet->bestMv.s32x1 & 0xFFFCFFFC;
                pictureRead         = picSet->pictureRead[listIdx][refIdx];
                meSet->refStride    = pictureRead->refStride[PLANE_LUMA];
                meSet->ref          = pictureRead->refBuf[PLANE_LUMA] + cu->cuPelX + meSet->refStride*cu->cuPelY;

                Xin26xMotionEstBbdgs (
                    meSet);

                if (fastBuf->imvIdx == XIN_IMV_HPEL || fastBuf->imvIdx == XIN_IMV_OFF)
                {
                    if ((seqSet->config.fastSubMe == 1) && (refIdx == 0))
                    {
                        Xin266FastSubPelMeVar (
                            meSet,
                            listIdx,
                            refIdx);
                    }
                    else if (seqSet->config.fastSubMe)
                    {
                        Xin26xFastSubPelMe (
                            meSet,
                            listIdx,
                            refIdx);
                    }
                    else
                    {
                        Xin266FullSubPelMe (
                            secSet,
                            listIdx,
                            refIdx);
                    }

                }

                meOutMv.s32x2[0] = meSet->bestMv.s16x2[0] << 2;
                meOutMv.s32x2[1] = meSet->bestMv.s16x2[1] << 2;

                Xin266LumaInterPred (
                    secSet,
                    pu,
                    meData->predBuf[listIdx],
                    PRED_BUF_STRIDE,
                    &meOutMv,
                    refIdx,
                    listIdx,
                    fastBuf->imvIdx == XIN_IMV_HPEL);

                if (bcwIdx == XIN_BCW_DEFAULT)
                {
                    funcSet->pfXinComputeAvgDist[pu->lgWidth] (
                        secSet->inputCu[PLANE_LUMA],
                        secSet->inputYStride,
                        meData->predBuf[0],
                        meData->predBuf[1],
                        PRED_BUF_STRIDE,
                        pu->width,
                        pu->height,
                        &biSad);
                }
                else
                {
                    funcSet->pfXinComputeWeightDist[pu->lgWidth] (
                        secSet->inputCu[PLANE_LUMA],
                        secSet->inputYStride,
                        meData->predBuf[0],
                        meData->predBuf[1],
                        PRED_BUF_STRIDE,
                        pu->width,
                        pu->height,
                        Xin266GetBcwWeight(bcwIdx, XIN_LIST_0),
                        Xin266GetBcwWeight(bcwIdx, XIN_LIST_1),
                        &biSad);
                }

                Xin266EstimateNormalMvd (
                    context,
                    FALSE,
                    &meOutMv,
                    &meData->tempPredMv[listIdx][refIdx],
                    fastBuf->imvIdx,
                    &mvdBit);

                Xin266EstimateRefIdx (
                    context,
                    FALSE,
                    refIdx,
                    pictureWrite->numOfRefs[listIdx],
                    &refIdxBit);

                Xin266EstimateMvpIdx (
                    context,
                    FALSE,
                    meData->tempMvpIdx[listIdx][refIdx],
                    &mvpIdxBit);

                if (seqSet->config.enableBcw)
                {
                    Xin266EstimateBcw (
                        context,
                        FALSE,
                        bcwIdx,
                        pictureWrite->checkLDC,
                        &bcwIdxBit);
                }
                else
                {
                    bcwIdxBit = 0;
                }

                sadCost  = biSad << XIN_COST_FRACTION;
                sadCost += CALC_SAD_COST (lambda, refIdxBit + mvdBit + mvpIdxBit + meData->idxBit[1-listIdx] + bcwIdxBit);
                sadCost += ((1 - listIdx) == XIN_LIST_1) && (pictureWrite->mvdL1Zero) ? 0 : CALC_SAD_COST (lambda, meData->mvdBit[1-listIdx]);

                if (sadCost < bestCost)
                {
                    bestCost                = sadCost;
                    meData->refIdx[listIdx] = (SINT8)refIdx;
                    meData->bestMv[listIdx] = meOutMv;
                    meData->predMv[listIdx] = meData->tempPredMv[listIdx][refIdx];
                    meData->mvpIdx[listIdx] = meData->tempMvpIdx[listIdx][refIdx];
                    meData->mvdBit[listIdx] = mvdBit;
                    meData->idxBit[listIdx] = refIdxBit+mvpIdxBit+bcwIdxBit;
                }

            }

            if (refNum > 1)
            {
                Xin266LumaInterPred (
                    secSet,
                    pu,
                    meData->predBuf[listIdx],
                    PRED_BUF_STRIDE,
                    &meData->bestMv[listIdx],
                    meData->refIdx[listIdx],
                    listIdx,
                    fastBuf->imvIdx == XIN_IMV_HPEL);
            }

            listIdx = 1 - listIdx;
            refNum  = (listIdx == XIN_LIST_0) ? pictureWrite->numOfRefs[XIN_LIST_0] : validRefFrame - pictureWrite->numOfRefs[XIN_LIST_0];

        }

        if (bcwIdx == XIN_BCW_DEFAULT)
        {
            funcSet->pfXinComputeAvgDist[pu->lgWidth] (
                secSet->reshapeCu[PLANE_LUMA],
                secSet->inputYStride,
                meData->predBuf[XIN_LIST_0],
                meData->predBuf[XIN_LIST_1],
                PRED_BUF_STRIDE,
                pu->width,
                pu->height,
                &biSad);
        }
        else
        {
            funcSet->pfXinComputeWeightDist[pu->lgWidth] (
                secSet->reshapeCu[PLANE_LUMA],
                secSet->inputYStride,
                meData->predBuf[XIN_LIST_0],
                meData->predBuf[XIN_LIST_1],
                PRED_BUF_STRIDE,
                pu->width,
                pu->height,
                Xin266GetBcwWeight(bcwIdx, XIN_LIST_0),
                Xin266GetBcwWeight(bcwIdx, XIN_LIST_1),
                &biSad);
        }

        Xin266EstimateInterPredIdc (
            context,
            FALSE,
            2,
            cu->lgWidth,
            cu->lgHeight,
            &predIdcBit);

        biCost  = biSad << XIN_COST_FRACTION;
        biCost += CALC_SAD_COST(lambda, meData->mvdBit[XIN_LIST_0]+meData->idxBit[XIN_LIST_0]+predIdcBit);
        biCost += (pictureWrite->mvdL1Zero) ? CALC_SAD_COST(lambda, meData->idxBit[XIN_LIST_1]) : CALC_SAD_COST(lambda, meData->mvdBit[XIN_LIST_1]+meData->idxBit[XIN_LIST_1]);

        if (biCost < *minCost)
        {
            fastBuf->predMv[XIN_LIST_0]   = meData->predMv[XIN_LIST_0];
            fastBuf->mv[XIN_LIST_0]       = meData->bestMv[XIN_LIST_0];
            fastBuf->refIdx[XIN_LIST_0]   = meData->refIdx[XIN_LIST_0];
            fastBuf->mvpIndex[XIN_LIST_0] = meData->mvpIdx[XIN_LIST_0];
            fastBuf->mergeFlag            = FALSE;

            fastBuf->predMv[XIN_LIST_1]   = meData->predMv[XIN_LIST_1];
            fastBuf->mv[XIN_LIST_1]       = meData->bestMv[XIN_LIST_1];
            fastBuf->refIdx[XIN_LIST_1]   = meData->refIdx[XIN_LIST_1];
            fastBuf->mvpIndex[XIN_LIST_1] = meData->mvpIdx[XIN_LIST_1];

            Xin266LumaMotionComp (
                secSet,
                pu->width,
                pu->height,
                fastBuf->predBuf[PLANE_LUMA] + predPos,
                fastBuf->lumaStride,
                meData->bestMv,
                meData->refIdx,
                (UINT8)bcwIdx,
                fastBuf->imvIdx == XIN_IMV_HPEL);

            *minCost = biCost;
            *minSad  = biSad;

            fastBuf->bcwIdx = bcwIdx;

        }

    }

}

static void Xin266AnalyzeInterPu (
    xin_sec_struct  *secSet,
    xin_pu_struct   *pu,
    xin_fast_md_buf *fastBuf)
{
    xin_me_struct   *meSet;
    xin_seq_struct  *seqSet;
    xin_func_struct *funcSet;
    xin_cu_struct   *cu;
    xin_cu_struct   *parentCu;
    xin_pic_struct  *picSet;
    xin_ref_picture *pictureRead;
    xin_ref_picture *pictureWrite;
    xin_motion_data *meData;
    xin_motion_data *refMeData;
    xin_lmcs_struct *lmcsSet;
    xin_prob_model  *context;
    SINT32          puPelX;
    SINT32          puPelY;
    SINT32          puOffX;
    SINT32          puOffY;
    intptr_t        predPos;
    PIXEL           *input;
    intptr_t        inputStride;
    UINT32          lambda;
    SINT32          refIdx;
    SINT32          refNum;
    UINT32          listIdx;
    UINT32          frameType;
    PIXEL           *predBuf[2];
    UINT32          predBufIdx;
    UINT32          bufIdx;
    UINT32          mvpIdx;
    UINT32          sad;
    UINT32          cost;
    UINT64          minCost;
    UINT32          minSad;
    SINT32          mvdX;
    SINT32          mvdY;
    xin_mv32_u      predMv;
    xin_mv32_u      bestMv;
    xin_mv32_u      meOutMv;
    xin_mv_u        bestMv16;
    UINT32          bestSad;
    UINT64          bestCost;
    UINT64          biCost;
    UINT32          biSad;
    UINT32          mvp0Bit, mvp1Bit;
    UINT32          predIdcBit;
    UINT32          bcwIdxBit;
    UINT32          mvpIdxBit;
    UINT32          refIdxBit;
    UINT32          mvdBit;
    UINT32          frameIdx;
    UINT32          validRefFrame;
    SINT32          mvdShift;
    UINT32          sadThr;

    picSet       = secSet->picSet;
    lmcsSet      = picSet->lmcsSet;
    seqSet       = secSet->seqSet;
    funcSet      = secSet->funcSet;
    pictureWrite = picSet->pictureWrite;
    inputStride  = secSet->inputYStride;
    frameType    = pictureWrite->frameType;
    biCost       = XIN_MAX_U64_COST;
    biSad        = XIN_MAX_U32_COST;
    meData       = &fastBuf->meData;
    refMeData    = &secSet->meData;

    meSet    = secSet->meSet;
    cu       = secSet->cu;
    parentCu = cu->parentCu;
    lambda   = secSet->sadLambda[PLANE_LUMA];
    input    = secSet->inputCu[PLANE_LUMA];
    puOffX   = cu->offX;
    puOffY   = cu->offY;
    puPelX   = cu->cuPelX;
    puPelY   = cu->cuPelY;
    context  = secSet->cabacSet->context;
    mvdShift = fastBuf->imvIdx == XIN_IMV_HPEL ? 1 : (fastBuf->imvIdx << 1);
    predPos  = puOffX + puOffY*fastBuf->lumaStride;
    sadThr   = seqSet->config.enableSkipMe ? GetInterSadThr (secSet, cu) : 0;

    meData->bestCost[0] = XIN_MAX_U64_COST;
    meData->bestCost[1] = XIN_MAX_U64_COST;
    validRefFrame       = picSet->validRefFrame;
    meSet->inputStride  = inputStride;
    meSet->input        = input;

    assert(fastBuf->imvIdx <= XIN_IMV_HPEL);
    assert(fastBuf->imvIdx >= XIN_IMV_OFF);

    for (frameIdx = 0; frameIdx < validRefFrame; frameIdx++)
    {
        listIdx = (frameIdx < pictureWrite->numOfRefs[XIN_LIST_0]) ? XIN_LIST_0 : XIN_LIST_1;
        refIdx  = (frameIdx < pictureWrite->numOfRefs[XIN_LIST_0]) ? frameIdx : (frameIdx - pictureWrite->numOfRefs[XIN_LIST_0]);
        refNum  = pictureWrite->numOfRefs[listIdx];

        if ((fastBuf->imvIdx != XIN_IMV_OFF) && (refIdx != refMeData->refIdx[listIdx]))
        {
            continue;
        }

        if ((cu->partType != XIN_CU_QUAD_PART) && (refIdx != parentCu->modeCtrl->refIdx[listIdx]) && (parentCu->modeCtrl->refIdx[listIdx] != -1))
        {
            continue;
        }

        predBufIdx  = (listIdx*XIN_MAX_REF_FRAMES + refIdx)*2;
        predBuf[0]  = secSet->predBuffer + predBufIdx*PRED_BUF_SIZE;
        predBuf[1]  = secSet->predBuffer + (predBufIdx+1)*PRED_BUF_SIZE;
        pictureRead = picSet->pictureRead[listIdx][refIdx];

        Xin266SelectMvp (
            secSet,
            pu,
            fastBuf->imvIdx,
            predBuf,
            &bufIdx,
            &sad,
            listIdx,
            refIdx);

        mvpIdx    = bufIdx;
        bestSad   = sad;
        bestCost  = sad << XIN_COST_FRACTION;
        bestCost += CALC_SAD_COST (lambda, CalcRefIdxBits(refIdx, refNum) << XIN_RATE_FRACTION);

        Xin266ChangeMv2Me (
            &secSet->amvpCand[mvpIdx],
            &bestMv16);

        meSet->searchWidth  = seqSet->config.searchRange;
        meSet->searchHeight = seqSet->config.searchRange;
        meSet->iMvMode      = fastBuf->imvIdx;
        meSet->refIdx       = refIdx;
        meSet->listIdx      = listIdx;
        meSet->refStride    = pictureRead->refStride[PLANE_LUMA];
        meSet->ref          = pictureRead->refBuf[PLANE_LUMA] + puPelX + meSet->refStride*puPelY;
        meSet->ref1Stride   = pictureRead->ref1Stride;
        meSet->ref2Stride   = pictureRead->ref2Stride;
        meSet->ref1         = pictureRead->ref1Buf + (puPelX + meSet->ref1Stride*puPelY)/2;
        meSet->ref2         = pictureRead->ref2Buf + (puPelX + meSet->ref2Stride*puPelY)/4;
        predMv.s64x1        = secSet->amvpCand[mvpIdx].s64x1;
        bestMv.s64x1        = predMv.s64x1;
        meSet->predMv.s32x1 = bestMv16.s32x1;
        meSet->bestMv.s32x1 = bestMv16.s32x1;
        meSet->biMe         = FALSE;
        meSet->width        = pu->width;
        meSet->height       = pu->height;
        meSet->iMvMode      = fastBuf->imvIdx;
        meSet->sadLambda    = secSet->sadLambda[PLANE_LUMA];
        meSet->sseLambda    = secSet->sseLambda[PLANE_LUMA];
        meSet->minMv.s32x1  = secSet->minMv.s32x1;
        meSet->maxMv.s32x1  = secSet->maxMv.s32x1;

        if ((listIdx == XIN_LIST_1) && (pictureWrite->mvdL1Zero))
        {
            meSet->bestMv = bestMv16;
        }
        else
        {
            if (bestSad > sadThr)
            {
                Xin266FillMeCand (
                    secSet,
                    meSet);

                if (seqSet->config.motionSearchMode == XIN_ME_BBDGS_SEARCH || meSet->iMvMode || pu->height != pu->width || !pictureWrite->isReferenced)
                {
                    Xin26xMotionEstBbdgs (
                        meSet);
                }
                else if (seqSet->config.motionSearchMode == XIN_ME_HIER_SEARCH)
                {
                    Xin26xMotionEstHier (
                        meSet);
                }
                else
                {
                    Xin26xMotionEstTz (
                        meSet);
                }

                if (fastBuf->imvIdx == XIN_IMV_HPEL || fastBuf->imvIdx == XIN_IMV_OFF)
                {
                    if ((seqSet->config.fastSubMe == 1) && (refIdx == 0))
                    {
                        Xin266FastSubPelMeVar (
                            meSet,
                            listIdx,
                            refIdx);
                    }
                    else if (seqSet->config.fastSubMe)
                    {
                        Xin26xFastSubPelMe (
                            meSet,
                            listIdx,
                            refIdx);
                    }
                    else
                    {
                        Xin266FullSubPelMe (
                            secSet,
                            listIdx,
                            refIdx);
                    }

                }

            }

        }

        meOutMv.s32x2[0] = meSet->bestMv.s16x2[0] << 2;
        meOutMv.s32x2[1] = meSet->bestMv.s16x2[1] << 2;

        if (meOutMv.s64x1 != bestMv.s64x1)
        {
            Xin266LumaInterPred (
                secSet,
                pu,
                predBuf[bufIdx^1],
                PRED_BUF_STRIDE,
                &meOutMv,
                refIdx,
                listIdx,
                fastBuf->imvIdx == XIN_IMV_HPEL);

            funcSet->pfXinComputeDist[pu->lgWidth] (
                meSet->input,
                meSet->inputStride,
                predBuf[bufIdx^1],
                PRED_BUF_STRIDE,
                pu->width,
                pu->height,
                &sad);

            cost  = sad << XIN_COST_FRACTION;
            mvdX  = meOutMv.s32x2[0] - predMv.s32x2[0];
            mvdY  = meOutMv.s32x2[1] - predMv.s32x2[1];
            cost += CALC_SAD_COST (lambda, (CalcRefIdxBits(refIdx, refNum) + CALC_MVD_BIT_SHIFT(mvdX>>2, mvdY>>2, mvdShift)) << XIN_RATE_FRACTION);

            if (cost < bestCost)
            {
                bestSad  = sad;
                bestCost = cost;
                bufIdx  ^= 1;

                bestMv.s64x1 = meOutMv.s64x1;
            }

            if ((secSet->amvpNum > 1) && (bestMv.s64x1 != predMv.s64x1))
            {
                Xin266EstimateNormalMvd (
                    context,
                    FALSE,
                    &bestMv,
                    secSet->amvpCand,
                    fastBuf->imvIdx,
                    &mvp0Bit);

                Xin266EstimateNormalMvd (
                    context,
                    FALSE,
                    &bestMv,
                    secSet->amvpCand + 1,
                    fastBuf->imvIdx,
                    &mvp1Bit);

                if (mvp1Bit != mvp0Bit)
                {
                    mvpIdx       = (mvp1Bit < mvp0Bit) ? 1 : 0;
                    predMv.s64x1 = secSet->amvpCand[mvpIdx].s64x1;
                }

            }

        }

        Xin266EstimateNormalMvd (
            context,
            FALSE,
            &bestMv,
            &predMv,
            fastBuf->imvIdx,
            &mvdBit);

        Xin266EstimateRefIdx (
            context,
            FALSE,
            refIdx,
            refNum,
            &refIdxBit);

        Xin266EstimateMvpIdx (
            context,
            FALSE,
            mvpIdx,
            &mvpIdxBit);

        bestCost  = bestSad << XIN_COST_FRACTION;
        bestCost += CALC_SAD_COST(lambda, refIdxBit+mvdBit+mvpIdxBit);

        meData->tempMv[listIdx][refIdx]     = bestMv;
        meData->tempPredMv[listIdx][refIdx] = predMv;
        meData->tempMvpIdx[listIdx][refIdx] = mvpIdx;

        if (bestCost < meData->bestCost[listIdx])
        {
            meData->bestMv[listIdx]   = bestMv;
            meData->predMv[listIdx]   = predMv;
            meData->mvpIdx[listIdx]   = mvpIdx;
            meData->bestCost[listIdx] = bestCost;
            meData->bestSad[listIdx]  = bestSad;
            meData->predBuf[listIdx]  = predBuf[bufIdx];
            meData->refIdx[listIdx]   = (SINT8)refIdx;
            meData->mvdBit[listIdx]   = mvdBit;
            meData->idxBit[listIdx]   = mvpIdxBit + refIdxBit;
        }
    }

    if ((frameType == XIN_B_FRAME) && (meData->bestCost[1] != XIN_MAX_U64_COST))
    {
        funcSet->pfXinComputeAvgDist[pu->lgWidth] (
            meSet->input,
            meSet->inputStride,
            meData->predBuf[XIN_LIST_0],
            meData->predBuf[XIN_LIST_1],
            PRED_BUF_STRIDE,
            pu->width,
            pu->height,
            &biSad);

        Xin266EstimateInterPredIdc (
            context,
            FALSE,
            2,
            cu->lgWidth,
            cu->lgHeight,
            &predIdcBit);

        if (seqSet->config.enableBcw)
        {
            Xin266EstimateBcw (
                context,
                FALSE,
                XIN_BCW_DEFAULT,
                pictureWrite->checkLDC,
                &bcwIdxBit);
        }
        else
        {
            bcwIdxBit = 0;
        }

        biCost  = biSad << XIN_COST_FRACTION;
        biCost += CALC_SAD_COST(lambda, meData->mvdBit[XIN_LIST_0] + meData->idxBit[XIN_LIST_0] + predIdcBit + bcwIdxBit);
        biCost += (pictureWrite->mvdL1Zero) ? CALC_SAD_COST(lambda, meData->idxBit[XIN_LIST_1]) : CALC_SAD_COST(lambda, meData->mvdBit[XIN_LIST_1] + meData->idxBit[XIN_LIST_1]);

        Xin266EstimateInterPredIdc (
            context,
            FALSE,
            XIN_LIST_0,
            cu->lgWidth,
            cu->lgHeight,
            &predIdcBit);

        meData->bestCost[XIN_LIST_0] += CALC_SAD_COST(lambda, predIdcBit);

        Xin266EstimateInterPredIdc (
            context,
            FALSE,
            XIN_LIST_1,
            cu->lgWidth,
            cu->lgHeight,
            &predIdcBit);

        meData->bestCost[XIN_LIST_1] += CALC_SAD_COST(lambda, predIdcBit);

        if ((biCost < meData->bestCost[XIN_LIST_0]) && (biCost < meData->bestCost[XIN_LIST_1]))
        {
            Xin266LumaMotionComp (
                secSet,
                pu->width,
                pu->height,
                fastBuf->predBuf[PLANE_LUMA] + predPos,
                fastBuf->lumaStride,
                meData->bestMv,
                meData->refIdx,
                XIN_BCW_DEFAULT,
                fastBuf->imvIdx == XIN_IMV_HPEL);

            minCost = biCost;
            minSad  = biSad;

            fastBuf->predMv[XIN_LIST_0]   = meData->predMv[XIN_LIST_0];
            fastBuf->mv[XIN_LIST_0]       = meData->bestMv[XIN_LIST_0];
            fastBuf->refIdx[XIN_LIST_0]   = meData->refIdx[XIN_LIST_0];
            fastBuf->mvpIndex[XIN_LIST_0] = meData->mvpIdx[XIN_LIST_0];
            fastBuf->mergeFlag            = FALSE;

            fastBuf->predMv[XIN_LIST_1]   = meData->predMv[XIN_LIST_1];
            fastBuf->mv[XIN_LIST_1]       = meData->bestMv[XIN_LIST_1];
            fastBuf->refIdx[XIN_LIST_1]   = meData->refIdx[XIN_LIST_1];
            fastBuf->mvpIndex[XIN_LIST_1] = meData->mvpIdx[XIN_LIST_1];

        }
        else if (meData->bestCost[XIN_LIST_0] <= meData->bestCost[XIN_LIST_1])
        {
            funcSet->pfXinBlockCopy[pu->lgWidth] (
                meData->predBuf[XIN_LIST_0],
                PRED_BUF_STRIDE,
                fastBuf->predBuf[PLANE_LUMA] + predPos,
                fastBuf->lumaStride,
                pu->width,
                pu->height);

            minCost = meData->bestCost[XIN_LIST_0];
            minSad  = meData->bestSad[XIN_LIST_0];

            fastBuf->predMv[XIN_LIST_0]   = meData->predMv[XIN_LIST_0];
            fastBuf->mv[XIN_LIST_0]       = meData->bestMv[XIN_LIST_0];
            fastBuf->refIdx[XIN_LIST_0]   = meData->refIdx[XIN_LIST_0];
            fastBuf->mvpIndex[XIN_LIST_0] = meData->mvpIdx[XIN_LIST_0];
            fastBuf->mergeFlag            = FALSE;

            fastBuf->predMv[XIN_LIST_1].s64x1 = 0;
            fastBuf->mv[XIN_LIST_1].s64x1     = -1;
            fastBuf->refIdx[XIN_LIST_1]       = -1;
            fastBuf->mvpIndex[XIN_LIST_1]     = 0;

        }
        else
        {
            funcSet->pfXinBlockCopy[pu->lgWidth] (
                meData->predBuf[XIN_LIST_1],
                PRED_BUF_STRIDE,
                fastBuf->predBuf[PLANE_LUMA] + predPos,
                fastBuf->lumaStride,
                pu->width,
                pu->height);

            minCost = meData->bestCost[XIN_LIST_1];
            minSad  = meData->bestSad[XIN_LIST_1];

            fastBuf->predMv[XIN_LIST_1]   = meData->predMv[XIN_LIST_1];
            fastBuf->mv[XIN_LIST_1]       = meData->bestMv[XIN_LIST_1];
            fastBuf->refIdx[XIN_LIST_1]   = meData->refIdx[XIN_LIST_1];
            fastBuf->mvpIndex[XIN_LIST_1] = meData->mvpIdx[XIN_LIST_1];
            fastBuf->mergeFlag            = FALSE;

            fastBuf->predMv[XIN_LIST_0].s64x1 =  0;
            fastBuf->mv[XIN_LIST_0].s64x1     = -1;
            fastBuf->refIdx[XIN_LIST_0]       = -1;
            fastBuf->mvpIndex[XIN_LIST_0]     =  0;

        }

    }
    else
    {
        funcSet->pfXinBlockCopy[pu->lgWidth] (
            meData->predBuf[XIN_LIST_0],
            PRED_BUF_STRIDE,
            fastBuf->predBuf[PLANE_LUMA] + predPos,
            fastBuf->lumaStride,
            pu->width,
            pu->height);

        minCost = meData->bestCost[XIN_LIST_0];
        minSad  = meData->bestSad[XIN_LIST_0];

        fastBuf->predMv[XIN_LIST_0]   = meData->predMv[XIN_LIST_0];
        fastBuf->mv[XIN_LIST_0]       = meData->bestMv[XIN_LIST_0];
        fastBuf->refIdx[XIN_LIST_0]   = meData->refIdx[XIN_LIST_0];
        fastBuf->mvpIndex[XIN_LIST_0] = meData->mvpIdx[XIN_LIST_0];
        fastBuf->mergeFlag            = FALSE;

        fastBuf->refIdx[XIN_LIST_1]   = -1;
        fastBuf->mv[XIN_LIST_1].s64x1 = -1;

    }

    if ((seqSet->config.biPredMe) && (pictureWrite->frameType == XIN_B_FRAME))
    {
        Xin266InterBiSearch (
            secSet,
            pu,
            fastBuf,
            &minCost,
            &minSad);
    }

    if (seqSet->config.enableLmcs && lmcsSet->lmcsParam.sliceReshaperEnabled)
    {
        Xin266ReshapeSignal (
            fastBuf->predBuf[PLANE_LUMA] + predPos,
            fastBuf->lumaStride,
            fastBuf->predBuf[PLANE_LUMA] + predPos,
            fastBuf->lumaStride,
            pu->width,
            pu->height,
            lmcsSet->fwdLUT);

        funcSet->pfXinComputeDist[pu->lgWidth] (
            secSet->reshapeCu[PLANE_LUMA],
            secSet->inputYStride,
            fastBuf->predBuf[PLANE_LUMA] + predPos,
            fastBuf->lumaStride,
            pu->width,
            pu->height,
            &minSad);

    }

    fastBuf->sadCost = minCost;
    fastBuf->sad     = minSad;

}

static BOOL Xin266CheckDiffMv (
    xin_fast_md_buf *curBuf,
    xin_fast_md_buf **mdBuf,
    UINT32          bufNum)
{
    UINT32  bufIdx;

    if ((!curBuf->imvIdx) && (!curBuf->mvRefine) && (!curBuf->affine))
    {
        for (bufIdx = 0; bufIdx < bufNum; bufIdx++)
        {
            if (mdBuf[bufIdx]->sadCost == XIN_MAX_U64_COST)
            {
                continue;
            }

            if (mdBuf[bufIdx] == curBuf)
            {
                continue;
            }

            if (mdBuf[bufIdx]->imvIdx || mdBuf[bufIdx]->mvRefine || mdBuf[bufIdx]->affine)
            {
                continue;
            }

            if (curBuf->refIdx[XIN_LIST_0] != mdBuf[bufIdx]->refIdx[XIN_LIST_0])
            {
                continue;
            }

            if ((curBuf->mv[XIN_LIST_0].s64x1 != mdBuf[bufIdx]->mv[XIN_LIST_0].s64x1) && (curBuf->refIdx[XIN_LIST_0] != -1))
            {
                continue;
            }

            if (curBuf->refIdx[XIN_LIST_1] != mdBuf[bufIdx]->refIdx[XIN_LIST_1])
            {
                continue;
            }

            if ((curBuf->mv[XIN_LIST_1].s64x1 != mdBuf[bufIdx]->mv[XIN_LIST_1].s64x1) && (curBuf->refIdx[XIN_LIST_1] != -1))
            {
                continue;
            }

            return FALSE;

        }

    }

    return TRUE;

}

void Xin266AnalyzeInterCu (
    xin_sec_struct  *secSet,
    xin_cu_struct   *cu,
    SINT32          amvrIdx)
{
    UINT32          lambda;
    xin_prob_model  *context;
    xin_fast_md_buf *fastBuf;
    xin_ref_picture *pictureWrite;
    xin_pic_struct  *picSet;

    picSet       = secSet->picSet;
    pictureWrite = picSet->pictureWrite;
    lambda       = secSet->sadLambda[PLANE_LUMA];
    context      = secSet->cabacSet->context;

    Xin266FindHighestSadBuf (
        secSet->interBuf,
        secSet->interBufNum,
        &fastBuf);

    fastBuf->sadCost     = 0;
    fastBuf->fullBuf     = NULL;
    fastBuf->sad         = 0;
    fastBuf->type        = XIN_INTER_MODE;
    fastBuf->mergeFlag   = FALSE;
    fastBuf->affine      = FALSE;
    fastBuf->mvRefine    = FALSE;
    fastBuf->fullBuf     = NULL;
    fastBuf->imvIdx      = amvrIdx;
    fastBuf->bcwIdx      = XIN_BCW_DEFAULT;
    fastBuf->didChromaMc = FALSE;

    assert(amvrIdx <= XIN_IMV_HPEL);
    assert(amvrIdx >= XIN_IMV_OFF);

    Xin266AnalyzeInterPu (
        secSet,
        &cu->pu,
        fastBuf);

    Xin266EstimateCuSynatax (
        secSet,
        context,
        FALSE,
        cu,
        fastBuf,
        &fastBuf->syntaxRate);

    fastBuf->sadCost  = fastBuf->sad << XIN_COST_FRACTION;
    fastBuf->sadCost += CALC_SAD_COST (lambda, fastBuf->syntaxRate);

    if (!Xin266CheckDiffMv (fastBuf, secSet->interBuf, secSet->interBufNum))
    {
        fastBuf->sadCost = XIN_MAX_U64_COST;
        fastBuf->sseCost = XIN_MAX_U64_COST;
    }

    if (amvrIdx == XIN_IMV_OFF)
    {
        memcpy (&secSet->meData, &fastBuf->meData, sizeof(xin_motion_data));
    }
    else if (!Xin266NonZeroMvd (fastBuf, pictureWrite->mvdL1Zero))
    {
        fastBuf->sadCost = XIN_MAX_U64_COST;
        fastBuf->sseCost = XIN_MAX_U64_COST;
    }

}

void Xin266EncodeInterCu (
    xin_sec_struct  *secSet,
    xin_fast_md_buf *fastBuf,
    xin_cu_struct   *cu)
{
    xin_seq_struct  *seqSet;
    xin_func_struct *funcSet;
    xin_prob_model  *context;
    intptr_t        yCoefPos;
    intptr_t        uvCoefPos;
    UINT32          mtsNumL;
    UINT32          mtsNumC;
    UINT32          mtsIdx;
    UINT32          planeIdx;
    UINT32          coefBits;
    UINT32          coefYuv[3];
    UINT32          cbfBits;
    UINT64          lambda;
    UINT64          sseCost;
    UINT64          testCost;
    UINT64          ssd[2];
    UINT64          bestCost[3];
    xin_full_md_buf *fullBuf;
    UINT32          bestIdx[3];
    UINT64          ssdYuv[3];
    UINT32          rate;

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
    yCoefPos  = cu->offX + cu->offY*fullBuf->coeffStride[0];
    uvCoefPos = (cu->offX + cu->offY*fullBuf->coeffStride[1]) >> 1;

    // Find correct transform unit size for a cu
    fullBuf->tuNum   = XIN_MAX (1, cu->width / seqSet->config.maxTrSize) * XIN_MAX (1, cu->height / seqSet->config.maxTrSize);
    fastBuf->fullBuf = fullBuf;

    mtsNumL = ((UINT32)((cu->lgTuWidth[0] <= seqSet->config.maxTrSkipLgSize) && cu->lgTuHeight[0] <= seqSet->config.maxTrSkipLgSize)) ? 2 : 1;
    mtsNumC = ((UINT32)((cu->lgTuWidth[1] <= seqSet->config.maxTrSkipLgSize) && cu->lgTuHeight[1] <= seqSet->config.maxTrSkipLgSize)) ? 2 : 1;

    bestCost[0] = XIN_MAX_U64_COST;
    bestCost[1] = XIN_MAX_U64_COST;
    bestCost[2] = XIN_MAX_U64_COST;
    coefYuv[0]  = XIN_MAX_U32_COST;
    ssdYuv[0]   = XIN_MAX_U64_COST;
    bestIdx[0]  = 0;

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
            fullBuf->coeffStride[0],
            fullBuf->rCoefBuf[mtsIdx][PLANE_LUMA] + yCoefPos,
            fullBuf->coeffStride[0],
            cu->lgTuWidth[0] + cu->lgTuHeight[0],
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
            testCost += CALC_SSE_COST(lambda, coefBits);

            if (testCost < bestCost[planeIdx])
            {
                bestCost[planeIdx] = testCost;
                bestIdx[planeIdx]  = mtsIdx;
                ssdYuv[planeIdx]   = ssd[0];
                coefYuv[planeIdx]  = coefBits;
            }

            if (coefBits == 0)
            {
                break;
            }

        }

    }

    fullBuf->mtsIdx[0] = (UINT8)bestIdx[0];
    fullBuf->mtsIdx[1] = (UINT8)bestIdx[1];
    fullBuf->mtsIdx[2] = (UINT8)bestIdx[2];

    ssdYuv[1] = (ssdYuv[1]*secSet->chromaWeight) >> XIN_UV_WEIGHT_FRAC;
    ssdYuv[2] = (ssdYuv[2]*secSet->chromaWeight) >> XIN_UV_WEIGHT_FRAC;

    fullBuf->rootCbf = fullBuf->yuvCbf[bestIdx[0]][PLANE_LUMA] || fullBuf->yuvCbf[bestIdx[1]][PLANE_CHROMA_U] || fullBuf->yuvCbf[bestIdx[2]][PLANE_CHROMA_V];

    Xin266EstimateCuCbf (
        context,
        FALSE,
        cu,
        fastBuf,
        &cbfBits);

    rate     = cbfBits + coefYuv[0] + coefYuv[1] + coefYuv[2] + fastBuf->syntaxRate;
    sseCost  = (ssdYuv[0] + ssdYuv[1] + ssdYuv[2]) << XIN_COST_FRACTION;
    sseCost += CALC_SSE_COST (lambda, rate);

    fullBuf->sseCost  = sseCost;
    fastBuf->sseCost  = fullBuf->sseCost;
    fullBuf->sse      = ssdYuv[0] + ssdYuv[1] + ssdYuv[2];
    fullBuf->rate     = rate;

}

