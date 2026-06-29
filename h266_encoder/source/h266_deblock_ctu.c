/***************************************************************************//**
 *
 * @file          h266_deblock_ctu.c
 * @brief         h266 deblocking filter on CTU level.
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
#include "h266_pic_struct.h"
#include "h266_loop_filter.h"
#include "h266_lmcs.h"
#include "h266_dep_quant_struct.h"
#include "h266_func_struct.h"

#define XIN_DEBLOCK_DELAY_PIXEL 8

static const SINT32 tcTable[] =
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,4,4,4,4,5,5,5,5,7,7,8,9,10,10,11,13,14,15,17,19,21,24,25,29,33,36,41,45,51,57,64,71,80,89,100,112,125,141,157,177,198,222,250,280,314,352,395
};

static const SINT32 betaTable[XIN_QP_NUM] =
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,7,8,9,10,11,12,13,14,15,16,17,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64
};

void Xin266DeblockCtuVer (
    xin_pic_struct *picSet,
    xin_ctu_struct *ctu)
{
    SINT32           rowIdx;
    SINT32           colIdx;
    SINT32           endRow;
    SINT32           endCol;
    SINT32           rowInc;
    SINT32           lgBlockSize;
    SINT32           widthInBlock;
    SINT32           heightInBlock;
    SINT32           pelX, pelY;
    PIXEL            *reconY;
    PIXEL            *reconU;
    PIXEL            *reconV;
    intptr_t         reconYStride;
    intptr_t         reconUvStride;
    UINT8            *verBs;
    UINT32           bs0, bs1;
    UINT32           yBs0, yBs1;
    UINT32           uBs0, vBs0;
    UINT32           uBs1, vBs1;
    UINT8            *qpMap;
    UINT8            *qpUvMap;
    SINT32           tcLuma[2];
    SINT32           tcCb[2];
    SINT32           tcCr[2];
    SINT32           beta[2];
    UINT32           lftQp;
    UINT32           curQp;
    UINT32           avgQp0, avgQp1;
    xin_ref_picture  *pictureWrite;
    xin_seq_struct   *seqSet;
    xin_block_struct *blockMap;
    xin_block_struct *currBlock;
    xin_func_struct  *funcSet;
    intptr_t         blockStride;
    intptr_t         ctuBlockIdx;
    intptr_t         curBlockIdx;
    BOOL             sidePisLarge;
    BOOL             sideQisLarge;
    BOOL             largeBoundary;
    SINT32           maxFltLengthP;
    SINT32           maxFltLengthQ;
    SINT32           chromaMask;
    UINT32           *chromaQpMap;
    UINT32           tranSize;
    SINT32           pelOffSet;
    SINT32           isTranEdge;
    SINT32           ctuSizeMask;
    
    seqSet        = picSet->seqSet;
    funcSet       = picSet->funcSet;
    pictureWrite  = picSet->pictureWrite;
    reconYStride  = pictureWrite->refStride[PLANE_LUMA];
    reconUvStride = pictureWrite->refStride[PLANE_CHROMA];
    ctuSizeMask   = seqSet->ctuSize - 1;
    lgBlockSize   = pictureWrite->lgBlockSize;
    endCol        = ctu->width >> lgBlockSize;
    endRow        = ctu->height >> lgBlockSize;
    widthInBlock  = pictureWrite->widthInBlock;
    heightInBlock = pictureWrite->heightInBlock;
    blockMap      = pictureWrite->blockSetMap;
    blockStride   = pictureWrite->blockSetWidth;
    rowInc        = 1 + (lgBlockSize == XIN_LOG_BLOCK_SIZE);
    chromaMask    = (lgBlockSize == XIN_LOG_BLOCK_SIZE) ? 3 : 1;
    chromaQpMap   = seqSet->chromaQpMap;

    reconY = pictureWrite->refBuf[PLANE_LUMA] + ctu->ctuPelY*reconYStride + ctu->ctuPelX;
    reconU = pictureWrite->refBuf[PLANE_CHROMA_U] + ((ctu->ctuPelY*reconUvStride + ctu->ctuPelX) >> 1);
    reconV = pictureWrite->refBuf[PLANE_CHROMA_V] + ((ctu->ctuPelY*reconUvStride + ctu->ctuPelX) >> 1);

    verBs   = pictureWrite->verBs + (ctu->ctuPelX >> lgBlockSize)*heightInBlock + (ctu->ctuPelY >> lgBlockSize);
    qpMap   = pictureWrite->qpMap + (ctu->ctuPelY >> lgBlockSize)*widthInBlock + (ctu->ctuPelX >> lgBlockSize);
    qpUvMap = pictureWrite->qpUvMap + (ctu->ctuPelY >> lgBlockSize)*widthInBlock + (ctu->ctuPelX >> lgBlockSize);

    PEL_XY_TO_BLOCK_INDEX (ctu->ctuPelX, ctu->ctuPelY, ctuBlockIdx, blockStride, lgBlockSize);
    blockMap += ctuBlockIdx;

    for (colIdx = 0; colIdx < endCol; colIdx++)
    {
        for (rowIdx = 0; rowIdx < endRow; rowIdx += rowInc)
        {
            pelX = colIdx << lgBlockSize;
            pelY = rowIdx << lgBlockSize;

            bs0  = verBs[rowIdx];
            bs1  = verBs[rowIdx + (lgBlockSize == XIN_LOG_BLOCK_SIZE)];

            yBs0 = (bs0 & 0x03);
            yBs1 = (bs1 & 0x03);

            PEL_XY_TO_BLOCK_INDEX (pelX, pelY, curBlockIdx, blockStride, lgBlockSize);
            currBlock = blockMap + curBlockIdx;

            if (yBs0 || yBs1)
            {
                curQp  = qpMap[rowIdx*widthInBlock];
                lftQp  = qpMap[rowIdx*widthInBlock - 1];
                avgQp0 = (curQp + lftQp + 1) >> 1;

                curQp  = qpMap[(rowIdx+(rowInc-1))*widthInBlock];
                lftQp  = qpMap[(rowIdx+(rowInc-1))*widthInBlock - 1];
                avgQp1 = (curQp + lftQp + 1) >> 1;

#ifdef ENABLE_10BIT_ENCODER
                tcLuma[0] = (yBs0 > 0) ? tcTable[avgQp0 + ((yBs0 > 1) << 1)] : 0;
                tcLuma[1] = (yBs1 > 0) ? tcTable[avgQp1 + ((yBs1 > 1) << 1)] : 0;

                beta[0]   = betaTable[avgQp0] << (XIN_INTERNAL_BIT_DEPTH - XIN_8_BIT_DEPTH);
                beta[1]   = betaTable[avgQp1] << (XIN_INTERNAL_BIT_DEPTH - XIN_8_BIT_DEPTH);
#else
                tcLuma[0] = (yBs0 > 0) ? (tcTable[avgQp0 + ((yBs0 > 1) << 1)] + 2) >> 2 : 0;
                tcLuma[1] = (yBs1 > 0) ? (tcTable[avgQp1 + ((yBs1 > 1) << 1)] + 2) >> 2 : 0;

                beta[0]   = betaTable[avgQp0] << (XIN_INTERNAL_BIT_DEPTH - XIN_8_BIT_DEPTH);
                beta[1]   = betaTable[avgQp1] << (XIN_INTERNAL_BIT_DEPTH - XIN_8_BIT_DEPTH);
#endif

                if ((currBlock[0].width <= 4) || (currBlock[-1].width <= 4))
                {
                    maxFltLengthP = 1;
                    maxFltLengthQ = 1;
                }
                else
                {
                    maxFltLengthP = (currBlock[-1].width >= 32) ? 7 : 3;
                    maxFltLengthQ = (currBlock[0].width >= 32) ? 7 : 3;
                }

                if (currBlock[0].affine)
                {
                    tranSize   = currBlock[0].width;
                    tranSize   = XIN_MIN (seqSet->config.maxTrSize, tranSize);
                    pelOffSet  = pelX - (currBlock[0].cuPelX & ctuSizeMask);
                    isTranEdge = !(pelOffSet & (tranSize - 1));

                    if (isTranEdge)
                    {
                        maxFltLengthQ = XIN_MIN (5, maxFltLengthQ);
                        maxFltLengthP = (pelOffSet) ? XIN_MIN (5, maxFltLengthP) : maxFltLengthP;
                    }
                    else if (pelOffSet && ((pelOffSet == 8) || (!((pelOffSet - 8) & (tranSize - 1))) || (pelOffSet + 8 == currBlock[0].width) || (!((pelOffSet + 8) & (tranSize - 1)))))
                    {
                        maxFltLengthP = 2;
                        maxFltLengthQ = 2;
                    }
                    else
                    {
                        maxFltLengthP = 3;
                        maxFltLengthQ = 3;
                    }
                    
                }

                if (maxFltLengthP > 5)
                {
                    // restrict filter length if sub-blocks are used (e.g affine or ATMVP)
                    if (currBlock[-1].affine)
                    {
                        maxFltLengthP = XIN_MIN (maxFltLengthP, 5);
                    }
                }

                sidePisLarge = maxFltLengthP > 3;
                sideQisLarge = maxFltLengthQ > 3;

                funcSet->pfXinLumaLoopFilter (
                    reconY + pelY*reconYStride + pelX,
                    reconYStride,
                    tcLuma,
                    beta,
                    sidePisLarge,
                    sideQisLarge,
                    maxFltLengthP,
                    maxFltLengthQ,
                    TRUE);

            }

            uBs0 = (bs0 & 0x0C) >> 2;
            vBs0 = (bs0 & 0x30) >> 4;
            uBs1 = (bs1 & 0x0C) >> 2;
            vBs1 = (bs1 & 0x30) >> 4;

            if ((!(colIdx & chromaMask)) && ((uBs0 > 0) || (vBs0 > 0) || (uBs1 > 0) || (vBs1 > 0)))
            {
                if ((currBlock[0].width >= 16) && (currBlock[-1].width >= 16))
                {
                    maxFltLengthP = 3;
                    maxFltLengthQ = 3;
                    largeBoundary = TRUE;
                }
                else
                {
                    maxFltLengthP = 1;
                    maxFltLengthQ = 1;
                    largeBoundary = FALSE;
                }

                if (((uBs0 > 1) || (uBs1 > 1)) || ((maxFltLengthP == 3) && ((uBs0 > 0) || (vBs0 > 0) || (uBs1 > 0) || (vBs1 > 0))))
                {
                    curQp   = qpUvMap[rowIdx*widthInBlock];
                    lftQp   = qpUvMap[rowIdx*widthInBlock - 1];
                    avgQp0  = (chromaQpMap[curQp] + chromaQpMap[lftQp] + 1) >> 1;
                    avgQp0 += seqSet->config.chromaQpOffset;
                    curQp   = qpUvMap[(rowIdx+(rowInc-1))*widthInBlock];
                    lftQp   = qpUvMap[(rowIdx+(rowInc-1))*widthInBlock - 1];
                    avgQp1  = (chromaQpMap[curQp] + chromaQpMap[lftQp] + 1) >> 1;
                    avgQp1 += seqSet->config.chromaQpOffset;
                    beta[0] = betaTable[avgQp0] << (XIN_INTERNAL_BIT_DEPTH - XIN_8_BIT_DEPTH);
                    beta[1] = betaTable[avgQp1] << (XIN_INTERNAL_BIT_DEPTH - XIN_8_BIT_DEPTH);

#ifdef ENABLE_10BIT_ENCODER
                    tcCb[0] = (uBs0 > 1) || ((maxFltLengthP == 3) && (uBs0 > 0)) ? tcTable[avgQp0 + 2 * (uBs0 > 1)] : 0;
                    tcCb[1] = (uBs1 > 1) || ((maxFltLengthP == 3) && (uBs1 > 0)) ? tcTable[avgQp1 + 2 * (uBs1 > 1)] : 0;
                    tcCr[0] = (vBs0 > 1) || ((maxFltLengthP == 3) && (vBs0 > 0)) ? tcTable[avgQp0 + 2 * (vBs0 > 1)] : 0;
                    tcCr[1] = (vBs1 > 1) || ((maxFltLengthP == 3) && (vBs1 > 0)) ? tcTable[avgQp1 + 2 * (vBs1 > 1)] : 0;
#else
                    tcCb[0] = (uBs0 > 1) || ((maxFltLengthP == 3) && (uBs0 > 0)) ? (tcTable[avgQp0 + 2 * (uBs0 > 1)] + 2) >> 2 : 0;
                    tcCb[1] = (uBs1 > 1) || ((maxFltLengthP == 3) && (uBs1 > 0)) ? (tcTable[avgQp1 + 2 * (uBs1 > 1)] + 2) >> 2 : 0;
                    tcCr[0] = (vBs0 > 1) || ((maxFltLengthP == 3) && (vBs0 > 0)) ? (tcTable[avgQp0 + 2 * (vBs0 > 1)] + 2) >> 2 : 0;
                    tcCr[1] = (vBs1 > 1) || ((maxFltLengthP == 3) && (vBs1 > 0)) ? (tcTable[avgQp1 + 2 * (vBs1 > 1)] + 2) >> 2 : 0;
#endif

                    Xin266ChromaLoopFilter (
                        reconU + ((pelY*reconUvStride + pelX)>>1),
                        reconUvStride,
                        tcCb,
                        beta,
                        largeBoundary,
                        FALSE,
                        TRUE);

                    Xin266ChromaLoopFilter (
                        reconV + ((pelY*reconUvStride + pelX)>>1),
                        reconUvStride,
                        tcCr,
                        beta,
                        largeBoundary,
                        FALSE,
                        TRUE);

                }

            }

        }

        verBs += heightInBlock;
        qpMap++;
        qpUvMap++;

    }
    
}

void Xin266DeblockCtuHor (
    xin_pic_struct *picSet,
    xin_ctu_struct *ctu)
{
    SINT32           rowIdx;
    SINT32           colIdx;
    SINT32           colInc;
    SINT32           endRow;
    SINT32           startCol;
    SINT32           endCol;
    SINT32           widthInBlock;
    SINT32           pelX, pelY;
    PIXEL            *reconY;
    PIXEL            *reconU;
    PIXEL            *reconV;
    intptr_t         reconYStride;
    intptr_t         reconUvStride;
    UINT8            *horBs;
    UINT32           bs0, bs1;
    UINT32           yBs0, yBs1;
    UINT32           uBs0, vBs0;
    UINT32           uBs1, vBs1;
    UINT8            *qpMap;
    UINT8            *qpUvMap;
    SINT32           tcLuma[2];
    SINT32           tcCb[2];
    SINT32           tcCr[2];
    SINT32           beta[2];
    UINT32           topQp;
    UINT32           curQp;
    UINT32           avgQp0, avgQp1;
    xin_ref_picture  *pictureWrite;
    xin_seq_struct   *seqSet;
    xin_func_struct  *funcSet;
    UINT32           lgBlockSize;
    xin_block_struct *blockMap;
    xin_block_struct *currBlock;
    intptr_t         blockStride;
    intptr_t         ctuBlockIdx;
    intptr_t         curBlockIdx;
    BOOL             sidePisLarge;
    BOOL             sideQisLarge;
    BOOL             largeBoundary;
    SINT32           maxFltLengthP;
    SINT32           maxFltLengthQ;
    SINT32           chromaMask;
    UINT32           *chromaQpMap;
    BOOL             isTranEdge;
    UINT32           tranSize;
    SINT32           pelOffSet;
    SINT32           ctuSizeMask;

    pictureWrite  = picSet->pictureWrite;
    seqSet        = picSet->seqSet;
    funcSet       = picSet->funcSet;
    ctuSizeMask   = seqSet->ctuSize - 1;
    reconYStride  = pictureWrite->refStride[PLANE_LUMA];
    reconUvStride = pictureWrite->refStride[PLANE_CHROMA];
    lgBlockSize   = pictureWrite->lgBlockSize;
    endCol        = ctu->width >> lgBlockSize;
    endRow        = ctu->height >> lgBlockSize;
    widthInBlock  = pictureWrite->widthInBlock;
    blockMap      = pictureWrite->blockSetMap;
    blockStride   = pictureWrite->blockSetWidth;
    colInc        = 1 + (lgBlockSize == XIN_LOG_BLOCK_SIZE);
    chromaMask    = (lgBlockSize == XIN_LOG_BLOCK_SIZE) ? 3 : 1;
    chromaQpMap   = seqSet->chromaQpMap;
    startCol      = 0;

    reconY = pictureWrite->refBuf[PLANE_LUMA] + ctu->ctuPelY*reconYStride + ctu->ctuPelX;
    reconU = pictureWrite->refBuf[PLANE_CHROMA_U] + ((ctu->ctuPelY*reconUvStride + ctu->ctuPelX) >> 1);
    reconV = pictureWrite->refBuf[PLANE_CHROMA_V] + ((ctu->ctuPelY*reconUvStride + ctu->ctuPelX) >> 1);

    horBs   = pictureWrite->horBs + (ctu->ctuPelY >> lgBlockSize)*widthInBlock + (ctu->ctuPelX >> lgBlockSize);
    qpMap   = pictureWrite->qpMap + (ctu->ctuPelY >> lgBlockSize)*widthInBlock + (ctu->ctuPelX >> lgBlockSize);
    qpUvMap = pictureWrite->qpUvMap + (ctu->ctuPelY >> lgBlockSize)*widthInBlock + (ctu->ctuPelX >> lgBlockSize);

    PEL_XY_TO_BLOCK_INDEX (ctu->ctuPelX, ctu->ctuPelY, ctuBlockIdx, blockStride, lgBlockSize);
    blockMap += ctuBlockIdx;

    if (ctu->ctuX)
    {
        startCol -= colInc;
    }

    if ((ctu->ctuX + 1) != seqSet->frameWidthInCtu)
    {
        endCol -= colInc;
    }

    for (rowIdx = 0; rowIdx < endRow; rowIdx++)
    {
        for (colIdx = startCol; colIdx < endCol; colIdx += colInc)
        {
            pelX = colIdx << lgBlockSize;
            pelY = rowIdx << lgBlockSize;

            bs0  = horBs[colIdx];
            bs1  = horBs[colIdx + (lgBlockSize==XIN_LOG_BLOCK_SIZE)];

            yBs0 = (bs0 & 0x03);
            yBs1 = (bs1 & 0x03);

            PEL_XY_TO_BLOCK_INDEX (pelX, pelY, curBlockIdx, blockStride, lgBlockSize);
            currBlock = blockMap + curBlockIdx;

            if (yBs0 || yBs1)
            {
                curQp  = qpMap[colIdx];
                topQp  = qpMap[colIdx - widthInBlock];
                avgQp0 = (curQp + topQp + 1) >> 1;

                curQp  = qpMap[colIdx + (colInc-1)];
                topQp  = qpMap[colIdx + (colInc-1) - widthInBlock];
                avgQp1 = (curQp + topQp + 1) >> 1;

#ifdef ENABLE_10BIT_ENCODER
                tcLuma[0] = (bs0 > 0) ? tcTable[avgQp0 + ((yBs0 > 1) << 1)] : 0;
                tcLuma[1] = (bs1 > 0) ? tcTable[avgQp1 + ((yBs1 > 1) << 1)] : 0;

                beta[0]   = betaTable[avgQp0] << (XIN_INTERNAL_BIT_DEPTH - XIN_8_BIT_DEPTH);
                beta[1]   = betaTable[avgQp1] << (XIN_INTERNAL_BIT_DEPTH - XIN_8_BIT_DEPTH);
#else
                tcLuma[0] = (yBs0 > 0) ? (tcTable[avgQp0 + ((yBs0 > 1) << 1)] + 2) >> 2 : 0;
                tcLuma[1] = (yBs1 > 0) ? (tcTable[avgQp1 + ((yBs1 > 1) << 1)] + 2) >> 2 : 0;

                beta[0]   = betaTable[avgQp0] << (XIN_INTERNAL_BIT_DEPTH - XIN_8_BIT_DEPTH);
                beta[1]   = betaTable[avgQp1] << (XIN_INTERNAL_BIT_DEPTH - XIN_8_BIT_DEPTH);
#endif
                
                if ((currBlock[0].height <= 4) || (currBlock[-blockStride].height <= 4))
                {
                    maxFltLengthP = 1;
                    maxFltLengthQ = 1;
                }
                else
                {
                    maxFltLengthP = (currBlock[-blockStride].height >= 32) ? 7 : 3;
                    maxFltLengthQ = (currBlock[0].height >= 32) ? 7 : 3;
                }

                if (currBlock[0].affine)
                {
                    tranSize   = currBlock[0].height;
                    tranSize   = XIN_MIN (seqSet->config.maxTrSize, tranSize);
                    pelOffSet  = pelY - (currBlock[0].cuPelY & ctuSizeMask);
                    isTranEdge = !(pelOffSet & (tranSize - 1));

                    if (isTranEdge)
                    {
                        maxFltLengthQ = XIN_MIN (5, maxFltLengthQ);
                        maxFltLengthP = (pelOffSet) ? XIN_MIN (5, maxFltLengthP) : maxFltLengthP;
                    }
                    else if (pelOffSet && ((pelOffSet == 8) || (!((pelOffSet - 8) & (tranSize - 1))) || (pelOffSet + 8 == currBlock[0].height) || (!((pelOffSet + 8) & (tranSize - 1)))))
                    {
                        maxFltLengthP = 2;
                        maxFltLengthQ = 2;
                    }
                    else
                    {
                        maxFltLengthP = 3;
                        maxFltLengthQ = 3;
                    }
                    
                }

                if (maxFltLengthP > 5)
                {
                    // restrict filter length if sub-blocks are used (e.g affine or ATMVP)
                    if (currBlock[-blockStride].affine)
                    {
                        maxFltLengthP = XIN_MIN (maxFltLengthP, 5);
                    }
                }

                sidePisLarge = maxFltLengthP > 3;
                sideQisLarge = maxFltLengthQ > 3;
                sidePisLarge = (pelY == 0) ? FALSE : sidePisLarge;

                funcSet->pfXinLumaLoopFilter (
                    reconY + pelY*reconYStride + pelX,
                    reconYStride,
                    tcLuma,
                    beta,
                    sidePisLarge,
                    sideQisLarge,
                    maxFltLengthP,
                    maxFltLengthQ,
                    FALSE);

            }

            uBs0 = (bs0 & 0x0C) >> 2;
            vBs0 = (bs0 & 0x30) >> 4;
            uBs1 = (bs1 & 0x0C) >> 2;
            vBs1 = (bs1 & 0x30) >> 4;

            if ((!(rowIdx & chromaMask)) && ((uBs0 > 0) || (vBs0 > 0) || (uBs1 > 0) || (vBs1 > 0)))
            {
                if ((currBlock[0].height >= 16) && (currBlock[-blockStride].height >= 16))
                {
                    maxFltLengthP = 3;
                    maxFltLengthQ = 3;
                    largeBoundary = TRUE;
                }
                else
                {
                    maxFltLengthP = 1;
                    maxFltLengthQ = 1;
                    largeBoundary = FALSE;
                }

                if ((uBs0 > 1) || (uBs1 > 1) || ((maxFltLengthP == 3) && ((uBs0 > 0) || (vBs0 > 0) || (uBs1 > 0) || (vBs1 > 0))))
                {
                    curQp   = qpUvMap[colIdx];
                    topQp   = qpUvMap[colIdx - widthInBlock];
                    avgQp0  = (chromaQpMap[curQp] + chromaQpMap[topQp] + 1) >> 1;
                    avgQp0 += seqSet->config.chromaQpOffset;
                    curQp   = qpUvMap[colIdx + (colInc-1)];
                    topQp   = qpUvMap[colIdx + (colInc-1) - widthInBlock];
                    avgQp1  = (chromaQpMap[curQp] + chromaQpMap[topQp] + 1) >> 1;
                    avgQp1 += seqSet->config.chromaQpOffset;

                    beta[0] = betaTable[avgQp0] << (XIN_INTERNAL_BIT_DEPTH - XIN_8_BIT_DEPTH);
                    beta[1] = betaTable[avgQp1] << (XIN_INTERNAL_BIT_DEPTH - XIN_8_BIT_DEPTH);

#ifdef ENABLE_10BIT_ENCODER
                    tcCb[0] = (uBs0 > 1) || ((maxFltLengthP == 3) && (uBs0 > 0)) ? tcTable[avgQp0 + 2*(uBs0 > 1)] : 0;
                    tcCr[0] = (vBs0 > 1) || ((maxFltLengthP == 3) && (vBs0 > 0)) ? tcTable[avgQp0 + 2*(vBs0 > 1)] : 0;
                    tcCb[1] = (uBs1 > 1) || ((maxFltLengthP == 3) && (uBs1 > 0)) ? tcTable[avgQp1 + 2*(uBs1 > 1)] : 0;
                    tcCr[1] = (vBs1 > 1) || ((maxFltLengthP == 3) && (vBs1 > 0)) ? tcTable[avgQp1 + 2*(vBs1 > 1)] : 0;
#else
                    tcCb[0] = (uBs0 > 1) || ((maxFltLengthP == 3) && (uBs0 > 0)) ? (tcTable[avgQp0 + 2*(uBs0 > 1)] + 2) >> 2 : 0;
                    tcCr[0] = (vBs0 > 1) || ((maxFltLengthP == 3) && (vBs0 > 0)) ? (tcTable[avgQp0 + 2*(vBs0 > 1)] + 2) >> 2 : 0;
                    tcCb[1] = (uBs1 > 1) || ((maxFltLengthP == 3) && (uBs1 > 0)) ? (tcTable[avgQp1 + 2*(uBs1 > 1)] + 2) >> 2 : 0;
                    tcCr[1] = (vBs1 > 1) || ((maxFltLengthP == 3) && (vBs1 > 0)) ? (tcTable[avgQp1 + 2*(vBs1 > 1)] + 2) >> 2 : 0;
#endif

                    Xin266ChromaLoopFilter (
                        reconU + ((pelY*reconUvStride + pelX)>>1),
                        reconUvStride,
                        tcCb,
                        beta,
                        largeBoundary,
                        !pelY,
                        FALSE);

                    Xin266ChromaLoopFilter (
                        reconV + ((pelY*reconUvStride + pelX)>>1),
                        reconUvStride,
                        tcCr,
                        beta,
                        largeBoundary,
                        !pelY,
                        FALSE);

                }

            }

        }

        horBs   += widthInBlock;
        qpMap   += widthInBlock;
        qpUvMap += widthInBlock;

    }

}

void Xin266DeblockCtu (
    xin_ctu_struct *ctu)
{
    xin_seq_struct  *seqSet;
    xin_pic_struct  *picSet;
    xin_lmcs_struct *lmcsSet;

    picSet  = ctu->picSet;
    seqSet  = picSet->seqSet;
    lmcsSet = picSet->lmcsSet;

    if ((seqSet->config.enableLmcs) && (lmcsSet->lmcsParam.sliceReshaperEnabled))
    {
        Xin266InvReshapeSignal (
            picSet,
            ctu);
    }

    if ((seqSet->config.disableDeblock) || ((!picSet->pictureWrite->isReferenced) && (!seqSet->config.needRecon) && (!picSet->enableAlf)))
    {
        return;
    }

    Xin266DeblockCtuVer (
        picSet,
        ctu);

    Xin266DeblockCtuHor (
        picSet,
        ctu);

}



