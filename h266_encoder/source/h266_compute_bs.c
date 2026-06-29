/***************************************************************************//**
 *
 * @file          h266_compute_bs.c
 * @brief         h266 loop filter boundary strength computation.
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
#include "h266_section_struct.h"

#define CHECK_MV_DIFF(MV0, MV1) ((XIN_ABS(MV0.mv.mv32X - MV1.mv.mv32X) >= 8) || (XIN_ABS(MV0.mv.mv32Y - MV1.mv.mv32Y) >= 8))

static void Xin266SetEdgefilterTu (
    xin_sec_struct *secSet,
    xin_cu_struct  *cu,
    UINT8          *horBs,
    UINT8          *verBs)
{
    UINT32          tuIdx;
    UINT32          idx;
    UINT8           *tuHorBs;
    UINT8           *tuVerBs;
    xin_tu_struct   *tu;
    xin_ref_picture *pictureWrite;
    SINT32          widthInBlock;
    SINT32          heightInBlock;
    UINT8           *cbfMap;
    UINT8           *curCbf;
    UINT32          blockIdx;
    UINT32          tuWidthInBlock;
    UINT32          tuHeightInBlock;
    UINT32          lgBlockSize;
    UINT8           yuvCbf;
    UINT32          width;
    UINT32          height;

    pictureWrite  = secSet->picSet->pictureWrite;
    widthInBlock  = pictureWrite->widthInBlock;
    heightInBlock = pictureWrite->heightInBlock;
    cbfMap        = pictureWrite->cbfMap;
    lgBlockSize   = pictureWrite->lgBlockSize;

    PEL_XY_TO_BLOCK_INDEX(cu->cuPelX, cu->cuPelY, blockIdx, widthInBlock, lgBlockSize);

    cbfMap += blockIdx;

    for (tuIdx = 0; tuIdx < cu->tuNum; tuIdx++)
    {
        tu = cu->tu[tuIdx];

        PEL_XY_TO_BLOCK_INDEX(tu->offsetX, tu->offsetY, blockIdx, widthInBlock, lgBlockSize);

        curCbf  = cbfMap + blockIdx;
        tuHorBs = horBs + (tu->offsetY >> lgBlockSize)*widthInBlock + (tu->offsetX >> lgBlockSize);
        width   = 1 << tu->lgWidth[PLANE_LUMA];

        tuWidthInBlock = width >> lgBlockSize;

        // Horizontal Edge
        if (tu->offsetY)
        {
            for (idx = 0; idx < tuWidthInBlock; idx++)
            {
                yuvCbf = curCbf[-widthInBlock] | curCbf[0];

                if (yuvCbf)
                {
                    tuHorBs[idx] |= (yuvCbf & 0x01) << 0;
                    tuHorBs[idx] |= (yuvCbf & 0x02) << 1;
                    tuHorBs[idx] |= (yuvCbf & 0x04) << 2;
                }

                curCbf++;
            }
        }

        curCbf  = cbfMap + blockIdx;
        tuVerBs = verBs + (tu->offsetX >> lgBlockSize)*heightInBlock + (tu->offsetY >> lgBlockSize);
        height  = 1 << tu->lgHeight[PLANE_LUMA];

        tuHeightInBlock = height >> lgBlockSize;

        // Vertical Edge
        if (tu->offsetX)
        {
            for (idx = 0; idx < tuHeightInBlock; idx++)
            {
                yuvCbf = curCbf[-1] | curCbf[0];

                if (yuvCbf)
                {
                    tuVerBs[idx] |= (yuvCbf & 0x01) << 0;
                    tuVerBs[idx] |= (yuvCbf & 0x02) << 1;
                    tuVerBs[idx] |= (yuvCbf & 0x04) << 2;
                }

                curCbf += widthInBlock;
            }
        }

    }

}

static void Xin266SetEdgefilterCu (
    xin_sec_struct *secSet,
    xin_cu_struct  *cu,
    UINT8          *horBs,
    UINT8          *verBs)
{
    UINT32           horIdx;
    UINT32           verIdx;
    UINT32           cuWidthInBlock;
    UINT32           cuHeightInBlock;
    UINT32           blockIdx;
    UINT8            *cbfMap;
    xin_block_struct *blockMap;
    xin_ref_picture  *pictureWrite;
    UINT32           lgBlockSize;
    UINT8            *curCbf;
    xin_block_struct *curBlock;
    SINT32           widthInBlock;
    SINT32           blockSetWidth;
    UINT32           frameType;
    SINT8            refIdxP0;
    SINT8            refIdxQ0;
    SINT8            refIdxP1;
    SINT8            refIdxQ1;
    UINT32           refPocP0;
    UINT32           refPocQ0;
    UINT32           refPocP1;
    UINT32           refPocQ1;
    UINT8            yuvCbf;
    UINT32           subBlockSize;

    pictureWrite    = secSet->picSet->pictureWrite;
    widthInBlock    = pictureWrite->widthInBlock;
    blockSetWidth   = pictureWrite->blockSetWidth;
    lgBlockSize     = pictureWrite->lgBlockSize;
    cuWidthInBlock  = cu->width >> lgBlockSize;
    cuHeightInBlock = cu->height >> lgBlockSize;
    blockMap        = pictureWrite->blockSetMap;
    cbfMap          = pictureWrite->cbfMap;
    frameType       = pictureWrite->frameType;
    subBlockSize    = 8;

    if (cu->type == XIN_INTRA_MODE)
    {
        if ((secSet->topCtu != NULL) || (cu->offY != 0))
        {
            memset (horBs, 0x2A, sizeof(UINT8)*cuWidthInBlock);
        }

        if ((secSet->lftCtu != NULL) || (cu->offX != 0))
        {
            memset (verBs, 0x2A, sizeof(UINT8)*cuHeightInBlock);
        }

        return;
    }

    PEL_XY_TO_BLOCK_INDEX(cu->cuPelX, cu->cuPelY, blockIdx, widthInBlock, lgBlockSize);
    cbfMap   += blockIdx;

    PEL_XY_TO_BLOCK_INDEX(cu->cuPelX, cu->cuPelY, blockIdx, blockSetWidth, lgBlockSize);
    blockMap += blockIdx;

    // Horizontal Edge
    curCbf   = cbfMap;
    curBlock = blockMap;

    if ((secSet->topCtu != NULL) || (cu->offY != 0))
    {
        for (horIdx = 0; horIdx < cuWidthInBlock; horIdx++)
        {
            yuvCbf = curCbf[-widthInBlock] | curCbf[0];

            if (curBlock[-blockSetWidth].type == XIN_INTRA_MODE)
            {
                horBs[horIdx] = 0x2A;
            }
            else if (yuvCbf)
            {
                horBs[horIdx] |= (yuvCbf & 0x01) << 0;
                horBs[horIdx] |= (yuvCbf & 0x02) << 1;
                horBs[horIdx] |= (yuvCbf & 0x04) << 2;
            }

            if (!(horBs[horIdx] & 0x03))
            {
                if (frameType == XIN_P_FRAME)
                {
                    if (CHECK_MV_DIFF(curBlock[-blockSetWidth].mv[XIN_LIST_0], curBlock[0].mv[XIN_LIST_0]))
                    {
                        horBs[horIdx] |= 0x01;
                    }
                    else if (curBlock[-blockSetWidth].refIdx[XIN_LIST_0] != curBlock[0].refIdx[XIN_LIST_0])
                    {
                        horBs[horIdx] |= 0x01;
                    }
                }
                else
                {
                    refIdxP0 = curBlock[-blockSetWidth].refIdx[XIN_LIST_0];
                    refIdxP1 = curBlock[-blockSetWidth].refIdx[XIN_LIST_1];
                    refIdxQ0 = curBlock[0].refIdx[XIN_LIST_0];
                    refIdxQ1 = curBlock[0].refIdx[XIN_LIST_1];

                    refPocP0 = refIdxP0 >= 0 ? pictureWrite->refFramePoc[XIN_LIST_0][refIdxP0] : 0x80000000;
                    refPocP1 = refIdxP1 >= 0 ? pictureWrite->refFramePoc[XIN_LIST_1][refIdxP1] : 0x80000000;
                    refPocQ0 = refIdxQ0 >= 0 ? pictureWrite->refFramePoc[XIN_LIST_0][refIdxQ0] : 0x80000000;
                    refPocQ1 = refIdxQ1 >= 0 ? pictureWrite->refFramePoc[XIN_LIST_1][refIdxQ1] : 0x80000000;

                    if (((refPocP0 == refPocQ0) && (refPocP1 == refPocQ1)) || ((refPocP0 == refPocQ1) && (refPocP1 == refPocQ0)))
                    {
                        if (refPocP0 != refPocP1)
                        {
                            if (refPocP0 == refPocQ0)
                            {
                                if (CHECK_MV_DIFF(curBlock[-blockSetWidth].mv[XIN_LIST_0], curBlock[0].mv[XIN_LIST_0]))
                                {
                                    horBs[horIdx] |= 0x01;
                                }
                                else if (CHECK_MV_DIFF(curBlock[-blockSetWidth].mv[XIN_LIST_1], curBlock[0].mv[XIN_LIST_1]))
                                {
                                    horBs[horIdx] |= 0x01;
                                }
                            }
                            else
                            {
                                if (CHECK_MV_DIFF(curBlock[-blockSetWidth].mv[XIN_LIST_1], curBlock[0].mv[XIN_LIST_0]))
                                {
                                    horBs[horIdx] |= 0x01;
                                }
                                else if (CHECK_MV_DIFF(curBlock[-blockSetWidth].mv[XIN_LIST_0], curBlock[0].mv[XIN_LIST_1]))
                                {
                                    horBs[horIdx] |= 0x01;
                                }
                            }
                        }
                        else
                        {
                            if (((CHECK_MV_DIFF(curBlock[-blockSetWidth].mv[XIN_LIST_0], curBlock[0].mv[XIN_LIST_0]))
                                    || (CHECK_MV_DIFF(curBlock[-blockSetWidth].mv[XIN_LIST_1], curBlock[0].mv[XIN_LIST_1])))
                                    && ((CHECK_MV_DIFF(curBlock[-blockSetWidth].mv[XIN_LIST_0], curBlock[0].mv[XIN_LIST_1]))
                                        || (CHECK_MV_DIFF(curBlock[-blockSetWidth].mv[XIN_LIST_1], curBlock[0].mv[XIN_LIST_0]))))
                            {
                                horBs[horIdx] |= 0x01;
                            }
                        }
                    }
                    else
                    {
                        horBs[horIdx] |= 0x01;
                    }

                }

            }

            curBlock++;
            curCbf++;

        }

    }

    // Vertical Edge
    curCbf   = cbfMap;
    curBlock = blockMap;

    if ((secSet->lftCtu != NULL) || (cu->offX != 0))
    {
        for (verIdx = 0; verIdx < cuHeightInBlock; verIdx++)
        {
            yuvCbf = curCbf[-1] | curCbf[0];

            if (curBlock[-1].type == XIN_INTRA_MODE)
            {
                verBs[verIdx] = 0x2A;
            }
            else if (yuvCbf)
            {
                verBs[verIdx] |= (yuvCbf & 0x01) << 0;
                verBs[verIdx] |= (yuvCbf & 0x02) << 1;
                verBs[verIdx] |= (yuvCbf & 0x04) << 2;
            }

            if (!(verBs[verIdx] & 0x03))
            {
                if (frameType == XIN_P_FRAME)
                {
                    if (CHECK_MV_DIFF(curBlock[-1].mv[XIN_LIST_0], curBlock[0].mv[XIN_LIST_0]))
                    {
                        verBs[verIdx] |= 0x01;
                    }
                    else if (curBlock[-1].refIdx[XIN_LIST_0] != curBlock[0].refIdx[XIN_LIST_0])
                    {
                        verBs[verIdx] |= 0x01;
                    }
                }
                else
                {
                    refIdxP0 = curBlock[-1].refIdx[XIN_LIST_0];
                    refIdxP1 = curBlock[-1].refIdx[XIN_LIST_1];
                    refIdxQ0 = curBlock[0].refIdx[XIN_LIST_0];
                    refIdxQ1 = curBlock[0].refIdx[XIN_LIST_1];

                    refPocP0 = refIdxP0 >= 0 ? pictureWrite->refFramePoc[XIN_LIST_0][refIdxP0] : 0x80000000;
                    refPocP1 = refIdxP1 >= 0 ? pictureWrite->refFramePoc[XIN_LIST_1][refIdxP1] : 0x80000000;
                    refPocQ0 = refIdxQ0 >= 0 ? pictureWrite->refFramePoc[XIN_LIST_0][refIdxQ0] : 0x80000000;
                    refPocQ1 = refIdxQ1 >= 0 ? pictureWrite->refFramePoc[XIN_LIST_1][refIdxQ1] : 0x80000000;

                    if (((refPocP0 == refPocQ0) && (refPocP1 == refPocQ1)) || ((refPocP0 == refPocQ1) && (refPocP1 == refPocQ0)))
                    {
                        if (refPocP0 != refPocP1)
                        {
                            if (refPocP0 == refPocQ0)
                            {
                                if (CHECK_MV_DIFF(curBlock[-1].mv[XIN_LIST_0], curBlock[0].mv[XIN_LIST_0]))
                                {
                                    verBs[verIdx] |= 0x01;
                                }
                                else if (CHECK_MV_DIFF(curBlock[-1].mv[XIN_LIST_1], curBlock[0].mv[XIN_LIST_1]))
                                {
                                    verBs[verIdx] |= 0x01;
                                }
                            }
                            else
                            {
                                if (CHECK_MV_DIFF(curBlock[-1].mv[XIN_LIST_1], curBlock[0].mv[XIN_LIST_0]))
                                {
                                    verBs[verIdx] |= 0x01;
                                }
                                else if (CHECK_MV_DIFF(curBlock[-1].mv[XIN_LIST_0], curBlock[0].mv[XIN_LIST_1]))
                                {
                                    verBs[verIdx] |= 0x01;
                                }
                            }
                        }
                        else
                        {
                            if (((CHECK_MV_DIFF(curBlock[-1].mv[XIN_LIST_0], curBlock[0].mv[XIN_LIST_0]))
                                    || (CHECK_MV_DIFF(curBlock[-1].mv[XIN_LIST_1], curBlock[0].mv[XIN_LIST_1])))
                                    && ((CHECK_MV_DIFF(curBlock[-1].mv[XIN_LIST_0], curBlock[0].mv[XIN_LIST_1]))
                                        || (CHECK_MV_DIFF(curBlock[-1].mv[XIN_LIST_1], curBlock[0].mv[XIN_LIST_0]))))
                            {
                                verBs[verIdx] |= 0x01;
                            }
                        }
                    }
                    else
                    {
                        verBs[verIdx] |= 0x01;
                    }

                }

            }

            curBlock += blockSetWidth;
            curCbf   += widthInBlock;

        }
        
    }

    if (cu->pu.affine)
    {    
        // Horizontal Edge
        for (verIdx = 1; verIdx < cu->height / subBlockSize; verIdx++)
        {
            horBs += (subBlockSize >> lgBlockSize)*pictureWrite->widthInBlock;

            PEL_XY_TO_BLOCK_INDEX (0, verIdx*subBlockSize, blockIdx, blockSetWidth, lgBlockSize);
            curBlock = blockMap + blockIdx;
            
            for (horIdx = 0; horIdx < cuWidthInBlock; horIdx++)
            {
                if (frameType == XIN_P_FRAME)
                {
                    if (CHECK_MV_DIFF(curBlock[-blockSetWidth].mv[XIN_LIST_0], curBlock[0].mv[XIN_LIST_0]))
                    {
                        horBs[horIdx] |= 0x01;
                    }
                    else if (curBlock[-blockSetWidth].refIdx[XIN_LIST_0] != curBlock[0].refIdx[XIN_LIST_0])
                    {
                        horBs[horIdx] |= 0x01;
                    }
                }
                else
                {
                    refIdxP0 = curBlock[-blockSetWidth].refIdx[XIN_LIST_0];
                    refIdxP1 = curBlock[-blockSetWidth].refIdx[XIN_LIST_1];
                    refIdxQ0 = curBlock[0].refIdx[XIN_LIST_0];
                    refIdxQ1 = curBlock[0].refIdx[XIN_LIST_1];

                    refPocP0 = refIdxP0 >= 0 ? pictureWrite->refFramePoc[XIN_LIST_0][refIdxP0] : 0x80000000;
                    refPocP1 = refIdxP1 >= 0 ? pictureWrite->refFramePoc[XIN_LIST_1][refIdxP1] : 0x80000000;
                    refPocQ0 = refIdxQ0 >= 0 ? pictureWrite->refFramePoc[XIN_LIST_0][refIdxQ0] : 0x80000000;
                    refPocQ1 = refIdxQ1 >= 0 ? pictureWrite->refFramePoc[XIN_LIST_1][refIdxQ1] : 0x80000000;

                    if (((refPocP0 == refPocQ0) && (refPocP1 == refPocQ1)) || ((refPocP0 == refPocQ1) && (refPocP1 == refPocQ0)))
                    {
                        if (refPocP0 != refPocP1)
                        {
                            if (refPocP0 == refPocQ0)
                            {
                                if (CHECK_MV_DIFF(curBlock[-blockSetWidth].mv[XIN_LIST_0], curBlock[0].mv[XIN_LIST_0]))
                                {
                                    horBs[horIdx] |= 0x01;
                                }
                                else if (CHECK_MV_DIFF(curBlock[-blockSetWidth].mv[XIN_LIST_1], curBlock[0].mv[XIN_LIST_1]))
                                {
                                    horBs[horIdx] |= 0x01;
                                }
                            }
                            else
                            {
                                if (CHECK_MV_DIFF(curBlock[-blockSetWidth].mv[XIN_LIST_1], curBlock[0].mv[XIN_LIST_0]))
                                {
                                    horBs[horIdx] |= 0x01;
                                }
                                else if (CHECK_MV_DIFF(curBlock[-blockSetWidth].mv[XIN_LIST_0], curBlock[0].mv[XIN_LIST_1]))
                                {
                                    horBs[horIdx] |= 0x01;
                                }
                            }
                        }
                        else
                        {
                            if (((CHECK_MV_DIFF(curBlock[-blockSetWidth].mv[XIN_LIST_0], curBlock[0].mv[XIN_LIST_0]))
                                    || (CHECK_MV_DIFF(curBlock[-blockSetWidth].mv[XIN_LIST_1], curBlock[0].mv[XIN_LIST_1])))
                                    && ((CHECK_MV_DIFF(curBlock[-blockSetWidth].mv[XIN_LIST_0], curBlock[0].mv[XIN_LIST_1]))
                                        || (CHECK_MV_DIFF(curBlock[-blockSetWidth].mv[XIN_LIST_1], curBlock[0].mv[XIN_LIST_0]))))
                            {
                                horBs[horIdx] |= 0x01;
                            }
                        }
                    }
                    else
                    {
                        horBs[horIdx] |= 0x01;
                    }

                }

                curBlock++;
                
            }

        }

        // vertical Edge
        for (horIdx = 1; horIdx < cu->width / subBlockSize; horIdx++)
        {
            verBs += (subBlockSize >> lgBlockSize)*pictureWrite->heightInBlock;

            PEL_XY_TO_BLOCK_INDEX(horIdx*subBlockSize, 0, blockIdx, blockSetWidth, lgBlockSize);
            curBlock = blockMap + blockIdx;
    
            for (verIdx = 0; verIdx < cuHeightInBlock; verIdx++)
            {   
                if (frameType == XIN_P_FRAME)
                {
                    if (CHECK_MV_DIFF(curBlock[-1].mv[XIN_LIST_0], curBlock[0].mv[XIN_LIST_0]))
                    {
                        verBs[verIdx] |= 0x01;
                    }
                    else if (curBlock[-1].refIdx[XIN_LIST_0] != curBlock[0].refIdx[XIN_LIST_0])
                    {
                        verBs[verIdx] |= 0x01;
                    }
                }
                else
                {
                    refIdxP0 = curBlock[-1].refIdx[XIN_LIST_0];
                    refIdxP1 = curBlock[-1].refIdx[XIN_LIST_1];
                    refIdxQ0 = curBlock[0].refIdx[XIN_LIST_0];
                    refIdxQ1 = curBlock[0].refIdx[XIN_LIST_1];

                    refPocP0 = refIdxP0 >= 0 ? pictureWrite->refFramePoc[XIN_LIST_0][refIdxP0] : 0x80000000;
                    refPocP1 = refIdxP1 >= 0 ? pictureWrite->refFramePoc[XIN_LIST_1][refIdxP1] : 0x80000000;
                    refPocQ0 = refIdxQ0 >= 0 ? pictureWrite->refFramePoc[XIN_LIST_0][refIdxQ0] : 0x80000000;
                    refPocQ1 = refIdxQ1 >= 0 ? pictureWrite->refFramePoc[XIN_LIST_1][refIdxQ1] : 0x80000000;

                    if (((refPocP0 == refPocQ0) && (refPocP1 == refPocQ1)) || ((refPocP0 == refPocQ1) && (refPocP1 == refPocQ0)))
                    {
                        if (refPocP0 != refPocP1)
                        {
                            if (refPocP0 == refPocQ0)
                            {
                                if (CHECK_MV_DIFF(curBlock[-1].mv[XIN_LIST_0], curBlock[0].mv[XIN_LIST_0]))
                                {
                                    verBs[verIdx] |= 0x01;
                                }
                                else if (CHECK_MV_DIFF(curBlock[-1].mv[XIN_LIST_1], curBlock[0].mv[XIN_LIST_1]))
                                {
                                    verBs[verIdx] |= 0x01;
                                }
                            }
                            else
                            {
                                if (CHECK_MV_DIFF(curBlock[-1].mv[XIN_LIST_1], curBlock[0].mv[XIN_LIST_0]))
                                {
                                    verBs[verIdx] |= 0x01;
                                }
                                else if (CHECK_MV_DIFF(curBlock[-1].mv[XIN_LIST_0], curBlock[0].mv[XIN_LIST_1]))
                                {
                                    verBs[verIdx] |= 0x01;
                                }
                            }
                        }
                        else
                        {
                            if (((CHECK_MV_DIFF(curBlock[-1].mv[XIN_LIST_0], curBlock[0].mv[XIN_LIST_0]))
                                    || (CHECK_MV_DIFF(curBlock[-1].mv[XIN_LIST_1], curBlock[0].mv[XIN_LIST_1])))
                                    && ((CHECK_MV_DIFF(curBlock[-1].mv[XIN_LIST_0], curBlock[0].mv[XIN_LIST_1]))
                                        || (CHECK_MV_DIFF(curBlock[-1].mv[XIN_LIST_1], curBlock[0].mv[XIN_LIST_0]))))
                            {
                                verBs[verIdx] |= 0x01;
                            }
                        }
                    }
                    else
                    {
                        verBs[verIdx] |= 0x01;
                    }

                }

                curBlock += blockSetWidth;

            }

        }
        
    }

}

static void Xin266SetQpMapCu (
    xin_ref_picture *pictureWrite,
    xin_cu_struct   *cu)
{
    UINT8   *qpMap;
    UINT8   *qpUvMap;
    UINT32  widthInBlock;
    UINT32  lgBlockSize;
    UINT32  idx;
    UINT32  cuWidthInBlock;
    UINT32  cuHeightInBlock;

    widthInBlock    = pictureWrite->widthInBlock;
    lgBlockSize     = pictureWrite->lgBlockSize;
    cuWidthInBlock  = cu->width >> lgBlockSize;
    cuHeightInBlock = cu->height >> lgBlockSize;
    qpMap           = pictureWrite->qpMap;
    qpUvMap         = pictureWrite->qpUvMap;
    qpMap          += (cu->cuPelX >> lgBlockSize) + (cu->cuPelY >> lgBlockSize)*widthInBlock;
    qpUvMap        += (cu->cuPelX >> lgBlockSize) + (cu->cuPelY >> lgBlockSize)*widthInBlock;

    if (cu->treeMask & XIN_CU_TREE_L_MASK)
    {
        for (idx = 0; idx < cuHeightInBlock; idx++)
        {
            memset (
                qpMap + idx * widthInBlock,
                cu->qp,
                sizeof(UINT8)*cuWidthInBlock);
        }
    }

    if (cu->treeMask & XIN_CU_TREE_C_MASK)
    {
        for (idx = 0; idx < cuHeightInBlock; idx++)
        {
            memset (
                qpUvMap + idx * widthInBlock,
                cu->qp,
                sizeof(UINT8)*cuWidthInBlock);

        }
    }

}

static void Xin266SetCbfMapCu (
    xin_ref_picture *pictureWrite,
    xin_cu_struct   *cu)
{
    UINT8         *cbfMap;
    UINT32        widthInBlock;
    UINT32        idx;
    UINT32        tuIdx;
    UINT32        tuWidthInBlock;
    UINT32        tuHeightInBlock;
    xin_tu_struct *tu;
    UINT32        cbfIndex;
    UINT8         yuvCbf;
    UINT32        lgBlockSize;
    UINT32        height;
    UINT32        width;

    widthInBlock = pictureWrite->widthInBlock;
    lgBlockSize  = pictureWrite->lgBlockSize;
    cbfMap       = pictureWrite->cbfMap;
    cbfMap      += (cu->cuPelX >> lgBlockSize) + (cu->cuPelY >> lgBlockSize)*widthInBlock;

    for (tuIdx = 0; tuIdx < cu->tuNum; tuIdx++)
    {
        tu     = cu->tu[tuIdx];
        yuvCbf = (UINT8)((tu->yCbf << PLANE_LUMA) | (tu->uCbf << PLANE_CHROMA_U) | (tu->vCbf << PLANE_CHROMA_V));
        width  = 1 << tu->lgWidth[PLANE_LUMA];
        height = 1 << tu->lgHeight[PLANE_LUMA];

        tuHeightInBlock = height >> lgBlockSize;
        tuWidthInBlock  = width >> lgBlockSize;
        cbfIndex        = (tu->offsetX >> lgBlockSize)  + (tu->offsetY >> lgBlockSize) * widthInBlock;

        for (idx = 0; idx < tuHeightInBlock; idx++)
        {
            memset (
                cbfMap + cbfIndex,
                yuvCbf,
                sizeof(UINT8)*tuWidthInBlock);

            cbfIndex += widthInBlock;
        }

    }

}

void Xin266ComputeBsCu (
    xin_sec_struct *secSet,
    xin_cu_struct  *cu)
{
    xin_ref_picture *pictureWrite;
    UINT32          lgBlockSize;
    UINT8           *horBs;
    UINT8           *verBs;

    pictureWrite = secSet->picSet->pictureWrite;
    lgBlockSize  = pictureWrite->lgBlockSize;

    horBs   = pictureWrite->horBs;
    verBs   = pictureWrite->verBs;
    horBs  += (cu->cuPelY >> lgBlockSize)*pictureWrite->widthInBlock + (cu->cuPelX >> lgBlockSize);
    verBs  += (cu->cuPelX >> lgBlockSize)*pictureWrite->heightInBlock + (cu->cuPelY >> lgBlockSize);

    Xin266SetQpMapCu (
        pictureWrite,
        cu);

    Xin266SetCbfMapCu (
        pictureWrite,
        cu);

    Xin266SetEdgefilterCu (
        secSet,
        cu,
        horBs,
        verBs);

    if (cu->tuNum > 1)
    {
        Xin266SetEdgefilterTu (
            secSet,
            cu,
            horBs,
            verBs);
    }

}

void Xin266ComputeBsCuRec (
    xin_sec_struct *secSet,
    xin_cu_struct  *cu)
{
    UINT32          partIdx;
    xin_cu_struct   *childCu;
    xin_ref_picture *pictureWrite;

    pictureWrite = secSet->picSet->pictureWrite;

    if (!(cu->geomFlag & XIN_CB_PRESENT))
    {
        return;
    }

    if (cu->splitType == XIN_CU_QUAD_SPLIT)
    {
        for (partIdx = 0; partIdx < 4; partIdx++)
        {
            childCu = cu->childCu[partIdx];

            Xin266ComputeBsCuRec (
                secSet,
                childCu);
        }

        if ((cu->width == 8) && (cu->height == 8))
        {
            childCu = cu->childCu[4];

            Xin266SetQpMapCu (
                pictureWrite,
                childCu);
        }
    }
    else if ((cu->splitType == XIN_CU_HORZ_SPLIT) || (cu->splitType == XIN_CU_VERT_SPLIT))
    {
        for (partIdx = 0; partIdx < 2; partIdx++)
        {
            childCu = cu->childCu[partIdx];

            Xin266ComputeBsCuRec (
                secSet,
                childCu);
        }
    }
    else if ((cu->splitType == XIN_CU_TRIH_SPLIT) || (cu->splitType == XIN_CU_TRIV_SPLIT))
    {
        for (partIdx = 0; partIdx < 3; partIdx++)
        {
            childCu = cu->childCu[partIdx];

            Xin266ComputeBsCuRec (
                secSet,
                childCu);
        }
    }
    else
    {
        Xin266ComputeBsCu (
            secSet,
            cu);
    }

}

void Xin266ComputeBsCtu (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu)
{
    xin_seq_struct *seqSet;
    xin_pic_struct *picSet;
    xin_cu_struct  *cu;

    cu     = ctu->cu;
    seqSet = secSet->seqSet;
    picSet = secSet->picSet;

    if ((seqSet->config.disableDeblock) || ((!picSet->pictureWrite->isReferenced) && (!seqSet->config.needRecon) && (!picSet->enableAlf)))
    {
        return;
    }

    Xin266ComputeBsCuRec (
        secSet,
        cu);

}

