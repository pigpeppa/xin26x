/***************************************************************************//**
 *
 * @file          h266_encode_ctu.c
 * @brief         Encode, analyze or reconstruct CTU.
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
#include "h26x_thread_wrapper.h"
#include "basic_macro.h"
#include "h266_enc_init.h"
#include "h266_alf_struct.h"
#include "h266_entropy_manipulate.h"
#include "h266_analyze_cu.h"
#include "h266_trans_recon.h"
#include "h26x_block_utility.h"
#include "h266_encode_ctu.h"
#include "h266_func_struct.h"

static const UINT32 childNum[XIN_CU_SPLIT_NUM] =
{
    1,
    4,
    2,
    2,
    3,
    3
};

void Xin266DepthDecsion (
    xin_sec_struct *secSet,
    xin_cu_struct  *parentCu,
    UINT8          splitType,
    xin_cu_struct  **childCu)
{
    xin_prob_model  *context;
    UINT32          splitBits;
    UINT64          splitCost;
    UINT64          lambda;
    BOOL            isAllSkip;
    xin_mode_struct *modeCtrl;

    lambda    = secSet->sseLambda[PLANE_LUMA];
    modeCtrl  = parentCu->modeCtrl;
    isAllSkip = FALSE;
    context   = secSet->cabacSet->context;

    if ((splitType == XIN_CU_HORZ_SPLIT) || (splitType == XIN_CU_VERT_SPLIT))
    {
        isAllSkip = (childCu[0]->type == XIN_SKIP_MODE) && (childCu[1]->type == XIN_SKIP_MODE);
    }
    else if (splitType == XIN_CU_QUAD_SPLIT)
    {
        isAllSkip = (childCu[0]->type == XIN_SKIP_MODE) && (childCu[1]->type == XIN_SKIP_MODE) && (childCu[2]->type == XIN_SKIP_MODE) && (childCu[3]->type == XIN_SKIP_MODE);
    }

    if (isAllSkip && (modeCtrl->doMoreSplits))
    {
        modeCtrl->doMoreSplits--;
    }

    if ((isAllSkip) && (splitType == XIN_CU_QUAD_SPLIT) && (modeCtrl->doMoreSplits))
    {
        modeCtrl->doMoreSplits--;
    }

    if (splitType == XIN_CU_QUAD_SPLIT)
    {
        Xin266EstimateSplitType (
            context,
            FALSE,
            parentCu,
            splitType,
            &splitBits);

        splitCost  = CALC_SSE_COST (lambda, splitBits);
        splitCost += childCu[0]->modeCtrl->sseCost;
        splitCost += childCu[1]->modeCtrl->sseCost;
        splitCost += childCu[2]->modeCtrl->sseCost;
        splitCost += childCu[3]->modeCtrl->sseCost;

        if ((parentCu->width == 8) && (parentCu->height == 8))
        {
            // Intra 4x4 is only used if one of luma cbf is not 0.
            if (childCu[0]->rootCbf | childCu[1]->rootCbf | childCu[2]->rootCbf | childCu[3]->rootCbf)
            {
                splitCost += childCu[4]->modeCtrl->sseCost;
            }
            else
            {
                splitCost = XIN_MAX_U64_COST;
            }
        }

        if (splitCost < modeCtrl->sseCost)
        {
            parentCu->splitType = splitType;
            modeCtrl->sseCost   = splitCost;

            modeCtrl->rate  = childCu[0]->modeCtrl->rate;
            modeCtrl->rate += childCu[1]->modeCtrl->rate;
            modeCtrl->rate += childCu[2]->modeCtrl->rate;
            modeCtrl->rate += childCu[3]->modeCtrl->rate;

            modeCtrl->sse  = childCu[0]->modeCtrl->sse;
            modeCtrl->sse += childCu[1]->modeCtrl->sse;
            modeCtrl->sse += childCu[2]->modeCtrl->sse;
            modeCtrl->sse += childCu[3]->modeCtrl->sse;

            if ((parentCu->width == 8) && (parentCu->height == 8))
            {
                modeCtrl->rate += childCu[4]->modeCtrl->rate;
                modeCtrl->sse  += childCu[4]->modeCtrl->sse;
            }

        }

        if (parentCu->splitType == XIN_CU_QUAD_SPLIT)
        {
            parentCu->childCu[0] = childCu[0];
            parentCu->childCu[1] = childCu[1];
            parentCu->childCu[2] = childCu[2];
            parentCu->childCu[3] = childCu[3];
            parentCu->childCu[4] = childCu[4];
        }

    }
    else if ((splitType == XIN_CU_HORZ_SPLIT) || (splitType == XIN_CU_VERT_SPLIT))
    {
        Xin266EstimateSplitType (
            context,
            FALSE,
            parentCu,
            splitType,
            &splitBits);

        splitCost  = CALC_SSE_COST (lambda, splitBits);
        splitCost += childCu[0]->modeCtrl->sseCost;
        splitCost += childCu[1]->modeCtrl->sseCost;

        if (splitCost < modeCtrl->sseCost)
        {
            parentCu->splitType = splitType;
            modeCtrl->sseCost   = splitCost;

            modeCtrl->rate  = childCu[0]->modeCtrl->rate;
            modeCtrl->rate += childCu[1]->modeCtrl->rate;

            modeCtrl->sse  = childCu[0]->modeCtrl->sse;
            modeCtrl->sse += childCu[1]->modeCtrl->sse;

            parentCu->modeCtrl->bestBufIdx = parentCu->modeCtrl->bestBufIdx ^ 1;
        }

        if (splitCost < XIN_MAX_U64_COST)
        {
            if (splitType == XIN_CU_HORZ_SPLIT)
            {
                modeCtrl->didHorzSplit = TRUE;
                modeCtrl->bestHorzCost = splitCost;
            }
            else
            {
                modeCtrl->didVertSplit = TRUE;
                modeCtrl->bestVertCost = splitCost;
            }
        }

        if (parentCu->splitType == splitType)
        {
            parentCu->childCu[0] = childCu[0];
            parentCu->childCu[1] = childCu[1];
        }

    }
    else if ((splitType == XIN_CU_TRIV_SPLIT) || (splitType == XIN_CU_TRIH_SPLIT))
    {
        Xin266EstimateSplitType (
            context,
            FALSE,
            parentCu,
            splitType,
            &splitBits);

        splitCost  = CALC_SSE_COST (lambda, splitBits);
        splitCost += childCu[0]->modeCtrl->sseCost;
        splitCost += childCu[1]->modeCtrl->sseCost;
        splitCost += childCu[2]->modeCtrl->sseCost;

        if (splitCost < modeCtrl->sseCost)
        {
            parentCu->splitType = splitType;
            modeCtrl->sseCost   = splitCost;

            modeCtrl->rate  = childCu[0]->modeCtrl->rate;
            modeCtrl->rate += childCu[1]->modeCtrl->rate;
            modeCtrl->rate += childCu[2]->modeCtrl->rate;

            modeCtrl->sse  = childCu[0]->modeCtrl->sse;
            modeCtrl->sse += childCu[1]->modeCtrl->sse;
            modeCtrl->sse += childCu[2]->modeCtrl->sse;

            parentCu->modeCtrl->bestBufIdx = parentCu->modeCtrl->bestBufIdx ^ 1;
        }

        if (parentCu->splitType == splitType)
        {
            parentCu->childCu[0] = childCu[0];
            parentCu->childCu[1] = childCu[1];
            parentCu->childCu[2] = childCu[2];
        }

    }
    else
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "Invalid cu split type.\n");
    }

    if (parentCu->geomFlag & XIN_CB_SPLIT)
    {
        parentCu->childCu[0] = childCu[0];
        parentCu->childCu[1] = childCu[1];
        parentCu->childCu[2] = childCu[2];
        parentCu->childCu[3] = childCu[3];
        parentCu->childCu[4] = childCu[4];

        parentCu->splitType = splitType;
    }

}

static void Xin266PartitionBackup (
    xin_sec_struct *secSet,
    xin_cu_struct  *cu)
{
    xin_pic_struct   *picSet;
    xin_seq_struct   *seqSet;
    xin_func_struct  *funcSet;
    xin_ref_picture  *pictureWrite;
    PIXEL            *srcRecon[PLANE_NUM];
    PIXEL            *dstRecon[PLANE_NUM];
    xin_block_struct *blockBuf;
    xin_block_struct *blockSrc;
    xin_block_struct *blockDst;
    xin_mv_u         *mvdBuf;
    xin_mv_u         *mvdSrc;
    xin_mv_u         *mvdDst;
    xin_affine_mv    *affineMvBuf;
    xin_affine_mv    *affineMvSrc;
    xin_affine_mv    *affineMvDst;
    intptr_t         blockStride;
    intptr_t         blockIdx;
    SINT32           widthInBlock;
    SINT32           heightInBlock;
    SINT32           rowIdx;
    UINT32           qtDepth;

    funcSet       = secSet->funcSet;
    seqSet        = secSet->seqSet;
    picSet        = secSet->picSet;
    qtDepth       = cu->qtDepth;
    pictureWrite  = picSet->pictureWrite;
    blockBuf      = pictureWrite->blockSetMap;
    mvdBuf        = picSet->subMvdMap;
    affineMvBuf   = picSet->affineMvMap;
    blockStride   = pictureWrite->blockSetWidth;
    widthInBlock  = cu->width >> pictureWrite->lgBlockSize;
    heightInBlock = cu->height >> pictureWrite->lgBlockSize;
    srcRecon[0]   = secSet->reconCtu[PLANE_LUMA] + cu->offX + cu->offY*secSet->reconYStride;
    srcRecon[1]   = secSet->reconCtu[PLANE_CHROMA_U] + ((cu->offX + cu->offY*secSet->reconUvStride) >> 1);
    srcRecon[2]   = secSet->reconCtu[PLANE_CHROMA_V] + ((cu->offX + cu->offY*secSet->reconUvStride) >> 1);
    dstRecon[0]   = secSet->reconData[qtDepth][PLANE_LUMA];
    dstRecon[1]   = secSet->reconData[qtDepth][PLANE_CHROMA_U];
    dstRecon[2]   = secSet->reconData[qtDepth][PLANE_CHROMA_V];

    funcSet->pfXinBlockCopy[cu->lgWidth] (
        srcRecon[PLANE_LUMA],
        secSet->reconYStride,
        dstRecon[PLANE_LUMA],
        secSet->reconDataStride[PLANE_LUMA],
        cu->width,
        cu->height);

    funcSet->pfXinBlockCopy[cu->lgWidth - 1] (
        srcRecon[PLANE_CHROMA_U],
        secSet->reconUvStride,
        dstRecon[PLANE_CHROMA_U],
        secSet->reconDataStride[PLANE_CHROMA],
        cu->width>>1,
        cu->height>>1);

    funcSet->pfXinBlockCopy[cu->lgWidth - 1] (
        srcRecon[PLANE_CHROMA_V],
        secSet->reconUvStride,
        dstRecon[PLANE_CHROMA_V],
        secSet->reconDataStride[PLANE_CHROMA],
        cu->width>>1,
        cu->height>>1);

    PEL_XY_TO_BLOCK_INDEX (cu->cuPelX, cu->cuPelY, blockIdx, blockStride, pictureWrite->lgBlockSize);

    blockSrc    = blockBuf + blockIdx;
    blockDst    = secSet->blockData[qtDepth];
    mvdSrc      = mvdBuf + blockIdx;
    mvdDst      = secSet->subMvdData[qtDepth];
    affineMvSrc = affineMvBuf + blockIdx;
    affineMvDst = secSet->affineMvData[qtDepth];

    for (rowIdx = 0; rowIdx < heightInBlock; rowIdx++)
    {
        memcpy (blockDst, blockSrc, sizeof(xin_block_struct)*widthInBlock);

        memcpy (mvdDst, mvdSrc, sizeof(xin_mv_u)*widthInBlock);

        if (seqSet->config.enableAffine)
        {
            memcpy (affineMvDst, affineMvSrc, sizeof(xin_affine_mv)*widthInBlock);

            affineMvDst += secSet->blockDataStride;
            affineMvSrc += blockStride;
        }

        blockDst += secSet->blockDataStride;
        blockSrc += blockStride;

        mvdDst += secSet->blockDataStride;
        mvdSrc += blockStride;

    }

    memcpy (
        secSet->hmvpLutData[qtDepth],
        secSet->hmvpLut,
        XIN_MAX_HMVP_CAND_NUM*sizeof(xin_neighbour_mv));

    secSet->hmvpNumData[qtDepth] = secSet->hmvpNum;

    memcpy (
        secSet->hmvpLut,
        secSet->hmvpLutBuf[qtDepth],
        XIN_MAX_HMVP_CAND_NUM*sizeof(xin_neighbour_mv));

    secSet->hmvpNum = secSet->hmvpNumBuf[qtDepth];

}

static void Xin266PartitionInvalid (
    xin_sec_struct *secSet,
    xin_cu_struct  *cu)
{
    xin_pic_struct   *picSet;
    xin_ref_picture  *pictureWrite;
    xin_block_struct *blockBuf;
    intptr_t         blockStride;
    intptr_t         blockIdx;
    SINT32           widthInBlock;
    SINT32           heightInBlock;
    SINT32           rowIdx;

    picSet        = secSet->picSet;
    pictureWrite  = picSet->pictureWrite;
    blockBuf      = pictureWrite->blockSetMap;

    blockStride   = pictureWrite->blockSetWidth;
    widthInBlock  = cu->width >> pictureWrite->lgBlockSize;
    heightInBlock = cu->height >> pictureWrite->lgBlockSize;

    PEL_XY_TO_BLOCK_INDEX (cu->cuPelX, cu->cuPelY, blockIdx, blockStride, pictureWrite->lgBlockSize);

    blockBuf      = blockBuf + blockIdx;

    for (rowIdx = 0; rowIdx < heightInBlock; rowIdx++)
    {
        memset (blockBuf, 0xFF, sizeof(xin_block_struct)*widthInBlock);

        blockBuf += blockStride;
    }

}

static void Xin266PartitionRestore (
    xin_sec_struct *secSet,
    xin_cu_struct  *cu)
{
    xin_pic_struct   *picSet;
    xin_seq_struct   *seqSet;
    xin_func_struct  *funcSet;
    xin_ref_picture  *pictureWrite;
    PIXEL            *srcRecon[PLANE_NUM];
    PIXEL            *dstRecon[PLANE_NUM];
    xin_block_struct *blockBuf;
    xin_block_struct *blockSrc;
    xin_block_struct *blockDst;
    xin_mv_u         *mvdBuf;
    xin_mv_u         *mvdSrc;
    xin_mv_u         *mvdDst;
    xin_affine_mv    *affineMvBuf;
    xin_affine_mv    *affineMvSrc;
    xin_affine_mv    *affineMvDst;
    intptr_t         blockStride;
    intptr_t         blockIdx;
    SINT32           widthInBlock;
    SINT32           heightInBlock;
    SINT32           rowIdx;
    UINT32           qtDepth;

    funcSet      = secSet->funcSet;
    picSet       = secSet->picSet;
    seqSet       = secSet->seqSet;
    qtDepth      = cu->qtDepth;
    pictureWrite = picSet->pictureWrite;

    dstRecon[PLANE_LUMA]     = secSet->reconCtu[PLANE_LUMA] + cu->offX + cu->offY * secSet->reconYStride;
    dstRecon[PLANE_CHROMA_U] = secSet->reconCtu[PLANE_CHROMA_U] + ((cu->offX + cu->offY * secSet->reconUvStride)>>1);
    dstRecon[PLANE_CHROMA_V] = secSet->reconCtu[PLANE_CHROMA_V] + ((cu->offX + cu->offY * secSet->reconUvStride)>>1);
    srcRecon[PLANE_LUMA]     = secSet->reconData[qtDepth][PLANE_LUMA];
    srcRecon[PLANE_CHROMA_U] = secSet->reconData[qtDepth][PLANE_CHROMA_U];
    srcRecon[PLANE_CHROMA_V] = secSet->reconData[qtDepth][PLANE_CHROMA_V];

    funcSet->pfXinBlockCopy[cu->lgWidth] (
        srcRecon[PLANE_LUMA],
        secSet->reconDataStride[PLANE_LUMA],
        dstRecon[PLANE_LUMA],
        secSet->reconYStride,
        cu->width,
        cu->height);

    funcSet->pfXinBlockCopy[cu->lgWidth - 1] (
        srcRecon[PLANE_CHROMA_U],
        secSet->reconDataStride[PLANE_CHROMA],
        dstRecon[PLANE_CHROMA_U],
        secSet->reconUvStride,
        cu->width>>1,
        cu->height>>1);

    funcSet->pfXinBlockCopy[cu->lgWidth - 1](
        srcRecon[PLANE_CHROMA_V],
        secSet->reconDataStride[PLANE_CHROMA],
        dstRecon[PLANE_CHROMA_V],
        secSet->reconUvStride,
        cu->width >> 1,
        cu->height >> 1);

    blockBuf      = pictureWrite->blockSetMap;
    mvdBuf        = picSet->subMvdMap;
    affineMvBuf   = picSet->affineMvMap;
    blockStride   = pictureWrite->blockSetWidth;
    widthInBlock  = cu->width >> pictureWrite->lgBlockSize;
    heightInBlock = cu->height >> pictureWrite->lgBlockSize;

    PEL_XY_TO_BLOCK_INDEX (cu->cuPelX, cu->cuPelY, blockIdx, blockStride, pictureWrite->lgBlockSize);

    blockSrc = secSet->blockData[qtDepth];
    blockDst = blockBuf + blockIdx;

    mvdSrc = secSet->subMvdData[qtDepth];
    mvdDst = mvdBuf + blockIdx;

    affineMvSrc = secSet->affineMvData[qtDepth];
    affineMvDst = affineMvBuf + blockIdx;

    for (rowIdx = 0; rowIdx < heightInBlock; rowIdx++)
    {
        memcpy (blockDst, blockSrc, sizeof(xin_block_struct)*widthInBlock);

        memcpy (mvdDst, mvdSrc, sizeof(xin_mv_u)*widthInBlock);

        if (seqSet->config.enableAffine)
        {
            memcpy (affineMvDst, affineMvSrc, sizeof(xin_affine_mv)*widthInBlock);

            affineMvDst += blockStride;
            affineMvSrc += secSet->blockDataStride;
        }

        blockDst += blockStride;
        blockSrc += secSet->blockDataStride;

        mvdDst += blockStride;
        mvdSrc += secSet->blockDataStride;

    }

    memcpy (
        secSet->hmvpLut,
        secSet->hmvpLutData[qtDepth],
        XIN_MAX_HMVP_CAND_NUM*sizeof(xin_neighbour_mv));

    secSet->hmvpNum = secSet->hmvpNumData[qtDepth];

}

static BOOL Xin266TrySplit (
    xin_sec_struct *secSet,
    xin_cu_struct  *parentCu,
    UINT8          splitType)
{
    xin_seq_struct  *seqSet;
    xin_pic_struct  *picSet;
    xin_mode_struct *modeCtrl;
    xin_ctu_struct  *ctu;
    xin_prob_model  *context;
    xin_tu_struct   *tu;
    double          factor;
    UINT64          lambda;
    UINT64          cost;
    UINT64          sse;
    UINT32          rate;
    UINT32          splitBits;
    UINT32          overhead;
    double          thr1, thr2;
    UINT32          tuIdx;
    BOOL            splitEnd;

    seqSet   = secSet->seqSet;
    picSet   = secSet->picSet;
    ctu      = secSet->ctu;
    modeCtrl = parentCu->modeCtrl;
    lambda   = secSet->sseLambda[PLANE_LUMA];
    context  = secSet->cabacSet->context;

    if ((parentCu->geomFlag & XIN_CB_SPLIT) && (splitType == XIN_CU_QUAD_SPLIT))
    {
        return TRUE;
    }

    if ((parentCu->width <= seqSet->config.minQtSize) && (parentCu->height <= seqSet->config.minQtSize) && (splitType == XIN_CU_QUAD_SPLIT))
    {
        return FALSE;
    }

    if (!(parentCu->canSplit & (1 << splitType)))
    {
        return FALSE;
    }

    if (((splitType == XIN_CU_HORZ_SPLIT) || (splitType == XIN_CU_VERT_SPLIT)) && ((parentCu->width <= 8) || (parentCu->height <= 8)))
    {
        return FALSE;
    }

    if (((splitType == XIN_CU_TRIH_SPLIT) || (splitType == XIN_CU_TRIV_SPLIT)) && ((parentCu->width <= 16) || (parentCu->height <= 16)))
    {
        return FALSE;
    }

    if ((parentCu->rootCbf == 0) && (seqSet->config.tuCoefDepthQuit == 1) && (modeCtrl->sseCost != XIN_MAX_U64_COST))
    {
        return FALSE;
    }

    if (modeCtrl->sseCost < ctu->sseThr[parentCu->lgHeight - 2][parentCu->lgWidth - 2])
    {
        return FALSE;
    }

    if ((seqSet->config.tuCoefDepthQuit > 1) && (modeCtrl->sseCost != XIN_MAX_U64_COST))
    {
        splitEnd = TRUE;

        for (tuIdx = 0; tuIdx < parentCu->tuNum; tuIdx++)
        {
            tu = parentCu->tu[tuIdx];

            if ((tu->nzCGMapEs[0] > 1) || (tu->nzCGMapEs[1] > 1) || (tu->nzCGMapEs[2] > 1))
            {
                splitEnd = FALSE;

                break;
            }

        }

        if (splitEnd)
        {
            return FALSE;
        }

    }

    if ((seqSet->config.qtbttSpeedUp > 1) && modeCtrl->didHorzSplit && modeCtrl->didVertSplit)
    {
        if ((modeCtrl->bestHorzCost < modeCtrl->bestVertCost) && (splitType == XIN_CU_TRIV_SPLIT))
        {
            return FALSE;
        }

        if ((modeCtrl->bestVertCost < modeCtrl->bestHorzCost) && (splitType == XIN_CU_TRIH_SPLIT))
        {
            return FALSE;
        }
    }

    if ((parentCu->geomFlag & XIN_CB_FORBIDDEN) && (splitType != XIN_CU_QUAD_SPLIT))
    {
        return FALSE;
    }

    if ((modeCtrl->continuousSkip >= seqSet->config.quitSkipDepths))
    {
        return FALSE;
    }

    if (splitType == XIN_CU_TRIH_SPLIT)
    {
        if (modeCtrl->didHorzSplit && !parentCu->rootCbf && parentCu->splitType == XIN_CU_NO_SPLIT)
        {
            return FALSE;
        }

        if (seqSet->config.qtbttSpeedUp > 1)
        {
            if (modeCtrl->didHorzSplit && modeCtrl->didVertSplit && modeCtrl->bestHorzCost > modeCtrl->bestVertCost)
            {
                return FALSE;
            }
        }

        if (seqSet->config.fastTTSplit && modeCtrl->bestHorzCost != XIN_MAX_U64_COST)
        {
            if (modeCtrl->bestUnsplitCost != XIN_MAX_U64_COST && (modeCtrl->bestHorzCost > modeCtrl->bestUnsplitCost))
            {
                return FALSE;
            }
            
            if (modeCtrl->bestVertCost != XIN_MAX_U64_COST && (modeCtrl->bestHorzCost > modeCtrl->bestVertCost))
            {
                return FALSE;
            }
        }
       
    }

    if (splitType == XIN_CU_TRIV_SPLIT)
    {
        if (modeCtrl->didVertSplit && !parentCu->rootCbf && parentCu->splitType == XIN_CU_NO_SPLIT)
        {
            return FALSE;
        }

        if (seqSet->config.qtbttSpeedUp > 1)
        {
            if (modeCtrl->didHorzSplit && modeCtrl->didVertSplit && modeCtrl->bestVertCost > modeCtrl->bestHorzCost)
            {
                return FALSE;
            }
        }

        if (seqSet->config.fastTTSplit && modeCtrl->bestVertCost != XIN_MAX_U64_COST)
        {
            if (modeCtrl->bestUnsplitCost != XIN_MAX_U64_COST && (modeCtrl->bestVertCost > modeCtrl->bestUnsplitCost))
            {
                return FALSE;
            }
            
            if (modeCtrl->bestHorzCost != XIN_MAX_U64_COST && (modeCtrl->bestVertCost > modeCtrl->bestHorzCost))
            {
                return FALSE;
            }
        }

    }

    if ((parentCu->width >= 32) && (parentCu->height >= 32) && (seqSet->config.gradientFastQtbt))
    {
        thr1 = 1.0;
        thr2 = (1.0 / sqrt(2));

        if (modeCtrl->cuGrad[XIN_CU_GRAD_HOR] > thr1 * modeCtrl->cuGrad[XIN_CU_GRAD_VER] && modeCtrl->cuGrad[XIN_CU_GRAD_HOR] > thr2 * modeCtrl->cuGrad[XIN_CU_GRAD_DOW] && modeCtrl->cuGrad[XIN_CU_GRAD_HOR] > thr2 * modeCtrl->cuGrad[XIN_CU_GRAD_DUP] && (splitType == XIN_CU_HORZ_SPLIT || splitType == XIN_CU_TRIH_SPLIT))
        {
            return FALSE;
        }

        if( thr2 * modeCtrl->cuGrad[XIN_CU_GRAD_DUP] < modeCtrl->cuGrad[XIN_CU_GRAD_VER] && thr2 * modeCtrl->cuGrad[XIN_CU_GRAD_DOW] < modeCtrl->cuGrad[XIN_CU_GRAD_VER] && thr1 * modeCtrl->cuGrad[XIN_CU_GRAD_HOR] < modeCtrl->cuGrad[XIN_CU_GRAD_VER] && (splitType == XIN_CU_VERT_SPLIT || splitType == XIN_CU_TRIV_SPLIT))
        {
            return FALSE;
        }
    }

    if (modeCtrl->sseCost != XIN_MAX_U64_COST)
    {
        factor   = (secSet->qp > 30 ? 1.1 : 1.075) + (seqSet->config.qtbttSpeedUp > 0 ? 0.01 : 0.0);
        overhead = seqSet->config.qtbttSpeedUp > 0 ? (childNum[splitType]<<XIN_RATE_FRACTION) : 0;

        sse  = modeCtrl->sse << XIN_COST_FRACTION;
        sse  = (UINT64)(sse / factor);
        rate = (UINT32)(modeCtrl->rate / factor);

        Xin266EstimateSplitType (
            context,
            FALSE,
            parentCu,
            splitType,
            &splitBits);

        cost = sse + CALC_SSE_COST (lambda, splitBits + rate + overhead);

        if (cost > modeCtrl->sseCost)
        {
            return FALSE;
        }

    }

    return seqSet->config.qtbttSpeedUp <= 1 || !!modeCtrl->doMoreSplits;

}

void Xin266AnanlyzeCuSplit (
    xin_sec_struct *secSet,
    xin_cu_struct  *parentCu,
    UINT8          splitType,
    UINT32         partNum)
{
    xin_cu_struct  *childCu[5];
    UINT32         partIdx;

    for (partIdx = 0; partIdx < partNum; partIdx++)
    {
        Xin266CuInit (
            secSet,
            parentCu,
            splitType,
            partIdx);

        childCu[partIdx] = secSet->cu;

        Xin266AnalyzeCuRec (
            secSet,
            secSet->cu);

    }

    // Try to analyze chroma component under some cases
    if ((parentCu->width == 8) && (parentCu->height == 8) && (splitType == XIN_CU_QUAD_SPLIT))
    {
        Xin266CuInit (
            secSet,
            parentCu,
            XIN_CU_NO_SPLIT,
            0);

        Xin266AnalyzeCu (
            secSet,
            secSet->cu);

        childCu[4] = secSet->cu;
    }

    Xin266DepthDecsion (
        secSet,
        parentCu,
        splitType,
        childCu);

    if ((parentCu->splitType == XIN_CU_QUAD_SPLIT) && (parentCu->width == 8) && (parentCu->height == 8))
    {
        Xin266ReconCu (
            secSet,
            childCu[4]);
    }

    if (parentCu->splitType != splitType)
    {
        memcpy (
            secSet->hmvpLut,
            secSet->hmvpLutBuf[parentCu->qtDepth],
            XIN_MAX_HMVP_CAND_NUM*sizeof(xin_neighbour_mv));

        secSet->hmvpNum = secSet->hmvpNumBuf[parentCu->qtDepth];
    }

    if ((parentCu->splitType == splitType) && (parentCu->canSplit & (XIN_CAN_HORZ_SPLIT | XIN_CAN_VERT_SPLIT | XIN_CAN_TRIH_SPLIT | XIN_CAN_TRIV_SPLIT)))
    {
        Xin266PartitionBackup (
            secSet,
            parentCu);
    }

}

void Xin266AnalyzeCuRec (
    xin_sec_struct *secSet,
    xin_cu_struct  *cu)
{
    xin_mode_struct *modeCtrl;
    BOOL            tryUnsplit;

    modeCtrl      = cu->modeCtrl;
    tryUnsplit    = !(cu->geomFlag & XIN_CB_FORBIDDEN);
    cu->splitType = XIN_CU_NO_SPLIT;

    if (!(cu->geomFlag & XIN_CB_PRESENT))
    {
        return;
    }

    if (tryUnsplit)
    {
        Xin266AnalyzeCu (
            secSet,
            cu);
    }

    if (modeCtrl->qtBeforeBt)
    {
        if (Xin266TrySplit (secSet, cu, XIN_CU_QUAD_SPLIT))
        {
            Xin266AnanlyzeCuSplit (
                secSet,
                cu,
                XIN_CU_QUAD_SPLIT,
                4);
        }

    }

    if (Xin266TrySplit (secSet, cu, XIN_CU_HORZ_SPLIT))
    {
        Xin266AnanlyzeCuSplit (
            secSet,
            cu,
            XIN_CU_HORZ_SPLIT,
            2);
    }

    if (Xin266TrySplit (secSet, cu, XIN_CU_VERT_SPLIT))
    {
        Xin266AnanlyzeCuSplit (
            secSet,
            cu,
            XIN_CU_VERT_SPLIT,
            2);
    }

    if (Xin266TrySplit (secSet, cu, XIN_CU_TRIH_SPLIT))
    {
        Xin266AnanlyzeCuSplit (
            secSet,
            cu,
            XIN_CU_TRIH_SPLIT,
            3);
    }

    if (Xin266TrySplit (secSet, cu, XIN_CU_TRIV_SPLIT))
    {
        Xin266AnanlyzeCuSplit (
            secSet,
            cu,
            XIN_CU_TRIV_SPLIT,
            3);
    }

    if ((cu->partType == XIN_CU_QUAD_PART) && (cu->canSplit & (XIN_CAN_HORZ_SPLIT | XIN_CAN_VERT_SPLIT | XIN_CAN_TRIH_SPLIT | XIN_CAN_TRIV_SPLIT)))
    {
        Xin266PartitionInvalid (
            secSet,
            cu);
    }

    if (!modeCtrl->qtBeforeBt)
    {
        if (Xin266TrySplit (secSet, cu, XIN_CU_QUAD_SPLIT))
        {
            Xin266AnanlyzeCuSplit (
                secSet,
                cu,
                XIN_CU_QUAD_SPLIT,
                4);
        }
    }

    if ((cu->splitType == XIN_CU_NO_SPLIT) && (cu->bestBuf != NULL))
    {
        Xin266ReconCu (
            secSet,
            cu);

        Xin266CuWrapUp (
            secSet,
            cu);
    }

    if ((cu->partType == XIN_CU_QUAD_PART) && (cu->splitType != XIN_CU_NO_SPLIT) && (cu->canSplit & (XIN_CAN_HORZ_SPLIT | XIN_CAN_VERT_SPLIT | XIN_CAN_TRIH_SPLIT | XIN_CAN_TRIV_SPLIT)))
    {
        Xin266PartitionRestore (
            secSet,
            cu);
    }

}

static void Xin266CalcThreshold (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu)
{
    UINT32          rowIdx;
    UINT32          colIdx;
    UINT64          localSad;
    UINT64          localSse;
    UINT32          localCount;
    xin_ctu_struct  *lftCtu;
    xin_ctu_struct  *topCtu;
    xin_ctu_struct  *topLftCtu;
    xin_ctu_struct  *topRgtCtu;

    lftCtu    = secSet->lftCtu;
    topCtu    = secSet->topCtu;
    topLftCtu = secSet->topLftCtu;
    topRgtCtu = secSet->topRgtCtu;

    for (rowIdx = 0; rowIdx < 6; rowIdx++)
    {
        for (colIdx = 0; colIdx < 6; colIdx++)
        {
            localSad   = 0;
            localSse   = 0;
            localCount = 0;

            if (lftCtu)
            {
                localSad   += lftCtu->sadCost[rowIdx][colIdx];
                localSse   += lftCtu->sseCost[rowIdx][colIdx];
                localCount += lftCtu->cuCount[rowIdx][colIdx];
            }

            if (topCtu)
            {
                localSad   += topCtu->sadCost[rowIdx][colIdx];
                localSse   += topCtu->sseCost[rowIdx][colIdx];
                localCount += topCtu->cuCount[rowIdx][colIdx];
            }

            if (topLftCtu)
            {
                localSad   += topLftCtu->sadCost[rowIdx][colIdx];
                localSse   += topLftCtu->sseCost[rowIdx][colIdx];
                localCount += topLftCtu->cuCount[rowIdx][colIdx];
            }

            if (topRgtCtu)
            {
                localSad   += topRgtCtu->sadCost[rowIdx][colIdx];
                localSse   += topRgtCtu->sseCost[rowIdx][colIdx];
                localCount += topRgtCtu->cuCount[rowIdx][colIdx];
            }

            ctu->sadThr[rowIdx][colIdx] = XIN_UNSIGNED_ROUND_DIV (localSad, localCount);
            ctu->sseThr[rowIdx][colIdx] = XIN_UNSIGNED_ROUND_DIV (localSse, localCount);

        }

    }

}

void Xin266EncodeCtu (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu)
{
    Xin266CalcThreshold (
        secSet,
        ctu);

    Xin266CuInit (
        secSet,
        NULL,
        XIN_CU_QUAD_SPLIT,
        0);

    Xin266AnalyzeCuRec (
        secSet,
        secSet->cu);

}

void Xin266CuInfoUpdateRec (
    xin_sec_struct *secSet,
    xin_cu_struct  *cu)
{
    xin_cu_struct     *childCu;
    SINT32            partIdx;

    // Skip absent CB.
    if (!(cu->geomFlag & XIN_CB_PRESENT))
    {
        return;
    }

    if (cu->splitType != XIN_CU_NO_SPLIT)
    {
        if (cu->splitType == XIN_CU_QUAD_SPLIT)
        {
            for (partIdx = 0; partIdx < 4; partIdx++)
            {
                childCu = cu->childCu[partIdx];

                Xin266CuInfoUpdateRec (
                    secSet,
                    childCu);
            }

            if ((cu->width == 8) && (cu->height == 8))
            {
                childCu = cu->childCu[4];

                Xin266CuInfoUpdate (
                    secSet,
                    childCu);
            }

        }
        else if ((cu->splitType == XIN_CU_HORZ_SPLIT) || (cu->splitType == XIN_CU_VERT_SPLIT))
        {
            for (partIdx = 0; partIdx < 2; partIdx++)
            {
                childCu = cu->childCu[partIdx];

                Xin266CuInfoUpdateRec (
                    secSet,
                    childCu);
            }
        }
        else if ((cu->splitType == XIN_CU_TRIH_SPLIT) || (cu->splitType == XIN_CU_TRIV_SPLIT))
        {
            for (partIdx = 0; partIdx < 3; partIdx++)
            {
                childCu = cu->childCu[partIdx];

                Xin266CuInfoUpdateRec (
                    secSet,
                    childCu);
            }
        }
        else
        {
            _XIN_LOGGER (XIN_LOGGER_ERROR, "Invalid cu split type.\n");
        }

    }
    else
    {
        Xin266CuInfoUpdate (
            secSet,
            cu);
    }

}


void Xin266CtuInfoUpdate (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu)
{
    xin_seq_struct  *seqSet;
    xin_cu_struct   *cu;
    xin_depth_range *drMap;
    xin_ref_picture *pictureWrite;
    SINT32           vaildCuCount;

    seqSet = secSet->seqSet;
    cu     = ctu->cu;

    pictureWrite = secSet->picSet->pictureWrite;
    drMap        = pictureWrite->drMap + ctu->ctuAddr;

    ctu->vaildCuCount = 1;

    Xin266CuInfoUpdateRec (
        secSet,
        cu);

    vaildCuCount = XIN_MIN (ctu->vaildCuCount, 79);
    *drMap       = seqSet->drConfig[vaildCuCount];

    if (pictureWrite->frameType >= XIN_I_FRAME)
    {
        drMap->minDepth = seqSet->drConfig[0].minDepth;
    }

}


void Xin266WriteCtu (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu,
    BOOL           realEntropy)
{
    xin_cu_struct    *cu;
    xin_pic_struct   *picSet;
    xin266_tile_dim  *tileDim;
    xin_seq_struct   *seqSet;
    xin_cabac_struct *cabac;
    UINT32           bitNumStart;
    UINT32           bitNumEnd;

    cu      = ctu->cu;
    tileDim = secSet->tileDim;
    picSet  = secSet->picSet;
    seqSet  = secSet->seqSet;
    cabac   = &(secSet->cabacSet->cabac);

    Xin266GetBitCount (
        cabac,
        &bitNumStart);

    Xin266WriteSaoParam (
        secSet->cabacSet,
        seqSet->config.enableSao,
        seqSet->config.internalBitDepth,
        ctu);

    if (realEntropy)
    {
        Xin266WriteAlfParam (
            secSet->cabacSet,
            picSet->alfSet,
            picSet->enableAlf,
            ctu->ctuAddr);
    }

    Xin266WriteCuRec (
        secSet,
        cu,
        realEntropy);

    if ((ctu->ctuIndex + 1) == tileDim->ctuNumInTile)
    {
        Xin266EncodeTerminate (
            cabac,
            TRUE);

        Xin266EncodeFinish (
            cabac);
    }

    Xin266GetBitCount (
        cabac,
        &bitNumEnd);

    ctu->bitUsed = bitNumEnd - bitNumStart;

}

#if 0
void Xin266StoreCudata (
    xin_ctu_struct *ctu,
    xin_cu_struct  *srcCu,
    xin_cu_struct  *dstCu)
{
    UINT32        tuIdx;
    UINT32        partIdx;
    xin_tu_struct *srcTu;
    xin_tu_struct *dstTu;
    SINT16        *coeffBuf[PLANE_NUM];
    UINT16        *gt0Buf[PLANE_NUM];
    UINT32        tuSize;
    UINT32        cGSize;
    UINT32        tuCGNum;

    dstTu = dstCu->tu;
    srcTu = srcCu->tu;

    for (tuIdx = 0; tuIdx < dstCu->tuNum; tuIdx++)
    {
        srcTu = srcCu->tu + tuIdx;
        dstTu = dstCu->tu + tuIdx;

        memcpy (dstTu, srcTu, sizeof(xin_tu_struct));

        partIdx = dstCu->partIdx + dstTu->partIdx;
        tuSize  = (1 << dstTu->lgWidth[0]) * (1 << dstTu->lgHeight[0]);
        cGSize  = (1 << dstTu->lgCGWidth[0]) * (1 << dstTu->lgCGHeight[0]);
        tuCGNum = tuSize / cGSize;

        if (dstTu->yCbf)
        {
            coeffBuf[PLANE_LUMA] = ctu->coeffBuf[PLANE_LUMA] + (dstCu->offX + dstTu->offsetX) + (dstCu->offY + dstTu->offsetY)*ctu->coeffStride[0];
            gt0Buf[PLANE_LUMA]   = ctu->gt0Buf[PLANE_LUMA] + partIdx;

            Xin26xBlockCopy16 (
                srcTu->qCoeff[PLANE_LUMA],
                srcTu->coeffStride[PLANE_LUMA],
                coeffBuf[PLANE_LUMA],
                ctu->coeffStride[PLANE_LUMA],
                1 << srcTu->lgWidth[0],
                1 << srcTu->lgHeight[0]);

            memcpy (gt0Buf[PLANE_LUMA], srcTu->gt0BitMap[PLANE_LUMA], tuCGNum*sizeof(UINT16));

            dstTu->qCoeff[PLANE_LUMA]    = coeffBuf[PLANE_LUMA];
            dstTu->gt0BitMap[PLANE_LUMA] = gt0Buf[PLANE_LUMA];

        }

        tuSize  = (1 << dstTu->lgWidth[1]) * (1 << dstTu->lgHeight[1]);
        cGSize  = (1 << dstTu->lgCGWidth[1]) * (1 << dstTu->lgCGHeight[1]);
        tuCGNum = tuSize / cGSize;
        partIdx = partIdx >> 2;

        if (dstTu->uCbf)
        {
            coeffBuf[PLANE_CHROMA_U] = ctu->coeffBuf[PLANE_CHROMA_U] + (((dstCu->offX + dstTu->offsetX) + (dstCu->offY + dstTu->offsetY)*ctu->coeffStride[1]) >> 1);
            gt0Buf[PLANE_CHROMA_U]   = ctu->gt0Buf[PLANE_CHROMA_U] + partIdx;

            Xin26xBlockCopy16(
                srcTu->qCoeff[PLANE_CHROMA_U],
                srcTu->coeffStride[PLANE_CHROMA],
                coeffBuf[PLANE_CHROMA_U],
                ctu->coeffStride[PLANE_CHROMA],
                1 << srcTu->lgWidth[1],
                1 << srcTu->lgHeight[1]);

            memcpy (gt0Buf[PLANE_CHROMA_U], srcTu->gt0BitMap[PLANE_CHROMA_U], tuCGNum*sizeof(UINT16));

            dstCu->tu[tuIdx].qCoeff[PLANE_CHROMA_U]    = coeffBuf[PLANE_CHROMA_U];
            dstCu->tu[tuIdx].gt0BitMap[PLANE_CHROMA_U] = gt0Buf[PLANE_CHROMA_U];
        }

        if (dstTu->vCbf)
        {
            coeffBuf[PLANE_CHROMA_V] = ctu->coeffBuf[PLANE_CHROMA_V] + partIdx*cGSize;
            gt0Buf[PLANE_CHROMA_V]   = ctu->gt0Buf[PLANE_CHROMA_V] + partIdx;

            Xin26xBlockCopy16 (
                srcTu->qCoeff[PLANE_CHROMA_V],
                srcTu->coeffStride[PLANE_CHROMA],
                coeffBuf[PLANE_CHROMA_V],
                ctu->coeffStride[PLANE_CHROMA],
                1 << srcTu->lgWidth[1],
                1 << srcTu->lgHeight[1]);

            memcpy (gt0Buf[PLANE_CHROMA_V], srcTu->gt0BitMap[PLANE_CHROMA_V], tuCGNum*sizeof(UINT16));

            dstCu->tu[tuIdx].qCoeff[PLANE_CHROMA_V]    = coeffBuf[PLANE_CHROMA_V];
            dstCu->tu[tuIdx].gt0BitMap[PLANE_CHROMA_V] = gt0Buf[PLANE_CHROMA_V];
        }

        dstTu->coeffStride[PLANE_LUMA]   = ctu->coeffStride[PLANE_LUMA];
        dstTu->coeffStride[PLANE_CHROMA] = ctu->coeffStride[PLANE_CHROMA];

    }

}

void Xin266StoreCudataRec (
    xin_ctu_struct *ctu,
    xin_cu_struct  *cu)
{
    xin_cu_struct *dstCu;
    xin_tu_struct *dstTu;
    xin_cu_struct *childCu;
    UINT32        partIdx;

    dstCu = ctu->cu + ctu->cuIdx;
    dstTu = ctu->tu + ctu->tuIdx;

    memcpy (dstCu, cu, sizeof(xin_cu_struct));

    dstCu->tu = dstTu;

    ctu->cuIdx++;
    ctu->tuIdx += cu->tuNum;

    if (!(cu->geomFlag & XIN_CB_PRESENT))
    {
        return;
    }

    if (cu->splitType != XIN_CU_NO_SPLIT)
    {
        if (cu->splitType == XIN_CU_QUAD_SPLIT)
        {
            for (partIdx = 0; partIdx < 4; partIdx++)
            {
                childCu = cu->childCu[partIdx];

                Xin266StoreCudataRec (
                    ctu,
                    childCu);
            }

            if ((cu->width == 8) && (cu->height == 8))
            {
                childCu = cu->childCu[4];

                Xin266StoreCudataRec (
                    ctu,
                    childCu);
            }
        }
        else if ((cu->splitType == XIN_CU_HORZ_SPLIT) || (cu->splitType == XIN_CU_VERT_SPLIT))
        {
            for (partIdx = 0; partIdx < 2; partIdx++)
            {
                childCu = cu->childCu[partIdx];

                Xin266StoreCudataRec (
                    ctu,
                    childCu);
            }
        }
        else if ((cu->splitType == XIN_CU_TRIH_SPLIT) || (cu->splitType == XIN_CU_TRIV_SPLIT))
        {
            for (partIdx = 0; partIdx < 3; partIdx++)
            {
                childCu = cu->childCu[partIdx];

                Xin266StoreCudataRec (
                    ctu,
                    childCu);
            }
        }
        else
        {
            _XIN_LOGGER (XIN_LOGGER_ERROR, "Invalid cu split type.\n");
        }

    }
    else
    {
        Xin266StoreCudata (
            ctu,
            cu,
            dstCu);
    }

}

void Xin266StoreCtuData (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu)
{
    xin_pic_struct *picSet;

    picSet = secSet->picSet;

    ctu->tu = picSet->tu + picSet->tuIdx;
    ctu->cu = picSet->cu + picSet->cuIdx;

    picSet->tuIdx += secSet->tuIdx;
    picSet->cuIdx += secSet->cuIdx;

    if ((picSet->tuIdx >= picSet->tuNum) || (picSet->cuIdx >= picSet->cuNum))
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "Tu or cu index is out of range.\n");
    }

    Xin266StoreCudataRec (
        ctu,
        secSet->cuData);
}
#else
void Xin266StoreCudata (
    xin_ctu_struct *ctu,
    xin_cu_struct  *cu)
{
    UINT32        tuIdx;
    UINT32        partIdx;
    xin_tu_struct *tu;
    COEFF         *coeffBuf[PLANE_NUM];
    UINT16        *gt0Buf[PLANE_NUM];
    UINT32        tuSize;
    UINT32        cGSize;
    UINT32        tuCGNum;

    for (tuIdx = 0; tuIdx < cu->tuNum; tuIdx++)
    {
        tu = cu->tu[tuIdx];

        partIdx = cu->partIdx + tu->partIdx;
        tuSize  = (1 << tu->lgWidth[0]) * (1 << tu->lgHeight[0]);
        cGSize  = (1 << tu->lgCGWidth[0]) * (1 << tu->lgCGHeight[0]);
        tuCGNum = tuSize / cGSize;

        if (tu->yCbf)
        {
            coeffBuf[PLANE_LUMA] = ctu->coeffBuf[PLANE_LUMA] + (cu->offX + tu->offsetX) + (cu->offY + tu->offsetY)*ctu->coeffStride[0];
            gt0Buf[PLANE_LUMA]   = ctu->gt0Buf[PLANE_LUMA] + partIdx;

            Xin26xBlockCopyCoeff (
                tu->qCoeff[PLANE_LUMA],
                tu->coeffStride[PLANE_LUMA],
                coeffBuf[PLANE_LUMA],
                ctu->coeffStride[PLANE_LUMA],
                1 << tu->lgWidth[0],
                1 << tu->lgHeight[0]);

            memcpy (gt0Buf[PLANE_LUMA], tu->gt0BitMap[PLANE_LUMA], tuCGNum*sizeof(UINT16));

            tu->qCoeff[PLANE_LUMA]    = coeffBuf[PLANE_LUMA];
            tu->gt0BitMap[PLANE_LUMA] = gt0Buf[PLANE_LUMA];
        }

        tuSize  = (1 << tu->lgWidth[1]) * (1 << tu->lgHeight[1]);
        cGSize  = (1 << tu->lgCGWidth[1]) * (1 << tu->lgCGHeight[1]);
        tuCGNum = tuSize / cGSize;
        partIdx = partIdx >> 2;

        if (tu->uCbf)
        {
            coeffBuf[PLANE_CHROMA_U] = ctu->coeffBuf[PLANE_CHROMA_U] + (((cu->offX + tu->offsetX) + (cu->offY + tu->offsetY)*ctu->coeffStride[1]) >> 1);
            gt0Buf[PLANE_CHROMA_U]   = ctu->gt0Buf[PLANE_CHROMA_U] + partIdx;

            Xin26xBlockCopyCoeff (
                tu->qCoeff[PLANE_CHROMA_U],
                tu->coeffStride[PLANE_CHROMA],
                coeffBuf[PLANE_CHROMA_U],
                ctu->coeffStride[PLANE_CHROMA],
                1 << tu->lgWidth[1],
                1 << tu->lgHeight[1]);

            memcpy (gt0Buf[PLANE_CHROMA_U], tu->gt0BitMap[PLANE_CHROMA_U], tuCGNum*sizeof(UINT16));

            tu->qCoeff[PLANE_CHROMA_U]    = coeffBuf[PLANE_CHROMA_U];
            tu->gt0BitMap[PLANE_CHROMA_U] = gt0Buf[PLANE_CHROMA_U];
        }

        if (tu->vCbf)
        {
            coeffBuf[PLANE_CHROMA_V] = ctu->coeffBuf[PLANE_CHROMA_V] + (((cu->offX + tu->offsetX) + (cu->offY + tu->offsetY)*ctu->coeffStride[1]) >> 1);;
            gt0Buf[PLANE_CHROMA_V]   = ctu->gt0Buf[PLANE_CHROMA_V] + partIdx;

            Xin26xBlockCopyCoeff (
                tu->qCoeff[PLANE_CHROMA_V],
                tu->coeffStride[PLANE_CHROMA],
                coeffBuf[PLANE_CHROMA_V],
                ctu->coeffStride[PLANE_CHROMA],
                1 << tu->lgWidth[1],
                1 << tu->lgHeight[1]);

            memcpy (gt0Buf[PLANE_CHROMA_V], tu->gt0BitMap[PLANE_CHROMA_V], tuCGNum*sizeof(UINT16));

            tu->qCoeff[PLANE_CHROMA_V]    = coeffBuf[PLANE_CHROMA_V];
            tu->gt0BitMap[PLANE_CHROMA_V] = gt0Buf[PLANE_CHROMA_V];
        }

        tu->coeffStride[PLANE_LUMA]   = ctu->coeffStride[PLANE_LUMA];
        tu->coeffStride[PLANE_CHROMA] = ctu->coeffStride[PLANE_CHROMA];

    }

}

void Xin266StoreCudataRec (
    xin_ctu_struct *ctu,
    xin_cu_struct  *cu)
{
    xin_cu_struct *childCu;
    UINT32        partIdx;

    if (!(cu->geomFlag & XIN_CB_PRESENT))
    {
        return;
    }

    if (cu->splitType != XIN_CU_NO_SPLIT)
    {
        if (cu->splitType == XIN_CU_QUAD_SPLIT)
        {
            for (partIdx = 0; partIdx < 4; partIdx++)
            {
                childCu = cu->childCu[partIdx];

                Xin266StoreCudataRec (
                    ctu,
                    childCu);
            }

            if ((cu->width == 8) && (cu->height == 8))
            {
                childCu = cu->childCu[4];

                Xin266StoreCudataRec (
                    ctu,
                    childCu);
            }
        }
        else if ((cu->splitType == XIN_CU_HORZ_SPLIT) || (cu->splitType == XIN_CU_VERT_SPLIT))
        {
            for (partIdx = 0; partIdx < 2; partIdx++)
            {
                childCu = cu->childCu[partIdx];

                Xin266StoreCudataRec (
                    ctu,
                    childCu);
            }
        }
        else if ((cu->splitType == XIN_CU_TRIH_SPLIT) || (cu->splitType == XIN_CU_TRIV_SPLIT))
        {
            for (partIdx = 0; partIdx < 3; partIdx++)
            {
                childCu = cu->childCu[partIdx];

                Xin266StoreCudataRec (
                    ctu,
                    childCu);
            }
        }
        else
        {
            _XIN_LOGGER (XIN_LOGGER_ERROR, "Invalid cu split type.\n");
        }

    }
    else
    {
        Xin266StoreCudata (
            ctu,
            cu);
    }

}

void Xin266StoreCtuData (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu)
{
    (void)secSet;

    Xin266StoreCudataRec (
        ctu,
        ctu->cu);
}

#endif


