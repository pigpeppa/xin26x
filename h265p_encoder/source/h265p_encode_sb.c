/***************************************************************************//**
 *
 * @file          h265p_encode_sb.c
 * @brief         Encode, analyze or reconstruct super block.
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
#include "assert.h"
#include "string.h"
#include "h26x_definition.h"
#include "h265p_definition.h"
#include "h265p_constant.h"
#include "xin_video_common.h"
#include "basic_macro.h"
#include "video_macro.h"
#include "h265p_trans_context.h"
#include "h265p_bit_stream_struct.h"
#include "h265p_cabac_struct.h"
#include "h265p_cabac_context.h"
#include "h265p_trans_unit_struct.h"
#include "h265p_picture_struct.h"
#include "h265p_macro_block_struct.h"
#include "h265p_section_struct.h"
#include "xin26x_logger.h"
#include "h26x_block_utility.h"
#include "h265p_analyze_mb.h"
#include "h265p_enc_init.h"
#include "h265p_entropy_manipulate.h"
#include "h265p_trans_recon.h"
#include "h265p_common_data.h"

void Xin265pDepthDecsion (
    xin_sec_struct *secSet,
    xin_mb_struct  *parentMb,
    UINT8          splitType,
    xin_mb_struct  **childMb)
{
    UINT32  unplitBits;
    UINT32  splitBits;
    UINT64  unsplitCost;
    UINT64  splitCost;
    UINT64  lambda;
    UINT32  partCtx;

    lambda  = secSet->sseLambda[PLANE_LUMA];
    partCtx = parentMb->partitionCtx;

    if (parentMb->bestBuf != NULL)
    {
        if (splitType == XIN_PARTITION_SPLIT)
        {
            unplitBits   = secSet->cabacEst.partitionRate[partCtx][XIN_PARTITION_NONE];
            unsplitCost  = CALC_RD_COST (lambda, unplitBits);
            unsplitCost += parentMb->sseCost;

            splitBits    = secSet->cabacEst.partitionRate[partCtx][XIN_PARTITION_SPLIT];
            splitCost    = CALC_RD_COST (lambda, splitBits);
            splitCost   += childMb[0]->sseCost;
            splitCost   += childMb[1]->sseCost;
            splitCost   += childMb[2]->sseCost;
            splitCost   += childMb[3]->sseCost;

            parentMb->splitType = (splitCost < unsplitCost) ? splitType : XIN_PARTITION_NONE;
            parentMb->sseCost   = (splitCost < unsplitCost) ? splitCost : unsplitCost;

            if (parentMb->splitType == XIN_PARTITION_SPLIT)
            {
                parentMb->childMb[0] = childMb[0];
                parentMb->childMb[1] = childMb[1];
                parentMb->childMb[2] = childMb[2];
                parentMb->childMb[3] = childMb[3];
            }

        }
        else if ((splitType == XIN_PARTITION_HORZ) || (splitType == XIN_PARTITION_VERT))
        {
            if (splitType == XIN_PARTITION_HORZ)
            {
                splitBits = secSet->cabacEst.partitionRate[partCtx][XIN_PARTITION_HORZ];
            }
            else
            {
                splitBits = secSet->cabacEst.partitionRate[partCtx][XIN_PARTITION_VERT];
            }

            splitCost  = CALC_RD_COST (lambda, splitBits);
            splitCost += childMb[0]->sseCost;
            splitCost += childMb[1]->sseCost;

            parentMb->splitType = (splitCost < parentMb->sseCost) ? splitType : parentMb->splitType;
            parentMb->sseCost   = (splitCost < parentMb->sseCost) ? splitCost : parentMb->sseCost;

            if (parentMb->splitType == splitType)
            {
                parentMb->childMb[0] = childMb[0];
                parentMb->childMb[1] = childMb[1];
            }

        }
        else
        {
            _XIN_LOGGER (XIN_LOGGER_ERROR, "Invalid macroblock partition type.\n");
        }

    }
    else
    {
        if (splitType == XIN_PARTITION_SPLIT)
        {
            parentMb->splitType = XIN_PARTITION_SPLIT;
            parentMb->childMb[0] = childMb[0];
            parentMb->childMb[1] = childMb[1];
            parentMb->childMb[2] = childMb[2];
            parentMb->childMb[3] = childMb[3];
        }
    }

}

static void Xin265pPartitionBackup (
    xin_sec_struct *secSet,
    xin_mb_struct  *mb)
{
    xin_pic_struct  *picSet;
    xin_ref_picture *pictureWrite;
    xin_func_struct *funcSet;
    PIXEL           *srcRecon[PLANE_NUM];
    PIXEL           *dstRecon[PLANE_NUM];
    xin_mi_struct   *miBuf;
    xin_mi_struct   *miSrc;
    xin_mi_struct   *miDst;
    intptr_t        miStride;
    intptr_t        miIdx;
    SINT32          widthInMi;
    SINT32          heightInMi;
    SINT32          rowIdx;

    picSet       = secSet->picSet;
    pictureWrite = picSet->pictureWrite;
    funcSet      = secSet->funcSet;
    miBuf        = pictureWrite->miBuf;
    miStride     = pictureWrite->miStride;
    widthInMi    = mb->width[PLANE_LUMA] >> XIN_LOG_MI_SIZE;
    heightInMi   = mb->height[PLANE_LUMA] >> XIN_LOG_MI_SIZE;
    srcRecon[0]  = secSet->reconSb[PLANE_LUMA] + mb->offX[PLANE_LUMA] + mb->offY[PLANE_LUMA] * secSet->reconStride[PLANE_LUMA];
    srcRecon[1]  = secSet->reconSb[PLANE_CHROMA_U] + mb->offX[PLANE_CHROMA] + mb->offY[PLANE_CHROMA] * secSet->reconStride[PLANE_CHROMA];
    srcRecon[2]  = secSet->reconSb[PLANE_CHROMA_V] + mb->offX[PLANE_CHROMA] + mb->offY[PLANE_CHROMA] * secSet->reconStride[PLANE_CHROMA];
    dstRecon[0]  = secSet->reconData[PLANE_LUMA];
    dstRecon[1]  = secSet->reconData[PLANE_CHROMA_U];
    dstRecon[2]  = secSet->reconData[PLANE_CHROMA_V];

    funcSet->pfXinBlockCopy[mb->lgWidth[PLANE_LUMA]] (
        srcRecon[PLANE_LUMA],
        secSet->reconStride[PLANE_LUMA],
        dstRecon[PLANE_LUMA],
        secSet->reconDataStride[PLANE_LUMA],
        mb->width[PLANE_LUMA],
        mb->height[PLANE_LUMA]);

    funcSet->pfXinBlockCopy[mb->lgWidth[PLANE_CHROMA]] (
        srcRecon[PLANE_CHROMA_U],
        secSet->reconStride[PLANE_CHROMA],
        dstRecon[PLANE_CHROMA_U],
        secSet->reconDataStride[PLANE_CHROMA],
        mb->width[PLANE_CHROMA],
        mb->height[PLANE_CHROMA]);

    funcSet->pfXinBlockCopy[mb->lgWidth[PLANE_CHROMA]] (
        srcRecon[PLANE_CHROMA_V],
        secSet->reconStride[PLANE_CHROMA],
        dstRecon[PLANE_CHROMA_V],
        secSet->reconDataStride[PLANE_CHROMA],
        mb->width[PLANE_CHROMA],
        mb->height[PLANE_CHROMA]);

    PEL_XY_TO_BLOCK_INDEX (mb->mbPelX[PLANE_LUMA], mb->mbPelY[PLANE_LUMA], miIdx, miStride, XIN_LOG_MI_SIZE);

    miSrc = miBuf + miIdx;
    miDst = secSet->miData;

    for (rowIdx = 0; rowIdx < heightInMi; rowIdx++)
    {
        memcpy (miDst, miSrc, sizeof(xin_mi_struct)*widthInMi);
        memset (miSrc, 0xFF,  sizeof(xin_mi_struct)*widthInMi);

        miDst += secSet->miDataStride;
        miSrc += miStride;
    }

}

static void Xin265pPartitionRestore (
    xin_sec_struct *secSet,
    xin_mb_struct  *mb,
    UINT32         splitType)
{
    xin_pic_struct  *picSet;
    xin_ref_picture *pictureWrite;
    xin_func_struct *funcSet;
    PIXEL           *srcRecon[PLANE_NUM];
    PIXEL           *dstRecon[PLANE_NUM];
    xin_mi_struct   *miBuf;
    xin_mi_struct   *miSrc;
    xin_mi_struct   *miDst;
    intptr_t        miStride;
    intptr_t        miIdx;
    SINT32          widthInMi;
    SINT32          heightInMi;
    SINT32          offXInMi;
    SINT32          offYInMi;
    SINT32          rowIdx;
    UINT32          mbDepth;

    funcSet      = secSet->funcSet;
    picSet       = secSet->picSet;
    pictureWrite = picSet->pictureWrite;
    mbDepth      = mb->depth;

    if (mb->splitType != splitType)
    {
        dstRecon[PLANE_LUMA]     = secSet->reconSb[PLANE_LUMA] + mb->offX[PLANE_LUMA] + mb->offY[PLANE_LUMA] * secSet->reconStride[PLANE_LUMA];
        dstRecon[PLANE_CHROMA_U] = secSet->reconSb[PLANE_CHROMA_U] + mb->offX[PLANE_CHROMA] + mb->offY[PLANE_CHROMA] * secSet->reconStride[PLANE_CHROMA];
        dstRecon[PLANE_CHROMA_V] = secSet->reconSb[PLANE_CHROMA_V] + mb->offX[PLANE_CHROMA] + mb->offY[PLANE_CHROMA] * secSet->reconStride[PLANE_CHROMA];
        srcRecon[PLANE_LUMA]     = secSet->reconData[PLANE_LUMA];
        srcRecon[PLANE_CHROMA_U] = secSet->reconData[PLANE_CHROMA_U];
        srcRecon[PLANE_CHROMA_V] = secSet->reconData[PLANE_CHROMA_V];

        funcSet->pfXinBlockCopy[mb->lgWidth[PLANE_LUMA]] (
            srcRecon[PLANE_LUMA],
            secSet->reconDataStride[PLANE_LUMA],
            dstRecon[PLANE_LUMA],
            secSet->reconStride[PLANE_LUMA],
            mb->width[PLANE_LUMA],
            mb->height[PLANE_LUMA]);

        funcSet->pfXinBlockCopy[mb->lgWidth[PLANE_CHROMA]] (
            srcRecon[PLANE_CHROMA_U],
            secSet->reconDataStride[PLANE_CHROMA],
            dstRecon[PLANE_CHROMA_U],
            secSet->reconStride[PLANE_CHROMA],
            mb->width[PLANE_CHROMA],
            mb->height[PLANE_CHROMA]);

        funcSet->pfXinBlockCopy[mb->lgWidth[PLANE_CHROMA]] (
            srcRecon[PLANE_CHROMA_V],
            secSet->reconDataStride[PLANE_CHROMA],
            dstRecon[PLANE_CHROMA_V],
            secSet->reconStride[PLANE_CHROMA],
            mb->width[PLANE_CHROMA],
            mb->height[PLANE_CHROMA]);


        miBuf      = pictureWrite->miBuf;
        miStride   = pictureWrite->miStride;
        widthInMi  = mb->width[PLANE_LUMA] >> XIN_LOG_MI_SIZE;
        heightInMi = mb->height[PLANE_LUMA] >> XIN_LOG_MI_SIZE;

        PEL_XY_TO_BLOCK_INDEX (mb->mbPelX[PLANE_LUMA], mb->mbPelY[PLANE_LUMA], miIdx, miStride, XIN_LOG_MI_SIZE);

        miSrc = secSet->miData;
        miDst = miBuf + miIdx;

        for (rowIdx = 0; rowIdx < heightInMi; rowIdx++)
        {
            memcpy (miDst, miSrc, sizeof(xin_mi_struct)*widthInMi);

            miDst += miStride;
            miSrc += secSet->miDataStride;
        }

    }
    else
    {
        widthInMi  = mb->width[PLANE_LUMA] >> XIN_LOG_MI_SIZE;
        heightInMi = mb->height[PLANE_LUMA] >> XIN_LOG_MI_SIZE;
        offXInMi   = mb->offX[PLANE_LUMA] >> XIN_LOG_MI_SIZE;
        offYInMi   = mb->offY[PLANE_LUMA] >> XIN_LOG_MI_SIZE;

        memcpy (secSet->sbLftCtx[PLANE_LUMA] + offYInMi, secSet->lftCtx[mbDepth + 1][splitType][PLANE_LUMA] + offYInMi, sizeof(UINT8)*heightInMi);
        memcpy (secSet->sbTopCtx[PLANE_LUMA] + offXInMi, secSet->topCtx[mbDepth + 1][splitType][PLANE_LUMA] + offXInMi, sizeof(UINT8)*widthInMi);

        widthInMi  = mb->width[PLANE_CHROMA] >> XIN_LOG_MI_SIZE;
        heightInMi = mb->height[PLANE_CHROMA] >> XIN_LOG_MI_SIZE;
        offXInMi   = mb->offX[PLANE_CHROMA] >> XIN_LOG_MI_SIZE;
        offYInMi   = mb->offY[PLANE_CHROMA] >> XIN_LOG_MI_SIZE;

        memcpy (secSet->sbLftCtx[PLANE_CHROMA_U] + offYInMi, secSet->lftCtx[mbDepth + 1][splitType][PLANE_CHROMA_U] + offYInMi, sizeof(UINT8)*heightInMi);
        memcpy (secSet->sbTopCtx[PLANE_CHROMA_U] + offXInMi, secSet->topCtx[mbDepth + 1][splitType][PLANE_CHROMA_U] + offXInMi, sizeof(UINT8)*widthInMi);
        memcpy (secSet->sbLftCtx[PLANE_CHROMA_V] + offYInMi, secSet->lftCtx[mbDepth + 1][splitType][PLANE_CHROMA_V] + offYInMi, sizeof(UINT8)*heightInMi);
        memcpy (secSet->sbTopCtx[PLANE_CHROMA_V] + offXInMi, secSet->topCtx[mbDepth + 1][splitType][PLANE_CHROMA_V] + offXInMi, sizeof(UINT8)*widthInMi);
    }

}

static void Xin265pComputePartitionCtx (
    xin_sec_struct *secSet,
    xin_mb_struct  *mb)
{
    UINT32        lftCtx;
    UINT32        topCtx;
    UINT32        curSize;
    UINT32        lftSize;
    UINT32        topSize;
    xin_mi_struct *lftMi;
    xin_mi_struct *topMi;

    (void)secSet;

    lftMi   = mb->lftMi;
    topMi   = mb->topMi;
    lftSize = lftMi->blockSize;
    topSize = topMi->blockSize;
    curSize = mb->lgWidth[PLANE_LUMA];
    lftSize = (lftSize == 0xFF) ? 255 : blockSize2LogDim[lftSize][1];
    topSize = (topSize == 0xFF) ? 255 : blockSize2LogDim[topSize][0];
    lftCtx  = (lftSize < curSize);
    topCtx  = (topSize < curSize);

    mb->partitionCtx = (UINT8)(lftCtx*2 + topCtx + XIN_PARTITION_PLOFFSET*(curSize - 3));

}

void Xin265pAnalyzeMbTopDown (
    xin_sec_struct *secSet,
    xin_mb_struct  *mb)
{
    UINT32         partIdx;
    BOOL           trySplit;
    BOOL           tryUnsplit;
    xin_mb_struct  *pqChildMb[4];
    xin_mb_struct  *phChildMb[2];
    xin_mb_struct  *pvChildMb[2];

    tryUnsplit    = (mb->geomFlag & XIN_MB_HAS_HOR) && (mb->geomFlag & XIN_MB_HAS_VER);
    trySplit      = ((mb->width[PLANE_LUMA] > 8) || (mb->height[PLANE_LUMA] > 8));
    mb->splitType = trySplit ? XIN_PARTITION_SPLIT : XIN_PARTITION_NONE;

    if (!(mb->geomFlag & XIN_MB_PRESENT))
    {
        return;
    }

    Xin265pComputePartitionCtx (
        secSet,
        mb);

    if (tryUnsplit)
    {
        Xin265pAnalyzeMb (
            secSet,
            mb);
    }

    if (mb->splitType == XIN_PARTITION_NONE)
    {
        trySplit = FALSE;
    }

    if (trySplit)
    {
        for (partIdx = 0; partIdx < 4; partIdx++)
        {
            Xin265pMbInit (
                secSet,
                mb,
                XIN_PART_SPLIT,
                partIdx);

            pqChildMb[partIdx] = secSet->mb;

            Xin265pAnalyzeMbTopDown (
                secSet,
                pqChildMb[partIdx]);
        }

        Xin265pDepthDecsion (
            secSet,
            mb,
            XIN_PARTITION_SPLIT,
            pqChildMb);

    }

    if ((mb->splitType == XIN_PARTITION_NONE) && (mb->bestBuf != NULL))
    {
        Xin265pReconMb (
            secSet,
            mb);

        Xin265pMbUpdateMi (
            secSet,
            mb);

    }

    if ((mb->canSplit & XIN_CAN_PART_HORZ) && (mb->skipCoeff != TRUE))
    {
        Xin265pPartitionBackup (
            secSet,
            mb);

        for (partIdx = 0; partIdx < 2; partIdx++)
        {
            Xin265pMbInit (
                secSet,
                mb,
                XIN_PART_HORZ,
                partIdx);

            phChildMb[partIdx] = secSet->mb;

            if (phChildMb[partIdx]->geomFlag & XIN_MB_PRESENT)
            {
                Xin265pAnalyzeMb (
                    secSet,
                    phChildMb[partIdx]);

                if ((phChildMb[partIdx]->bestBuf != NULL))
                {
                    Xin265pReconMb (
                        secSet,
                        phChildMb[partIdx]);

                    Xin265pMbUpdateMi (
                        secSet,
                        phChildMb[partIdx]);
                }
            }

        }

        Xin265pDepthDecsion (
            secSet,
            mb,
            XIN_PART_HORZ,
            phChildMb);

        Xin265pPartitionRestore (
            secSet,
            mb,
            XIN_PARTITION_HORZ);

    }

    if ((mb->canSplit & XIN_CAN_PART_VERT) && (mb->skipCoeff != TRUE))
    {
        Xin265pPartitionBackup (
            secSet,
            mb);

        for (partIdx = 0; partIdx < 2; partIdx++)
        {
            Xin265pMbInit (
                secSet,
                mb,
                XIN_PART_VERT,
                partIdx);

            pvChildMb[partIdx] = secSet->mb;

            if (pvChildMb[partIdx]->geomFlag & XIN_MB_PRESENT)
            {
                Xin265pAnalyzeMb (
                    secSet,
                    pvChildMb[partIdx]);

                if ((pvChildMb[partIdx]->bestBuf != NULL))
                {
                    Xin265pReconMb (
                        secSet,
                        pvChildMb[partIdx]);

                    Xin265pMbUpdateMi (
                        secSet,
                        pvChildMb[partIdx]);
                }
            }

        }

        Xin265pDepthDecsion (
            secSet,
            mb,
            XIN_PARTITION_VERT,
            pvChildMb);

        Xin265pPartitionRestore (
            secSet,
            mb,
            XIN_PARTITION_VERT);

    }

}

void Xin265pEncodeSb (
    xin_sec_struct *secSet,
    xin_sb_struct  *sb)
{
    (void)sb;

    Xin265pMbInit (
        secSet,
        NULL,
        XIN_PARTITION_NONE,
        0);

    Xin265pAnalyzeMbTopDown (
        secSet,
        secSet->mb);

}


void Xin265pWriteSb (
    xin_sec_struct *secSet,
    xin_sb_struct  *sb)
{
    xin_mb_struct *mb;

    (void)sb;

    mb = secSet->pqMbData[0];

    Xin265pWriteMbRec (
        secSet,
        mb);
}

