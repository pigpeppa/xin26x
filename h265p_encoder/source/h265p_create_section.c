/***************************************************************************//**
*
* @file          h265p_create_section.c
* @brief         Section level memory allocation and initialization.
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#include "xin_typedef.h"
#include "stdlib.h"
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
#include "h26x_calc_log.h"
#include "xin26x_logger.h"
#include "h265p_common_data.h"

static void Xin265pScanMb (
    xin_sec_struct *secSet,
    xin_mb_struct  *mbData,
    UINT32         mbWidth,
    UINT32         mbHeight)
{
    xin_seq_struct *seqSet;
    xin_mb_struct  *mb;
    UINT32         sbSize;
    UINT32         rowIdx;
    UINT32         colIdx;
    UINT32         mbIdx;
    UINT32         sbWidthInMb;
    UINT32         sbHeightInMb;
    UINT32         mbWidthUv;
    UINT32         mbHeightUv;

    seqSet  = secSet->seqSet;
    sbSize  = seqSet->sbSize;

    sbWidthInMb  = sbSize / mbWidth;
    sbHeightInMb = sbSize / mbHeight;
    mbWidthUv    = XIN_MAX (XIN_MIN_BLOCK_SIZE, mbWidth / 2);
    mbHeightUv   = XIN_MAX (XIN_MIN_BLOCK_SIZE, mbHeight / 2);

    for (rowIdx = 0; rowIdx < sbHeightInMb; rowIdx++)
    {
        for (colIdx = 0; colIdx < sbWidthInMb; colIdx++)
        {
            mbIdx = rowIdx*sbWidthInMb + colIdx;
            mb    = mbData + mbIdx;

            mb->width[PLANE_LUMA]    = (UINT8)mbWidth;
            mb->height[PLANE_LUMA]   = (UINT8)mbHeight;
            mb->width[PLANE_CHROMA]  = (UINT8)mbWidthUv;
            mb->height[PLANE_CHROMA] = (UINT8)mbHeightUv;

            mb->lgWidth[PLANE_LUMA]    = (UINT8)calcLog2[mbWidth];
            mb->lgHeight[PLANE_LUMA]   = (UINT8)calcLog2[mbHeight];
            mb->lgWidth[PLANE_CHROMA]  = (UINT8)calcLog2[mbWidthUv];
            mb->lgHeight[PLANE_CHROMA] = (UINT8)calcLog2[mbHeightUv];

            mb->offX[PLANE_LUMA]   = (UINT8)(colIdx*mbWidth);
            mb->offY[PLANE_LUMA]   = (UINT8)(rowIdx*mbHeight);
            mb->offX[PLANE_CHROMA] = (UINT8)((colIdx*mbWidth/2) & 0xFFFFFFFC);
            mb->offY[PLANE_CHROMA] = (UINT8)((rowIdx*mbHeight/2) & 0xFFFFFFFC);

            mb->blockSize = (UINT8)blockDim2BlockSize[mb->lgWidth[PLANE_LUMA]][mb->lgHeight[PLANE_LUMA]];
            mb->canSplit  = ((mbWidth > 8) && (mbHeight > 8) && (mbWidth == mbHeight)) ? seqSet->config.enablePartMask : 0;

        }

    }

}

static SINT32 Xin265pScanTu (
    xin_mb_struct  *mb)
{
    xin_tu_struct *tu;
    UINT32        tuNum;
    UINT32        depthNum;
    UINT32        depthIdx;
    UINT8         tuIdx;
    UINT32        tuBase;
    UINT8         mbWidth;
    UINT8         mbHeight;
    UINT8         mbWidthUv;
    UINT8         mbHeightUv;
    UINT8         tuHeight[2];
    UINT8         tuWidth[2];
    UINT32        rowIdx;
    UINT32        colIdx;
    UINT32        mbWidthInTu;
    UINT32        mbHeightInTu;

    mbWidth  = (UINT8)mb->width[PLANE_LUMA];
    mbHeight = (UINT8)mb->height[PLANE_LUMA];
    tuNum    = 0;
    tuBase   = 0;

    if (mbWidth == mbHeight)
    {
        depthNum    = (mbWidth != 4) ? 2 : 1;
        tuHeight[0] = mbHeight;
        tuWidth[0]  = mbWidth;
        tuHeight[1] = mbHeight >> 1;
        tuWidth[1]  = mbWidth >> 1;
    }
    else if ((mbWidth == 2*mbHeight) || (mbWidth*2 == mbHeight))
    {
        depthNum    = 2;
        tuHeight[0] = mbHeight;
        tuWidth[0]  = mbWidth;
        tuHeight[1] = XIN_MIN (mbHeight, mbWidth);
        tuWidth[1]  = XIN_MIN (mbHeight, mbWidth);
    }
    else
    {
        depthNum    = 1;
        tuHeight[0] = mbHeight;
        tuWidth[0]  = mbWidth;
        tuHeight[1] = 0;
        tuWidth[1]  = 0;
    }

    for (depthIdx = 0; depthIdx < depthNum; depthIdx++)
    {
        mbHeightInTu = mbHeight / tuHeight[depthIdx];
        mbWidthInTu  = mbWidth / tuWidth[depthIdx];

        tuNum += mbHeightInTu*mbWidthInTu;
    }

    XIN_MALLOC_CHECK (mb->tu[PLANE_LUMA], tuNum*sizeof(xin_tu_struct));

    for (depthIdx = 0; depthIdx < depthNum; depthIdx++)
    {
        mbHeightInTu = mbHeight / tuHeight[depthIdx];
        mbWidthInTu  = mbWidth / tuWidth[depthIdx];

        for (rowIdx = 0; rowIdx < mbHeightInTu; rowIdx++)
        {
            for (colIdx = 0; colIdx < mbWidthInTu; colIdx++)
            {
                tuIdx = (UINT8)(rowIdx*mbWidthInTu + colIdx);
                tu    = mb->tu[PLANE_LUMA] + tuIdx + tuBase;

                tu->tuIdx    = tuIdx;
                tu->width    = tuWidth[depthIdx];
                tu->height   = tuHeight[depthIdx];
                tu->offsetX  = (UINT8)(tuWidth[depthIdx] * colIdx);
                tu->offsetY  = (UINT8)(tuHeight[depthIdx] * rowIdx);

                tu->lgWidth  = (UINT8)calcLog2[tuWidth[depthIdx]];
                tu->lgHeight = (UINT8)calcLog2[tuHeight[depthIdx]];

                tu->widthInMi  = tuWidth[depthIdx] >> XIN_LOG_MI_SIZE;
                tu->heightInMi = tuHeight[depthIdx] >> XIN_LOG_MI_SIZE;

            }
        }

        tuBase += mbHeightInTu*mbWidthInTu;

    }

    mbWidthUv  = (UINT8)mb->width[PLANE_CHROMA];
    mbHeightUv = (UINT8)mb->height[PLANE_CHROMA];
    tuNum      = 0;
    tuBase     = 0;

    if ((mbWidth == XIN_MAX_SB_SIZE) || (mbHeight == XIN_MAX_SB_SIZE))
    {
        depthNum    = 2;
        tuHeight[0] = mbHeightUv;
        tuWidth[0]  = mbWidthUv;

        tuHeight[1] = mbHeightUv >> 1;
        tuWidth[1]  = mbWidthUv >> 1;
    }
    else
    {
        depthNum    = 1;
        tuHeight[0] = mbHeightUv;
        tuWidth[0]  = mbWidthUv;
    }

    for (depthIdx = 0; depthIdx < depthNum; depthIdx++)
    {
        mbHeightInTu = mbHeightUv / tuHeight[depthIdx];
        mbWidthInTu  = mbWidthUv / tuWidth[depthIdx];

        tuNum += mbHeightInTu*mbWidthInTu;
    }

    XIN_MALLOC_CHECK (mb->tu[PLANE_CHROMA_U], tuNum*sizeof(xin_tu_struct));
    XIN_MALLOC_CHECK (mb->tu[PLANE_CHROMA_V], tuNum*sizeof(xin_tu_struct));

    for (depthIdx = 0; depthIdx < depthNum; depthIdx++)
    {
        mbHeightInTu = mbHeightUv / tuHeight[depthIdx];
        mbWidthInTu  = mbWidthUv / tuWidth[depthIdx];

        for (rowIdx = 0; rowIdx < mbHeightInTu; rowIdx++)
        {
            for (colIdx = 0; colIdx < mbWidthInTu; colIdx++)
            {
                tuIdx = (UINT8)(rowIdx*mbWidthInTu + colIdx);

                tu = mb->tu[PLANE_CHROMA_U] + tuIdx + tuBase;

                tu->tuIdx    = tuIdx;
                tu->width    = tuWidth[depthIdx];
                tu->height   = tuHeight[depthIdx];
                tu->offsetX  = (UINT8)(tuWidth[depthIdx]*colIdx);
                tu->offsetY  = (UINT8)(tuHeight[depthIdx]*rowIdx);

                tu->lgWidth  = (UINT8)calcLog2[tuWidth[depthIdx]];
                tu->lgHeight = (UINT8)calcLog2[tuHeight[depthIdx]];

                tu->widthInMi  = tuWidth[depthIdx] >> XIN_LOG_MI_SIZE;
                tu->heightInMi = tuHeight[depthIdx] >> XIN_LOG_MI_SIZE;

                tu = mb->tu[PLANE_CHROMA_V] + tuIdx + tuBase;

                tu->tuIdx    = tuIdx;
                tu->width    = tuWidth[depthIdx];
                tu->height   = tuHeight[depthIdx];
                tu->offsetX  = (UINT8)(tuWidth[depthIdx]*colIdx);
                tu->offsetY  = (UINT8)(tuHeight[depthIdx]*rowIdx);

                tu->lgWidth  = (UINT8)calcLog2[tuWidth[depthIdx]];
                tu->lgHeight = (UINT8)calcLog2[tuHeight[depthIdx]];

                tu->widthInMi  = tuWidth[depthIdx] >> XIN_LOG_MI_SIZE;
                tu->heightInMi = tuHeight[depthIdx] >> XIN_LOG_MI_SIZE;

            }

        }

        tuBase += mbHeightInTu*mbWidthInTu;

    }

    return XIN_SUCCESS;

}

static SINT32 ConstructMb (
    xin_sec_struct *secSet)
{
    xin_seq_struct *seqSet;
    xin_mb_struct  *mb;
    UINT32         qtDepth;
    UINT32         qtMaxDepth;
    UINT32         btDepth;
    UINT32         btMaxDepth;
    UINT32         sbSize;
    UINT32         mbWidth;
    UINT32         mbHeight;
    UINT32         mbNum;
    UINT32         mbIdx;

    seqSet     = secSet->seqSet;
    qtMaxDepth = seqSet->config.sbSize == XIN_MAX_SB_SIZE ? 5 : 4;
    btMaxDepth = seqSet->config.sbSize == XIN_MAX_SB_SIZE ? 4 : 3;
    sbSize     = seqSet->sbSize;

    for (qtDepth = 0; qtDepth < qtMaxDepth; qtDepth++)
    {
        mbWidth  = sbSize >> qtDepth;
        mbHeight = sbSize >> qtDepth;
        mbNum    = (sbSize*sbSize) / (mbWidth*mbHeight);

        XIN_MALLOC_CHECK (secSet->pqMbData[qtDepth], sizeof(xin_mb_struct)*mbNum);

        Xin265pScanMb (
            secSet,
            secSet->pqMbData[qtDepth],
            mbWidth,
            mbHeight);

        for (mbIdx = 0; mbIdx < mbNum; mbIdx++)
        {
            mb = secSet->pqMbData[qtDepth] + mbIdx;

            if (Xin265pScanTu (mb) == XIN_FAIL)
            {
                return XIN_FAIL;
            }
        }

    }

    for (btDepth = 0; btDepth < btMaxDepth; btDepth++)
    {
        // horizontal split
        mbWidth  = sbSize >> btDepth;
        mbHeight = sbSize >> (btDepth + 1);
        mbNum    = (sbSize*sbSize) / (mbWidth*mbHeight);

        XIN_MALLOC_CHECK (secSet->phMbData[btDepth], sizeof(xin_mb_struct)*mbNum);

        Xin265pScanMb (
            secSet,
            secSet->phMbData[btDepth],
            mbWidth,
            mbHeight);

        for (mbIdx = 0; mbIdx < mbNum; mbIdx++)
        {
            mb = secSet->phMbData[btDepth] + mbIdx;

            if (Xin265pScanTu (mb) == XIN_FAIL)
            {
                return XIN_FAIL;
            }
        }

        // vertical split
        mbWidth  = sbSize >> (btDepth + 1);
        mbHeight = sbSize >> btDepth;
        mbNum    = (sbSize*sbSize) / (mbWidth*mbHeight);

        XIN_MALLOC_CHECK (secSet->pvMbData[btDepth], sizeof(xin_mb_struct)*mbNum);

        Xin265pScanMb (
            secSet,
            secSet->pvMbData[btDepth],
            mbWidth,
            mbHeight);

        for (mbIdx = 0; mbIdx < mbNum; mbIdx++)
        {
            mb = secSet->pvMbData[btDepth] + mbIdx;

            if (Xin265pScanTu (mb) == XIN_FAIL)
            {
                return XIN_FAIL;
            }
        }

    }

    return XIN_SUCCESS;

}

static SINT32 ConstructMe (
    xin_sec_struct *secSet)
{
    xin_me_struct  *meSet;
    xin_seq_struct *seqSet;
    UINT32         sbSize;
    PIXEL          *halfPelH;
    PIXEL          *halfPelV;
    PIXEL          *halfPelHv;
    PIXEL          *integPel;

    seqSet  = secSet->seqSet;
    sbSize  = seqSet->sbSize;

    XIN_MALLOC_CHECK (meSet, sizeof(xin_me_struct));
    XIN_MALLOC_CHECK (meSet->interpBuf, XIN_ME_BUF_STRIDE*XIN_ME_BUF_STRIDE*4);
    XIN_MALLOC_CHECK (meSet->input1, sbSize*sbSize / 4);
    XIN_MALLOC_CHECK (meSet->input2, sbSize*sbSize / 16);

    meSet->input1Stride = sbSize / 2;
    meSet->input2Stride = sbSize / 4;
    meSet->interpStride = XIN_ME_BUF_STRIDE;

    integPel    = meSet->interpBuf;
    halfPelH    = meSet->interpBuf + XIN_ME_BUF_STRIDE*XIN_ME_BUF_STRIDE;
    halfPelV    = meSet->interpBuf + XIN_ME_BUF_STRIDE*XIN_ME_BUF_STRIDE*2;
    halfPelHv   = meSet->interpBuf + XIN_ME_BUF_STRIDE*XIN_ME_BUF_STRIDE*3;

    meSet->integPel  = integPel;
    meSet->halfPelH  = halfPelH;
    meSet->halfPelV  = halfPelV;
    meSet->halfPelHv = halfPelHv;

    halfPelH   += (XIN_ME_FILTER_TAP>>1)*XIN_ME_BUF_STRIDE;
    halfPelV   += 1;
    integPel   += XIN_ME_BUF_STRIDE + 1;

    meSet->halfPel[LFT_POS]     = halfPelH;
    meSet->halfPel[RGT_POS]     = halfPelH + 1;
    meSet->halfPel[TOP_POS]     = halfPelV;
    meSet->halfPel[BOT_POS]     = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->halfPel[TOP_LFT_POS] = halfPelHv;
    meSet->halfPel[TOP_RGT_POS] = halfPelHv + 1;
    meSet->halfPel[BOT_LFT_POS] = halfPelHv + XIN_ME_BUF_STRIDE;
    meSet->halfPel[BOT_RGT_POS] = halfPelHv + XIN_ME_BUF_STRIDE + 1;

    // The following is seting quarter pixel motion estimation buffer
    meSet->qBufA[LFT_POS][LFT_POS]     = integPel - 1;
    meSet->qBufB[LFT_POS][LFT_POS]     = halfPelH;
    meSet->qBufA[LFT_POS][RGT_POS]     = integPel;
    meSet->qBufB[LFT_POS][RGT_POS]     = halfPelH;
    meSet->qBufA[LFT_POS][TOP_POS]     = halfPelHv;
    meSet->qBufB[LFT_POS][TOP_POS]     = halfPelH;
    meSet->qBufA[LFT_POS][BOT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE;
    meSet->qBufB[LFT_POS][BOT_POS]     = halfPelH;
    meSet->qBufA[LFT_POS][TOP_LFT_POS] = halfPelV - 1;
    meSet->qBufB[LFT_POS][TOP_LFT_POS] = halfPelH;
    meSet->qBufA[LFT_POS][TOP_RGT_POS] = halfPelV;
    meSet->qBufB[LFT_POS][TOP_RGT_POS] = halfPelH;
    meSet->qBufA[LFT_POS][BOT_LFT_POS] = halfPelV + XIN_ME_BUF_STRIDE - 1;
    meSet->qBufB[LFT_POS][BOT_LFT_POS] = halfPelH;
    meSet->qBufA[LFT_POS][BOT_RGT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufB[LFT_POS][BOT_RGT_POS] = halfPelH;

    meSet->qBufA[RGT_POS][LFT_POS]     = integPel;
    meSet->qBufB[RGT_POS][LFT_POS]     = halfPelH + 1;
    meSet->qBufA[RGT_POS][RGT_POS]     = integPel + 1;
    meSet->qBufB[RGT_POS][RGT_POS]     = halfPelH + 1;
    meSet->qBufA[RGT_POS][TOP_POS]     = halfPelHv + 1;
    meSet->qBufB[RGT_POS][TOP_POS]     = halfPelH + 1;
    meSet->qBufA[RGT_POS][BOT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[RGT_POS][BOT_POS]     = halfPelH + 1;
    meSet->qBufA[RGT_POS][TOP_LFT_POS] = halfPelV;
    meSet->qBufB[RGT_POS][TOP_LFT_POS] = halfPelH + 1;
    meSet->qBufA[RGT_POS][TOP_RGT_POS] = halfPelV + 1;
    meSet->qBufB[RGT_POS][TOP_RGT_POS] = halfPelH + 1;
    meSet->qBufA[RGT_POS][BOT_LFT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufB[RGT_POS][BOT_LFT_POS] = halfPelH + 1;
    meSet->qBufA[RGT_POS][BOT_RGT_POS] = halfPelV + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[RGT_POS][BOT_RGT_POS] = halfPelH + 1;

    meSet->qBufA[TOP_POS][LFT_POS]     = halfPelHv;
    meSet->qBufB[TOP_POS][LFT_POS]     = halfPelV;
    meSet->qBufA[TOP_POS][RGT_POS]     = halfPelHv + 1;
    meSet->qBufB[TOP_POS][RGT_POS]     = halfPelV;
    meSet->qBufA[TOP_POS][TOP_POS]     = integPel - XIN_ME_BUF_STRIDE;
    meSet->qBufB[TOP_POS][TOP_POS]     = halfPelV;
    meSet->qBufA[TOP_POS][BOT_POS]     = integPel;
    meSet->qBufB[TOP_POS][BOT_POS]     = halfPelV;
    meSet->qBufA[TOP_POS][TOP_LFT_POS] = halfPelH - XIN_ME_BUF_STRIDE;
    meSet->qBufB[TOP_POS][TOP_LFT_POS] = halfPelV;
    meSet->qBufA[TOP_POS][TOP_RGT_POS] = halfPelH - XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[TOP_POS][TOP_RGT_POS] = halfPelV;
    meSet->qBufA[TOP_POS][BOT_LFT_POS] = halfPelH;
    meSet->qBufB[TOP_POS][BOT_LFT_POS] = halfPelV;
    meSet->qBufA[TOP_POS][BOT_RGT_POS] = halfPelH + 1;
    meSet->qBufB[TOP_POS][BOT_RGT_POS] = halfPelV;

    meSet->qBufA[BOT_POS][LFT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_POS][LFT_POS]     = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_POS][RGT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[BOT_POS][RGT_POS]     = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_POS][TOP_POS]     = integPel;
    meSet->qBufB[BOT_POS][TOP_POS]     = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_POS][BOT_POS]     = integPel + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_POS][BOT_POS]     = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_POS][TOP_LFT_POS] = halfPelH;
    meSet->qBufB[BOT_POS][TOP_LFT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_POS][TOP_RGT_POS] = halfPelH + 1;
    meSet->qBufB[BOT_POS][TOP_RGT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_POS][BOT_LFT_POS] = halfPelH + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_POS][BOT_LFT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_POS][BOT_RGT_POS] = halfPelH + 1 + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_POS][BOT_RGT_POS] = halfPelV + XIN_ME_BUF_STRIDE;

    meSet->qBufA[TOP_LFT_POS][LFT_POS]     = halfPelV - 1;
    meSet->qBufB[TOP_LFT_POS][LFT_POS]     = halfPelHv;
    meSet->qBufA[TOP_LFT_POS][RGT_POS]     = halfPelV;
    meSet->qBufB[TOP_LFT_POS][RGT_POS]     = halfPelHv;
    meSet->qBufA[TOP_LFT_POS][TOP_POS]     = halfPelH - XIN_ME_BUF_STRIDE;
    meSet->qBufB[TOP_LFT_POS][TOP_POS]     = halfPelHv;
    meSet->qBufA[TOP_LFT_POS][BOT_POS]     = halfPelH;
    meSet->qBufB[TOP_LFT_POS][BOT_POS]     = halfPelHv;
    meSet->qBufA[TOP_LFT_POS][TOP_LFT_POS] = halfPelV - 1;
    meSet->qBufB[TOP_LFT_POS][TOP_LFT_POS] = halfPelH - XIN_ME_BUF_STRIDE;
    meSet->qBufA[TOP_LFT_POS][TOP_RGT_POS] = halfPelH - XIN_ME_BUF_STRIDE;
    meSet->qBufB[TOP_LFT_POS][TOP_RGT_POS] = halfPelV;
    meSet->qBufA[TOP_LFT_POS][BOT_LFT_POS] = halfPelH;
    meSet->qBufB[TOP_LFT_POS][BOT_LFT_POS] = halfPelV - 1;
    meSet->qBufA[TOP_LFT_POS][BOT_RGT_POS] = halfPelV;
    meSet->qBufB[TOP_LFT_POS][BOT_RGT_POS] = halfPelH;

    meSet->qBufA[TOP_RGT_POS][LFT_POS]     = halfPelV;
    meSet->qBufB[TOP_RGT_POS][LFT_POS]     = halfPelHv + 1;
    meSet->qBufA[TOP_RGT_POS][RGT_POS]     = halfPelV + 1;
    meSet->qBufB[TOP_RGT_POS][RGT_POS]     = halfPelHv + 1;
    meSet->qBufA[TOP_RGT_POS][TOP_POS]     = halfPelH - XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[TOP_RGT_POS][TOP_POS]     = halfPelHv + 1;
    meSet->qBufA[TOP_RGT_POS][BOT_POS]     = halfPelH + 1;
    meSet->qBufB[TOP_RGT_POS][BOT_POS]     = halfPelHv + 1;
    meSet->qBufA[TOP_RGT_POS][TOP_LFT_POS] = halfPelV;
    meSet->qBufB[TOP_RGT_POS][TOP_LFT_POS] = halfPelH - XIN_ME_BUF_STRIDE + 1;
    meSet->qBufA[TOP_RGT_POS][TOP_RGT_POS] = halfPelH - XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[TOP_RGT_POS][TOP_RGT_POS] = halfPelV + 1;
    meSet->qBufA[TOP_RGT_POS][BOT_LFT_POS] = halfPelH + 1;
    meSet->qBufB[TOP_RGT_POS][BOT_LFT_POS] = halfPelV;
    meSet->qBufA[TOP_RGT_POS][BOT_RGT_POS] = halfPelV + 1;
    meSet->qBufB[TOP_RGT_POS][BOT_RGT_POS] = halfPelH + 1;

    meSet->qBufA[BOT_LFT_POS][LFT_POS]     = halfPelV + XIN_ME_BUF_STRIDE - 1;
    meSet->qBufB[BOT_LFT_POS][LFT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_LFT_POS][RGT_POS]     = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_LFT_POS][RGT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_LFT_POS][TOP_POS]     = halfPelH;
    meSet->qBufB[BOT_LFT_POS][TOP_POS]     = halfPelHv + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_LFT_POS][BOT_POS]     = halfPelH + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_LFT_POS][BOT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_LFT_POS][TOP_LFT_POS] = halfPelV + XIN_ME_BUF_STRIDE - 1;
    meSet->qBufB[BOT_LFT_POS][TOP_LFT_POS] = halfPelH;
    meSet->qBufA[BOT_LFT_POS][TOP_RGT_POS] = halfPelH;
    meSet->qBufB[BOT_LFT_POS][TOP_RGT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_LFT_POS][BOT_LFT_POS] = halfPelH + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_LFT_POS][BOT_LFT_POS] = halfPelV + XIN_ME_BUF_STRIDE - 1;
    meSet->qBufA[BOT_LFT_POS][BOT_RGT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_LFT_POS][BOT_RGT_POS] = halfPelH + XIN_ME_BUF_STRIDE;

    meSet->qBufA[BOT_RGT_POS][LFT_POS]     = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_RGT_POS][LFT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufA[BOT_RGT_POS][RGT_POS]     = halfPelV + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[BOT_RGT_POS][RGT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufA[BOT_RGT_POS][TOP_POS]     = halfPelH + 1;
    meSet->qBufB[BOT_RGT_POS][TOP_POS]     = halfPelHv + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufA[BOT_RGT_POS][BOT_POS]     = halfPelH + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[BOT_RGT_POS][BOT_POS]     = halfPelHv + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufA[BOT_RGT_POS][TOP_LFT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufB[BOT_RGT_POS][TOP_LFT_POS] = halfPelH + 1;
    meSet->qBufA[BOT_RGT_POS][TOP_RGT_POS] = halfPelH + 1;
    meSet->qBufB[BOT_RGT_POS][TOP_RGT_POS] = halfPelV + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufA[BOT_RGT_POS][BOT_LFT_POS] = halfPelH + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[BOT_RGT_POS][BOT_LFT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufA[BOT_RGT_POS][BOT_RGT_POS] = halfPelV + XIN_ME_BUF_STRIDE + 1;
    meSet->qBufB[BOT_RGT_POS][BOT_RGT_POS] = halfPelH + XIN_ME_BUF_STRIDE + 1;

    meSet->qBufA[CEN_POS][LFT_POS]     = halfPelH;
    meSet->qBufB[CEN_POS][LFT_POS]     = integPel;
    meSet->qBufA[CEN_POS][RGT_POS]     = halfPelH + 1;
    meSet->qBufB[CEN_POS][RGT_POS]     = integPel;
    meSet->qBufA[CEN_POS][TOP_POS]     = halfPelV;
    meSet->qBufB[CEN_POS][TOP_POS]     = integPel;
    meSet->qBufA[CEN_POS][BOT_POS]     = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufB[CEN_POS][BOT_POS]     = integPel;
    meSet->qBufA[CEN_POS][TOP_LFT_POS] = halfPelV;
    meSet->qBufB[CEN_POS][TOP_LFT_POS] = halfPelH;
    meSet->qBufA[CEN_POS][TOP_RGT_POS] = halfPelV;
    meSet->qBufB[CEN_POS][TOP_RGT_POS] = halfPelH + 1;
    meSet->qBufA[CEN_POS][BOT_LFT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufB[CEN_POS][BOT_LFT_POS] = halfPelH;
    meSet->qBufA[CEN_POS][BOT_RGT_POS] = halfPelV + XIN_ME_BUF_STRIDE;
    meSet->qBufB[CEN_POS][BOT_RGT_POS] = halfPelH + 1;

    secSet->meSet = meSet;

    return XIN_SUCCESS;

}

static SINT32 Xin265pConstructMdBuffer (
    xin_fast_md_buf **dblFastBuf,
    UINT32          fastBufNum,
    xin_full_md_buf **dblFullBuf,
    UINT32          fullBufNum,
    UINT32          sbSize)
{
    xin_fast_md_buf *fastBuf;
    xin_full_md_buf *fullBuf;
    UINT32          bufIdx;

    XIN_MALLOC_CHECK (fastBuf, fastBufNum*sizeof(xin_fast_md_buf));

    for (bufIdx = 0; bufIdx < fastBufNum; bufIdx++)
    {
        XIN_MALLOC_CHECK (fastBuf[bufIdx].predBuf[PLANE_LUMA],     sbSize*sbSize*sizeof(PIXEL));
        XIN_MALLOC_CHECK (fastBuf[bufIdx].predBuf[PLANE_CHROMA_U], (sbSize*sbSize/4)*sizeof(PIXEL));
        XIN_MALLOC_CHECK (fastBuf[bufIdx].predBuf[PLANE_CHROMA_V], (sbSize*sbSize/4)*sizeof(PIXEL));

        fastBuf[bufIdx].predStride[0] = sbSize;
        fastBuf[bufIdx].predStride[1] = sbSize/2;
    }

    XIN_MALLOC_CHECK (fullBuf, fullBufNum*sizeof(xin_full_md_buf));

    for (bufIdx = 0; bufIdx < fullBufNum; bufIdx++)
    {
        XIN_MALLOC_CHECK (fullBuf[bufIdx].qCoefBuf[PLANE_LUMA],     sbSize*sbSize*sizeof(COEFF));
        XIN_MALLOC_CHECK (fullBuf[bufIdx].qCoefBuf[PLANE_CHROMA_U], (sbSize*sbSize/4)*sizeof(COEFF));
        XIN_MALLOC_CHECK (fullBuf[bufIdx].qCoefBuf[PLANE_CHROMA_V], (sbSize*sbSize/4)*sizeof(COEFF));

        XIN_MALLOC_CHECK (fullBuf[bufIdx].tCoefBuf[PLANE_LUMA],     sbSize*sbSize*sizeof(SINT32));
        XIN_MALLOC_CHECK (fullBuf[bufIdx].tCoefBuf[PLANE_CHROMA_U], (sbSize*sbSize/4)*sizeof(SINT32));
        XIN_MALLOC_CHECK (fullBuf[bufIdx].tCoefBuf[PLANE_CHROMA_V], (sbSize*sbSize/4)*sizeof(SINT32));

        XIN_MALLOC_CHECK (fullBuf[bufIdx].rCoefBuf[PLANE_LUMA],     sbSize*sbSize*sizeof(SINT32));
        XIN_MALLOC_CHECK (fullBuf[bufIdx].rCoefBuf[PLANE_CHROMA_U], (sbSize*sbSize/4)*sizeof(SINT32));
        XIN_MALLOC_CHECK (fullBuf[bufIdx].rCoefBuf[PLANE_CHROMA_V], (sbSize*sbSize/4)*sizeof(SINT32));

        fullBuf[bufIdx].coefStride[0] = sbSize;
        fullBuf[bufIdx].coefStride[1] = sbSize / 2;
    }

    *dblFastBuf = fastBuf;
    *dblFullBuf = fullBuf;

    return XIN_SUCCESS;

}

static void Xin265pDestructMdBuffer (
    xin_fast_md_buf *fastBuf,
    UINT32          fastBufNum,
    xin_full_md_buf *fullBuf,
    UINT32          fullBufNum)
{
    UINT32  bufIdx;

    if (fastBufNum)
    {
        for (bufIdx = 0; bufIdx < fastBufNum; bufIdx++)
        {
            free (fastBuf[bufIdx].predBuf[PLANE_LUMA]);
            free (fastBuf[bufIdx].predBuf[PLANE_CHROMA_U]);
            free (fastBuf[bufIdx].predBuf[PLANE_CHROMA_V]);
        }

        free (fastBuf);
    }

    if (fullBufNum)
    {
        for (bufIdx = 0; bufIdx < fullBufNum; bufIdx++)
        {
            free (fullBuf[bufIdx].qCoefBuf[PLANE_LUMA]);
            free (fullBuf[bufIdx].qCoefBuf[PLANE_CHROMA_U]);
            free (fullBuf[bufIdx].qCoefBuf[PLANE_CHROMA_V]);

            free (fullBuf[bufIdx].tCoefBuf[PLANE_LUMA]);
            free (fullBuf[bufIdx].tCoefBuf[PLANE_CHROMA_U]);
            free (fullBuf[bufIdx].tCoefBuf[PLANE_CHROMA_V]);

            free (fullBuf[bufIdx].rCoefBuf[PLANE_LUMA]);
            free (fullBuf[bufIdx].rCoefBuf[PLANE_CHROMA_U]);
            free (fullBuf[bufIdx].rCoefBuf[PLANE_CHROMA_V]);
        }

        free (fullBuf);
    }

}

SINT32 Xin265pSecCreate (
    xin_sec_struct **dblSecSet,
    xin_seq_struct *seqSet)
{
    xin_sec_struct  *secSet;
    UINT32          depthIdx;
    UINT32          sbSize;
    UINT32          sbSizeInMi;
    UINT32          maxDepth;
    UINT32          partIdx;
    UINT32          mdBufferWidth;

    XIN_MALLOC_CHECK (secSet, sizeof(xin_sec_struct));

    memset (secSet, 0, sizeof(xin_sec_struct));

    *dblSecSet     = secSet;
    secSet->seqSet = seqSet;

    maxDepth       = (seqSet->config.sbSize == 128) ? 5 : 4;
    sbSize         = seqSet->sbSize;
    sbSizeInMi     = sbSize >> XIN_LOG_MI_SIZE;
    mdBufferWidth  = seqSet->config.mergeRdoNum + 1;
    mdBufferWidth += seqSet->config.intraRdoNum + 1;

    secSet->fastMdBufStart[XIN_SKIP_MODE]  = 0;
    secSet->fastMdBufStart[XIN_INTER_MODE] = seqSet->config.mergeRdoNum + 1;
    secSet->fastMdBufStart[XIN_INTRA_MODE] = secSet->fastMdBufStart[XIN_INTER_MODE];

    for (depthIdx = 0; depthIdx <= maxDepth; depthIdx++)
    {
        for (partIdx = 0; partIdx < XIN_PART_NUM; partIdx++)
        {
            if ((seqSet->config.enablePartMask & (1 << partIdx)) && ((depthIdx != 0) || (partIdx == XIN_PART_SPLIT)))
            {
                Xin265pConstructMdBuffer (
                    &secSet->fastMdBuf[partIdx][depthIdx],
                    mdBufferWidth,
                    &secSet->fullMdBuf[partIdx][depthIdx],
                    seqSet->config.maxMdCandNum,
                    sbSize);

                secSet->fastMdBufNum[partIdx][depthIdx] = mdBufferWidth;
                secSet->fullMdBufNum[partIdx][depthIdx] = seqSet->config.maxMdCandNum;
            }
            else
            {
                secSet->fastMdBufNum[partIdx][depthIdx] = 0;
                secSet->fullMdBufNum[partIdx][depthIdx] = 0;
            }

        }

    }

    XIN_MALLOC_CHECK (secSet->inputSb[PLANE_LUMA],     sbSize*sbSize*sizeof(PIXEL));
    XIN_MALLOC_CHECK (secSet->inputSb[PLANE_CHROMA_U], (sbSize*sbSize/4)*sizeof(PIXEL));
    XIN_MALLOC_CHECK (secSet->inputSb[PLANE_CHROMA_V], (sbSize*sbSize/4)*sizeof(PIXEL));

    secSet->inputYStride  = sbSize;
    secSet->inputUvStride = sbSize/2;

    secSet->reconStride[0] = seqSet->allocPictures->refStride[0];
    secSet->reconStride[1] = seqSet->allocPictures->refStride[1];

    XIN_MALLOC_CHECK (secSet->tempBuffer, 8192*sizeof(SINT32));
    XIN_MALLOC_CHECK (secSet->predBuffer, 2*2*XIN_MAX_REF_FRAMES*sbSize*sbSize*sizeof(PIXEL));

    if (ConstructMb (secSet) == XIN_FAIL)
    {
        return XIN_FAIL;
    }

    if (ConstructMe (secSet) == XIN_FAIL)
    {
        return XIN_FAIL;
    }

    XIN_MALLOC_CHECK (secSet->sbLftCtx[PLANE_LUMA],     sbSizeInMi*sizeof(UINT8));
    XIN_MALLOC_CHECK (secSet->sbLftCtx[PLANE_CHROMA_U], sbSizeInMi*sizeof(UINT8));
    XIN_MALLOC_CHECK (secSet->sbLftCtx[PLANE_CHROMA_V], sbSizeInMi*sizeof(UINT8));

    XIN_MALLOC_CHECK (secSet->intraBuf, XIN_MAX_SB_SIZE*12);

    secSet->intraLftBuf[PLANE_LUMA]     = secSet->intraBuf + 1;
    secSet->intraLftBuf[PLANE_CHROMA_U] = secSet->intraLftBuf[PLANE_LUMA] + XIN_MAX_SB_SIZE*2 + 1;
    secSet->intraLftBuf[PLANE_CHROMA_V] = secSet->intraLftBuf[PLANE_CHROMA_U] + XIN_MAX_SB_SIZE + 1;

    secSet->intraTopBuf[PLANE_LUMA]     = secSet->intraLftBuf[PLANE_CHROMA_V] + XIN_MAX_SB_SIZE + 1;
    secSet->intraTopBuf[PLANE_CHROMA_U] = secSet->intraTopBuf[PLANE_LUMA] + XIN_MAX_SB_SIZE*2 + 1;
    secSet->intraTopBuf[PLANE_CHROMA_V] = secSet->intraTopBuf[PLANE_CHROMA_U] + XIN_MAX_SB_SIZE + 1;

    if (seqSet->config.enablePartMask | XIN_CAN_PART_HORZ)
    {
        for (depthIdx = 0; depthIdx < maxDepth; depthIdx++)
        {
            XIN_MALLOC_CHECK (secSet->topCtx[depthIdx][XIN_PART_HORZ][PLANE_LUMA],     sbSizeInMi*sizeof(UINT8));
            XIN_MALLOC_CHECK (secSet->topCtx[depthIdx][XIN_PART_HORZ][PLANE_CHROMA_U], sbSizeInMi*sizeof(UINT8));
            XIN_MALLOC_CHECK (secSet->topCtx[depthIdx][XIN_PART_HORZ][PLANE_CHROMA_V], sbSizeInMi*sizeof(UINT8));

            XIN_MALLOC_CHECK (secSet->lftCtx[depthIdx][XIN_PART_HORZ][PLANE_LUMA],     sbSizeInMi*sizeof(UINT8));
            XIN_MALLOC_CHECK (secSet->lftCtx[depthIdx][XIN_PART_HORZ][PLANE_CHROMA_U], sbSizeInMi*sizeof(UINT8));
            XIN_MALLOC_CHECK (secSet->lftCtx[depthIdx][XIN_PART_HORZ][PLANE_CHROMA_V], sbSizeInMi*sizeof(UINT8));
        }
    }

    if (seqSet->config.enablePartMask | XIN_CAN_PART_VERT)
    {
        for (depthIdx = 0; depthIdx < maxDepth; depthIdx++)
        {
            XIN_MALLOC_CHECK (secSet->topCtx[depthIdx][XIN_PART_VERT][PLANE_LUMA],     sbSizeInMi*sizeof(UINT8));
            XIN_MALLOC_CHECK (secSet->topCtx[depthIdx][XIN_PART_VERT][PLANE_CHROMA_U], sbSizeInMi*sizeof(UINT8));
            XIN_MALLOC_CHECK (secSet->topCtx[depthIdx][XIN_PART_VERT][PLANE_CHROMA_V], sbSizeInMi*sizeof(UINT8));

            XIN_MALLOC_CHECK (secSet->lftCtx[depthIdx][XIN_PART_VERT][PLANE_LUMA],     sbSizeInMi*sizeof(UINT8));
            XIN_MALLOC_CHECK (secSet->lftCtx[depthIdx][XIN_PART_VERT][PLANE_CHROMA_U], sbSizeInMi*sizeof(UINT8));
            XIN_MALLOC_CHECK (secSet->lftCtx[depthIdx][XIN_PART_VERT][PLANE_CHROMA_V], sbSizeInMi*sizeof(UINT8));
        }
    }

    XIN_MALLOC_CHECK (secSet->reconData[PLANE_LUMA],     sbSize*sbSize*sizeof(PIXEL));
    XIN_MALLOC_CHECK (secSet->reconData[PLANE_CHROMA_U], sbSize*sbSize*sizeof(PIXEL)/4);
    XIN_MALLOC_CHECK (secSet->reconData[PLANE_CHROMA_V], sbSize*sbSize*sizeof(PIXEL)/4);

    secSet->reconDataStride[PLANE_LUMA]   = sbSize;
    secSet->reconDataStride[PLANE_CHROMA] = sbSize / 2;

    XIN_MALLOC_CHECK (secSet->miData, sbSizeInMi*sbSizeInMi*sizeof(xin_mi_struct));

    secSet->miDataStride = sbSizeInMi;
    secSet->funcSet      = seqSet->funcSet;

    return XIN_SUCCESS;

}

static void DestructMb (
    xin_sec_struct *secSet)
{
    xin_seq_struct *seqSet;
    xin_mb_struct  *mb;
    UINT32         qtDepth;
    UINT32         qtMaxDepth;
    UINT32         btDepth;
    UINT32         btMaxDepth;
    UINT32         sbSize;
    UINT32         mbWidth;
    UINT32         mbHeight;
    UINT32         mbNum;
    UINT32         mbIdx;

    seqSet     = secSet->seqSet;
    qtMaxDepth = seqSet->config.sbSize == XIN_MAX_SB_SIZE ? 5 : 4;
    btMaxDepth = seqSet->config.sbSize == XIN_MAX_SB_SIZE ? 4 : 3;
    sbSize     = seqSet->sbSize;

    for (qtDepth = 0; qtDepth < qtMaxDepth; qtDepth++)
    {
        mbWidth  = sbSize >> qtDepth;
        mbHeight = sbSize >> qtDepth;
        mbNum    = (sbSize*sbSize) / (mbWidth*mbHeight);

        for (mbIdx = 0; mbIdx < mbNum; mbIdx++)
        {
            mb = secSet->pqMbData[qtDepth] + mbIdx;

            free (mb->tu[PLANE_LUMA]);
            free (mb->tu[PLANE_CHROMA_U]);
            free (mb->tu[PLANE_CHROMA_V]);
        }

        free (secSet->pqMbData[qtDepth]);
    }

    for (btDepth = 0; btDepth < btMaxDepth; btDepth++)
    {
        // horizontal split
        mbWidth  = sbSize >> btDepth;
        mbHeight = sbSize >> (btDepth + 1);
        mbNum    = (sbSize*sbSize) / (mbWidth*mbHeight);

        for (mbIdx = 0; mbIdx < mbNum; mbIdx++)
        {
            mb = secSet->phMbData[btDepth] + mbIdx;

            free (mb->tu[PLANE_LUMA]);
            free (mb->tu[PLANE_CHROMA_U]);
            free (mb->tu[PLANE_CHROMA_V]);
        }

        free (secSet->phMbData[btDepth]);

        // vertical split
        mbWidth  = sbSize >> (btDepth + 1);
        mbHeight = sbSize >> btDepth;
        mbNum    = (sbSize*sbSize) / (mbWidth*mbHeight);

        for (mbIdx = 0; mbIdx < mbNum; mbIdx++)
        {
            mb = secSet->pvMbData[btDepth] + mbIdx;

            free (mb->tu[PLANE_LUMA]);
            free (mb->tu[PLANE_CHROMA_U]);
            free (mb->tu[PLANE_CHROMA_V]);
        }

        free (secSet->pvMbData[btDepth]);

    }

}

void Xin265pSecDelete (
    xin_sec_struct *secSet,
    xin_seq_struct *seqSet)
{
    UINT32  maxDepth;
    UINT32  depthIdx;
    UINT32  partIdx;

    maxDepth = (seqSet->config.sbSize == 128) ? 5 : 4;

    for (depthIdx = 0; depthIdx <= maxDepth; depthIdx++)
    {
        for (partIdx = 0; partIdx < XIN_PART_NUM; partIdx++)
        {
            Xin265pDestructMdBuffer (
                secSet->fastMdBuf[partIdx][depthIdx],
                secSet->fastMdBufNum[partIdx][depthIdx],
                secSet->fullMdBuf[partIdx][depthIdx],
                secSet->fullMdBufNum[partIdx][depthIdx]);
        }

    }

    DestructMb (
        secSet);

    free (secSet->inputSb[PLANE_LUMA]);
    free (secSet->inputSb[PLANE_CHROMA_U]);
    free (secSet->inputSb[PLANE_CHROMA_V]);

    free (secSet->tempBuffer);
    free (secSet->predBuffer);

    free (secSet->meSet->interpBuf);
    free (secSet->meSet->input1);
    free (secSet->meSet->input2);
    free (secSet->meSet);

    free (secSet->sbLftCtx[PLANE_LUMA]);
    free (secSet->sbLftCtx[PLANE_CHROMA_U]);
    free (secSet->sbLftCtx[PLANE_CHROMA_V]);
    free (secSet->intraBuf);

    for (depthIdx = 0; depthIdx < maxDepth; depthIdx++)
    {
        free (secSet->topCtx[depthIdx][XIN_PART_HORZ][PLANE_LUMA]);
        free (secSet->topCtx[depthIdx][XIN_PART_HORZ][PLANE_CHROMA_U]);
        free (secSet->topCtx[depthIdx][XIN_PART_HORZ][PLANE_CHROMA_V]);

        free (secSet->lftCtx[depthIdx][XIN_PART_HORZ][PLANE_LUMA]);
        free (secSet->lftCtx[depthIdx][XIN_PART_HORZ][PLANE_CHROMA_U]);
        free (secSet->lftCtx[depthIdx][XIN_PART_HORZ][PLANE_CHROMA_V]);
    }

    for (depthIdx = 0; depthIdx < maxDepth; depthIdx++)
    {
        free (secSet->topCtx[depthIdx][XIN_PART_VERT][PLANE_LUMA]);
        free (secSet->topCtx[depthIdx][XIN_PART_VERT][PLANE_CHROMA_U]);
        free (secSet->topCtx[depthIdx][XIN_PART_VERT][PLANE_CHROMA_V]);

        free (secSet->lftCtx[depthIdx][XIN_PART_VERT][PLANE_LUMA]);
        free (secSet->lftCtx[depthIdx][XIN_PART_VERT][PLANE_CHROMA_U]);
        free (secSet->lftCtx[depthIdx][XIN_PART_VERT][PLANE_CHROMA_V]);
    }

    free (secSet->reconData[PLANE_LUMA]);
    free (secSet->reconData[PLANE_CHROMA_U]);
    free (secSet->reconData[PLANE_CHROMA_V]);


    free (secSet->miData);


    free (secSet);

}

