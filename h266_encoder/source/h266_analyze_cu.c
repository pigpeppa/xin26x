/***************************************************************************//**
 *
 * @file          h266_analyze_cu.c
 * @brief         Analyze coding unit.
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
#include "h266_coding_unit_struct.h"
#include "basic_macro.h"
#include "h266_md_buf_manipulate.h"
#include "h266_analyze_cu.h"
#include "h266_intra_prediction.h"
#include "h266_enc_init.h"
#include "assert.h"

static void Xin266FillParentMv (
    xin_sec_struct  *secSet)
{
    xin_cu_struct   *cu;
    xin_cu_struct   *parentCu;
    xin_ref_picture *pictureWrite;

    cu           = secSet->cu;
    parentCu     = cu->parentCu;
    pictureWrite = secSet->picSet->pictureWrite;

    if ((parentCu->type >= XIN_INTRA_MODE) || parentCu->pu.affine)
    {
        secSet->parentRefIdx[XIN_LIST_0] = -1;
        secSet->parentRefIdx[XIN_LIST_1] = -1;

        return;
    }

    if (pictureWrite->frameType != XIN_B_FRAME)
    {
        secSet->parentRefIdx[XIN_LIST_0]   = parentCu->pu.refIdx[XIN_LIST_0];
        secSet->parentMv[XIN_LIST_0].s64x1 = parentCu->pu.mv[XIN_LIST_0].s64x1;
    }
    else
    {
        secSet->parentRefIdx[XIN_LIST_0]   = parentCu->pu.refIdx[XIN_LIST_0];
        secSet->parentMv[XIN_LIST_0].s64x1 = parentCu->pu.mv[XIN_LIST_0].s64x1;

        secSet->parentRefIdx[XIN_LIST_1]   = parentCu->pu.refIdx[XIN_LIST_1];
        secSet->parentMv[XIN_LIST_1].s64x1 = parentCu->pu.mv[XIN_LIST_1].s64x1;
    }

}

static void Xin266FillFastBuf (
    xin_sec_struct  *secSet,
    xin_cu_struct   *cu,
    xin_fast_md_buf	*fastBuf)
{
    xin_pic_struct  *picSet;
    xin_ref_picture *pictureWrite;
    intptr_t        blockSetWidth;

    picSet        = secSet->picSet;
    pictureWrite  = picSet->pictureWrite;
    blockSetWidth = pictureWrite->blockSetWidth;

    assert(cu->type == XIN_INTRA_MODE);

    fastBuf->type          = cu->type;
    fastBuf->syntaxRate    = 0;
    fastBuf->sad           = 0;
    fastBuf->sadCost       = 0;
    fastBuf->intraLumaMode = secSet->curBlock[1+blockSetWidth].iLMode;

}

static void Xin266FilterFastBuf (
    xin_fast_md_buf **intraBuf,
    UINT32          *intraBufNum,
    xin_fast_md_buf **interBuf,
    UINT32          *interBufNum,
    UINT32          maxMdCandNum)
{
    xin_fast_md_buf *fastBuf[20];
    xin_fast_md_buf *currBuf;
    UINT32          valBufNum;
    UINT32          bufIdx;
    UINT32          interNum;
    UINT32          intraNum;
    UINT64          sadThresh;

    valBufNum = 0;

    for (bufIdx = 0; bufIdx < *intraBufNum + *interBufNum; bufIdx++)
    {
        currBuf = (bufIdx < *interBufNum) ? interBuf[bufIdx] : intraBuf[bufIdx - *interBufNum];

        if (currBuf->sadCost != XIN_MAX_U64_COST)
        {
            fastBuf[valBufNum++] = currBuf;
        }
    }

    Xin266SortMdBufSad (
        fastBuf,
        valBufNum);

    valBufNum = XIN_MIN (maxMdCandNum, valBufNum);
    interNum  = 0;
    intraNum  = 0;
    sadThresh = fastBuf[0]->sadCost * 120 / 100;

    for (bufIdx = 0; bufIdx < valBufNum; bufIdx++)
    {
        currBuf = fastBuf[bufIdx];

        if (currBuf->sadCost > sadThresh)
        {
            break;
        }

        if (currBuf->type == XIN_INTRA_MODE)
        {
            intraBuf[intraNum++] = currBuf;
        }
        else
        {
            interBuf[interNum++] = currBuf;
        }
    }

    *interBufNum = interNum;
    *intraBufNum = intraNum;

}

static BOOL Xin266CheckImvMode (
    xin_sec_struct *secSet)
{
    xin_fast_md_buf *fastBuf;

    Xin266SortMdBufSad (
        secSet->interBuf,
        secSet->interBufNum);

    fastBuf = secSet->interBuf[0];

    if (fastBuf->type != XIN_INTER_MODE)
    {
        return FALSE;
    }
    else if (((fastBuf->refIdx[XIN_LIST_0] < 0) || (fastBuf->mv[XIN_LIST_0].s64x1 == fastBuf->predMv[XIN_LIST_0].s64x1))
             && ((fastBuf->refIdx[XIN_LIST_1] < 0) || (fastBuf->mv[XIN_LIST_1].s64x1 == fastBuf->predMv[XIN_LIST_1].s64x1)))
    {
        return FALSE;
    }

    return TRUE;

}

static BOOL Xin266CheckImv4PelMode (
    xin_sec_struct *secSet)
{
    xin_fast_md_buf *fastBuf;

    Xin266SortMdBufSad (
        secSet->interBuf,
        secSet->interBufNum);

    fastBuf = secSet->interBuf[0];

    if (fastBuf->imvIdx != XIN_IMV_FPEL)
    {
        return FALSE;
    }

    return TRUE;

}

static BOOL Xin266CheckImvHPelMode (
    xin_sec_struct *secSet)
{
    xin_fast_md_buf *fastBuf;

    Xin266SortMdBufSad (
        secSet->interBuf,
        secSet->interBufNum);

    fastBuf = secSet->interBuf[0];

    if (fastBuf->imvIdx == XIN_IMV_OFF)
    {
        return TRUE;
    }

    return FALSE;

}

static void Xin266MdBufInit (
    xin_sec_struct *secSet)
{
    xin_cu_struct   *cu;
    xin_mode_struct *modeCtrl;
    UINT32          fullBufIdx;
    UINT32          fastBufIdx;

    cu       = secSet->cu;
    modeCtrl = cu->modeCtrl;

    for (fullBufIdx = 0; fullBufIdx < secSet->fullMdBufNum; fullBufIdx++)
    {
        secSet->fullBuf[fullBufIdx] = secSet->fullMdBuf[cu->qtDepth * 2 + modeCtrl->bestBufIdx] + fullBufIdx;

        secSet->fullBuf[fullBufIdx]->sseCost = XIN_MAX_U64_COST;
    }

    for (fastBufIdx = 0; fastBufIdx < secSet->intraFastBufNum; fastBufIdx++)
    {
        secSet->intraBuf[fastBufIdx] = secSet->fastMdBuf[cu->qtDepth * 2 + modeCtrl->bestBufIdx] + fastBufIdx;

        secSet->intraBuf[fastBufIdx]->sadCost = XIN_MAX_U64_COST;
        secSet->intraBuf[fastBufIdx]->type    = XIN_INVALID_MODE;
    }

    for (fastBufIdx = 0; fastBufIdx < secSet->interFastBufNum; fastBufIdx++)
    {
        secSet->interBuf[fastBufIdx] = secSet->fastMdBuf[cu->qtDepth * 2 + modeCtrl->bestBufIdx] + fastBufIdx + secSet->intraFastBufNum;

        secSet->interBuf[fastBufIdx]->sadCost = XIN_MAX_U64_COST;
        secSet->interBuf[fastBufIdx]->type    = XIN_INVALID_MODE;
    }

    for (fullBufIdx = 0; fullBufIdx < XIN_INTRA_CHROMA_CAND_NUM; fullBufIdx++)
    {
        secSet->fullUvBuf[fullBufIdx] = secSet->fullUvMdBuf + fullBufIdx;

        secSet->fullUvBuf[fullBufIdx]->sseCost = XIN_MAX_U64_COST;
    }

    for (fastBufIdx = 0; fastBufIdx < XIN_INTRA_CHROMA_CAND_NUM; fastBufIdx++)
    {
        secSet->fastUvBuf[fastBufIdx] = secSet->fastUvMdBuf + fastBufIdx;

        secSet->fastUvBuf[fastBufIdx]->sadCost = XIN_MAX_U64_COST;
    }

    secSet->fullBufNum   = secSet->fullMdBufNum;
    secSet->intraBufNum  = secSet->intraFastBufNum;
    secSet->interBufNum  = secSet->interFastBufNum;
    secSet->fullUvBufNum = secSet->fullUvMdBufNum;
    secSet->fastUvBufNum = secSet->fastUvMdBufNum;

}

void Xin266AnalyzeCu (
    xin_sec_struct  *secSet,
    xin_cu_struct   *cu)
{
    xin_pic_struct     *picSet;
    xin_seq_struct     *seqSet;
    xin_ctu_struct     *ctu;
    xin_cu_struct      *parentCu;
    xin_ref_picture    *pictureWrite;
    xin_mode_struct    *modeCtrl;
    BOOL               analyzeIntra;
    BOOL               analyzeInter;
    xin_fast_md_buf    *fastBuf;
    xin_fast_md_buf    *bestBuf;
    xin_fast_md_buf    *bestInterBuf;
    xin_fast_md_buf    *bestIntraBuf;
    UINT32             bufIdx;
    UINT64             sadThr;
    UINT32             maxMdCandNum;

    seqSet       = secSet->seqSet;
    picSet       = secSet->picSet;
    ctu          = secSet->ctu;
    parentCu     = cu->parentCu;
    modeCtrl     = cu->modeCtrl;
    pictureWrite = picSet->pictureWrite;
    sadThr       = ctu->sadThr[cu->lgHeight - 2][cu->lgWidth - 2];
    analyzeIntra = (cu->width <= seqSet->config.maxTrSize) && (cu->height <= seqSet->config.maxTrSize) && ((parentCu->modeCtrl->skipIntra != TRUE) || (!seqSet->config.skipIntraMode));
    analyzeInter = (pictureWrite->frameType < XIN_I_FRAME) && (!((cu->width == XIN_MIN_CU_SIZE) && (cu->height == XIN_MIN_CU_SIZE)));
    bestInterBuf = NULL;
    maxMdCandNum = (pictureWrite->isReferenced || !seqSet->config.fastNonRef) ? seqSet->config.maxMdCandNum : XIN_MIN (seqSet->config.maxMdCandNum, 3);
    maxMdCandNum = ((pictureWrite->temporalId != 0) && (seqSet->config.fastNonQt) && (cu->partType != XIN_CU_QUAD_PART)) ? XIN_MIN (maxMdCandNum, 3) : maxMdCandNum;
    bestBuf      = NULL;

    Xin266MdBufInit (
        secSet);

    if (((!analyzeInter) && (!analyzeIntra)) || (cu->qtDepth < ctu->minDepth))
    {
        return;
    }

    if (cu->type == XIN_INVALID_MODE)
    {
        if (analyzeInter)
        {
            Xin266AnalyzeSkipCu (
                secSet,
                cu);

            Xin266SortMdBufSad (
                secSet->interBuf,
                secSet->interBufNum);

            bestInterBuf = secSet->interBuf[0];

            if ((bestInterBuf->sadCost > sadThr) && ((bestInterBuf->type != XIN_SKIP_MODE) || (!seqSet->config.cuEarlySkip))
                    && (((cu->height <= 64) && (cu->width <= 64)) || (!seqSet->config.disableBigInter)))
            {
                Xin266FillParentMv (
                    secSet);

                Xin266AnalyzeInterCu (
                    secSet,
                    cu,
                    XIN_IMV_OFF);

                if (seqSet->config.enableAmvr && Xin266CheckImvMode(secSet))
                {
                    Xin266AnalyzeInterCu (
                        secSet,
                        cu,
                        XIN_IMV_FPEL);

                    if (Xin266CheckImv4PelMode (secSet))
                    {
                        Xin266AnalyzeInterCu (
                            secSet,
                            cu,
                            XIN_IMV_4PEL);
                    }

                    if (Xin266CheckImvHPelMode (secSet))
                    {
                        Xin266AnalyzeInterCu (
                            secSet,
                            cu,
                            XIN_IMV_HPEL);
                    }

                }

            }

            Xin266SortMdBufSad (
                secSet->interBuf,
                secSet->interBufNum);

            bestInterBuf = secSet->interBuf[0];

            if (seqSet->config.maxMdCandNum > 1)
            {
                if (bestInterBuf->type == XIN_MERGE_MODE)
                {
                    Xin266EncodeMergeCu (
                        secSet,
                        bestInterBuf,
                        cu);
                }
                else if (bestInterBuf->type == XIN_INTER_MODE)
                {
                    Xin266EncodeInterCu (
                        secSet,
                        bestInterBuf,
                        cu);
                }
                else if (bestInterBuf->type == XIN_INTRA_MODE)
                {
                    _XIN_LOGGER(XIN_LOGGER_ERROR, "Intra Cu is not here.\n");
                }
                else if (bestInterBuf->type == XIN_SKIP_MODE)
                {
                    Xin266EncodeSkipCu (
                        secSet,
                        bestInterBuf,
                        cu);
                }
                
            }

        }

        if ((analyzeIntra) && ((bestInterBuf == NULL) || ((bestInterBuf->type != XIN_SKIP_MODE) && ((bestInterBuf->fullBuf == NULL) || (bestInterBuf->fullBuf->rootCbf)))))
        {
            Xin266AnalyzeIntraCu (
                secSet,
                cu,
                bestInterBuf);

            bestIntraBuf = secSet->intraBuf[0];

            if ((bestInterBuf != NULL) && (bestInterBuf->sadCost < bestIntraBuf->sadCost/2))
            {
                modeCtrl->skipIntra = TRUE;
            }
        }

    }
    else
    {
        Xin266FillFastBuf (
            secSet,
            cu,
            secSet->intraBuf[0]);
    }

    Xin266FilterFastBuf (
        secSet->intraBuf,
        &secSet->intraBufNum,
        secSet->interBuf,
        &secSet->interBufNum,
        maxMdCandNum);

    for (bufIdx = 0; bufIdx < secSet->interBufNum; bufIdx++)
    {
        fastBuf = secSet->interBuf[bufIdx];

        if (fastBuf->type == XIN_MERGE_MODE)
        {
            Xin266EncodeMergeCu (
                secSet,
                fastBuf,
                cu);
        }
        else if (fastBuf->type == XIN_INTER_MODE)
        {
            Xin266EncodeInterCu (
                secSet,
                fastBuf,
                cu);
        }
        else if (fastBuf->type == XIN_INTRA_MODE)
        {
            _XIN_LOGGER(XIN_LOGGER_ERROR, "Intra Cu is not here.\n");
        }
        else if (fastBuf->type == XIN_SKIP_MODE)
        {
            Xin266EncodeSkipCu (
                secSet,
                fastBuf,
                cu);
        }

        if (bestBuf == NULL)
        {
            bestBuf = fastBuf;
        }
        else if (fastBuf->sseCost < bestBuf->sseCost)
        {
            bestBuf = fastBuf;
        }

        if ((bestBuf->type == XIN_SKIP_MODE) && (seqSet->config.cuEarlySkip))
        {
            break;
        }

    }

    if ((secSet->intraBufNum) && ((bestBuf == NULL) || (bestBuf->type != XIN_SKIP_MODE)))
    {
        Xin266EncodeIntraCu (
            secSet,
            &fastBuf,
            cu);

        if ((bestBuf == NULL) || (fastBuf->sseCost < bestBuf->sseCost))
        {
            bestBuf = fastBuf;
        }
    }

    cu->bestBuf = bestBuf;

    Xin266CuPostInit (
        secSet,
        cu);

}


