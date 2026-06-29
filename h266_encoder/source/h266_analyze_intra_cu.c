/***************************************************************************//**
 *
 * @file          h266_analyze_intra_cu.c
 * @brief         Analyze intra coding unit.
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
#include "string.h"
#include "assert.h"
#include "h266_func_struct.h"

#define INTRA_LUMA_STAGE_0_CANDS    7
#define INTRA_LUMA_STAGE_1_OFFSET   8

static SINT32 stage0CandList[INTRA_LUMA_STAGE_0_CANDS] =
{
    XIN_INTRA_PLANAR,
    XIN_INTRA_DC,
    XIN_INTRA_2,
    XIN_INTRA_HOR,
    XIN_INTRA_DIA,
    XIN_INTRA_VER,
    XIN_INTRA_VDIA
};

static UINT32 intraChromaModeCand[8] =
{
    XIN_PLANAR_IDX,
    XIN_VER_IDX,
    XIN_HOR_IDX,
    XIN_DC_IDX,
    XIN_LM_CHROMA_IDX,
    XIN_LM_CHROMA_L_IDX,
    XIN_LM_CHROMA_T_IDX,
    XIN_DM_CHROMA_IDX
};

static void Xin266IntraFullSearch (
    xin_sec_struct  *secSet,
    xin_pu_struct   *pu)
{
    xin_cu_struct   *cu;
    xin_func_struct *funcSet;
    xin_fast_md_buf *curBuf;

    PIXEL     *pred;
    SINT32    modeIdx;
    intptr_t  predStride;
    intptr_t  predPos;
    UINT32    modeBits;
    UINT32    lambda;
    UINT32    lgWidth;
    UINT32    lgHeight;
    UINT32    sad;
    UINT32    sadCost;
    PIXEL     *input;
    intptr_t  intputStride;
    SINT32    offsetX;
    SINT32    offsetY;

    input        = secSet->reshapeCu[PLANE_LUMA];
    intputStride = secSet->inputYStride;
    cu           = secSet->cu;
    funcSet      = secSet->funcSet;
    offsetX      = cu->offX;
    offsetY      = cu->offY;
    lgWidth      = pu->lgWidth;
    lgHeight     = pu->lgHeight;
    lambda       = secSet->sadLambda[PLANE_LUMA];

    for (modeIdx = XIN_INTRA_PLANAR; modeIdx < XIN_INTRA_NUM; modeIdx++)
    {
        Xin266FindHighestSadBuf (
            secSet->intraBuf,
            secSet->intraBufNum,
            &curBuf);

        predStride = curBuf->lumaStride;
        predPos    = offsetX + offsetY*predStride;
        pred       = curBuf->predBuf[PLANE_LUMA] + predPos;

        Xin266IntraPred (
            secSet,
            pred,
            predStride,
            modeIdx,
            0,
            lgWidth,
            lgHeight);

        funcSet->pfXinComputeDist[pu->lgWidth] (
            input,
            intputStride,
            pred,
            predStride,
            pu->width,
            pu->height,
            &sad);

        Xin266EstimateIntraPredMode (
            secSet->cabacSet->context,
            FALSE,
            modeIdx,
            pu,
            &modeBits);

        sadCost  = sad << XIN_COST_FRACTION;
        sadCost += CALC_SAD_COST(lambda, modeBits);

        curBuf->intraLumaMode = modeIdx;
        curBuf->sadCost       = sadCost;
        curBuf->sad           = sad;
        curBuf->fullBuf       = NULL;

    }

}

static void Xin266IntraFasterSearch (
    xin_sec_struct  *secSet,
    xin_pu_struct   *pu,
    UINT32          interSad)
{
    xin_cu_struct   *cu;
    xin_func_struct *funcSet;
    xin_fast_md_buf *curBuf;
    PIXEL           *pred;
    SINT32          modeIdx;
    SINT32          candIdx;
    intptr_t        predStride;
    intptr_t        predPos;
    UINT32          modeBits;
    UINT32          lambda;
    UINT32          lgWidth;
    UINT32          lgHeight;
    UINT32          sad;
    UINT32          sadCost;
    PIXEL           *input;
    intptr_t        intputStride;
    SINT32          offsetX;
    SINT32          offsetY;
    UINT32          bestMode;
    UINT32          centreMode;
    UINT64          bestCost;
    SINT32          modeOffset;
    BOOL            isRgtSide;

    input        = secSet->reshapeCu[PLANE_LUMA];
    intputStride = secSet->inputYStride;
    cu           = secSet->cu;
    funcSet      = secSet->funcSet;
    offsetX      = cu->offX;
    offsetY      = cu->offY;
    lgWidth      = pu->lgWidth;
    lgHeight     = pu->lgHeight;
    lambda       = secSet->sadLambda[PLANE_LUMA];

    for (candIdx = 0; candIdx < INTRA_LUMA_STAGE_0_CANDS; candIdx++)
    {
        Xin266FindHighestSadBuf (
            secSet->intraBuf,
            secSet->intraBufNum,
            &curBuf);

        predStride = curBuf->lumaStride;
        predPos    = offsetX + offsetY*predStride;
        pred       = curBuf->predBuf[PLANE_LUMA] + predPos;
        modeIdx    = stage0CandList[candIdx];

        Xin266IntraPred (
            secSet,
            pred,
            predStride,
            modeIdx,
            0,
            lgWidth,
            lgHeight);

        funcSet->pfXinComputeDist[pu->lgWidth] (
            input,
            intputStride,
            pred,
            predStride,
            pu->width,
            pu->height,
            &sad);

        Xin266EstimateIntraPredMode (
            secSet->cabacSet->context,
            FALSE,
            modeIdx,
            pu,
            &modeBits);

        sadCost  = sad << XIN_COST_FRACTION;
        sadCost += CALC_SAD_COST(lambda, modeBits);

        curBuf->sad           = sad;
        curBuf->sadCost       = sadCost;
        curBuf->intraLumaMode = modeIdx;
        curBuf->fullBuf       = NULL;

    }

    Xin266FindLowestSadBuf (
        secSet->intraBuf,
        secSet->intraBufNum,
        &curBuf);

    bestMode = curBuf->intraLumaMode;
    bestCost = curBuf->sadCost;

    if ((bestMode > XIN_INTRA_DC) && (curBuf->sad < 3*interSad/2))
    {
        for (modeOffset = INTRA_LUMA_STAGE_1_OFFSET; modeOffset > 0; modeOffset >>= 1)
        {
            centreMode = bestMode;

            for (isRgtSide = 0; isRgtSide < 2; isRgtSide++)
            {
                modeOffset = -modeOffset;
                modeIdx    = centreMode + modeOffset;

                if ((modeIdx < XIN_INTRA_2) || (modeIdx > XIN_INTRA_VDIA))
                {
                    continue;
                }

                Xin266FindHighestSadBuf (
                    secSet->intraBuf,
                    secSet->intraBufNum,
                    &curBuf);

                predStride = curBuf->lumaStride;
                predPos    = offsetX + offsetY*predStride;
                pred       = curBuf->predBuf[PLANE_LUMA] + predPos;

                Xin266IntraPred (
                    secSet,
                    pred,
                    predStride,
                    modeIdx,
                    0,
                    lgWidth,
                    lgHeight);

                funcSet->pfXinComputeDist[pu->lgWidth] (
                    input,
                    intputStride,
                    pred,
                    predStride,
                    pu->width,
                    pu->height,
                    &sad);

                Xin266EstimateIntraPredMode (
                    secSet->cabacSet->context,
                    FALSE,
                    modeIdx,
                    pu,
                    &modeBits);

                sadCost  = (sad << XIN_COST_FRACTION);
                sadCost += CALC_SAD_COST(lambda, modeBits);

                curBuf->sad           = sad;
                curBuf->sadCost       = sadCost;
                curBuf->intraLumaMode = modeIdx;
                curBuf->fullBuf       = NULL;

                if (sadCost < bestCost)
                {
                    bestCost = sadCost;
                    bestMode = modeIdx;
                }

            }

        }

    }

}

static void Xin266IntraFastSearch (
    xin_sec_struct  *secSet,
    xin_pu_struct   *pu,
    UINT32          interSad)
{
    xin_cu_struct   *cu;
    xin_func_struct *funcSet;
    xin_pic_struct  *picSet;
    xin_ref_picture *pictureWrite;
    xin_seq_struct  *seqSet;
    xin_fast_md_buf *curBuf;
    PIXEL           *pred;
    SINT32          modeIdx;
    SINT32          parentMode;
    UINT32          parentSad;
    SINT32          subModeIdx;
    SINT32          decStep;
    SINT32          decMask;
    intptr_t        predStride;
    intptr_t        predPos;
    UINT32          modeBits;
    UINT32          lambda;
    UINT32          lgWidth;
    UINT32          lgHeight;
    UINT32          sad;
    UINT64          sadCost;
    UINT32          bufIdx;
    PIXEL           *input;
    intptr_t        intputStride;
    SINT32          offsetX;
    SINT32          offsetY;
    BOOL            costChecked[XIN_INTRA_NUM];
    SINT32          candMode[XIN_INTRA_NUM];
    UINT32          candSad[XIN_INTRA_NUM];
    UINT32          modeCandNum;

    bufIdx       = 0;
    input        = secSet->reshapeCu[PLANE_LUMA];
    intputStride = secSet->inputYStride;
    cu           = secSet->cu;
    funcSet      = secSet->funcSet;
    seqSet       = secSet->seqSet;
    offsetX      = cu->offX;
    offsetY      = cu->offY;
    lgWidth      = pu->lgWidth;
    lgHeight     = pu->lgHeight;
    picSet       = secSet->picSet;
    pictureWrite = picSet->pictureWrite;
    lambda       = secSet->sadLambda[PLANE_LUMA];
    decStep      = 1 << (seqSet->config.intraLgDec + (!pictureWrite->isReferenced));
    decMask      = decStep - 1;

    memset (costChecked, 0, sizeof(BOOL)*XIN_INTRA_NUM);

    for (modeIdx = XIN_INTRA_PLANAR; modeIdx < XIN_INTRA_NUM; modeIdx++)
    {
        // Skip checking extended Angular modes in the first round of SATD
        if ((modeIdx > XIN_INTRA_DC) && ((modeIdx - XIN_INTRA_2) & decMask))
        {
            continue;
        }

        Xin266FindHighestSadBuf (
            secSet->intraBuf,
            secSet->intraBufNum,
            &curBuf);

        predStride = curBuf->lumaStride;
        predPos    = offsetX + offsetY*predStride;
        pred       = curBuf->predBuf[PLANE_LUMA] + predPos;

        Xin266IntraPred (
            secSet,
            pred,
            predStride,
            modeIdx,
            0,
            lgWidth,
            lgHeight);

        funcSet->pfXinComputeDist[pu->lgWidth] (
            input,
            intputStride,
            pred,
            predStride,
            pu->width,
            pu->height,
            &sad);

        Xin266EstimateIntraPredMode (
            secSet->cabacSet->context,
            FALSE,
            modeIdx,
            pu,
            &modeBits);

        sadCost  = sad << XIN_COST_FRACTION;
        sadCost += CALC_SAD_COST(lambda, modeBits);

        curBuf->intraLumaMode = modeIdx;
        curBuf->sadCost       = sadCost;
        curBuf->sad           = sad;
        curBuf->fullBuf       = NULL;
        costChecked[modeIdx]  = TRUE;

    }

    for (decStep >>= 1; decStep > 0; decStep >>= 1)
    {
        Xin266SortMdBufSad (
            secSet->intraBuf,
            secSet->intraBufNum);

        modeCandNum = 0;

        for (bufIdx = 0; bufIdx < secSet->intraBufNum; bufIdx++)
        {
            curBuf = secSet->intraBuf[bufIdx];

            if ((curBuf->sadCost != XIN_MAX_U64_COST) && (curBuf->intraLumaMode >= XIN_INTRA_2) && (curBuf->intraLumaMode <= XIN_INTRA_VDIA))
            {
                candMode[modeCandNum] = curBuf->intraLumaMode;
                candSad[modeCandNum]  = curBuf->sad;

                modeCandNum++;

                if (modeCandNum == seqSet->config.intraRdoNum)
                {
                    break;
                }
            }
        }
        
        for (bufIdx = 0; bufIdx < modeCandNum; bufIdx++)
        {
            parentMode = candMode[bufIdx];
            parentSad  = candSad[bufIdx];

            if ((parentMode >= XIN_INTRA_2) && (parentMode <= XIN_INTRA_VDIA) && (parentSad < 2*interSad))
            {
                for (subModeIdx = -decStep; subModeIdx <= decStep; subModeIdx += 2*decStep)
                {
                    modeIdx = parentMode + subModeIdx;

                    if (costChecked[modeIdx] || (modeIdx < XIN_INTRA_PLANAR) || (modeIdx > XIN_INTRA_VDIA))
                    {
                        continue;
                    }

                    Xin266FindHighestSadBuf (
                        secSet->intraBuf,
                        secSet->intraBufNum,
                        &curBuf);

                    predStride = curBuf->lumaStride;
                    predPos    = offsetX + offsetY*predStride;
                    pred       = curBuf->predBuf[PLANE_LUMA] + predPos;

                    Xin266IntraPred (
                        secSet,
                        pred,
                        predStride,
                        modeIdx,
                        0,
                        lgWidth,
                        lgHeight);

                    funcSet->pfXinComputeDist[pu->lgWidth] (
                        input,
                        intputStride,
                        pred,
                        predStride,
                        pu->width,
                        pu->height,
                        &sad);

                    Xin266EstimateIntraPredMode (
                        secSet->cabacSet->context,
                        FALSE,
                        modeIdx,
                        pu,
                        &modeBits);

                    sadCost  = sad << XIN_COST_FRACTION;
                    sadCost += CALC_SAD_COST(lambda, modeBits);

                    curBuf->intraLumaMode = modeIdx;
                    curBuf->sadCost       = sadCost;
                    curBuf->sad           = sad;
                    curBuf->fullBuf       = NULL;
                    costChecked[modeIdx]  = TRUE;

                }

            }

        }

    }

}

static void Xin266ArraySort (
    SINT8   *array,
    SINT32  len)
{
    SINT32 i, j;

    for (i = 0; i < len-1; i++)
    {
        for (j = 0; j < len-1-i; j++)
        {
            if (array[j] > array[j+1])
            {
                XIN_SWAP (SINT8, array[j], array[j+1]);
            }
        }
    }
}

static void Xin266GetIntraMPM (
    xin_sec_struct *secSet,
    SINT32         cuOffsetY,
    SINT8          *intraMPM,
    SINT8          *sortedIntraMPM,
    SINT32         *mpmNum)
{
    SINT32            numCand;
    SINT32            offset;
    SINT32            maxCandModeIdx;
    SINT32            minCandModeIdx;
    xin_block_struct  *lftBlock;
    xin_block_struct  *topBlock;
    UINT8             lftMode;
    UINT8             topMode;

    lftBlock = secSet->lftBBlock;
    topBlock = secSet->topRBlock;
    lftMode  = (lftBlock->type == XIN_INTRA_MODE) ? lftBlock->iLMode : XIN_PLANAR_IDX;
    topMode  = (!cuOffsetY) ? XIN_PLANAR_IDX : (topBlock->type == XIN_INTRA_MODE) ? topBlock->iLMode : XIN_PLANAR_IDX;
    numCand  = -1;
    offset   = (SINT32)XIN_LUMA_MODE_NUM - 6;

    intraMPM[0] = XIN_PLANAR_IDX;
    intraMPM[1] = XIN_DC_IDX;
    intraMPM[2] = XIN_VER_IDX;
    intraMPM[3] = XIN_HOR_IDX;
    intraMPM[4] = XIN_VER_IDX - 4;
    intraMPM[5] = XIN_VER_IDX + 4;

    if (lftMode == topMode)
    {
        numCand = 1;

        if (lftMode > XIN_DC_IDX)
        {
            intraMPM[0] = XIN_PLANAR_IDX;
            intraMPM[1] = lftMode;
            intraMPM[2] = ((lftMode + offset) & XIN_INTRA_MOD_MASK) + 2;
            intraMPM[3] = ((lftMode - 1) & XIN_INTRA_MOD_MASK) + 2;
            intraMPM[4] = ((lftMode + offset - 1) & XIN_INTRA_MOD_MASK) + 2;
            intraMPM[5] = ( lftMode               & XIN_INTRA_MOD_MASK) + 2;
        }
    }
    else //L!=A
    {
        numCand = 2;

        maxCandModeIdx = intraMPM[0] > intraMPM[1] ? 0 : 1;

        if ((lftMode > XIN_DC_IDX) && (topMode > XIN_DC_IDX))
        {
            intraMPM[0] = XIN_PLANAR_IDX;
            intraMPM[1] = lftMode;
            intraMPM[2] = topMode;
            maxCandModeIdx = intraMPM[1] > intraMPM[2] ? 1 : 2;
            minCandModeIdx = intraMPM[1] > intraMPM[2] ? 2 : 1;

            if (intraMPM[maxCandModeIdx] - intraMPM[minCandModeIdx] == 1)
            {
                intraMPM[3] = ((intraMPM[minCandModeIdx] + offset)     & XIN_INTRA_MOD_MASK) + 2;
                intraMPM[4] = ((intraMPM[maxCandModeIdx] - 1)          & XIN_INTRA_MOD_MASK) + 2;
                intraMPM[5] = ((intraMPM[minCandModeIdx] + offset - 1) & XIN_INTRA_MOD_MASK) + 2;
            }
            else if (intraMPM[maxCandModeIdx] - intraMPM[minCandModeIdx] >= 62)
            {
                intraMPM[3] = ((intraMPM[minCandModeIdx] - 1)      & XIN_INTRA_MOD_MASK) + 2;
                intraMPM[4] = ((intraMPM[maxCandModeIdx] + offset) & XIN_INTRA_MOD_MASK) + 2;
                intraMPM[5] = ( intraMPM[minCandModeIdx]           & XIN_INTRA_MOD_MASK) + 2;
            }
            else if (intraMPM[maxCandModeIdx] - intraMPM[minCandModeIdx] == 2)
            {
                intraMPM[3] = ((intraMPM[minCandModeIdx] - 1)      & XIN_INTRA_MOD_MASK) + 2;
                intraMPM[4] = ((intraMPM[minCandModeIdx] + offset) & XIN_INTRA_MOD_MASK) + 2;
                intraMPM[5] = ((intraMPM[maxCandModeIdx] - 1)      & XIN_INTRA_MOD_MASK) + 2;
            }
            else
            {
                intraMPM[3] = ((intraMPM[minCandModeIdx] + offset) & XIN_INTRA_MOD_MASK) + 2;
                intraMPM[4] = ((intraMPM[minCandModeIdx] - 1)      & XIN_INTRA_MOD_MASK) + 2;
                intraMPM[5] = ((intraMPM[maxCandModeIdx] + offset) & XIN_INTRA_MOD_MASK) + 2;
            }

        }
        else if (lftMode + topMode >= 2)
        {
            intraMPM[0] = XIN_PLANAR_IDX;
            intraMPM[1] = (lftMode < topMode) ? topMode : lftMode;
            maxCandModeIdx = 1;
            intraMPM[2] = ((intraMPM[maxCandModeIdx] + offset)     & XIN_INTRA_MOD_MASK) + 2;
            intraMPM[3] = ((intraMPM[maxCandModeIdx] - 1)          & XIN_INTRA_MOD_MASK) + 2;
            intraMPM[4] = ((intraMPM[maxCandModeIdx] + offset - 1) & XIN_INTRA_MOD_MASK) + 2;
            intraMPM[5] = ( intraMPM[maxCandModeIdx]               & XIN_INTRA_MOD_MASK) + 2;
        }

    }

    assert(XIN_INTRA_MPM_NUM == 6);

    sortedIntraMPM[0] = intraMPM[0];
    sortedIntraMPM[1] = intraMPM[1];
    sortedIntraMPM[2] = intraMPM[2];
    sortedIntraMPM[3] = intraMPM[3];
    sortedIntraMPM[4] = intraMPM[4];
    sortedIntraMPM[5] = intraMPM[5];

    Xin266ArraySort (
        sortedIntraMPM,
        XIN_INTRA_MPM_NUM);

    *mpmNum = numCand;

}

static void Xin266AnalyzeIntraPu (
    xin_sec_struct  *secSet,
    xin_pu_struct   *pu,
    UINT32          interSad)
{
    xin_seq_struct  *seqSet;
    xin_pic_struct  *picSet;
    xin_ref_picture *pictureWrite;
    xin_cu_struct   *cu;

    seqSet = secSet->seqSet;
    picSet = secSet->picSet;
    cu     = secSet->cu;

    pictureWrite = picSet->pictureWrite;

    Xin266GetIntraAvail (
        secSet,
        pu,
        pictureWrite->lgBlockSize,
        pictureWrite->blockSetWidth);

    Xin266ExtractIntraNB (
        secSet,
        pu,
        0);

    Xin266GetIntraMPM (
        secSet,
        cu->offY,
        pu->intraMPM,
        pu->sortedIntraMPM,
        &pu->mpmNum);

    if (seqSet->config.fastIntraMd > 1)
    {
        Xin266IntraFasterSearch (
            secSet,
            pu,
            interSad);
    }
    else if (seqSet->config.fastIntraMd)
    {
        Xin266IntraFastSearch (
            secSet,
            pu,
            interSad);
    }
    else
    {
        Xin266IntraFullSearch (
            secSet,
            pu);
    }

}

void Xin266AnalyzeIntraCu (
    xin_sec_struct  *secSet,
    xin_cu_struct   *cu,
    xin_fast_md_buf *interMdBuf)
{
    UINT32          bitNum;
    UINT32          lambda;
    UINT32          interSad;
    UINT32          bufIdx;
    xin_fast_md_buf *curBuf;
    xin_prob_model  *context;

    lambda   = secSet->sadLambda[PLANE_LUMA];
    interSad = (interMdBuf == NULL) ? XIN_MAX_U32_COST : interMdBuf->sad;
    context  = secSet->cabacSet->context;

    Xin266AnalyzeIntraPu (
        secSet,
        &cu->pu,
        interSad);

    Xin266SortMdBufSad (
        secSet->intraBuf,
        secSet->intraBufNum);

    for (bufIdx = 0; bufIdx < secSet->intraBufNum; bufIdx++)
    {
        curBuf           = secSet->intraBuf[bufIdx];
        curBuf->type     = XIN_INTRA_MODE;
        curBuf->affine   = FALSE;
        curBuf->mvRefine = FALSE;
        curBuf->bcwIdx   = XIN_BCW_DEFAULT;
        curBuf->imvIdx   = XIN_IMV_OFF;

        Xin266EstimateCuSynatax (
            secSet,
            context,
            FALSE,
            cu,
            curBuf,
            &bitNum);

        curBuf->syntaxRate = bitNum;
        curBuf->sadCost    = curBuf->sad << XIN_COST_FRACTION;
        curBuf->sadCost   += CALC_SAD_COST(lambda, curBuf->syntaxRate);

    }

    Xin266SortMdBufSad (
        secSet->intraBuf,
        secSet->intraBufNum);

}

static void Xin266AnalyzeIntraChroma (
    xin_sec_struct  *secSet,
    xin_cu_struct   *cu,
    xin_fast_md_buf *bestBuf)
{
    UINT32          modeIdx;
    intptr_t        predStride;
    intptr_t        predPos;
    PIXEL           *predU;
    PIXEL           *predV;
    PIXEL           *inputU;
    PIXEL           *inputV;
    intptr_t        inputStride;
    PIXEL           *rec;
    intptr_t        recStride;
    UINT32          lgWidth;
    UINT32          lgHeight;
    UINT32          sadU;
    UINT32          sadV;
    UINT32          predMode;
    xin_pu_struct   *pu;
    xin_pic_struct  *picSet;
    xin_seq_struct  *seqSet;
    xin_ref_picture *pictureWrite;
    xin_func_struct *funcSet;
    xin_fast_md_buf *curBuf;
    xin_full_md_buf *fullBuf;
    UINT32          sadCost;
    UINT32          modeBits;
    UINT32          lambda;
    UINT32          lumaMode;
    SINT32          mtsIdx;

    funcSet      = secSet->funcSet;
    seqSet       = secSet->seqSet;
    pu           = &cu->pu;
    lambda       = secSet->sadLambda[PLANE_CHROMA];
    lgWidth      = cu->lgWidth - 1;
    lgHeight     = cu->lgHeight - 1;
    inputU       = secSet->inputCu[1];
    inputV       = secSet->inputCu[2];
    inputStride  = secSet->inputUvStride;
    picSet       = secSet->picSet;
    pictureWrite = picSet->pictureWrite;
    rec          = (PIXEL *)(secSet->tempBuffer + 1000);
    recStride    = XIN_MAX_TU_SIZE*2 + 1;
    lumaMode     = bestBuf->intraLumaMode;
    fullBuf      = bestBuf->fullBuf;
    mtsIdx       = fullBuf->mtsIdx[0];

    if (!(cu->treeMask & XIN_CU_TREE_L_MASK))
    {
        Xin266GetIntraAvail (
            secSet,
            pu,
            pictureWrite->lgBlockSize,
            pictureWrite->blockSetWidth);
    }

    Xin266ExtractIntraNBChroma (
        secSet,
        pu);

    if ((seqSet->config.enableCclm) && (cu->treeMask != XIN_CU_TREE_C_MASK))
    {
        funcSet->pfXinBlockCopy[cu->lgWidth] (
            fullBuf->reconBuf[mtsIdx][PLANE_LUMA] + cu->offX + cu->offY*fullBuf->coeffStride[0],
            fullBuf->coeffStride[0],
            secSet->reconCu[PLANE_LUMA],
            secSet->reconYStride,
            cu->width,
            cu->height);
    }

    for (modeIdx = 0; modeIdx < XIN_INTRA_CHROMA_CAND_NUM; modeIdx++)
    {
        predMode = intraChromaModeCand[modeIdx];

        if ((predMode >= XIN_LM_CHROMA_IDX) && (predMode <= XIN_LM_CHROMA_T_IDX) && (seqSet->config.enableCclm == FALSE))
        {
            continue;
        }

        if (lumaMode == predMode)
        {
            if (seqSet->config.fastIntraMd > 1)
            {
                continue;
            }
            
            predMode = XIN_VDIA_IDX;
        }

        Xin266FindHighestSadBuf (
            secSet->fastUvBuf,
            secSet->fastUvBufNum,
            &curBuf);

        if ((predMode >= XIN_LM_CHROMA_IDX) && (predMode <= XIN_LM_CHROMA_T_IDX))
        {
            Xin266LoadLMLumaRec (
                secSet,
                rec,
                recStride,
                predMode);
        }

        predStride = curBuf->chromaStride;
        predPos    = (cu->offX + cu->offY*predStride)>>1;
        predU      = curBuf->predBuf[PLANE_CHROMA_U] + predPos;
        predV      = curBuf->predBuf[PLANE_CHROMA_V] + predPos;

        Xin266IntraPredChroma (
            secSet,
            predU,
            predStride,
            PLANE_CHROMA_U,
            predMode == XIN_DM_CHROMA_IDX ? lumaMode : predMode,
            lgWidth,
            lgHeight);

        funcSet->pfXinComputeDist[cu->lgWidth-1] (
            inputU,
            inputStride,
            predU,
            predStride,
            cu->width>>1,
            cu->height>>1,
            &sadU);

        Xin266IntraPredChroma (
            secSet,
            predV,
            predStride,
            PLANE_CHROMA_V,
            predMode == XIN_DM_CHROMA_IDX ? lumaMode : predMode,
            lgWidth,
            lgHeight);

        funcSet->pfXinComputeDist[cu->lgWidth-1] (
            inputV,
            inputStride,
            predV,
            predStride,
            cu->width>>1,
            cu->height>>1,
            &sadV);

        Xin266EstimateChromaIntraPredMode (
            secSet->cabacSet->context,
            FALSE,
            predMode,
            seqSet->config.enableCclm,
            &modeBits);

        sadCost  = (sadU + sadV) << XIN_COST_FRACTION;
        sadCost += CALC_SAD_COST(lambda, modeBits);

        curBuf->intraChromaMode = predMode;
        curBuf->intraLumaMode   = lumaMode;
        curBuf->sadCost         = sadCost;
        curBuf->fullBuf         = NULL;

    }

    Xin266SortMdBufSad (
        secSet->fastUvBuf,
        secSet->fastUvBufNum);

}

void Xin266EncodeIntraCuLuma (
    xin_sec_struct  *secSet,
    xin_fast_md_buf *fastBuf,
    xin_cu_struct   *cu)
{
    xin_seq_struct  *seqSet;
    xin_func_struct *funcSet;
    xin_full_md_buf *fullBuf;
    UINT32          coefBits;
    UINT32          coefLuma;
    UINT64          ssd[2];
    UINT64          ssdYuv;
    UINT64          testCost;
    UINT64          sseCost;
    xin_prob_model  *context;
    UINT64          lambda;
    intptr_t        yCoefPos;
    UINT32          mtsIdx;
    UINT32          mtsNumL;
    UINT64          bestCost;
    UINT32          bestIdx;

    fullBuf   = fastBuf->fullBuf;
    funcSet   = secSet->funcSet;
    seqSet    = secSet->seqSet;
    lambda    = secSet->sseLambda[PLANE_LUMA];
    context   = secSet->cabacSet->context;
    yCoefPos  = cu->offX + cu->offY*fullBuf->coeffStride[0];

    fullBuf->tuNum   = 1;
    fullBuf->sse     = 0;
    fullBuf->sseCost = 0;
    fullBuf->rootCbf = 0;

    sseCost  = 0;
    mtsNumL  = ((UINT32)((cu->lgTuWidth[0] <= seqSet->config.maxTrSkipLgSize) && cu->lgTuHeight[0] <= seqSet->config.maxTrSkipLgSize)) ? 2 : 1;
    bestCost = XIN_MAX_U64_COST;
    bestIdx  = 0;
    ssdYuv   = 0;
    coefLuma = 0;

    if (fastBuf->intraLumaMode >= XIN_INTRA_2)
    {
        Xin266ApplyAngPDPC (
            secSet,
            fastBuf,
            cu);
    }

    if (cu->treeMask & XIN_CU_TREE_L_MASK)
    {
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

#ifndef XIN266_COMPUTE_DIST_PIXEL_DOMAIN
            if (seqSet->config.enableCclm)
#endif
            {
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

            }
#ifndef XIN266_COMPUTE_DIST_PIXEL_DOMAIN
            else
            {
                funcSet->pfXinComputeSsdFd[cu->lgWidth] (
                    fullBuf->tCoefBuf[mtsIdx][PLANE_LUMA] + yCoefPos,
                    fullBuf->coeffStride[PLANE_LUMA],
                    fullBuf->rCoefBuf[mtsIdx][PLANE_LUMA] + yCoefPos,
                    fullBuf->coeffStride[PLANE_LUMA],
                    cu->lgTuWidth[PLANE_LUMA] + cu->lgTuHeight[PLANE_LUMA],
                    cu->width,
                    cu->height,
                    ssd);
            }
#endif
            testCost  = ssd[0] << XIN_COST_FRACTION;
            testCost += CALC_SSE_COST (lambda, coefBits);

            if (testCost < bestCost)
            {
                bestCost = testCost;
                bestIdx  = mtsIdx;
                ssdYuv   = ssd[0];
                coefLuma = coefBits;
            }

            if (coefBits == 0)
            {
                break;
            }

        }

        fullBuf->rootCbf |= fullBuf->yuvCbf[bestIdx][0];

        sseCost += ssdYuv << XIN_COST_FRACTION;
        sseCost += CALC_SSE_COST (lambda, coefLuma + fastBuf->syntaxRate);

        fullBuf->mtsIdx[0] = (UINT8)bestIdx;
        fullBuf->sseCost   = sseCost;
        fullBuf->sse       = ssdYuv;
        fullBuf->rate      = coefLuma + fastBuf->syntaxRate;
        fastBuf->sseCost   = sseCost;

    }

}

void Xin266EncodeIntraCuChroma (
    xin_sec_struct  *secSet,
    xin_fast_md_buf *fastBuf,
    xin_cu_struct   *cu)
{
    xin_seq_struct  *seqSet;
    xin_func_struct *funcSet;
    xin_full_md_buf *fullBuf;
    UINT32          coefBits;
    UINT32          coefYuv[3];
    UINT64          ssd[2];
    UINT64          ssdYuv[3];
    UINT64          testCost;
    UINT64          sseCost;
    xin_prob_model  *context;
    UINT64          lambda;
    intptr_t        uvCoefPos;
    UINT32          planeIdx;
    UINT32          mtsIdx;
    UINT32          mtsNumC;
    UINT64          bestCost[3];
    UINT32          bestIdx[3];
    UINT32          intraChromaBits;

    fullBuf   = fastBuf->fullBuf;
    funcSet   = secSet->funcSet;
    seqSet    = secSet->seqSet;
    lambda    = secSet->sseLambda[PLANE_LUMA];
    uvCoefPos = (cu->offX + cu->offY*fullBuf->coeffStride[1])>>1;
    context   = secSet->cabacSet->context;
    mtsNumC   = ((UINT32)((cu->lgTuWidth[1] <= seqSet->config.maxTrSkipLgSize) && cu->lgTuHeight[1] <= seqSet->config.maxTrSkipLgSize)) ? 2 : 1;

    bestCost[1] = XIN_MAX_U64_COST;
    bestCost[2] = XIN_MAX_U64_COST;

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
                fullBuf->coeffStride[1],
                fullBuf->rCoefBuf[mtsIdx][planeIdx] + uvCoefPos,
                fullBuf->coeffStride[1],
                cu->lgTuWidth[1] + cu->lgTuHeight[1],
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

            if (coefBits == 0)
            {
                break;
            }

        }

    }

    fullBuf->rootCbf |= fullBuf->yuvCbf[bestIdx[1]][1] || fullBuf->yuvCbf[bestIdx[2]][2];

    ssdYuv[1] = (ssdYuv[1]*secSet->chromaWeight) >> XIN_UV_WEIGHT_FRAC;
    ssdYuv[2] = (ssdYuv[2]*secSet->chromaWeight) >> XIN_UV_WEIGHT_FRAC;
    sseCost   = (ssdYuv[1] + ssdYuv[2]) << XIN_COST_FRACTION;
    sseCost  += CALC_SSE_COST (lambda, coefYuv[1] + coefYuv[2]);

    fullBuf->mtsIdx[1] = (UINT8)bestIdx[1];
    fullBuf->mtsIdx[2] = (UINT8)bestIdx[2];

    Xin266EstimateChromaIntraPredMode (
        context,
        FALSE,
        fastBuf->intraChromaMode,
        seqSet->config.enableCclm,
        &intraChromaBits);

    fullBuf->sseCost += sseCost;
    fullBuf->sseCost += CALC_SSE_COST (lambda, intraChromaBits);
    fullBuf->sse     += ssdYuv[1] + ssdYuv[2];
    fullBuf->rate    += intraChromaBits + coefYuv[1] + coefYuv[2];
    fastBuf->sseCost  = fullBuf->sseCost;

}

void Xin266UvMdFastBufCopy (
    xin_sec_struct  *secSet,
    xin_cu_struct   *cu,
    xin_fast_md_buf *srcUvBuf,
    xin_fast_md_buf *dstUvBuf)
{
    xin_func_struct *funcSet;
    intptr_t        srcStride;
    intptr_t        dstStride;

    funcSet   = secSet->funcSet;
    srcStride = srcUvBuf->chromaStride;
    dstStride = dstUvBuf->chromaStride;

    dstUvBuf->intraChromaMode = srcUvBuf->intraChromaMode;

    funcSet->pfXinBlockCopy[cu->lgWidth - 1] (
        srcUvBuf->predBuf[PLANE_CHROMA_U] + ((cu->offX + cu->offY*srcStride)>>1),
        srcStride,
        dstUvBuf->predBuf[PLANE_CHROMA_U] + ((cu->offX + cu->offY*dstStride)>>1),
        dstStride,
        cu->width>>1,
        cu->height>>1);

    funcSet->pfXinBlockCopy[cu->lgWidth - 1] (
        srcUvBuf->predBuf[PLANE_CHROMA_V] + ((cu->offX + cu->offY*srcStride)>>1),
        srcStride,
        dstUvBuf->predBuf[PLANE_CHROMA_V] + ((cu->offX + cu->offY*dstStride)>>1),
        dstStride,
        cu->width>>1,
        cu->height>>1);

}

void Xin266EncodeIntraCu (
    xin_sec_struct  *secSet,
    xin_fast_md_buf **outputBuf,
    xin_cu_struct   *cu)
{
    xin_seq_struct  *seqSet;
    xin_prob_model  *context;
    xin_fast_md_buf *bestBuf;
    xin_fast_md_buf *bestUvBuf;
    xin_full_md_buf *fullBuf;
    xin_fast_md_buf *fastBuf;
    UINT32          bufIdx;
    UINT32          uvRdoNum;
    UINT64          lambda;
    UINT32          cbfBits;

    seqSet    = secSet->seqSet;
    bestBuf   = NULL;
    bestUvBuf = NULL;
    uvRdoNum  = XIN_MIN (XIN_INTRA_CHROMA_CAND_NUM - 1, seqSet->config.maxMdCandNum);
    lambda    = secSet->sseLambda[PLANE_LUMA];

    for (bufIdx = 0; bufIdx < secSet->intraBufNum; bufIdx++)
    {
        fastBuf = secSet->intraBuf[bufIdx];

        Xin266FindFreeSseBuf (
            secSet->fullBuf,
            secSet->fullBufNum,
            &fullBuf);

        fastBuf->fullBuf = fullBuf;

        Xin266EncodeIntraCuLuma (
            secSet,
            fastBuf,
            cu);

        if ((bestBuf == NULL) || (fastBuf->sseCost < bestBuf->sseCost))
        {
            bestBuf = fastBuf;
        }

    }

    if (cu->treeMask & XIN_CU_TREE_C_MASK)
    {
        Xin266AnalyzeIntraChroma (
            secSet,
            cu,
            bestBuf);

        if (uvRdoNum > 1)
        {
            for (bufIdx = 0; bufIdx < uvRdoNum; bufIdx++)
            {
                fastBuf = secSet->fastUvBuf[bufIdx];

                Xin266FindFreeSseBuf (
                    secSet->fullUvBuf,
                    secSet->fullUvBufNum,
                    &fullBuf);

                fastBuf->fullBuf  = fullBuf;
                fullBuf->tuNum    = 1;
                fullBuf->sseCost  = 0;
                fullBuf->sse      = 0;
                fullBuf->rootCbf  = 0;

                Xin266EncodeIntraCuChroma (
                    secSet,
                    fastBuf,
                    cu);

                if (bestUvBuf == NULL)
                {
                    bestUvBuf = fastBuf;
                }
                else if (fastBuf->sseCost < bestUvBuf->sseCost)
                {
                    bestUvBuf = fastBuf;
                }

            }

        }
        else
        {
            bestUvBuf = secSet->fastUvBuf[0];

        }

        Xin266UvMdFastBufCopy (
            secSet,
            cu,
            bestUvBuf,
            bestBuf);

        Xin266EncodeIntraCuChroma (
            secSet,
            bestBuf,
            cu);

    }

    fullBuf = bestBuf->fullBuf;
    context = secSet->cabacSet->context;

    Xin266EstimateCuCbf (
        context,
        FALSE,
        cu,
        bestBuf,
        &cbfBits);

    fullBuf->sseCost += CALC_SSE_COST (lambda, cbfBits);
    fullBuf->rate    += cbfBits;
    bestBuf->sseCost  = fullBuf->sseCost;

    *outputBuf = bestBuf;

}
