/***************************************************************************//**
*
* @file          h265p_compute_bs.c
* @brief         Av1 loop filter boundary strength computation.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
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
#include "h265p_md_buf_manipulate.h"
#include "h265p_enc_init.h"
#include "h265p_common_data.h"

static void SetEdgefilterMb (
    xin_sec_struct *secSet,
    xin_mb_struct  *mb,
    UINT8          *lumaFlt[2],
    UINT8          *chromaFlt[2])
{
    SINT32           idx;
    xin_mi_struct    *miBuf;
    xin_mi_struct    *curMi;
    xin_ref_picture  *pictureWrite;
    SINT32           widthInMi[2];
    SINT32           heightInMi[2];
    intptr_t         miStride;
    intptr_t         miIdx;
    UINT8            currSize;
    UINT8            prevSize;
    UINT8            minSize;

    pictureWrite  = secSet->picSet->pictureWrite;
    widthInMi[0]  = mb->width[PLANE_LUMA] >> XIN_LOG_MI_SIZE;
    heightInMi[0] = mb->height[PLANE_LUMA] >> XIN_LOG_MI_SIZE;
    widthInMi[1]  = mb->width[PLANE_CHROMA] >> XIN_LOG_MI_SIZE;
    heightInMi[1] = mb->height[PLANE_CHROMA] >> XIN_LOG_MI_SIZE;
    miStride      = pictureWrite->miStride;
    miBuf         = pictureWrite->miBuf;

    PEL_XY_TO_BLOCK_INDEX (mb->mbPelX[PLANE_LUMA], mb->mbPelY[PLANE_LUMA], miIdx, miStride, XIN_LOG_MI_SIZE);
    miBuf += miIdx;

    // Horizontal Edge
    if ((secSet->topSb != NULL) || (mb->offY[PLANE_LUMA] != 0))
    {
        currSize = mb->txSize;
        currSize = txSize2TxDim[currSize][1];
        curMi    = miBuf;

        for (idx = 0; idx < widthInMi[0]; idx++)
        {
            prevSize = curMi[-miStride].txSize;
            prevSize = txSize2TxDim[prevSize][1];
            minSize  = XIN_MIN (currSize, prevSize);

            lumaFlt[1][idx] = XIN_MIN (minSize, 14);

            curMi++;

        }

        currSize = mb->blockSize;
        currSize = (UINT8)blockSize2TxSizeUv[currSize];
        currSize = txSize2TxDim[currSize][1];
        curMi    = miBuf;

        for (idx = 0; idx < widthInMi[1]; idx++)
        {
            prevSize = curMi[-miStride].blockSize;
            prevSize = (UINT8)blockSize2TxSizeUv[prevSize];
            prevSize = txSize2TxDim[prevSize][1];
            minSize  = XIN_MIN (currSize, prevSize);

            chromaFlt[1][idx] = XIN_MIN (minSize, 6);

            curMi += 2;
        }

    }

    // Vertical Edge
    if ((secSet->lftSb != NULL) || (mb->offX[PLANE_LUMA] != 0))
    {
        currSize = mb->txSize;
        currSize = txSize2TxDim[currSize][0];
        curMi    = miBuf;

        for (idx = 0; idx < heightInMi[0]; idx++)
        {
            prevSize = curMi[-1].txSize;
            prevSize = txSize2TxDim[prevSize][0];
            minSize  = XIN_MIN (currSize, prevSize);
            
            lumaFlt[0][idx] = XIN_MIN (minSize, 14);

            curMi += miStride;
        }

        currSize = mb->blockSize;
        currSize = (UINT8)blockSize2TxSizeUv[currSize];
        currSize = txSize2TxDim[currSize][0];
        curMi    = miBuf;

        for (idx = 0; idx < heightInMi[1]; idx++)
        {
            prevSize = curMi[-1].blockSize;
            prevSize = (UINT8)blockSize2TxSizeUv[prevSize];
            prevSize = txSize2TxDim[prevSize][0];
            minSize  = XIN_MIN (currSize, prevSize);

            chromaFlt[0][idx] = XIN_MIN (minSize, 6);

            curMi += 2*miStride;

        }

    }

}

static void SetEdgefilterTu (
    xin_sec_struct *secSet,
    xin_mb_struct  *mb,
    UINT8          *lumaFlt[2],
    UINT8          *chromaFlt[2])
{
    UINT32          tuIdx;
    SINT32          widthInMi[2];
    SINT32          heightInMi[2];
    xin_tu_struct   *tu;
    xin_ref_picture *pictureWrite;
    UINT8           currSize;
    UINT8           fltLenVer;
    UINT8           fltLenHor;
    UINT8           *fltVer;
    UINT8           *fltHor;

    pictureWrite  = secSet->picSet->pictureWrite;
    widthInMi[0]  = mb->width[PLANE_LUMA] >> XIN_LOG_MI_SIZE;
    heightInMi[0] = mb->height[PLANE_LUMA] >> XIN_LOG_MI_SIZE;
    widthInMi[1]  = mb->width[PLANE_CHROMA] >> XIN_LOG_MI_SIZE;
    heightInMi[1] = mb->height[PLANE_CHROMA] >> XIN_LOG_MI_SIZE;

    currSize  = mb->txSize;
    fltLenHor = txSize2TxDim[currSize][1];
    fltLenHor = XIN_MIN (fltLenHor, 14);
    fltLenVer = txSize2TxDim[currSize][0];
    fltLenVer = XIN_MIN (fltLenHor, 14);

    for (tuIdx = 0; tuIdx < mb->tuNum; tuIdx++)
    {
        tu     = mb->tu[PLANE_LUMA] + tuIdx;
        fltHor = lumaFlt[1] + (tu->offsetY >> XIN_LOG_MI_SIZE)*pictureWrite->lumaFltStride[1] + (tu->offsetX >> XIN_LOG_MI_SIZE);
        fltVer = lumaFlt[0] + (tu->offsetX >> XIN_LOG_MI_SIZE)*pictureWrite->lumaFltStride[0] + (tu->offsetY >> XIN_LOG_MI_SIZE);

        // Horizontal Edge
        if (tu->offsetY)
        {
            memset (fltHor, fltLenHor, sizeof(UINT8)*widthInMi[0]);
        }

        // Vertical Edge
        if (tu->offsetX)
        {
            memset (fltVer, fltLenVer, sizeof(UINT8)*heightInMi[0]);
        }
    }

    if ((mb->height[PLANE_LUMA] > 64) || (mb->width[PLANE_LUMA] > 64))
    {
        currSize  = mb->blockSize;
        currSize  = (UINT8)blockSize2TxSizeUv[currSize];
        fltLenHor = txSize2TxDim[currSize][1];
        fltLenVer = txSize2TxDim[currSize][0];

        for (tuIdx = 0; tuIdx < mb->tuNum; tuIdx++)
        {
            tu     = mb->tu[PLANE_CHROMA] + tuIdx;
            fltHor = chromaFlt[1] + (tu->offsetY >> XIN_LOG_MI_SIZE)*pictureWrite->chromaFltStride[1] + (tu->offsetX >> XIN_LOG_MI_SIZE);
            fltVer = chromaFlt[0] + (tu->offsetX >> XIN_LOG_MI_SIZE)*pictureWrite->chromaFltStride[0] + (tu->offsetY >> XIN_LOG_MI_SIZE);

            // Horizontal Edge
            if (tu->offsetY)
            {
                memset (fltHor, fltLenHor, sizeof(UINT8)*widthInMi[1]);
            }

            // Vertical Edge
            if (tu->offsetX)
            {
                memset (fltVer, fltLenVer, sizeof(UINT8)*heightInMi[1]);
            }
        }

    }

}

void Xin265pComputeBsMb (
    xin_sec_struct *secSet,
    xin_mb_struct  *mb)
{
    xin_ref_picture *pictureWrite;
    UINT8           *lumaFlt[2];
    UINT8           *chromaFlt[2];

    pictureWrite = secSet->picSet->pictureWrite;
    lumaFlt[0]   = pictureWrite->lumaFlt[0];
    lumaFlt[1]   = pictureWrite->lumaFlt[1];
    chromaFlt[0] = pictureWrite->chromaFlt[0];
    chromaFlt[1] = pictureWrite->chromaFlt[1];

    lumaFlt[0]   += (mb->mbPelX[PLANE_LUMA] >> XIN_LOG_MI_SIZE)*pictureWrite->lumaFltStride[0] + (mb->mbPelY[PLANE_LUMA] >> XIN_LOG_MI_SIZE);
    lumaFlt[1]   += (mb->mbPelY[PLANE_LUMA] >> XIN_LOG_MI_SIZE)*pictureWrite->lumaFltStride[1] + (mb->mbPelX[PLANE_LUMA] >> XIN_LOG_MI_SIZE);
    chromaFlt[0] += (mb->mbPelX[PLANE_CHROMA] >> XIN_LOG_MI_SIZE)*pictureWrite->chromaFltStride[0] + (mb->mbPelY[PLANE_CHROMA] >> XIN_LOG_MI_SIZE);
    chromaFlt[1] += (mb->mbPelY[PLANE_CHROMA] >> XIN_LOG_MI_SIZE)*pictureWrite->chromaFltStride[1] + (mb->mbPelX[PLANE_CHROMA] >> XIN_LOG_MI_SIZE);

    SetEdgefilterMb (
        secSet,
        mb,
        lumaFlt,
        chromaFlt);

    if (mb->tuNum > 1)
    {
        SetEdgefilterTu (
            secSet,
            mb,
            lumaFlt,
            chromaFlt);
    }

}

void Xin265pComputeBsMbRec (
    xin_sec_struct *secSet,
    xin_mb_struct  *mb)
{

    if (!(mb->geomFlag & XIN_MB_PRESENT))
    {
        return;
    }

    if (mb->splitType == XIN_PARTITION_SPLIT)
    {
        Xin265pComputeBsMbRec (
            secSet,
            mb->childMb[0]);

        Xin265pComputeBsMbRec (
            secSet,
            mb->childMb[1]);

        Xin265pComputeBsMbRec (
            secSet,
            mb->childMb[2]);

        Xin265pComputeBsMbRec (
            secSet,
            mb->childMb[3]);

    }
    else if (mb->splitType == XIN_PARTITION_NONE)
    {
        Xin265pComputeBsMb (
            secSet,
            mb);
    }
    else
    {
        Xin265pComputeBsMb (
            secSet,
            mb->childMb[0]);

        Xin265pComputeBsMb (
            secSet,
            mb->childMb[1]);
    }

}


void Xin265pComputeBsSb (
    xin_sec_struct *secSet)
{
    xin_pic_struct  *picSet;
    xin_mb_struct   *mb;
    xin_ref_picture *pictureWrite;

    mb           = secSet->pqMbData[0];
    picSet       = secSet->picSet;
    pictureWrite = picSet->pictureWrite;

    if ((pictureWrite->fltLvl[0] == 0) && (pictureWrite->fltLvl[1] == 0))
    {
        return;
    }
    
    Xin265pComputeBsMbRec (
        secSet,
        mb);

}

