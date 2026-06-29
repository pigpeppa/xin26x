/***************************************************************************//**
 *
 * @file          h266_enc_init.c
 * @brief         This file contains frame, section, CTU and
 *                CU level initialization.
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
#include "h266_rate_control.h"
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
#include "h266_lmcs.h"
#include "h266_func_struct.h"

// 0.57*pow (2.0, (qp-12)/3.0)*65536
static const UINT64 sseLambda[XIN_QP_NUM + XIN_QP_SHIFT] =
{
    2335,       2942,       3706,       4669,
    5883,       7412,       9339,       11766,
    14825,      18678,      23533,      29649,
    37356,      47065,      59298,      74711,
    94130,      118596,     149422,     188260,
    237193,     298844,     376520,     474386,
    597688,     753040,     948771,     1195377,
    1506080,    1897542,    2390753,    3012160,
    3795084,    4781507,    6024321,    7590169,
    9563013,    12048642,   15180337,   19126026,
    24097283,   30360674,   38252052,   48194566,
    60721348,   76504105,   96389132,   121442697,
    153008210,  192778264,  242885393,  306016420,
    385556529,  485770787,  612032840,  771113058,
    971541574,  1224065679, 1542226116, 1943083147,
    2448131359, 3084452232, 3886166294, 4896262717
};

// sqrt(0.57*pow (2.0, (qp-12)/3.0))*256
static const UINT32 sadLambda[XIN_QP_NUM + XIN_QP_SHIFT] =
{

    64,    72,    81,    91,
    102,   114,   128,   144,
    161,   181,   203,   228,
    256,   287,   323,   362,
    406,   456,   512,   575,
    645,   724,   813,   912,
    1024,  1149,  1290,  1448,
    1625,  1825,  2048,  2299,
    2580,  2896,  3251,  3649,
    4096,  4598,  5161,  5793,
    6502,  7298,  8192,  9195,
    10321, 11585, 13004, 14596,
    16384, 18390, 20643, 23170,
    26008, 29193, 32768, 36781,
    41285, 46341, 52016, 58386,
    65536, 73562, 82570, 92682,
};

static const UINT32 partOffset[XIN_CU_SPLIT_NUM][4] =
{
    {0, 0, 0, 0},
    {0, 1, 2, 3},
    {0, 2, 0, 0},
    {0, 2, 0, 0},
    {0, 1, 3, 0},
    {0, 1, 3, 0},
};

SINT32  Xin266CreateModeChunk (
    xin_sec_struct *secSet,
    UINT32         chunkNum)
{
    xin_mode_list *modeChunk;
    UINT32        chunkIdx;
    UINT32        modeIdx;

    for (chunkIdx = 0; chunkIdx < chunkNum; chunkIdx++)
    {
        XIN_MALLOC_CHECK (modeChunk, sizeof(xin_mode_list));

        XIN_MALLOC_CHECK (modeChunk->modeBuf, sizeof(xin_mode_struct)*XIN_SECTION_CHUNK_SIZE);

        modeChunk->nextList = NULL;

        if (secSet->firstModeChunk)
        {
            secSet->lastModeChunk->nextList = modeChunk;
        }
        else
        {
            secSet->firstModeChunk = modeChunk;
        }

        secSet->lastModeChunk = modeChunk;
        modeChunk->nextList   = NULL;

        if (!secSet->modeList)
        {
            XIN_MALLOC_CHECK (secSet->modeList, sizeof(xin_mode_struct *)*XIN_SECTION_CHUNK_SIZE);
        }
        else
        {
            XIN_REMALLOC_CHECK (secSet->modeList, sizeof(xin_mode_struct *)*(secSet->modeListSize + XIN_SECTION_CHUNK_SIZE));
        }

        for (modeIdx = 0; modeIdx < XIN_SECTION_CHUNK_SIZE; modeIdx++)
        {
            secSet->modeList[secSet->modeListSize] = modeChunk->modeBuf + modeIdx;

            secSet->modeListSize++;
        }

    }

    return XIN_SUCCESS;

}

static SINT32 Xin266GetModeBuf (
    xin_sec_struct  *secSet,
    xin_mode_struct **modeCtrl)
{

    if ((secSet->modeListIdx + 1) < secSet->modeListSize)
    {
        *modeCtrl = secSet->modeList[secSet->modeListIdx];

        secSet->modeListIdx++;
    }
    else
    {
        Xin266CreateModeChunk (
            secSet,
            1);

        *modeCtrl = secSet->modeList[secSet->modeListIdx];

        secSet->modeListIdx++;
    }

    return XIN_SUCCESS;

}

SINT32  Xin266CreateCuChunk (
    xin_pic_struct *picSet,
    UINT32         tileIdx)
{
    xin_cu_list *cuChunk;
    UINT32      cuIdx;

    XIN_MALLOC_CHECK (cuChunk, sizeof(xin_cu_list));

    XIN_MALLOC_CHECK (cuChunk->cuBuf, sizeof(xin_cu_struct)*XIN_SECTION_CHUNK_SIZE);

    Xin26xGetLock (
        picSet->listLock);

    if (picSet->firstCuChunk)
    {
        picSet->lastCuChunk->nextList = cuChunk;
    }
    else
    {
        picSet->firstCuChunk = cuChunk;
    }

    picSet->lastCuChunk = cuChunk;
    cuChunk->nextList   = NULL;

    Xin26xReleaseLock (
        picSet->listLock);

    if (!picSet->cuList[tileIdx])
    {
        XIN_MALLOC_CHECK (picSet->cuList[tileIdx], sizeof(xin_cu_struct *)*XIN_SECTION_CHUNK_SIZE);
    }
    else
    {
        XIN_REMALLOC_CHECK (picSet->cuList[tileIdx], sizeof(xin_cu_struct *)*(picSet->cuListSize[tileIdx] + XIN_SECTION_CHUNK_SIZE));
    }

    for (cuIdx = 0; cuIdx < XIN_SECTION_CHUNK_SIZE; cuIdx++)
    {
        picSet->cuList[tileIdx][picSet->cuListSize[tileIdx]] = cuChunk->cuBuf + cuIdx;

        picSet->cuListSize[tileIdx]++;
    }

    return XIN_SUCCESS;

}

static SINT32 Xin266GetCuBuf (
    xin_sec_struct *secSet,
    xin_cu_struct  **cuBuf)
{
    xin_pic_struct *picSet;
    UINT32         tileIdx;

    picSet  = secSet->picSet;
    tileIdx = secSet->sectionIdx;

    if ((picSet->cuListIdx[tileIdx] + 1) < picSet->cuListSize[tileIdx])
    {
        *cuBuf = picSet->cuList[tileIdx][picSet->cuListIdx[tileIdx]];

        picSet->cuListIdx[tileIdx]++;
    }
    else
    {
        Xin266CreateCuChunk (
            picSet,
            tileIdx);

        *cuBuf = picSet->cuList[tileIdx][picSet->cuListIdx[tileIdx]];

        picSet->cuListIdx[tileIdx]++;
    }

    return XIN_SUCCESS;

}

SINT32  Xin266CreateTuChunk (
    xin_pic_struct *picSet,
    UINT32         tileIdx)
{
    xin_tu_list *tuChunk;
    UINT32      tuIdx;

    XIN_MALLOC_CHECK (tuChunk, sizeof(xin_tu_list));

    XIN_MALLOC_CHECK (tuChunk->tuBuf, sizeof(xin_tu_struct)*XIN_SECTION_CHUNK_SIZE);

    Xin26xGetLock (
        picSet->listLock);

    if (picSet->firstTuChunk)
    {
        picSet->lastTuChunk->nextList = tuChunk;
    }
    else
    {
        picSet->firstTuChunk = tuChunk;
    }

    picSet->lastTuChunk = tuChunk;
    tuChunk->nextList   = NULL;

    Xin26xReleaseLock (
        picSet->listLock);

    if (!picSet->tuList[tileIdx])
    {
        XIN_MALLOC_CHECK (picSet->tuList[tileIdx], sizeof(xin_tu_struct *)*XIN_SECTION_CHUNK_SIZE);
    }
    else
    {
        XIN_REMALLOC_CHECK (picSet->tuList[tileIdx], sizeof(xin_tu_struct *)*(picSet->tuListSize[tileIdx] + XIN_SECTION_CHUNK_SIZE));
    }

    for (tuIdx = 0; tuIdx < XIN_SECTION_CHUNK_SIZE; tuIdx++)
    {
        picSet->tuList[tileIdx][picSet->tuListSize[tileIdx]] = tuChunk->tuBuf + tuIdx;

        picSet->tuListSize[tileIdx]++;
    }

    return XIN_SUCCESS;

}

static SINT32 Xin266GetTuBuf (
    xin_sec_struct *secSet,
    xin_tu_struct  **tuBuf)
{
    xin_pic_struct *picSet;
    UINT32         tileIdx;

    picSet  = secSet->picSet;
    tileIdx = secSet->sectionIdx;

    if ((picSet->tuListIdx[tileIdx] + 1) < picSet->tuListSize[tileIdx])
    {
        *tuBuf = picSet->tuList[tileIdx][picSet->tuListIdx[tileIdx]];

        picSet->tuListIdx[tileIdx]++;
    }
    else
    {
        Xin266CreateTuChunk (
            picSet,
            tileIdx);

        *tuBuf = picSet->tuList[tileIdx][picSet->tuListIdx[tileIdx]];

        picSet->tuListIdx[tileIdx]++;
    }

    return XIN_SUCCESS;

}

void Xin266CtuInit (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu)
{
    xin_seq_struct    *seqSet;
    xin_pic_struct    *picSet;
    xin_input_picture *inputPicture;
    xin_ref_picture   *pictureWrite;
    xin_ref_picture   *pictureRead0;
    xin_ref_picture   *pictureRead1;
    UINT32            ctuAddr;
    xin_ctu_struct    *lftCtu;
    xin_ctu_struct    *topCtu;
    xin_ctu_struct    *topLftCtu;
    xin_ctu_struct    *topRgtCtu;
    SINT32            deltaQp;
    BOOL              is10Bit;
    double            lambdaScaling;

    seqSet       = secSet->seqSet;
    picSet       = secSet->picSet;
    pictureWrite = picSet->pictureWrite;
    pictureRead0 = picSet->pictureRead[XIN_LIST_0][0];
    pictureRead1 = picSet->pictureRead[XIN_LIST_1][0];
    ctuAddr      = ctu->ctuAddr;
    is10Bit      = XIN_INTERNAL_BIT_DEPTH == XIN_10_BIT_DEPTH;
    inputPicture = picSet->inputPicture;

    secSet->uvQp = seqSet->chromaQpMap[secSet->qp] + seqSet->config.chromaQpOffset;
    secSet->ctu  = ctu;

    secSet->sadLambda[PLANE_LUMA]   = sadLambda[secSet->qp + is10Bit*XIN_QP_SHIFT];
    secSet->sseLambda[PLANE_LUMA]   = sseLambda[secSet->qp + is10Bit*XIN_QP_SHIFT];
    secSet->sadLambda[PLANE_CHROMA] = sadLambda[secSet->uvQp + is10Bit*XIN_QP_SHIFT];
    secSet->sseLambda[PLANE_CHROMA] = sseLambda[secSet->uvQp + is10Bit*XIN_QP_SHIFT];

    if ((inputPicture->isMctfFrame) && (seqSet->config.enableBim))
    {
        lambdaScaling                   = pow (2, inputPicture->dqpMap[ctuAddr] / 3);
        secSet->sadLambda[PLANE_LUMA]   = (UINT32)(secSet->sadLambda[PLANE_LUMA] * lambdaScaling);
        secSet->sadLambda[PLANE_CHROMA] = (UINT32)(secSet->sadLambda[PLANE_CHROMA] * lambdaScaling);
        secSet->sseLambda[PLANE_LUMA]   = (UINT32)(secSet->sseLambda[PLANE_LUMA] * lambdaScaling);
        secSet->sseLambda[PLANE_CHROMA] = (UINT32)(secSet->sseLambda[PLANE_CHROMA] * lambdaScaling);
    }

    if (seqSet->config.enableRdoq)
    {
        secSet->cabacEst.estValid[0] = FALSE;
        secSet->cabacEst.estValid[1] = FALSE;

        secSet->cabacEst.lastXValid[0][2] = FALSE;
        secSet->cabacEst.lastXValid[0][3] = FALSE;
        secSet->cabacEst.lastXValid[0][4] = FALSE;
        secSet->cabacEst.lastXValid[0][5] = FALSE;
        secSet->cabacEst.lastXValid[0][6] = FALSE;

        secSet->cabacEst.lastXValid[1][2] = FALSE;
        secSet->cabacEst.lastXValid[1][3] = FALSE;
        secSet->cabacEst.lastXValid[1][4] = FALSE;
        secSet->cabacEst.lastXValid[1][5] = FALSE;
        secSet->cabacEst.lastXValid[1][6] = FALSE;

        secSet->cabacEst.lastYValid[0][2] = FALSE;
        secSet->cabacEst.lastYValid[0][3] = FALSE;
        secSet->cabacEst.lastYValid[0][4] = FALSE;
        secSet->cabacEst.lastYValid[0][5] = FALSE;
        secSet->cabacEst.lastYValid[0][6] = FALSE;

        secSet->cabacEst.lastYValid[1][2] = FALSE;
        secSet->cabacEst.lastYValid[1][3] = FALSE;
        secSet->cabacEst.lastYValid[1][4] = FALSE;
        secSet->cabacEst.lastYValid[1][5] = FALSE;
        secSet->cabacEst.lastYValid[1][6] = FALSE;

    }

    lftCtu    = (ctu->availField & XIN_LFT_CTU_AVAIL) ? (ctu - 1) : NULL;
    topCtu    = (ctu->availField & XIN_TOP_CTU_AVAIL) ? (ctu - seqSet->frameWidthInCtu) : NULL;
    topLftCtu = ((lftCtu != NULL) & (topCtu != NULL)) ? (ctu - seqSet->frameWidthInCtu - 1) : NULL;
    topRgtCtu = ((ctu->availField & XIN_RGT_CTU_AVAIL) && (topCtu != NULL) && (!seqSet->config.enableWpp)) ? (ctu - seqSet->frameWidthInCtu + 1) : NULL;

    secSet->reconCtu[PLANE_LUMA]     = pictureWrite->refBuf[PLANE_LUMA] + ctu->ctuPelX + ctu->ctuPelY*secSet->reconYStride;
    secSet->reconCtu[PLANE_CHROMA_U] = pictureWrite->refBuf[PLANE_CHROMA_U] + ((ctu->ctuPelX + ctu->ctuPelY*secSet->reconUvStride) >> 1);
    secSet->reconCtu[PLANE_CHROMA_V] = pictureWrite->refBuf[PLANE_CHROMA_V] + ((ctu->ctuPelX + ctu->ctuPelY*secSet->reconUvStride) >> 1);

    deltaQp = secSet->qp - secSet->uvQp;
    deltaQp = XIN_CLIP (deltaQp, -XIN_MAX_UV_QP_DIF, +XIN_MAX_UV_QP_DIF);

    secSet->codingDeltaQp = seqSet->config.enableCuQpDelta;
    secSet->chromaWeight  = seqSet->chromaWeight[deltaQp];
    secSet->modeListIdx   = 0;

    if (!picSet->offlineMode)
    {
        picSet->tuListIdx[secSet->sectionIdx] = 0;
        picSet->cuListIdx[secSet->sectionIdx] = 0;
    }

    secSet->lftCtu    = lftCtu;
    secSet->topCtu    = topCtu;
    secSet->topLftCtu = topLftCtu;
    secSet->topRgtCtu = topRgtCtu;
    ctu->ctuQp        = secSet->qp;

    if ((pictureWrite->frameType >= XIN_I_FRAME) || ((ctu->width != seqSet->ctuSize) || (ctu->height != seqSet->ctuSize)))
    {
        ctu->minDepth = seqSet->drConfig[0].minDepth;
        ctu->maxDepth = seqSet->drConfig[0].maxDepth;
    }
    else if (pictureWrite->frameType == XIN_P_FRAME)
    {
        ctu->minDepth = pictureRead0->drMap[ctuAddr].minDepth;
        ctu->maxDepth = pictureRead0->drMap[ctuAddr].maxDepth;
    }
    else
    {
        ctu->minDepth = XIN_MIN (pictureRead0->drMap[ctuAddr].minDepth, pictureRead1->drMap[ctuAddr].minDepth);
        ctu->maxDepth = XIN_MAX (pictureRead0->drMap[ctuAddr].maxDepth, pictureRead1->drMap[ctuAddr].maxDepth);
    }

    memset (&ctu->skipSse[0][0],   0, sizeof(UINT64)*6*6);
    memset (&ctu->skipCount[0][0], 0, sizeof(UINT32)*6*6);

    memset (&ctu->interSad[0][0],   0, sizeof(UINT64)*6*6);
    memset (&ctu->interCount[0][0], 0, sizeof(UINT32)*6*6);

    memset (&ctu->sseCost[0][0], 0, sizeof(UINT64)*6*6);
    memset (&ctu->sadCost[0][0], 0, sizeof(UINT64)*6*6);
    memset (&ctu->cuCount[0][0], 0, sizeof(UINT32)*6*6);

}

void Xin266CtuPostInit (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu)
{
    xin_seq_struct  *seqSet;
    xin_pic_struct  *picSet;
    UINT32          sectionIdx;
    UINT32          ctuIndex;

    seqSet     = secSet->seqSet;
    picSet     = secSet->picSet;
    sectionIdx = secSet->sectionIdx;
    ctuIndex   = ctu->ctuX;

    if ((!seqSet->config.enableWpp) && (seqSet->config.enableTiles))
    {
        return;
    }

    if ((ctuIndex == 0) && ((sectionIdx + 1) != seqSet->frameHeightInCtu))
    {
        memcpy (
            picSet->cabacContext[secSet->sectionIdx + 1],
            &secSet->cabacSet->context,
            XIN_NUM_OF_CTX*sizeof(xin_prob_model));
    }

    picSet->ctuRowRefQp[sectionIdx] = secSet->refQp;
    picSet->hmvpNum[sectionIdx]     = secSet->hmvpNum;

}


static void Xin266SpanSubMvd (
    xin_mv_u       *subMvd,
    xin_mv_u       *subMvdMap,
    UINT32         subMvdStride,
    UINT32         subWidthInBlock,
    UINT32         subHeightInBlock)
{
    UINT32      colIdx;
    UINT32      rowIdx;

    for (rowIdx = 0; rowIdx < subHeightInBlock; rowIdx++)
    {
        for (colIdx = 0; colIdx < subWidthInBlock; colIdx++)
        {
            subMvdMap[colIdx].s32x1 = subMvd->s32x1;
        }

        subMvdMap += subMvdStride;
    }

}

static void Xin266SpanSubMv (
    xin_block_struct *blockMap,
    UINT32           blockSetWidth,
    xin_mv32_u       subMv[1024][XIN_LIST_NUM],
    SINT8            refIdx[1024][XIN_LIST_NUM],
    UINT32           subWidthInBlock,
    UINT32           subHeightInBlock)
{
    UINT32      colIdx;
    UINT32      rowIdx;

    for (rowIdx = 0; rowIdx < subHeightInBlock; rowIdx++)
    {
        for (colIdx = 0; colIdx < subWidthInBlock; colIdx++)
        {
            blockMap[colIdx].mv[XIN_LIST_0].s64x1 = refIdx[0][XIN_LIST_0] >= 0 ? subMv[0][XIN_LIST_0].s64x1 : 0;
            blockMap[colIdx].mv[XIN_LIST_1].s64x1 = refIdx[0][XIN_LIST_1] >= 0 ? subMv[0][XIN_LIST_1].s64x1 : 0;
            blockMap[colIdx].refIdx[XIN_LIST_0]   = refIdx[0][XIN_LIST_0];
            blockMap[colIdx].refIdx[XIN_LIST_1]   = refIdx[0][XIN_LIST_1];
        }

        blockMap += blockSetWidth;
    }

}

void Xin266CuWrapUp (
    xin_sec_struct *secSet,
    xin_cu_struct  *cu)
{
    UINT32           cuPelX;
    UINT32           cuPelY;
    UINT32           blockIndex;
    UINT32           blockSetWidth;
    UINT32           widthInBlock;
    UINT32           heightInBlock;
    UINT32           subWidth;
    UINT32           subHeight;
    UINT32           rowIdx;
    UINT32           colIdx;
    UINT32           mvIdx;
    UINT32           lgBlockSize;
    xin_pic_struct   *picSet;
    xin_block_struct *blockMap;
    xin_block_struct *curBlock;
    xin_mv_u         *subMvdMap;
    xin_mv_u         *subMvd;
    xin_affine_mv    *affineMvMap;
    xin_affine_mv    *affineMv;
    intptr_t         affineMvStride;
    xin_ref_picture  *pictureWrite;
    xin_neighbour_mv neighbourMv;
    xin_pu_struct    *pu;
    xin_mode_struct  *modeCtrl;

    if (!(cu->treeMask & XIN_CU_TREE_L_MASK))
    {
        return;
    }

    picSet = secSet->picSet;
    cuPelX = cu->cuPelX;
    cuPelY = cu->cuPelY;
    mvIdx  = 0;

    pictureWrite   = picSet->pictureWrite;
    lgBlockSize    = pictureWrite->lgBlockSize;
    blockMap       = pictureWrite->blockSetMap;
    blockSetWidth  = pictureWrite->blockSetWidth;
    subMvdMap      = picSet->subMvdMap;
    affineMvMap    = picSet->affineMvMap;
    affineMvStride = picSet->affineMvStride;
    modeCtrl       = cu->modeCtrl;

    PEL_XY_TO_BLOCK_INDEX (cuPelX, cuPelY, blockIndex, blockSetWidth, lgBlockSize);

    blockMap    += blockIndex;
    subMvdMap   += blockIndex;
    affineMvMap += blockIndex;

    pu = &cu->pu;

    widthInBlock  = pu->width >> lgBlockSize;
    heightInBlock = pu->height >> lgBlockSize;
    curBlock      = blockMap;
    subMvd        = subMvdMap;
    affineMv      = affineMvMap;

    for (rowIdx = 0; rowIdx < heightInBlock; rowIdx++)
    {
        for (colIdx = 0; colIdx < widthInBlock; colIdx++)
        {
            curBlock[colIdx].qtDepth = cu->qtDepth;
            curBlock[colIdx].type    = cu->type;
            curBlock[colIdx].iLMode  = (UINT8)pu->intraLumaMode;
            curBlock[colIdx].width   = cu->width;
            curBlock[colIdx].height  = cu->height;
            curBlock[colIdx].cuPelX  = (UINT16)cu->cuPelX;
            curBlock[colIdx].cuPelY  = (UINT16)cu->cuPelY;

            curBlock[colIdx].mv[XIN_LIST_0].s64x1 = pu->mv[XIN_LIST_0].s64x1;
            curBlock[colIdx].refIdx[XIN_LIST_0]   = (UINT8)pu->refIdx[XIN_LIST_0];
            curBlock[colIdx].mv[XIN_LIST_1].s64x1 = pu->mv[XIN_LIST_1].s64x1;
            curBlock[colIdx].refIdx[XIN_LIST_1]   = (UINT8)pu->refIdx[XIN_LIST_1];
            curBlock[colIdx].affine               = (UINT8)pu->affine;
            curBlock[colIdx].affineType           = (UINT8)pu->affineType;
            curBlock[colIdx].imvIdx               = (UINT8)pu->imvIdx;
            curBlock[colIdx].bcwIdx               = (UINT8)pu->bcwIdx;
        }

        curBlock += blockSetWidth;

    }

    if (pu->affine)
    {
        subWidth  = pu->affineType == XIN_AFFINE_SBTMVP ? XIN_ATMVP_SUB_BLOCK_SIZE : XIN_AFFINE_SUB_BLOCK_SIZE;
        subHeight = pu->affineType == XIN_AFFINE_SBTMVP ? XIN_ATMVP_SUB_BLOCK_SIZE : XIN_AFFINE_SUB_BLOCK_SIZE;
        subWidth  = subWidth >> lgBlockSize;
        subHeight = subHeight >> lgBlockSize;
        curBlock  = blockMap;

        if (pu->affineType == XIN_AFFINE_SBTMVP)
        {
            for (rowIdx = 0; rowIdx < heightInBlock; rowIdx += subHeight)
            {
                for (colIdx = 0; colIdx < widthInBlock; colIdx += subWidth)
                {
                    Xin266SpanSubMv (
                        curBlock + colIdx,
                        blockSetWidth,
                        modeCtrl->subMv + mvIdx,
                        modeCtrl->subRefIdx + mvIdx,
                        subWidth,
                        subHeight);

                    mvIdx++;
                }

                curBlock += blockSetWidth*subHeight;
            }

        }
        else
        {
            if (pu->refIdx[0] >= 0)
            {
                Xin266FillAffineMotionVector (
                    pu,
                    modeCtrl->affineMv,
                    pu->refIdx,
                    pu->affineType,
                    XIN_LIST_0,
                    secSet->affMvBuf[0][0]);
            }

            if (pu->refIdx[1] >= 0)
            {
                Xin266FillAffineMotionVector (
                    pu,
                    modeCtrl->affineMv,
                    pu->refIdx,
                    pu->affineType,
                    XIN_LIST_1,
                    secSet->affMvBuf[0][1]);
            }

            for (rowIdx = 0; rowIdx < heightInBlock; rowIdx += subHeight)
            {
                for (colIdx = 0; colIdx < widthInBlock; colIdx += subWidth)
                {
                    affineMv[colIdx].mv[0][0] = modeCtrl->affineMv[0][0];
                    affineMv[colIdx].mv[0][1] = modeCtrl->affineMv[0][1];
                    affineMv[colIdx].mv[0][2] = modeCtrl->affineMv[0][2];

                    affineMv[colIdx].mv[1][0] = modeCtrl->affineMv[1][0];
                    affineMv[colIdx].mv[1][1] = modeCtrl->affineMv[1][1];
                    affineMv[colIdx].mv[1][2] = modeCtrl->affineMv[1][2];

                    curBlock[colIdx].mv[XIN_LIST_0].s64x1 = pu->refIdx[XIN_LIST_0] >= 0 ? secSet->affMvBuf[0][XIN_LIST_0][mvIdx].s64x1 : 0;
                    curBlock[colIdx].mv[XIN_LIST_1].s64x1 = pu->refIdx[XIN_LIST_1] >= 0 ? secSet->affMvBuf[0][XIN_LIST_1][mvIdx].s64x1 : 0;
                    curBlock[colIdx].refIdx[XIN_LIST_0]   = pu->refIdx[XIN_LIST_0];
                    curBlock[colIdx].refIdx[XIN_LIST_1]   = pu->refIdx[XIN_LIST_1];

                    mvIdx++;
                }

                curBlock += blockSetWidth*subHeight;
                affineMv += affineMvStride;
            }
        }

    }

    if (modeCtrl->mvRefine)
    {
        subWidth  = XIN_MIN (pu->width, DMVR_MAX_SUB_SIZE);
        subHeight = XIN_MIN (pu->height, DMVR_MAX_SUB_SIZE);
        subWidth  = subWidth >> pictureWrite->lgBlockSize;
        subHeight = subHeight >> pictureWrite->lgBlockSize;

        for (rowIdx = 0; rowIdx < heightInBlock; rowIdx += subHeight)
        {
            for (colIdx = 0; colIdx < widthInBlock; colIdx += subWidth)
            {
                Xin266SpanSubMvd (
                    modeCtrl->mvdL0SubPu + mvIdx,
                    subMvd + colIdx,
                    blockSetWidth,
                    subWidth,
                    subHeight);

                mvIdx++;
            }

            subMvd += blockSetWidth*subHeight;
        }

    }
    else
    {
        for (rowIdx = 0; rowIdx < heightInBlock; rowIdx++)
        {
            for (colIdx = 0; colIdx < widthInBlock; colIdx++)
            {
                subMvd[colIdx].s32x1 = 0;
            }

            subMvd += blockSetWidth;
        }
    }

    if ((pu->width >= 4) && (pu->height >= 4) && (cu->type < XIN_INTRA_MODE) && (!pu->affine))
    {
        neighbourMv.refIdx[0]    = (SINT8)pu->refIdx[0];
        neighbourMv.refIdx[1]    = (SINT8)pu->refIdx[1];
        neighbourMv.mv[0]        = pu->mv[0];
        neighbourMv.mv[1]        = pu->mv[1];
        neighbourMv.useAltHpelIf = pu->imvIdx == XIN_IMV_HPEL;
        neighbourMv.bcwIdx       = pu->bcwIdx;

        Xin266AddMvpCandHmvpLut (
            secSet,
            &neighbourMv);
    }

}

void Xin266CanSplit (
    xin_sec_struct *secSet,
    xin_cu_struct  *cu)
{
    xin_seq_struct  *seqSet;
    xin_mode_struct *modeCtrl;
    SINT32          cuWidth;
    SINT32          cuHeight;
    BOOL            isQtAllowed;
    BOOL            isBtAllowed;
    UINT32          implicitSplit;
    BOOL            isBlInPic;
    BOOL            isTrInPic;
    UINT32          canSplit;
    UINT32          canBtt;
    SINT32          maxBtSize;
    SINT32          minBtSize;
    SINT32          maxTtSize;
    SINT32          minTtSize;
    SINT32          minQtSize;
    UINT32          frameWidth;
    UINT32          frameHeight;
    UINT32          lastSplit;

    seqSet        = secSet->seqSet;
    modeCtrl      = cu->modeCtrl;
    frameHeight   = seqSet->frameHeight;
    frameWidth    = seqSet->frameWidth;
    cuWidth       = cu->width;
    cuHeight      = cu->height;
    canSplit      = XIN_CAN_QUAD_SPLIT | XIN_CAN_HORZ_SPLIT | XIN_CAN_VERT_SPLIT | XIN_CAN_NO_SPLIT | XIN_CAN_TRIH_SPLIT | XIN_CAN_TRIV_SPLIT;
    maxBtSize     = seqSet->config.maxBtSize;
    minBtSize     = seqSet->config.minBtSize;
    maxTtSize     = seqSet->config.maxTtSize;
    minTtSize     = seqSet->config.minTtSize;
    minQtSize     = seqSet->config.minQtSize;
    isBtAllowed   = (cuWidth <= maxBtSize) && (cuHeight <= maxBtSize);
    isQtAllowed   = (cuWidth > minQtSize) && (cuHeight > minQtSize);
    implicitSplit = XIN_CU_NO_SPLIT;
    isBlInPic     = cu->cuPelY + cu->height <= frameHeight;
    isTrInPic     = cu->cuPelX + cu->width <= frameWidth;
    canBtt        = cu->mtDepth < seqSet->config.maxMttDepth;
    lastSplit     = modeCtrl->lastSplit;

    if (cu->treeMask != XIN_CU_TREE_D_MASK)
    {
        cu->canSplit = XIN_CU_NO_SPLIT;

        return;
    }

    if (!isBlInPic && !isTrInPic && isQtAllowed)
    {
        implicitSplit = XIN_CU_QUAD_SPLIT;
    }
    else if (!isBlInPic && isBtAllowed)
    {
        implicitSplit = XIN_CU_HORZ_SPLIT;
    }
    else if (!isTrInPic && isBtAllowed)
    {
        implicitSplit = XIN_CU_VERT_SPLIT;
    }
    else if (!isBlInPic || !isTrInPic)
    {
        implicitSplit = XIN_CU_QUAD_SPLIT;
    }

    if ((!isBlInPic || !isTrInPic) && (cuWidth > XIN_MAX_TU_SIZE || cuHeight > XIN_MAX_TU_SIZE))
    {
        implicitSplit = XIN_CU_QUAD_SPLIT;
    }

    if (lastSplit != XIN_CU_CTU_LEVEL && lastSplit != XIN_CU_QUAD_SPLIT)
    {
        canSplit &= ~XIN_CAN_QUAD_SPLIT;
    }

    if (cuWidth <= minQtSize)
    {
        canSplit &= ~XIN_CAN_QUAD_SPLIT;
    }

    if (implicitSplit != XIN_CU_NO_SPLIT)
    {
        canSplit &= ~(XIN_CAN_HORZ_SPLIT | XIN_CAN_VERT_SPLIT | XIN_CAN_NO_SPLIT | XIN_CAN_TRIH_SPLIT | XIN_CAN_TRIV_SPLIT);

        if (implicitSplit == XIN_CU_HORZ_SPLIT)
        {
            canSplit |= XIN_CAN_HORZ_SPLIT;
        }
        else if (implicitSplit == XIN_CU_VERT_SPLIT)
        {
            canSplit |= XIN_CAN_VERT_SPLIT;
        }
        else if (implicitSplit == XIN_CU_QUAD_SPLIT)
        {
            canSplit |= XIN_CAN_QUAD_SPLIT;
        }
    }
    else
    {
        if (canBtt && (cuWidth <= minBtSize && cuHeight <= minBtSize) && (cuWidth <= minTtSize && cuHeight <= minTtSize))
        {
            canBtt = FALSE;
        }

        if (canBtt && (cuWidth > maxBtSize || cuHeight > maxBtSize) && (cuWidth > maxTtSize || cuHeight > maxTtSize))
        {
            canBtt = FALSE;
        }

        if (!canBtt)
        {
            canSplit &= ~XIN_CAN_VERT_SPLIT;
            canSplit &= ~XIN_CAN_HORZ_SPLIT;
            canSplit &= ~XIN_CAN_TRIV_SPLIT;
            canSplit &= ~XIN_CAN_TRIH_SPLIT;
        }
        else
        {
            if (cuWidth > maxBtSize || cuHeight > maxBtSize)
            {
                canSplit &= ~XIN_CAN_VERT_SPLIT;
                canSplit &= ~XIN_CAN_HORZ_SPLIT;
            }

            if (cuHeight <= minBtSize)
            {
                canSplit &= ~XIN_CAN_HORZ_SPLIT;
            }

            if (cuWidth > XIN_MAX_TU_SIZE && cuHeight <= XIN_MAX_TU_SIZE)
            {
                canSplit &= ~XIN_CAN_HORZ_SPLIT;
            }

            if (cuWidth <= minBtSize)
            {
                canSplit &= ~XIN_CAN_VERT_SPLIT;
            }

            if (cuHeight > XIN_MAX_TU_SIZE && cuWidth <= XIN_MAX_TU_SIZE)
            {
                canSplit &= ~XIN_CAN_VERT_SPLIT;
            }

            if (cuHeight <= 2*minTtSize || cuHeight > maxTtSize || cuWidth > maxTtSize)
            {
                canSplit &= ~XIN_CAN_TRIH_SPLIT;
            }

            if (cuWidth > XIN_MAX_TU_SIZE || cuHeight > XIN_MAX_TU_SIZE)
            {
                canSplit &= ~XIN_CAN_TRIH_SPLIT;
            }

            if (cuWidth <= 2 * minTtSize || cuWidth > maxTtSize || cuHeight > maxTtSize)
            {
                canSplit &= ~XIN_CAN_TRIV_SPLIT;
            }

            if (cuWidth > XIN_MAX_TU_SIZE || cuHeight > XIN_MAX_TU_SIZE)
            {
                canSplit &= ~XIN_CAN_TRIV_SPLIT;
            }

        }

    }

    cu->canSplit = (UINT8)canSplit;

}

void Xin266CalcCuContext (
    xin_sec_struct *secSet,
    xin_cu_struct  *cu)
{
    xin_block_struct *lftBlock;
    xin_block_struct *topBlock;
    UINT32           height;
    UINT32           width;
    UINT32           splitCtx;
    UINT32           qtCtx;
    UINT32           hvCtx;
    UINT32           splitNum;
    UINT32           horNum;
    UINT32           verNum;
    UINT32           lftHeight;
    UINT32           topWidth;
    UINT32           depTop;
    UINT32           depLft;
    UINT32           horBtCtx;
    UINT32           verBtCtx;

    splitCtx  = 0;
    splitNum  = 0;
    height    = cu->height;
    width     = cu->width;
    lftBlock  = secSet->lftTBlock;
    topBlock  = secSet->topLBlock;
    lftHeight = lftBlock->height;
    topWidth  = topBlock->width;

    if (lftHeight < height)
    {
        splitCtx++;
    }

    if (topWidth < width)
    {
        splitCtx++;
    }

    if (cu->canSplit & XIN_CAN_QUAD_SPLIT)
    {
        splitNum += 2;
    }

    if (cu->canSplit & XIN_CAN_HORZ_SPLIT)
    {
        splitNum += 1;
    }

    if (cu->canSplit & XIN_CAN_VERT_SPLIT)
    {
        splitNum += 1;
    }

    if (cu->canSplit & XIN_CAN_TRIH_SPLIT)
    {
        splitNum += 1;
    }

    if (cu->canSplit & XIN_CAN_TRIV_SPLIT)
    {
        splitNum += 1;
    }

    if (splitNum > 0)
    {
        splitNum--;
    }

    splitCtx += 3 * (splitNum >> 1);

    //////////////////////////
    // CTX is qt split (0-5)
    //////////////////////////
    qtCtx  = (lftBlock->qtDepth > cu->qtDepth) ? 1 : 0;
    qtCtx += (topBlock->qtDepth > cu->qtDepth) ? 1 : 0;
    qtCtx += cu->qtDepth < 2 ? 0 : 3;

    ////////////////////////////
    // CTX is ver split (0-4)
    ////////////////////////////
    hvCtx = 0;

    horNum = ((cu->canSplit & XIN_CAN_HORZ_SPLIT) ? 1 : 0) + ((cu->canSplit & XIN_CAN_TRIH_SPLIT) ? 1 : 0);
    verNum = ((cu->canSplit & XIN_CAN_VERT_SPLIT) ? 1 : 0) + ((cu->canSplit & XIN_CAN_TRIV_SPLIT) ? 1 : 0);

    if (verNum == horNum)
    {
        depTop = width / topWidth;
        depLft = height / lftHeight;

        if (depTop == depLft || lftHeight == 255 || topWidth == 255)
        {
            hvCtx = 0;
        }
        else if (depTop < depLft )
        {
            hvCtx = 1;
        }
        else
        {
            hvCtx = 2;
        }
    }
    else if (verNum < horNum)
    {
        hvCtx = 3;
    }
    else
    {
        hvCtx = 4;
    }

    //////////////////////////
    // CTX is h/v bt (0-3)
    //////////////////////////
    horBtCtx = ( cu->mtDepth <= 1 ? 1 : 0 );
    verBtCtx = ( cu->mtDepth <= 1 ? 3 : 2 );

    cu->splitCtx = (UINT8)splitCtx;
    cu->qtCtx    = (UINT8)qtCtx;
    cu->hvCtx    = (UINT8)hvCtx;
    cu->horBtCtx = (UINT8)horBtCtx;
    cu->verBtCtx = (UINT8)verBtCtx;

    cu->skipContext  = (lftBlock->type == XIN_SKIP_MODE) ? 1 : 0;
    cu->skipContext += (topBlock->type == XIN_SKIP_MODE) ? 1 : 0;

    cu->affineContext  = (lftBlock->affine) ? 1 : 0;
    cu->affineContext += (topBlock->affine) ? 1 : 0;

    cu->predModeContext = (lftBlock->type == XIN_INTRA_MODE) || (topBlock->type == XIN_INTRA_MODE) ? 1 : 0;

}

static void Xin266InitTu (
    xin_seq_struct *seqSet,
    xin_tu_struct  **tu,
    UINT32         cuWidth,
    UINT32         cuHeight,
    UINT32         tuWidth,
    UINT32         tuHeight)
{
    UINT32        tuIdx;
    UINT32        rowIdx;
    UINT32        colIdx;
    UINT32        cuWidthInTu;
    UINT32        cuHeightInTu;
    UINT32        partIdx;
    UINT32        lgWidthY;
    UINT32        lgWidthUv;
    UINT32        lgHeightY;
    UINT32        lgHeightUv;
    UINT32        lgCGWidthY;
    UINT32        lgCGHeightY;
    UINT32        lgCGWidthUv;
    UINT32        lgCGHeightUv;
    UINT32        cgWidth;
    UINT32        cgHeight;

    partIdx      = 0;
    cuHeightInTu = cuHeight / tuHeight;
    cuWidthInTu  = cuWidth / tuWidth;
    lgWidthY     = calcLog2[tuWidth];
    lgHeightY    = calcLog2[tuHeight];
    lgWidthUv    = lgWidthY - 1;
    lgHeightUv   = lgHeightY - 1;
    lgCGWidthY   = log2SbbSize[lgWidthY][lgHeightY][0];
    lgCGHeightY  = log2SbbSize[lgWidthY][lgHeightY][1];
    lgCGWidthUv  = log2SbbSize[lgWidthUv][lgHeightUv][0];
    lgCGHeightUv = log2SbbSize[lgWidthUv][lgHeightUv][1];
    cgWidth      = 1 << lgCGWidthY;
    cgHeight     = 1 << lgCGHeightY;

    for (rowIdx = 0; rowIdx < cuHeightInTu; rowIdx++)
    {
        for (colIdx = 0; colIdx < cuWidthInTu; colIdx++)
        {
            tuIdx = rowIdx*cuWidthInTu + colIdx;

            tu[tuIdx]->partIdx     = partIdx;
            tu[tuIdx]->tuIdx       = tuIdx;
            tu[tuIdx]->offsetX     = tuWidth*colIdx;
            tu[tuIdx]->offsetY     = tuHeight*rowIdx;

            tu[tuIdx]->lgWidth[0]  = (UINT8)lgWidthY;
            tu[tuIdx]->lgHeight[0] = (UINT8)lgHeightY;
            tu[tuIdx]->lgWidth[1]  = (UINT8)lgWidthUv;
            tu[tuIdx]->lgHeight[1] = (UINT8)lgHeightUv;

            tu[tuIdx]->lgCGWidth[0]  = (UINT8)lgCGWidthY;
            tu[tuIdx]->lgCGHeight[0] = (UINT8)lgCGHeightY;

            tu[tuIdx]->lgCGWidth[1]  = (UINT8)lgCGWidthUv;
            tu[tuIdx]->lgCGHeight[1] = (UINT8)lgCGHeightUv;

            tu[tuIdx]->scanOrder[0] = seqSet->scanOrder[lgCGHeightY][lgCGWidthY];
            tu[tuIdx]->scanOrder[1] = seqSet->scanOrder[lgCGHeightUv][lgCGWidthUv];

            tu[tuIdx]->scanOrderCG[0] = seqSet->scanOrderCG[lgHeightY][lgWidthY];
            tu[tuIdx]->scanOrderCG[1] = seqSet->scanOrderCG[lgHeightUv][lgWidthUv];

            partIdx += (tuWidth*tuHeight) / (cgWidth*cgHeight);

        }

    }

}

void Xin266ComputeGradient (
    PIXEL    *input,
    intptr_t inputStride,
    SINT32   *cuGrad,
    SINT32   width,
    SINT32   height)
{
    SINT32  colIdx, rowIdx;
    SINT32  horVal;
    SINT32  verVal;
    SINT32  dowVal;
    SINT32  dupVal;

    horVal  = 0;
    verVal  = 0;
    dowVal  = 0;
    dupVal  = 0;

    for (rowIdx = 0; rowIdx < height - 1; rowIdx++)
    {
        for (colIdx = 0; colIdx < width - 1; colIdx++ )
        {
            horVal += XIN_ABS(input[(rowIdx + 0)*inputStride + colIdx + 1] - input[(rowIdx + 0)*inputStride + colIdx + 0]);
            verVal += XIN_ABS(input[(rowIdx + 1)*inputStride + colIdx + 0] - input[(rowIdx + 0)*inputStride + colIdx + 0]);
            dowVal += XIN_ABS(input[(rowIdx + 0)*inputStride + colIdx + 1] - input[(rowIdx + 1)*inputStride + colIdx + 0]);
            dupVal += XIN_ABS(input[(rowIdx + 1)*inputStride + colIdx + 1] - input[(rowIdx + 0)*inputStride + colIdx + 0]);
        }
    }

    cuGrad[XIN_CU_GRAD_HOR] = horVal;
    cuGrad[XIN_CU_GRAD_VER] = verVal;
    cuGrad[XIN_CU_GRAD_DOW] = dowVal;
    cuGrad[XIN_CU_GRAD_DUP] = dupVal;

}

void Xin266CuInit (
    xin_sec_struct *secSet,
    xin_cu_struct  *parentCu,
    UINT32         splitType,
    UINT32         partIdx)
{
    xin_ctu_struct  *ctu;
    xin_cu_struct   *cu;
    xin_mode_struct *modeCtrl;
    xin_seq_struct  *seqSet;
    xin_pic_struct  *picSet;
    xin_lmcs_struct *lmcsSet;
    xin_func_struct *funcSet;
    SINT32          qtDepth;
    SINT32          mtDepth;
    SINT32          cuWidth;
    SINT32          cuHeight;
    SINT32          partNum;
    SINT32          tuWidth;
    SINT32          tuHeight;
    SINT32          tuNum;
    SINT32          maxTrSize;
    SINT32          cuOffX;
    SINT32          cuOffY;
    SINT32          cuPelX;
    SINT32          cuPelY;
    SINT32          ctuSize;
    UINT32          frameWidth;
    UINT32          frameHeight;
    UINT32          parentPartIdx;
    UINT32          parentCuWidth;
    UINT32          parentCuHeight;
    BOOL            presentFlag;
    BOOL            forBiddenFlag;
    BOOL            qtBeforeBt;
    SINT32          tuIdx;

    xin_block_struct *lftBlock;
    xin_block_struct *topBlock;

    if (partIdx > 3)
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "Child index is out of range.\n");
    }

    ctu         = secSet->ctu;
    seqSet      = secSet->seqSet;
    picSet      = secSet->picSet;
    lmcsSet     = picSet->lmcsSet;
    ctuSize     = seqSet->ctuSize;
    frameWidth  = seqSet->frameWidth;
    frameHeight = seqSet->frameHeight;
    maxTrSize   = seqSet->config.maxTrSize;
    partNum     = 1;
    qtDepth     = 0;
    mtDepth     = 0;
    cuPelX      = 0;
    cuPelY      = 0;
    cuOffX      = 0;
    cuOffY      = 0;
    cuWidth     = ctuSize;
    cuHeight    = ctuSize;
    funcSet     = secSet->funcSet;

    if (parentCu)
    {
        parentPartIdx  = parentCu->partIdx;
        parentCuWidth  = parentCu->width;
        parentCuHeight = parentCu->height;
    }
    else
    {
        parentPartIdx  = 0;
        parentCuWidth  = 0;
        parentCuHeight = 0;
    }

    if (parentCu == NULL)
    {
        cuPelX   = ctu->ctuPelX;
        cuPelY   = ctu->ctuPelY;
        partNum  = 1;
    }
    else
    {
        switch (splitType)
        {

        case XIN_CU_QUAD_SPLIT:
            cuWidth  = parentCuWidth >> 1;
            cuHeight = parentCuHeight >> 1;
            qtDepth  = parentCu->qtDepth + 1;
            cuOffX   = (partIdx & 1) ? cuWidth  : 0;
            cuOffY   = (partIdx > 1) ? cuHeight : 0;
            cuOffX  += parentCu->offX;
            cuOffY  += parentCu->offY;
            cuPelX   = ctu->ctuPelX + cuOffX;
            cuPelY   = ctu->ctuPelY + cuOffY;
            partNum  = 1;
            break;

        case XIN_CU_HORZ_SPLIT:
            cuWidth  = parentCuWidth;
            cuHeight = parentCu->height >> 1;
            qtDepth  = parentCu->qtDepth;
            cuOffX   = 0;
            cuOffY   = (partIdx > 0) ? cuHeight : 0;
            cuOffX  += parentCu->offX;
            cuOffY  += parentCu->offY;
            cuPelX   = ctu->ctuPelX + cuOffX;
            cuPelY   = ctu->ctuPelY + cuOffY;
            partNum  = 2;
            mtDepth  = parentCu->mtDepth + 1;

            break;

        case XIN_CU_VERT_SPLIT:
            cuWidth  = parentCuWidth >> 1;
            cuHeight = parentCuHeight;
            qtDepth  = parentCu->qtDepth;
            cuOffX   = (partIdx > 0) ? cuWidth: 0;
            cuOffY   = 0;
            cuOffX  += parentCu->offX;
            cuOffY  += parentCu->offY;
            cuPelX   = ctu->ctuPelX + cuOffX;
            cuPelY   = ctu->ctuPelY + cuOffY;
            partNum  = 2;
            mtDepth  = parentCu->mtDepth + 1;
            break;

        case XIN_CU_TRIH_SPLIT:
            cuWidth  = parentCuWidth;
            cuHeight = (partIdx == 1) ? parentCuHeight>>1 : parentCuHeight>>2;
            qtDepth  = parentCu->qtDepth;
            cuOffX   = 0;
            cuOffY   = (partOffset[splitType][partIdx]*parentCuHeight)>>2;
            cuOffX  += parentCu->offX;
            cuOffY  += parentCu->offY;
            cuPelX   = ctu->ctuPelX + cuOffX;
            cuPelY   = ctu->ctuPelY + cuOffY;
            partNum  = 3;
            mtDepth  = parentCu->mtDepth + 1;
            break;

        case XIN_CU_TRIV_SPLIT:
            cuHeight = parentCuHeight;
            cuWidth  = (partIdx == 1) ? parentCuWidth >> 1 : parentCuWidth >> 2;
            qtDepth  = parentCu->qtDepth;
            cuOffX   = (partOffset[splitType][partIdx] * parentCuWidth) >> 2;
            cuOffY   = 0;
            cuOffX  += parentCu->offX;
            cuOffY  += parentCu->offY;
            cuPelX   = ctu->ctuPelX + cuOffX;
            cuPelY   = ctu->ctuPelY + cuOffY;
            partNum  = 3;
            mtDepth  = parentCu->mtDepth + 1;
            break;

        case XIN_CU_NO_SPLIT:
            cuWidth  = parentCuWidth;
            cuHeight = parentCuHeight;
            qtDepth  = parentCu->qtDepth + 1;
            cuOffX   = parentCu->offX;
            cuOffY   = parentCu->offY;
            cuPelX   = ctu->ctuPelX + cuOffX;
            cuPelY   = ctu->ctuPelY + cuOffY;
            partNum  = 1;
            mtDepth  = parentCu->mtDepth;
            break;

        default:
            break;

        }

    }

    tuWidth  = (cuWidth > maxTrSize) ? maxTrSize : cuWidth;
    tuHeight = (cuHeight > maxTrSize) ? maxTrSize : cuHeight;
    tuNum    = (cuWidth*cuHeight) / (tuWidth*tuHeight);

    if (Xin266GetCuBuf (secSet, &cu))
    {
        _XIN_LOGGER (XIN_LOGGER_ERROR, "Cu buffer allocation is failed.\n");
    }

    for (tuIdx = 0; tuIdx < tuNum; tuIdx++)
    {
        if (Xin266GetTuBuf (secSet, cu->tu + tuIdx))
        {
            _XIN_LOGGER(XIN_LOGGER_ERROR, "Tu buffer allocation is failed.\n");
        }
    }

    cu->cuPelX  = cuPelX;
    cu->cuPelY  = cuPelY;
    cu->qtDepth = (UINT8)qtDepth;
    cu->mtDepth = (UINT8)mtDepth;
    cu->depth   = cu->mtDepth + cu->qtDepth;
    cu->width   = (UINT8)cuWidth;
    cu->height  = (UINT8)cuHeight;
    cu->offX    = cuOffX;
    cu->offY    = cuOffY;

    if (Xin266GetModeBuf(secSet, &modeCtrl))
    {
        _XIN_LOGGER(XIN_LOGGER_ERROR, "Mode control buffer allocation is failed.\n");
    }

    cu->modeCtrl  = modeCtrl;
    cu->lgWidth   = (UINT8)calcLog2[cuWidth];
    cu->lgHeight  = (UINT8)calcLog2[cuHeight];
    cu->geomFlag  = 0;
    cu->parentCu  = (parentCu) ? parentCu : cu;
    cu->partNum   = (UINT8)partNum;
    cu->partIdx   = (UINT16)(parentPartIdx + ((partOffset[splitType][partIdx]*parentCuWidth*parentCuHeight)>>6));
    cu->rootCbf   = TRUE;

    if (parentCu == NULL)
    {
        modeCtrl->continuousSkip = 0;
    }
    else
    {
        modeCtrl->continuousSkip = parentCu->modeCtrl->continuousSkip;
    }

    presentFlag   = (cu->cuPelX < frameWidth) && (cu->cuPelY < frameHeight);
    forBiddenFlag = presentFlag && (cu->cuPelX + cu->width > frameWidth || cu->cuPelY + cu->height > frameHeight);

    if (presentFlag)
    {
        cu->geomFlag |= XIN_CB_PRESENT;
    }

    if (forBiddenFlag)
    {
        cu->geomFlag |= (XIN_CB_FORBIDDEN | XIN_CB_SPLIT);
    }

    secSet->inputCu[0] = secSet->inputCtu[0] + cu->offX + cu->offY*secSet->inputYStride;
    secSet->inputCu[1] = secSet->inputCtu[1] + ((cu->offX + cu->offY*secSet->inputUvStride) >> 1);
    secSet->inputCu[2] = secSet->inputCtu[2] + ((cu->offX + cu->offY*secSet->inputUvStride) >> 1);

    secSet->reconCu[0] = secSet->reconCtu[0] + cu->offX + cu->offY*secSet->reconYStride;
    secSet->reconCu[1] = secSet->reconCtu[1] + ((cu->offX + cu->offY*secSet->reconUvStride) >> 1);
    secSet->reconCu[2] = secSet->reconCtu[2] + ((cu->offX + cu->offY*secSet->reconUvStride) >> 1);

    secSet->reshapeCu[0] = (seqSet->config.enableLmcs && lmcsSet->lmcsParam.sliceReshaperEnabled) ? secSet->reshapeCtuY + cu->offX + cu->offY*secSet->inputYStride : secSet->inputCu[0];
    secSet->reshapeCu[1] = secSet->inputCu[1];
    secSet->reshapeCu[2] = secSet->inputCu[2];

    secSet->minMv.mv.mvX = (SINT16)(-1 * cuPelX - ctuSize - (XIN_PADDING_OFFSET_X - 8));
    secSet->minMv.mv.mvY = (SINT16)(-1 * cuPelY - ctuSize - (XIN_PADDING_OFFSET_Y - 8));
    secSet->maxMv.mv.mvX = (SINT16)(frameWidth + (ctuSize - cu->width + XIN_PADDING_OFFSET_X - 8) - cuPelX);
    secSet->maxMv.mv.mvY = (SINT16)(frameHeight + (ctuSize - cu->height + XIN_PADDING_OFFSET_Y - 8) - cuPelY);

    secSet->meData.refIdx[0] = -1;
    secSet->meData.refIdx[1] = -1;

    cu->bestBuf   = NULL;
    cu->type      = XIN_INVALID_MODE;
    cu->splitType = XIN_CU_NO_SPLIT;
    cu->partType  = (UINT8)((splitType > 0) ? (splitType - 1) : splitType);
    cu->treeMask  = ((cuWidth < 8) || (cuHeight*cuWidth < 16) || ((splitType == XIN_CU_TRIV_SPLIT) && (parentCuWidth == 16) && (parentCuHeight == 16))) ? XIN_CU_TREE_L_MASK : XIN_CU_TREE_D_MASK;
    cu->treeMask  = (splitType == XIN_CU_NO_SPLIT) ? XIN_CU_TREE_C_MASK : cu->treeMask;
    cu->type      = (cu->treeMask == XIN_CU_TREE_C_MASK) ? XIN_INTRA_MODE : XIN_INVALID_MODE;
    cu->tuNum     = tuNum;
    cu->canSplit  = 0;

    modeCtrl->skipIntra    = FALSE;
    modeCtrl->refIdx[0]    = -1;
    modeCtrl->refIdx[1]    = -1;

    modeCtrl->sseCost      = XIN_MAX_U64_COST;
    modeCtrl->sadCost      = XIN_MAX_U64_COST;
    modeCtrl->lastSplit    = (UINT8)splitType;
    modeCtrl->didHorzSplit = FALSE;
    modeCtrl->didVertSplit = FALSE;
    modeCtrl->bestHorzCost = XIN_MAX_U64_COST;
    modeCtrl->bestVertCost = XIN_MAX_U64_COST;
    modeCtrl->bestBufIdx   = ((splitType == XIN_CU_QUAD_SPLIT) || (splitType == XIN_CU_NO_SPLIT)) ? 0 : parentCu->modeCtrl->bestBufIdx ^ 1;

    cu->lgTuWidth[0]  = (UINT8)(calcLog2[tuWidth]);
    cu->lgTuWidth[1]  = (UINT8)(calcLog2[tuWidth] - 1);
    cu->lgTuHeight[0] = (UINT8)(calcLog2[tuHeight]);
    cu->lgTuHeight[1] = (UINT8)(calcLog2[tuHeight] - 1);
    cu->pu.height     = (UINT8)cuHeight;
    cu->pu.width      = (UINT8)cuWidth;
    cu->pu.lgHeight   = (UINT8)calcLog2[cuHeight];
    cu->pu.lgWidth    = (UINT8)calcLog2[cuWidth];

    Xin266InitTu (
        seqSet,
        cu->tu,
        cuWidth,
        cuHeight,
        tuWidth,
        tuHeight);

    secSet->cu = cu;
    ctu->cu    = (parentCu == NULL) ? cu : ctu->cu;

    Xin266CanSplit (
        secSet,
        cu);

    Xin266GetBlockAvail (
        secSet,
        &cu->pu);

    Xin266CalcCuContext (
        secSet,
        cu);

    if ((seqSet->config.enableLmcs) && (lmcsSet->lmcsParam.enableChromaAdj))
    {
        Xin266CalcChromaAdjVpdu (
            secSet,
            &secSet->chromaResScaleInv);
    }

    if (splitType == XIN_CU_QUAD_SPLIT)
    {
        memcpy (
            secSet->hmvpLutBuf[qtDepth],
            secSet->hmvpLut,
            XIN_MAX_HMVP_CAND_NUM*sizeof(xin_neighbour_mv));

        secSet->hmvpNumBuf[qtDepth] = secSet->hmvpNum;
    }

    lftBlock   = secSet->lftTBlock->type != XIN_INVALID_MODE ? secSet->lftTBlock : NULL;
    topBlock   = secSet->lftTBlock->type != XIN_INVALID_MODE ? secSet->lftTBlock : NULL;
    qtBeforeBt = ((lftBlock  &&  topBlock  && lftBlock->qtDepth > cu->qtDepth && topBlock->qtDepth > cu->qtDepth)
                  || (lftBlock  && !topBlock  && lftBlock ->qtDepth > cu->qtDepth)
                  || (!lftBlock &&  topBlock  && topBlock->qtDepth > cu->qtDepth )
                  || (!topBlock && !lftBlock  && cu->width >= ( 32 << picSet->depth)))
                 && (cu->width > (seqSet->config.minQtSize << 1));

    modeCtrl->qtBeforeBt   = (UINT8)qtBeforeBt;
    modeCtrl->doMoreSplits = 3;

    if ((cu->width >= 32) && (cu->height >= 32) && (cu->width == cu->height) && (seqSet->config.maxMttDepth) && (seqSet->config.gradientFastQtbt))
    {
        funcSet->pfXinComputeGradient[cu->lgWidth] (
            secSet->inputCu[PLANE_LUMA],
            secSet->inputYStride,
            modeCtrl->cuGrad,
            cu->width,
            cu->height);
    }

}

void Xin266CuInfoUpdate (
    xin_sec_struct *secSet,
    xin_cu_struct  *cu)
{
    xin_ctu_struct  *ctu;
    xin_seq_struct  *seqSet;
    xin_pic_struct  *picSet;
    xin_mode_struct *modeCtrl;
    UINT32          rowIdx;
    UINT32          colIdx;

    ctu      = secSet->ctu;
    modeCtrl = cu->modeCtrl;
    seqSet   = secSet->seqSet;
    picSet   = secSet->picSet;
    rowIdx   = cu->lgHeight - 2;
    colIdx   = cu->lgWidth - 2;

    ctu->vaildCuCount += (cu->type != XIN_SKIP_MODE);

    if (seqSet->config.cuModeQuit)
    {
        ctu->sadCost[rowIdx][colIdx] += modeCtrl->sadCost;
    }

    if (seqSet->config.cuDepthQuit)
    {
        ctu->sseCost[rowIdx][colIdx] += modeCtrl->sseCost;
    }

    if ((cu->type <= XIN_INTER_MODE) && (seqSet->config.enableSkipMe))
    {
        ctu->interSad[rowIdx][colIdx]   += modeCtrl->sad;
        ctu->interCount[rowIdx][colIdx] += 1;
    }

    ctu->cuCount[rowIdx][colIdx] += 1;

}

void Xin266CuPostInit (
    xin_sec_struct *secSet,
    xin_cu_struct  *cu)
{
    xin_seq_struct  *seqSet;
    xin_tu_struct   *tu;
    xin_pu_struct   *pu;
    xin_mode_struct *modeCtrl;
    xin_prob_model  *context;
    UINT32          tuIdx;
    UINT32          partIdx;
    intptr_t        lumaStride;
    intptr_t        chromaStride;
    xin_full_md_buf *fullBuf;
    xin_fast_md_buf *fastBuf;
    UINT32          mtsIdxY;
    UINT32          mtsIdxU;
    UINT32          mtsIdxV;
    UINT32          cbfY;
    UINT32          cbfU;
    UINT32          cbfV;
    UINT32          unplitBits;
    UINT32          subWidth;
    UINT32          subHeight;
    UINT32          mvNum;

    fastBuf      = cu->bestBuf;
    fullBuf      = fastBuf->fullBuf;
    seqSet       = secSet->seqSet;
    context      = secSet->cabacSet->context;
    pu           = &cu->pu;
    modeCtrl     = cu->modeCtrl;
    cu->type     = (UINT8)fastBuf->type;
    cu->tuNum    = fullBuf->tuNum;
    cu->rootCbf  = (UINT8)fullBuf->rootCbf;
    lumaStride   = fullBuf->coeffStride[0];
    chromaStride = fullBuf->coeffStride[1];
    mtsIdxY      = (cu->treeMask & XIN_CU_TREE_L_MASK) ? fullBuf->mtsIdx[0] : 0;
    mtsIdxU      = (cu->treeMask & XIN_CU_TREE_C_MASK) ? fullBuf->mtsIdx[1] : 0;
    mtsIdxV      = (cu->treeMask & XIN_CU_TREE_C_MASK) ? fullBuf->mtsIdx[2] : 0;
    cbfY         = (cu->treeMask & XIN_CU_TREE_L_MASK) ? fullBuf->yuvCbf[mtsIdxY][0] : 0;
    cbfU         = (cu->treeMask & XIN_CU_TREE_C_MASK) ? fullBuf->yuvCbf[mtsIdxU][1] : 0;
    cbfV         = (cu->treeMask & XIN_CU_TREE_C_MASK) ? fullBuf->yuvCbf[mtsIdxV][2] : 0;

    modeCtrl->sseCost   = fullBuf->sseCost;
    modeCtrl->sadCost   = fastBuf->sadCost;
    modeCtrl->sad       = fastBuf->sad;
    modeCtrl->sse       = fullBuf->sse;
    modeCtrl->rate      = fullBuf->rate;
    modeCtrl->refIdx[0] = secSet->meData.refIdx[0];
    modeCtrl->refIdx[1] = secSet->meData.refIdx[1];

    for (tuIdx = 0; tuIdx < cu->tuNum; tuIdx++)
    {
        tu       = cu->tu[tuIdx];
        tu->yCbf = (UINT8)((cbfY >> tuIdx) & 1);
        tu->uCbf = (UINT8)((cbfU >> tuIdx) & 1);
        tu->vCbf = (UINT8)((cbfV >> tuIdx) & 1);

        tu->nzCGMapRs[PLANE_LUMA]     = tu->yCbf ? fullBuf->nzCGMapRs[mtsIdxY][tuIdx][PLANE_LUMA] : 0;
        tu->nzCGMapRs[PLANE_CHROMA_U] = tu->uCbf ? fullBuf->nzCGMapRs[mtsIdxU][tuIdx][PLANE_CHROMA_U] : 0;
        tu->nzCGMapRs[PLANE_CHROMA_V] = tu->vCbf ? fullBuf->nzCGMapRs[mtsIdxV][tuIdx][PLANE_CHROMA_V] : 0;

        tu->nzCGMapEs[PLANE_LUMA]     = tu->yCbf ? fullBuf->nzCGMapEs[mtsIdxY][tuIdx][PLANE_LUMA] : 0;
        tu->nzCGMapEs[PLANE_CHROMA_U] = tu->uCbf ? fullBuf->nzCGMapEs[mtsIdxU][tuIdx][PLANE_CHROMA_U] : 0;
        tu->nzCGMapEs[PLANE_CHROMA_V] = tu->vCbf ? fullBuf->nzCGMapEs[mtsIdxV][tuIdx][PLANE_CHROMA_V] : 0;

        partIdx = cu->partIdx + tu->partIdx;

        tu->qCoeff[PLANE_LUMA]     = fullBuf->qCoefBuf[mtsIdxY][PLANE_LUMA] + (cu->offX + tu->offsetX) + (cu->offY + tu->offsetY)*lumaStride;
        tu->qCoeff[PLANE_CHROMA_U] = fullBuf->qCoefBuf[mtsIdxU][PLANE_CHROMA_U] + (((cu->offX + tu->offsetX) + (cu->offY + tu->offsetY)*chromaStride)>>1);
        tu->qCoeff[PLANE_CHROMA_V] = fullBuf->qCoefBuf[mtsIdxV][PLANE_CHROMA_V] + (((cu->offX + tu->offsetX) + (cu->offY + tu->offsetY)*chromaStride)>>1);

        tu->coeffStride[PLANE_LUMA]   = lumaStride;
        tu->coeffStride[PLANE_CHROMA] = chromaStride;

        tu->gt0BitMap[PLANE_LUMA]     = fullBuf->gt0BitMap[mtsIdxY][PLANE_LUMA] + partIdx;
        tu->gt0BitMap[PLANE_CHROMA_U] = fullBuf->gt0BitMap[mtsIdxU][PLANE_CHROMA_U] + (partIdx>>2);
        tu->gt0BitMap[PLANE_CHROMA_V] = fullBuf->gt0BitMap[mtsIdxV][PLANE_CHROMA_V] + (partIdx>>2);

        tu->mtsIdx[PLANE_LUMA]     = fullBuf->mtsIdx[PLANE_LUMA];
        tu->mtsIdx[PLANE_CHROMA_U] = fullBuf->mtsIdx[PLANE_CHROMA_U];
        tu->mtsIdx[PLANE_CHROMA_V] = fullBuf->mtsIdx[PLANE_CHROMA_V];

    }

    if (cu->type <= XIN_INTER_MODE)
    {
        pu->mergeFlag      = fastBuf->mergeFlag;
        pu->mergeIndex     = (UINT8)fastBuf->mergeIndex;
        pu->affine         = fastBuf->mergeFlag && fastBuf->affine;
        pu->imvIdx         = (UINT8)fastBuf->imvIdx;
        pu->bcwIdx         = (UINT8)fastBuf->bcwIdx;
        pu->affineType     = pu->affine ? (UINT8)fastBuf->affineType : 0;
        modeCtrl->mvRefine = fastBuf->mergeFlag && fastBuf->mvRefine;

        pu->mvpIndex[XIN_LIST_0] = fastBuf->mvpIndex[XIN_LIST_0];
        pu->mvpIndex[XIN_LIST_1] = fastBuf->mvpIndex[XIN_LIST_1];
        pu->refIdx[XIN_LIST_0]   = fastBuf->refIdx[XIN_LIST_0];
        pu->refIdx[XIN_LIST_1]   = fastBuf->refIdx[XIN_LIST_1];
        pu->mv[XIN_LIST_0].s64x1 = pu->refIdx[XIN_LIST_0] >= 0 ? fastBuf->mv[XIN_LIST_0].s64x1 : 0;
        pu->mv[XIN_LIST_1].s64x1 = pu->refIdx[XIN_LIST_1] >= 0 ? fastBuf->mv[XIN_LIST_1].s64x1 : 0;
        pu->predMv[XIN_LIST_0]   = fastBuf->predMv[XIN_LIST_0];
        pu->predMv[XIN_LIST_1]   = fastBuf->predMv[XIN_LIST_1];

        if (modeCtrl->mvRefine)
        {
            subWidth  = XIN_MIN (cu->width,  DMVR_MAX_SUB_SIZE);
            subHeight = XIN_MIN (cu->height, DMVR_MAX_SUB_SIZE);
            mvNum     = (cu->width*cu->height) / (subWidth*subHeight);

            memcpy (modeCtrl->mvdL0SubPu, fastBuf->mvdL0SubPu, sizeof(xin_mv_u)*mvNum);
        }
        else if ((pu->affineType != XIN_AFFINE_SBTMVP) && (pu->affine))
        {
            modeCtrl->affineMv[0][0] = fastBuf->affineMv[0][0];
            modeCtrl->affineMv[0][1] = fastBuf->affineMv[0][1];
            modeCtrl->affineMv[0][2] = fastBuf->affineMv[0][2];
            modeCtrl->affineMv[1][0] = fastBuf->affineMv[1][0];
            modeCtrl->affineMv[1][1] = fastBuf->affineMv[1][1];
            modeCtrl->affineMv[1][2] = fastBuf->affineMv[1][2];
        }

    }
    else
    {
        pu->mergeFlag      = 0;
        pu->mergeIndex     = 0;
        pu->affine         = 0;
        pu->affineType     = 0;
        pu->imvIdx         = 0;
        modeCtrl->mvRefine = 0;

        pu->refIdx[XIN_LIST_0]       = -1;
        pu->refIdx[XIN_LIST_1]       = -1;
        pu->mvpIndex[XIN_LIST_0]     = 0;
        pu->mvpIndex[XIN_LIST_1]     = 0;
        pu->mv[XIN_LIST_0].s64x1     = 0;
        pu->mv[XIN_LIST_1].s64x1     = 0;
        pu->predMv[XIN_LIST_0].s64x1 = 0;
        pu->predMv[XIN_LIST_1].s64x1 = 0;
        pu->intraLumaMode            = fastBuf->intraLumaMode;
        pu->intraChromaMode          = fastBuf->intraChromaMode;

    }

    Xin266EstimateSplitType (
        context,
        FALSE,
        cu,
        XIN_CU_NO_SPLIT,
        &unplitBits);

    modeCtrl->sseCost        += CALC_SSE_COST(secSet->sseLambda[PLANE_LUMA], unplitBits);
    modeCtrl->bestUnsplitCost = modeCtrl->sseCost;

    if (cu->type == XIN_SKIP_MODE)
    {
        modeCtrl->doMoreSplits--;
        modeCtrl->continuousSkip++;
    }
    else
    {
        modeCtrl->continuousSkip = 0;
    }

}

void Xin266ConstructPictureRead (
    xin_pic_struct  *picSet,
    xin_ref_picture *pictureWrite)
{
    xin_rps_struct  *rps;
    xin_seq_struct  *seqSet;
    xin_ref_picture *pictureRead;
    UINT32          predIdxInGop;
    SINT32          refIdx;
    UINT32          dpbIdx;
    UINT32          readIdx;
    SINT32          targetPoc;
    SINT32          anchorPoc;
    SINT32          deltaPos[XIN_MAX_REF_FRAMES];
    UINT32          refPicNum;
    UINT32          unfoldFrame;
    BOOL            foundIFrame;

    seqSet  = picSet->seqSet;
    rps     = pictureWrite->rps;

    memset (deltaPos,                        0, sizeof(SINT32)*XIN_MAX_REF_FRAMES);
    memset (picSet->pictureRead[XIN_LIST_0], 0, sizeof(xin_ref_picture *)*XIN_MAX_REF_FRAMES);
    memset (picSet->pictureRead[XIN_LIST_1], 0, sizeof(xin_ref_picture *)*XIN_MAX_REF_FRAMES);

    readIdx      = 0;
    pictureRead  = NULL;
    anchorPoc    = pictureWrite->framePoc;
    refPicNum    = 0;
    foundIFrame  = FALSE;
    predIdxInGop = pictureWrite->predIdxInGop;

    pictureWrite->useRpsInSps[XIN_LIST_0] = TRUE;
    pictureWrite->useRpsInSps[XIN_LIST_1] = TRUE;

    for (refIdx = 0; refIdx < rps->numOfNegPics; refIdx++)
    {
        targetPoc   = anchorPoc - rps->deltaNegPos[refIdx];
        pictureRead = NULL;

        if ((targetPoc >= 0) && (foundIFrame != TRUE))
        {
            for (dpbIdx = 0; dpbIdx < seqSet->dpbSize; dpbIdx++)
            {
                if (targetPoc == seqSet->dpbQueue[dpbIdx]->framePoc)
                {
                    pictureRead = seqSet->dpbQueue[dpbIdx];

                    break;
                }
            }

            if (pictureRead == NULL)
            {
                _XIN_LOGGER (XIN_LOGGER_ERROR, "Fail to find refernece picture.\n");
            }
            else
            {
                if (rps->usedByNegPicFlag[refIdx])
                {
                    picSet->pictureRead[XIN_LIST_0][readIdx] = pictureRead;
                    readIdx++;
                }

                anchorPoc = pictureRead->framePoc;
                refPicNum++;
            }

            pictureWrite->refFramePoc[XIN_LIST_0][refIdx] = pictureRead->framePoc;

            foundIFrame = (pictureRead->frameType >= XIN_I_FRAME);

        }

    }

    pictureWrite->numOfRefs[XIN_LIST_0] = readIdx;
    pictureWrite->numOfNegPics          = rps->numOfNegPics;
    pictureWrite->refPicNum[XIN_LIST_0] = refPicNum;

    readIdx     = 0;
    pictureRead = NULL;
    anchorPoc   = pictureWrite->framePoc;
    refPicNum   = 0;

    for (refIdx = 0; refIdx < rps->numOfPosPics; refIdx++)
    {
        targetPoc   = anchorPoc + rps->deltaPosPos[refIdx];
        pictureRead = NULL;

        if ((targetPoc >= 0) && (targetPoc + (SINT32)seqSet->predGopSize >= seqSet->craFramePoc))
        {
            for (dpbIdx = 0; dpbIdx < seqSet->dpbSize; dpbIdx++)
            {
                if (targetPoc == seqSet->dpbQueue[dpbIdx]->framePoc)
                {
                    pictureRead = seqSet->dpbQueue[dpbIdx];

                    break;
                }
            }

            if (pictureRead == NULL)
            {
                _XIN_LOGGER (XIN_LOGGER_ERROR, "Fail to find refernece picture.\n");
            }
            else
            {
                if (rps->usedByPosPicFlag[refIdx])
                {
                    picSet->pictureRead[XIN_LIST_1][readIdx] = pictureRead;
                    readIdx++;
                }

                anchorPoc = pictureRead->framePoc;
                refPicNum++;
            }

            pictureWrite->refFramePoc[XIN_LIST_1][refIdx] = pictureRead->framePoc;

        }

    }

    pictureWrite->numOfRefs[XIN_LIST_1] = readIdx;
    pictureWrite->numOfPosPics          = rps->numOfPosPics;
    pictureWrite->refPicNum[XIN_LIST_1] = refPicNum;

    memcpy (
        picSet->pictureRef,
        picSet->pictureRead[XIN_LIST_0],
        sizeof(xin_ref_picture *)*pictureWrite->numOfRefs[XIN_LIST_0]);

    memcpy (
        picSet->pictureRef + pictureWrite->numOfRefs[XIN_LIST_0],
        picSet->pictureRead[XIN_LIST_1],
        sizeof(xin_ref_picture *)*pictureWrite->numOfRefs[XIN_LIST_1]);

    picSet->validRefFrame = pictureWrite->numOfRefs[XIN_LIST_0] + pictureWrite->numOfRefs[XIN_LIST_1];

    if ((!predIdxInGop) && (pictureWrite->frameType == XIN_B_FRAME))
    {
        memcpy (
            picSet->pictureRead[XIN_LIST_1],
            picSet->pictureRead[XIN_LIST_0],
            sizeof(xin_ref_picture *)*XIN_MAX_REF_FRAMES);

        memcpy (
            pictureWrite->refFramePoc[XIN_LIST_1],
            pictureWrite->refFramePoc[XIN_LIST_0],
            sizeof(UINT32)*XIN_MAX_REF_FRAMES);

        pictureWrite->numOfRefs[XIN_LIST_1]   = pictureWrite->numOfRefs[XIN_LIST_0];
        pictureWrite->refPicNum[XIN_LIST_1]   = pictureWrite->refPicNum[XIN_LIST_0];
        pictureWrite->useRpsInSps[XIN_LIST_1] = pictureWrite->useRpsInSps[XIN_LIST_0];

        memcpy (
            rps->deltaPosPos,
            rps->deltaNegPos,
            sizeof(SINT32)*pictureWrite->numOfRefs[XIN_LIST_0]);

        picSet->validRefFrame += pictureWrite->numOfRefs[XIN_LIST_0];

    }
    else if ((seqSet->config.unfoldRefList0) && (pictureWrite->frameType == XIN_B_FRAME))
    {
        if (pictureWrite->numOfRefs[XIN_LIST_1] < seqSet->config.refFrameNum)
        {
            unfoldFrame = XIN_MIN (seqSet->config.refFrameNum - pictureWrite->numOfRefs[XIN_LIST_1], pictureWrite->numOfRefs[XIN_LIST_0]);

            memcpy (
                picSet->pictureRead[XIN_LIST_1] + pictureWrite->numOfRefs[XIN_LIST_1],
                picSet->pictureRead[XIN_LIST_0],
                sizeof(xin_ref_picture *)*unfoldFrame);

            memcpy (
                pictureWrite->refFramePoc[XIN_LIST_1] + pictureWrite->numOfRefs[XIN_LIST_1],
                pictureWrite->refFramePoc[XIN_LIST_0],
                sizeof(UINT32)*unfoldFrame);

            pictureWrite->numOfRefs[XIN_LIST_1] += unfoldFrame;
        }
    }

    memset (&pictureWrite->rpl[XIN_LIST_0], 0, sizeof(xin_rpl_struct));
    memset (&pictureWrite->rpl[XIN_LIST_1], 0, sizeof(xin_rpl_struct));

    pictureWrite->rpl[XIN_LIST_0].numOfPics = pictureWrite->refPicNum[XIN_LIST_0];
    pictureWrite->rpl[XIN_LIST_1].numOfPics = pictureWrite->refPicNum[XIN_LIST_1];

    for (refIdx = 0; refIdx < pictureWrite->rpl[XIN_LIST_0].numOfPics; refIdx++)
    {
        deltaPos[refIdx] = pictureWrite->framePoc - pictureWrite->refFramePoc[XIN_LIST_0][refIdx];
    }

    pictureWrite->rpl[XIN_LIST_0].deltaPos[0] = deltaPos[0];

    for (refIdx = 1; refIdx < pictureWrite->rpl[XIN_LIST_0].numOfPics; refIdx++)
    {
        pictureWrite->rpl[XIN_LIST_0].deltaPos[refIdx] = deltaPos[refIdx] - deltaPos[refIdx - 1];
    }

    for (refIdx = 0; refIdx < pictureWrite->rpl[XIN_LIST_1].numOfPics; refIdx++)
    {
        deltaPos[refIdx] = pictureWrite->framePoc - pictureWrite->refFramePoc[XIN_LIST_1][refIdx];
    }

    pictureWrite->rpl[XIN_LIST_1].deltaPos[0] = deltaPos[0];

    for (refIdx = 1; refIdx < pictureWrite->rpl[XIN_LIST_1].numOfPics; refIdx++)
    {
        pictureWrite->rpl[XIN_LIST_1].deltaPos[refIdx] = deltaPos[refIdx] - deltaPos[refIdx - 1];
    }

    pictureWrite->useRpsInSps[XIN_LIST_0] = memcmp (seqSet->rplList[XIN_LIST_0] + predIdxInGop, &pictureWrite->rpl[XIN_LIST_0], sizeof(xin_rpl_struct));
    pictureWrite->useRpsInSps[XIN_LIST_0] = !pictureWrite->useRpsInSps[XIN_LIST_0];
    pictureWrite->useRpsInSps[XIN_LIST_1] = memcmp (seqSet->rplList[XIN_LIST_1] + predIdxInGop, &pictureWrite->rpl[XIN_LIST_1], sizeof(xin_rpl_struct));
    pictureWrite->useRpsInSps[XIN_LIST_1] = !pictureWrite->useRpsInSps[XIN_LIST_1];

}

static void Xin266SetSearchRange (
    xin_pic_struct  *picSet,
    xin_ref_picture *pictureWrite)
{
    xin_seq_struct *seqSet;
    SINT32         listIdx;
    SINT32         refIdx;
    SINT32         deltaPoc;
    SINT32         gopSize;
    SINT32         offset;
    UINT32         maxSr;

    seqSet  = picSet->seqSet;
    gopSize = seqSet->predGopSize;
    offset  = gopSize >> 1;
    maxSr   = seqSet->config.searchRange;

    for (listIdx = 0; listIdx < XIN_LIST_NUM; listIdx++)
    {
        for (refIdx = 0; refIdx < XIN_MAX_REF_FRAMES; refIdx++)
        {
            picSet->adaSearchRange[listIdx][refIdx] = maxSr;
        }

        if (seqSet->config.bFrameNum)
        {
            for (refIdx = 0; refIdx < pictureWrite->rpl[listIdx].numOfPics; refIdx++)
            {
                deltaPoc = pictureWrite->framePoc - pictureWrite->refFramePoc[listIdx][refIdx];

                picSet->adaSearchRange[listIdx][refIdx] = XIN_CLIP ((maxSr*XIN_ABS(deltaPoc)+offset)/gopSize, 64, maxSr);
            }
        }
    }

}

void Xin266ContructPictureRps (
    xin_pic_struct  *picSet,
    xin_ref_picture *pictureWrite,
    xin_rps_struct  *inputRps)
{
    xin_seq_struct *seqSet;
    xin_rps_struct *outputRps;
    SINT32         refNum;
    SINT32         refNum0;
    SINT32         refIdx;
    SINT32         refNumExt;
    SINT32         validNumExt;
    SINT32         refNumNeg;
    SINT32         refNumPos;
    SINT32         valRefNum;

    seqSet     = picSet->seqSet;
    outputRps  = pictureWrite->rps;
    refNum     = seqSet->config.refFrameNum;
    refNum     = (seqSet->config.bFrameNum >= 15) ? XIN_MIN (refNum, 5) : refNum;
    refNum0    = seqSet->config.bFrameNum ? XIN_REF_PRED0_NUM : refNum;
    refNum0    = XIN_MAX (refNum, refNum0);
    refNum     = pictureWrite->isReferenced ? refNum : XIN_MIN (2, refNum);

    if (pictureWrite->frameType == XIN_IDR_FRAME)
    {
        memset (seqSet->gopSizeBuf, 0, sizeof(SINT8)*XIN_MAX_REF_FRAMES);
    }
    else if (pictureWrite->predIdxInGop == 0)
    {
        memmove (seqSet->gopSizeBuf + 1, seqSet->gopSizeBuf, sizeof(SINT8)*(XIN_MAX_REF_FRAMES - 1));

        seqSet->gopSizeBuf[0] = (SINT8)pictureWrite->predGopSize;
    }

    memcpy (outputRps, inputRps, sizeof(xin_rps_struct));

    if (pictureWrite->frameType != XIN_IDR_FRAME)
    {
        if (pictureWrite->predIdxInGop == 0)
        {
            refNumNeg   = inputRps->numOfNegPics;
            refNumExt   = refNum0 - refNumNeg;
            validNumExt = 0;

            for (refIdx = 0; refIdx < refNumExt; refIdx++)
            {
                outputRps->deltaNegPos[refIdx + refNumNeg]      = seqSet->gopSizeBuf[refIdx + 1];
                outputRps->usedByNegPicFlag[refIdx + refNumNeg] = seqSet->gopSizeBuf[refIdx + 1] != 0;

                validNumExt += outputRps->usedByNegPicFlag[refIdx + refNumNeg];
            }

            outputRps->numOfNegPics = refNumNeg + validNumExt;

            if (pictureWrite->frameType == XIN_I_FRAME)
            {
                memset (outputRps->usedByNegPicFlag, 0, sizeof(BOOL)*XIN_MAX_REF_FRAMES);
            }

        }
        else
        {
            // List 0
            refNumNeg = XIN_MIN ((SINT32)inputRps->numOfNegPics, refNum);
            refNumPos = (pictureWrite->frameType == XIN_B_FRAME) ? XIN_MIN ((SINT32)inputRps->numOfPosPics, refNum) : 0;

            for (refIdx = 0; refIdx < refNumNeg; refIdx++)
            {
                outputRps->usedByNegPicFlag[refIdx] = TRUE;
            }

            refNumNeg   = inputRps->numOfNegPics;
            refNumExt   = refNum - refNumNeg;
            refNumExt   = XIN_MAX (refNumExt, XIN_MAX(refNum0 - 2, 0));
            refNumExt   = XIN_MIN (refNumExt, XIN_MAX_REF_FRAMES - refNumNeg);
            validNumExt = 0;
            valRefNum   = XIN_MIN (refNumNeg + refNumExt, XIN_MAX_DPB_FRAMES - refNumPos);
            valRefNum   = XIN_MIN (valRefNum, refNum);

            for (refIdx = 0; refIdx < refNumExt; refIdx++)
            {
                outputRps->deltaNegPos[refIdx + refNumNeg]      = seqSet->gopSizeBuf[refIdx + 1];
                outputRps->usedByNegPicFlag[refIdx + refNumNeg] = (seqSet->gopSizeBuf[refIdx + 1] != 0) && ((refNumNeg + refIdx) < valRefNum);

                validNumExt += (seqSet->gopSizeBuf[refIdx + 1] != 0);
            }

            outputRps->numOfNegPics = refNumNeg + validNumExt;

            // List 1
            for (refIdx = 0; refIdx < refNumPos; refIdx++)
            {
                outputRps->usedByPosPicFlag[refIdx] = TRUE;
            }

        }

    }

}

static void Xin266CalcQpOffet (
    xin_pic_struct    *picSet)
{
    xin_seq_struct    *seqSet;
    xin_ref_picture   *pictureWrite;
    xin_input_picture *inputPicture;
    UINT32            unitX, unitY;
    UINT32            ctuPelX, ctuPelY;
    UINT32            ctuAddr;
    UINT32            unitIdx;
    UINT32            laUnitSize;
    double            totalQpOffset;

    seqSet        = picSet->seqSet;
    laUnitSize    = seqSet->laUnitSize;
    inputPicture  = picSet->inputPicture;
    pictureWrite  = picSet->pictureWrite;
    totalQpOffset = 0.0;

    memset (pictureWrite->qpOffset, 0, seqSet->frameWidthInCtu*seqSet->frameHeightInCtu*sizeof(double));
    memset (pictureWrite->qpNum,    0, seqSet->frameWidthInCtu*seqSet->frameHeightInCtu*sizeof(UINT8));

    if (pictureWrite->isReferenced == FALSE)
    {
        pictureWrite->avgQpOffset = 0;

        return;
    }

    for (unitY = 0; unitY < inputPicture->laHgtInUnit; unitY++)
    {
        for (unitX = 0; unitX < inputPicture->laWdtInUnit; unitX++)
        {
            unitIdx = unitX + unitY*inputPicture->laWdtInUnit;
            ctuPelX = unitX*laUnitSize*2;
            ctuPelY = unitY*laUnitSize*2;
            ctuAddr = (ctuPelX >> seqSet->lgCtuSize) + (ctuPelY >> seqSet->lgCtuSize)*seqSet->frameWidthInCtu;

            pictureWrite->qpOffset[ctuAddr] += inputPicture->qpOffset[unitIdx];
            pictureWrite->qpNum[ctuAddr]++;
        }
    }

    for (ctuAddr = 0; ctuAddr < seqSet->frameSizeInCtu; ctuAddr++)
    {
        pictureWrite->qpOffset[ctuAddr] = pictureWrite->qpNum[ctuAddr] ? pictureWrite->qpOffset[ctuAddr] / pictureWrite->qpNum[ctuAddr] : 0;

        totalQpOffset += pictureWrite->qpOffset[ctuAddr];
    }

    pictureWrite->avgQpOffset = XIN_ABS (totalQpOffset / seqSet->frameSizeInCtu);

}

static void Xin266AlfInit (
    xin_pic_struct *picSet)
{
    SINT32          ctuIdx;
    SINT32          classIdx;
    xin_alf_struct  *alfSet;

    alfSet       = picSet->alfSet;
    alfSet->ctu  = picSet->ctu;

    memset (alfSet->alfCovarianceY, 0, alfSet->frameSizeInCtu*XIN_ALF_MAX_CLS_NUM*sizeof(xin_alf_cov));
    memset (alfSet->alfCovarianceU, 0, alfSet->frameSizeInCtu*sizeof(xin_alf_cov));
    memset (alfSet->alfCovarianceV, 0, alfSet->frameSizeInCtu*sizeof(xin_alf_cov));

    memset (alfSet->alfCovarianceCcAlf[0], 0, alfSet->frameSizeInCtu*sizeof(xin_alf_cov));
    memset (alfSet->alfCovarianceCcAlf[1], 0, alfSet->frameSizeInCtu*sizeof(xin_alf_cov));

    memset (alfSet->ccAlfFilterControl[0], 0, alfSet->frameSizeInCtu*sizeof(UINT8));
    memset (alfSet->ccAlfFilterControl[1], 0, alfSet->frameSizeInCtu*sizeof(UINT8));

    alfSet->alfEnabled[0] = FALSE;
    alfSet->alfEnabled[1] = FALSE;
    alfSet->alfEnabled[2] = FALSE;

    alfSet->ccAlfCbEnabled = FALSE;
    alfSet->ccAlfCrEnabled = FALSE;

    for (ctuIdx = 0; ctuIdx < alfSet->frameSizeInCtu; ctuIdx++)
    {
        for (classIdx = 0; classIdx < XIN_ALF_MAX_CLS_NUM; classIdx++)
        {
            alfSet->alfCovarianceY[ctuIdx*XIN_ALF_MAX_CLS_NUM + classIdx].numBins  = alfSet->useNonLinearAlfLuma ? XIN_ALF_CLIP_NUM : 1;
            alfSet->alfCovarianceY[ctuIdx*XIN_ALF_MAX_CLS_NUM + classIdx].numCoeff = XIN_ALF_MAX_LUMA_COEF_NUM;
        }
    }

    for (ctuIdx = 0; ctuIdx < alfSet->frameSizeInCtu; ctuIdx++)
    {
        alfSet->alfCovarianceU[ctuIdx].numBins  = alfSet->useNonLinearAlfChroma ? XIN_ALF_CLIP_NUM : 1;
        alfSet->alfCovarianceU[ctuIdx].numCoeff = XIN_ALF_MAX_CHROMA_COEF_NUM;

        alfSet->alfCovarianceV[ctuIdx].numBins  = alfSet->useNonLinearAlfChroma ? XIN_ALF_CLIP_NUM : 1;
        alfSet->alfCovarianceV[ctuIdx].numCoeff = XIN_ALF_MAX_CHROMA_COEF_NUM;
    }

    for (ctuIdx = 0; ctuIdx < alfSet->frameSizeInCtu; ctuIdx++)
    {
        alfSet->alfCovarianceCcAlf[0][ctuIdx].numBins  = 1;
        alfSet->alfCovarianceCcAlf[0][ctuIdx].numCoeff = XIN_CC_ALF_MAX_COEFF_NUM;

        alfSet->alfCovarianceCcAlf[1][ctuIdx].numBins  = 1;
        alfSet->alfCovarianceCcAlf[1][ctuIdx].numCoeff = XIN_CC_ALF_MAX_COEFF_NUM;
    }

    for (classIdx = 0; classIdx < XIN_CC_ALF_MAX_FILTER_NUM; classIdx++)
    {
        alfSet->alfCovarianceFrameCcAlf[0][classIdx].numBins  = 1;
        alfSet->alfCovarianceFrameCcAlf[0][classIdx].numCoeff = XIN_CC_ALF_MAX_COEFF_NUM;

        alfSet->alfCovarianceFrameCcAlf[1][classIdx].numBins  = 1;
        alfSet->alfCovarianceFrameCcAlf[1][classIdx].numCoeff = XIN_CC_ALF_MAX_COEFF_NUM;
    }

}

void Xin266FramePreInit (
    xin_pic_struct    *picSet,
    xin_input_picture *inputPicture,
    xin_ref_picture   *pictureWrite)
{
    xin_seq_struct  *seqSet;
    xin_rps_struct  *rps;
    xin_rc_context  *rcContext;
    UINT32          predGopIdx;
    BOOL            mvdL1Zero;
    UINT32          rowIdx;

    seqSet     = picSet->seqSet;
    predGopIdx = inputPicture->predGopIdx;
    rps        = &inputPicture->rps;
    mvdL1Zero  = (inputPicture->frameType == XIN_B_FRAME) && (predGopIdx == 0);
    rcContext  = &picSet->rcContext;

    seqSet->encLastIdr = (inputPicture->frameType == XIN_IDR_FRAME) ? inputPicture->inputNumber : seqSet->encLastIdr;

    pictureWrite->framePoc        = inputPicture->inputNumber - seqSet->encLastIdr;
    pictureWrite->gopIdx          = pictureWrite->framePoc / seqSet->predGopSize;
    pictureWrite->isReferenced    = rps->isRefFrame;
    pictureWrite->predIdxInGop    = predGopIdx;
    pictureWrite->predGopSize     = inputPicture->predGopSize;
    pictureWrite->temporalId      = rps->temporalId;
    pictureWrite->frameType       = inputPicture->frameType;
    pictureWrite->mvdL1Zero       = mvdL1Zero;

    Xin266ContructPictureRps (
        picSet,
        pictureWrite,
        rps);

    pictureWrite->inputBuf[PLANE_LUMA]     = inputPicture->inputBuf[PLANE_LUMA];
    pictureWrite->inputBuf[PLANE_CHROMA_U] = inputPicture->inputBuf[PLANE_CHROMA_U];
    pictureWrite->inputBuf[PLANE_CHROMA_V] = inputPicture->inputBuf[PLANE_CHROMA_V];

    pictureWrite->inputStride[PLANE_LUMA]   = inputPicture->inputStride[PLANE_LUMA];
    pictureWrite->inputStride[PLANE_CHROMA] = inputPicture->inputStride[PLANE_CHROMA];

    pictureWrite->checkLDC  = (seqSet->config.bFrameNum) ? (!predGopIdx) : TRUE;
    picSet->pictureWrite    = pictureWrite;
    picSet->temporalId      = pictureWrite->temporalId;
    picSet->depth           = pictureWrite->temporalId;
    seqSet->craFramePoc     = (pictureWrite->frameType >= XIN_I_FRAME) ? pictureWrite->framePoc : seqSet->craFramePoc;
    picSet->picQp           = seqSet->config.qp;

    for (rowIdx = 0; rowIdx < seqSet->tileNum; rowIdx++)
    {
        picSet->cuListIdx[rowIdx]  = 0;
        picSet->tuListIdx[rowIdx]  = 0;
    }

    if (seqSet->config.enableAlf && pictureWrite->isReferenced)
    {
        picSet->enableAlf = seqSet->config.alfMode > 1 ? (picSet->depth <= 1) : TRUE;
    }
    else
    {
        picSet->enableAlf = FALSE;
    }

    picSet->enableCcAlf = picSet->enableAlf;
    picSet->offlineMode = seqSet->config.offlineMode;

    if (seqSet->config.offlineMode)
    {
        picSet->offlineMode  = picSet->enableAlf;
        picSet->offlineMode |= (!seqSet->config.enableTiles) && (!seqSet->config.enableWpp);
    }

    if (pictureWrite->frameType == XIN_IDR_FRAME)
    {
        pictureWrite->nalType = NAL_UNIT_CODED_SLICE_IDR_W_RADL;
    }
    else if (pictureWrite->frameType == XIN_I_FRAME)
    {
        pictureWrite->nalType = NAL_UNIT_CODED_SLICE_CRA;
    }
    else
    {
        pictureWrite->nalType = (pictureWrite->framePoc < seqSet->craFramePoc) ? NAL_UNIT_CODED_SLICE_RASL : NAL_UNIT_CODED_SLICE_TRAIL;
    }

    if (pictureWrite->frameType <= XIN_I_FRAME)
    {
        Xin266ConstructPictureRead (
            picSet,
            pictureWrite);

        Xin266SetSearchRange (
            picSet,
            pictureWrite);
    }

    // Reset block type to an invalid value
    memset (
        pictureWrite->blockSetMap,
        XIN_INVALID_MODE,
        pictureWrite->blockSetSize*sizeof(xin_block_struct));

    if (!seqSet->config.disableDeblock)
    {
        memset (
            pictureWrite->horBs,
            0,
            pictureWrite->heightInBlock * pictureWrite->widthInBlock);

        memset (
            pictureWrite->verBs,
            0,
            pictureWrite->heightInBlock * pictureWrite->widthInBlock);

        memset (
            pictureWrite->cbfMap,
            0,
            pictureWrite->heightInBlock * pictureWrite->widthInBlock);
    }

    if (seqSet->config.enableAffine)
    {
        memset (
            picSet->affineMvMap,
            0,
            sizeof(xin_affine_mv)*seqSet->blockSetHeight*seqSet->blockSetWidth);
    }

    picSet->saoEnabledFlag[0] = seqSet->config.enableSao;
    picSet->saoEnabledFlag[1] = seqSet->config.enableSao;
    picSet->codingFrame       = TRUE;
    picSet->inputPicture      = inputPicture;

    if (seqSet->config.lookAhead)
    {
        Xin266CalcQpOffet (
            picSet);
    }

    rcContext->rcSet        = seqSet->rcSet;
    rcContext->isReferenced = pictureWrite->isReferenced;
    rcContext->pictureInput = inputPicture;
    rcContext->avgQpOffset  = pictureWrite->avgQpOffset;

    if (picSet->enableAlf)
    {
        Xin266AlfInit (
            picSet);
    }

    inputPicture->bufStage = XIN_BUF_ENCODE;

}

void Xin266SectionInit (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu)
{
    xin_seq_struct    *seqSet;
    xin_pic_struct    *picSet;
    xin_ref_picture   *pictureWrite;
    xin_cabac_context *cabacSet;
    BOOL              resetContext;
    UINT32            frameType;
    UINT32            sectionIdx;

    seqSet       = secSet->seqSet;
    picSet       = secSet->picSet;
    sectionIdx   = secSet->sectionIdx;
    pictureWrite = picSet->pictureWrite;
    cabacSet     = picSet->cabacSet[sectionIdx];
    resetContext = ((!seqSet->config.enableWpp) || (seqSet->config.enableTiles)) || (!sectionIdx);
    frameType    = XIN_MIN (pictureWrite->frameType, XIN_I_FRAME);

    secSet->qp       = picSet->picQp;
    secSet->tileDim  = seqSet->tileDim + sectionIdx;
    secSet->cabacSet = cabacSet;
    secSet->hmvpNum  = (ctu->ctuX == 0) ? 0 : picSet->hmvpNum[ctu->ctuY];
    secSet->hmvpLut  = picSet->hmvpLut[ctu->ctuY];
    secSet->refQp    = picSet->ctuRowRefQp[ctu->ctuY];

    if (seqSet->config.enableDepQuant)
    {
        secSet->depQuant->context = cabacSet->context;
    }

    if (ctu->ctuX == 0)
    {
        Xin266InitBitstream (
            &cabacSet->cabac.bitstream);

        Xin266CabacContextInit (
            cabacSet,
            seqSet->cabacContext,
            frameType,
            secSet->qp,
            resetContext);
    }

    secSet->saoTopBuf[PLANE_LUMA]     = picSet->saoTopBuf[PLANE_LUMA];
    secSet->saoTopBuf[PLANE_CHROMA_U] = picSet->saoTopBuf[PLANE_CHROMA_U];
    secSet->saoTopBuf[PLANE_CHROMA_V] = picSet->saoTopBuf[PLANE_CHROMA_V];

}

static void CalcFramePsnr (
    xin_ref_picture *pictureWrite,
    double          *psnrYuv)
{

    Xin26xCalcPsnr (
        pictureWrite->inputBuf[PLANE_LUMA],
        pictureWrite->inputStride[PLANE_LUMA],
        pictureWrite->refBuf[PLANE_LUMA],
        pictureWrite->refStride[PLANE_LUMA],
        pictureWrite->inputWidth,
        pictureWrite->inputHeight,
        psnrYuv);

    Xin26xCalcPsnr (
        pictureWrite->inputBuf[PLANE_CHROMA_U],
        pictureWrite->inputStride[PLANE_CHROMA],
        pictureWrite->refBuf[PLANE_CHROMA_U],
        pictureWrite->refStride[PLANE_CHROMA],
        pictureWrite->inputWidth/2,
        pictureWrite->inputHeight/2,
        psnrYuv+1);

    Xin26xCalcPsnr (
        pictureWrite->inputBuf[PLANE_CHROMA_V],
        pictureWrite->inputStride[PLANE_CHROMA],
        pictureWrite->refBuf[PLANE_CHROMA_V],
        pictureWrite->refStride[PLANE_CHROMA],
        pictureWrite->inputWidth/2,
        pictureWrite->inputHeight/2,
        psnrYuv+2);

}

void Xin266FrameProInit (
    xin_pic_struct *picSet)
{
    xin_seq_struct    *seqSet;
    xin_ref_picture   *pictureWrite;
    xin_lmcs_struct   *lmcsSet;
    xin_bs_struct     *bitstream;
    xin_lb_struct     *outputBuf;
    UINT32            tileIdx;
    UINT32            outByteSize;
    UINT32            headerByte;
    UINT32            anchByte;
    xin_alf_aps       *alfAps;

    seqSet       = picSet->seqSet;
    lmcsSet      = picSet->lmcsSet;
    pictureWrite = picSet->pictureWrite;
    bitstream    = picSet->bitstream;
    outputBuf    = seqSet->outputBuf;
    outByteSize  = outputBuf->index;
    headerByte   = 0;
    alfAps       = picSet->alfSet->alfAps;

    // Encode parameter sets, if this frame is IDR.
    if (pictureWrite->frameType == XIN_IDR_FRAME)
    {
        anchByte = outputBuf->index;

        Xin266InitBitstream (
            bitstream);

        Xin266WriteSps (
            bitstream,
            seqSet);

        Xin266OutputBitToLinearBuffer (
            bitstream,
            outputBuf,
            TRUE);

        Xin266InitBitstream (
            bitstream);

        Xin266WritePps (
            bitstream,
            seqSet);

        Xin266OutputBitToLinearBuffer (
            bitstream,
            outputBuf,
            TRUE);

        headerByte += outputBuf->index - anchByte;

    }

    // Packet bitstream and send it out
    if (picSet->enableAlf && (alfAps->alfParam.newFilterFlag[0] || alfAps->alfParam.newFilterFlag[1] || alfAps->ccAlfParam.newCcAlfFilter[0] || alfAps->ccAlfParam.newCcAlfFilter[1]))
    {
        Xin266InitBitstream (
            bitstream);

        Xin266WriteApsAlf (
            bitstream,
            picSet);

        Xin266OutputBitToLinearBuffer (
            bitstream,
            outputBuf,
            TRUE);
    }

    if (seqSet->config.enableLmcs && lmcsSet->lmcsParam.sliceReshaperEnabled && lmcsSet->lmcsParam.lmcsParamChanged)
    {
        Xin266InitBitstream (
            bitstream);

        Xin266WriteApsLmcs (
            bitstream,
            picSet);

        Xin266OutputBitToLinearBuffer (
            bitstream,
            outputBuf,
            TRUE);

        lmcsSet->lmcsParam.lmcsParamChanged = FALSE;
    }

    if (seqSet->config.enableWpp)
    {
        for (tileIdx = 0; tileIdx < seqSet->tileNum; tileIdx++)
        {
            if (tileIdx == 0)
            {
                anchByte = outputBuf->index;

                Xin266InitBitstream (
                    bitstream);

                // Write slice header
                Xin266WriteSliceHeader (
                    seqSet->tileDim[0].firstRsCtu,
                    picSet->picQp,
                    bitstream,
                    picSet);

                Xin266OutputBitToLinearBuffer (
                    bitstream,
                    outputBuf,
                    TRUE);

                headerByte += outputBuf->index - anchByte;

            }

            Xin266OutputBitToLinearBuffer (
                &(picSet->cabacSet[tileIdx]->cabac.bitstream),
                outputBuf,
                FALSE);

        }

    }
    else if (!seqSet->config.enableTiles)
    {
        anchByte = outputBuf->index;

        Xin266InitBitstream (
            bitstream);

        // Write slice header
        Xin266WriteSliceHeader (
            seqSet->tileDim[0].firstRsCtu,
            picSet->picQp,
            bitstream,
            picSet);

        Xin266OutputBitToLinearBuffer (
            bitstream,
            outputBuf,
            TRUE);

        headerByte += outputBuf->index - anchByte;

        Xin266OutputBitToLinearBuffer (
            &(picSet->cabacSet[0]->cabac.bitstream),
            outputBuf,
            FALSE);

    }
    else
    {
        for (tileIdx = 0; tileIdx < seqSet->tileNum; tileIdx++)
        {
            anchByte = outputBuf->index;

            Xin266InitBitstream (
                bitstream);

            // Write slice header
            Xin266WriteSliceHeader (
                seqSet->tileDim[tileIdx].firstRsCtu,
                picSet->picQp,
                bitstream,
                picSet);

            Xin266OutputBitToLinearBuffer (
                bitstream,
                outputBuf,
                TRUE);

            headerByte += outputBuf->index - anchByte;

            Xin266OutputBitToLinearBuffer (
                &(picSet->cabacSet[tileIdx]->cabac.bitstream),
                outputBuf,
                FALSE);

        }

    }

    outByteSize = outputBuf->index - outByteSize;

    if (seqSet->config.calcPsnr)
    {
        seqSet->psnrYuv[PLANE_LUMA]     += pictureWrite->psnrYuv[PLANE_LUMA];
        seqSet->psnrYuv[PLANE_CHROMA_U] += pictureWrite->psnrYuv[PLANE_CHROMA_U];
        seqSet->psnrYuv[PLANE_CHROMA_V] += pictureWrite->psnrYuv[PLANE_CHROMA_V];
    }

}

void Xin266ProFrame (
    xin_pic_struct *picSet)
{
    xin_seq_struct    *seqSet;
    xin_sec_struct    *secSet;
    xin_func_struct   *funcSet;
    xin_ref_picture   *pictureWrite;
    UINT32            tileBitSize;
    UINT32            frameBitSize;
    UINT32            rowIdx;
    UINT32            colIdx;

    seqSet       = picSet->seqSet;
    funcSet      = picSet->funcSet;
    pictureWrite = picSet->pictureWrite;
    frameBitSize = 0;

    if (seqSet->config.enableWpp)
    {
        if (picSet->offlineMode)
        {
            for (rowIdx = 0; rowIdx < seqSet->frameHeightInCtu; rowIdx++)
            {
                picSet->ctuRowRefQp[rowIdx] = picSet->picQp;
            }
        }

        for (rowIdx = 0; rowIdx < seqSet->frameHeightInCtu; rowIdx++)
        {
            if (picSet->offlineMode)
            {
                secSet             = picSet->secSet;
                secSet->picSet     = picSet;
                secSet->seqSet     = seqSet;
                secSet->sectionIdx = rowIdx;

                Xin266WriteSection (
                    secSet);
            }

            Xin266BitstreamSize (
                &(picSet->cabacSet[rowIdx]->cabac.bitstream),
                &tileBitSize);

            // Correct entry point offset
            picSet->entryPointOffset[rowIdx] = tileBitSize - 1;

            frameBitSize += tileBitSize;

        }

    }
    else if (!seqSet->config.enableTiles)
    {
        if (picSet->offlineMode)
        {
            for (rowIdx = 0; rowIdx < seqSet->frameHeightInCtu; rowIdx++)
            {
                picSet->ctuRowRefQp[rowIdx] = picSet->picQp;
            }
        }

        if (picSet->offlineMode)
        {
            secSet             = picSet->secSet;
            secSet->picSet     = picSet;
            secSet->seqSet     = seqSet;
            secSet->sectionIdx = 0;

            Xin266WriteSection (
                secSet);
        }

        Xin266BitstreamSize (
            &(picSet->cabacSet[0]->cabac.bitstream),
            &tileBitSize);

        // Correct entry point offset
        picSet->entryPointOffset[0] = tileBitSize - 1;

        frameBitSize += tileBitSize;

    }

    Xin266RcUpdatePic (
        picSet,
        frameBitSize*8,
        200);

    // Extend picture frame, if current frame
    // is used for reference.
    if (pictureWrite->isReferenced)
    {
        Xin26xExtendPicture (
            pictureWrite->refBuf[PLANE_LUMA],
            pictureWrite->refStride[PLANE_LUMA],
            pictureWrite->lumaWidth,
            pictureWrite->lumaHeight,
            pictureWrite->paddingWidth,
            pictureWrite->paddingHeight);

        Xin26xExtendPicture (
            pictureWrite->refBuf[PLANE_CHROMA_U],
            pictureWrite->refStride[PLANE_CHROMA],
            pictureWrite->lumaWidth / 2,
            pictureWrite->lumaHeight / 2,
            pictureWrite->paddingWidth / 2,
            pictureWrite->paddingHeight / 2);

        Xin26xExtendPicture (
            pictureWrite->refBuf[PLANE_CHROMA_V],
            pictureWrite->refStride[PLANE_CHROMA],
            pictureWrite->lumaWidth / 2,
            pictureWrite->lumaHeight / 2,
            pictureWrite->paddingWidth / 2,
            pictureWrite->paddingHeight / 2);

        if ((seqSet->config.motionSearchMode == XIN_ME_HIER_SEARCH) && ((picSet->temporalId + 1 != seqSet->maxTemporalId) || (seqSet->maxTemporalId == 1)))
        {
            funcSet->pfXinDownscale2x2 (
                pictureWrite->refBuffer,
                pictureWrite->refStride[PLANE_LUMA],
                FALSE,
                FALSE,
                pictureWrite->ref1Buffer,
                pictureWrite->ref1Stride,
                (pictureWrite->lumaWidth + 2 * pictureWrite->paddingWidth) / 2,
                (pictureWrite->lumaHeight + 2 * pictureWrite->paddingHeight) / 2);


            funcSet->pfXinDownscale2x2 (
                pictureWrite->ref1Buffer,
                pictureWrite->ref1Stride,
                FALSE,
                FALSE,
                pictureWrite->ref2Buffer,
                pictureWrite->ref2Stride,
                (pictureWrite->lumaWidth + 2 * pictureWrite->paddingWidth) / 4,
                (pictureWrite->lumaHeight + 2 * pictureWrite->paddingHeight) / 4);

        }

        if (seqSet->config.enableDmvr)
        {
            for (rowIdx = 0; rowIdx < pictureWrite->heightInBlock; rowIdx++)
            {
                for (colIdx = 0; colIdx < pictureWrite->widthInBlock; colIdx++)
                {
                    if (picSet->subMvdMap[rowIdx*pictureWrite->blockSetWidth + colIdx].s32x1)
                    {
                        pictureWrite->blockSetMap[rowIdx*pictureWrite->blockSetWidth + colIdx].mv[XIN_LIST_0].mv.mv32X += picSet->subMvdMap[rowIdx*pictureWrite->blockSetWidth + colIdx].mv.mvX;
                        pictureWrite->blockSetMap[rowIdx*pictureWrite->blockSetWidth + colIdx].mv[XIN_LIST_0].mv.mv32Y += picSet->subMvdMap[rowIdx*pictureWrite->blockSetWidth + colIdx].mv.mvY;

                        pictureWrite->blockSetMap[rowIdx*pictureWrite->blockSetWidth + colIdx].mv[XIN_LIST_1].mv.mv32X -= picSet->subMvdMap[rowIdx*pictureWrite->blockSetWidth + colIdx].mv.mvX;
                        pictureWrite->blockSetMap[rowIdx*pictureWrite->blockSetWidth + colIdx].mv[XIN_LIST_1].mv.mv32Y -= picSet->subMvdMap[rowIdx*pictureWrite->blockSetWidth + colIdx].mv.mvY;
                    }
                }
            }
        }

    }

    if (seqSet->config.calcPsnr)
    {
        CalcFramePsnr (
            pictureWrite,
            pictureWrite->psnrYuv);
    }

}

void Xin266PreFrame (
    xin_pic_struct *picSet)
{
    xin_seq_struct  *seqSet;
    xin_ref_picture *pictureWrite;
    UINT32          predGopIdx;
    UINT32          tileIdx;

    seqSet       = picSet->seqSet;
    pictureWrite = picSet->pictureWrite;
    predGopIdx   = pictureWrite->predIdxInGop;

    Xin266RcSkipDec (
        picSet);

    if (picSet->codingFrame)
    {
        if (predGopIdx == 0)
        {
            Xin266RcGop (
                picSet,
                (pictureWrite->frameType == XIN_IDR_FRAME) ? 1 : seqSet->predGopSize);
        }

        Xin266RcPic (
            picSet);
    }
    else
    {
        Xin266RcUpdateSkip (
            picSet);
    }

    for (tileIdx = 0; tileIdx < seqSet->tileNum + 1; tileIdx++)
    {
        picSet->ctuRowRefQp[tileIdx]  = picSet->picQp;
    }

    if (seqSet->config.enableLmcs)
    {
        Xin266LmcsPreAnalyzer (
            picSet,
            XIN_RESHAPE_SIGNAL_SDR);
    }

}

