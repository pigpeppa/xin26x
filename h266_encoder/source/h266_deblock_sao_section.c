/***************************************************************************//**
 *
 * @file          h266_deblock_sao_section.c
 * @brief         Deblock and SAO a picture section in a frame.
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
#include "memory.h"
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
#include "h266_coding_unit_struct.h"
#include "h266_enc_init.h"
#include "h266_encode_ctu.h"
#include "h266_sao_rdo.h"
#include "h26x_thread_wrapper.h"
#include "h26x_thread_pool.h"
#include "h266_alf_rdo.h"
#include "h266_alf.h"

void Xin266DeblockCtuRow (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctuRow,
    BOOL           isVert)
{
    UINT32          colIdx;
    xin_pic_struct  *picSet;
    xin266_tile_dim *tileDim;
    UINT32          widthInCtu;

    picSet     = secSet->picSet;
    tileDim    = secSet->tileDim;
    widthInCtu = tileDim->tileWidthInCtu;

    if (isVert)
    {
        for (colIdx = 0; colIdx < widthInCtu; colIdx++)
        {
            Xin266DeblockCtuVer (
                picSet,
                ctuRow + colIdx);
        }
    }
    else
    {
        for (colIdx = 0; colIdx < widthInCtu; colIdx++)
        {
            Xin266DeblockCtuHor (
                picSet,
                ctuRow + colIdx);
        }
    }

}

void Xin266ExtendSectionHor (
    PIXEL    *buf,
    intptr_t bufStride,
    UINT32   pictureWidth,
    UINT32   pictureHeight,
    UINT32   paddingWidth,
    UINT32   paddingHeight,
    BOOL     isTop)
{
    UINT32  idx;
    PIXEL   *srcBuf;
    PIXEL   *dstBuf;

    (void)paddingWidth;
    srcBuf    = (isTop) ? buf : buf + (pictureHeight - 1)*bufStride;
    bufStride = (isTop) ? bufStride : -bufStride;
    dstBuf    = srcBuf - bufStride;

    for (idx = 0; idx < paddingHeight; idx++)
    {
        // Pad top and bottom
        memcpy(dstBuf, srcBuf, pictureWidth*sizeof(PIXEL));

        dstBuf -= bufStride;
    }

}

void Xin266ExtendSectionVer (
    PIXEL    *buf,
    intptr_t bufStride,
    UINT32   pictureWidth,
    UINT32   pictureHeight,
    UINT32   paddingWidth,
    UINT32   paddingHeight,
    BOOL     isLft)
{
    UINT32  rowIdx;
#ifdef ENABLE_10BIT_ENCODER
    UINT32  colIdx;
#endif
    PIXEL   *srcBuf;
    PIXEL   *dstBuf;

    (void)paddingHeight;
    srcBuf = (isLft) ? buf : buf + pictureWidth - 1;
    dstBuf = (isLft) ? buf - paddingWidth : buf + pictureWidth;

    for (rowIdx = 0; rowIdx < pictureHeight; rowIdx++)
    {
        // Pad left and right
#ifdef ENABLE_10BIT_ENCODER
        for (colIdx = 0; colIdx < paddingWidth; colIdx++)
        {
            dstBuf[colIdx] = srcBuf[0];
        }
#else
        memset (dstBuf, srcBuf[0], paddingWidth);
#endif

        srcBuf += bufStride;
        dstBuf += bufStride;
    }

}

void Xin266AlfPadPixel (
    xin_ctu_struct *ctu)
{
    xin_pic_struct  *picSet;
    xin_seq_struct  *seqSet;
    xin_ref_picture *pictureWrite;
    UINT32          frameWidthInCtu;
    UINT32          frameHeightInCtu;
    PIXEL           *reconCtu[PLANE_NUM];
    intptr_t        reconYStride;
    intptr_t        reconUvStride;

    picSet           = ctu->picSet;
    seqSet           = picSet->seqSet;
    pictureWrite     = picSet->pictureWrite;
    frameWidthInCtu  = seqSet->frameWidthInCtu;
    frameHeightInCtu = seqSet->frameHeightInCtu;
    reconYStride     = pictureWrite->refStride[0];
    reconUvStride    = pictureWrite->refStride[1];

    reconCtu[PLANE_LUMA]     = pictureWrite->refBuf[PLANE_LUMA] + ctu->ctuPelX + ctu->ctuPelY*reconYStride;
    reconCtu[PLANE_CHROMA_U] = pictureWrite->refBuf[PLANE_CHROMA_U] + ((ctu->ctuPelX + ctu->ctuPelY*reconUvStride) >> 1);
    reconCtu[PLANE_CHROMA_V] = pictureWrite->refBuf[PLANE_CHROMA_V] + ((ctu->ctuPelX + ctu->ctuPelY*reconUvStride) >> 1);

    if (ctu->ctuX == 0)
    {
        Xin266ExtendSectionVer (
            reconCtu[PLANE_LUMA],
            reconYStride,
            ctu->width,
            ctu->height,
            4,
            ctu->height,
            TRUE);

        Xin266ExtendSectionVer (
            reconCtu[PLANE_CHROMA_U],
            reconUvStride,
            ctu->width >> 1,
            ctu->height >> 1,
            4,
            ctu->height >> 1,
            TRUE);

        Xin266ExtendSectionVer (
            reconCtu[PLANE_CHROMA_V],
            reconUvStride,
            ctu->width >> 1,
            ctu->height >> 1,
            4,
            ctu->height >> 1,
            TRUE);

    }

    if (ctu->ctuX + 1 == frameWidthInCtu)
    {
        Xin266ExtendSectionVer (
            reconCtu[PLANE_LUMA],
            reconYStride,
            ctu->width,
            ctu->height,
            4,
            ctu->height,
            FALSE);

        Xin266ExtendSectionVer (
            reconCtu[PLANE_CHROMA_U],
            reconUvStride,
            ctu->width >> 1,
            ctu->height >> 1,
            4,
            ctu->height >> 1,
            FALSE);

        Xin266ExtendSectionVer (
            reconCtu[PLANE_CHROMA_V],
            reconUvStride,
            ctu->width >> 1,
            ctu->height >> 1,
            4,
            ctu->height >> 1,
            FALSE);

    }

    if (ctu->ctuY == 0)
    {
        Xin266ExtendSectionHor (
            reconCtu[PLANE_LUMA],
            reconYStride,
            ctu->width,
            ctu->height,
            ctu->width,
            4,
            TRUE);

        Xin266ExtendSectionHor (
            reconCtu[PLANE_CHROMA_U],
            reconUvStride,
            ctu->width >> 1,
            ctu->height >> 1,
            ctu->width >> 1,
            4,
            TRUE);

        Xin266ExtendSectionHor (
            reconCtu[PLANE_CHROMA_V],
            reconUvStride,
            ctu->width >> 1,
            ctu->height >> 1,
            ctu->width >> 1,
            4,
            TRUE);

        if (ctu->ctuX == 0)
        {
            Xin266ExtendSectionHor (
                reconCtu[PLANE_LUMA] - 4,
                reconYStride,
                4,
                ctu->height,
                4,
                4,
                TRUE);

            Xin266ExtendSectionHor (
                reconCtu[PLANE_CHROMA_U] - 4,
                reconUvStride,
                4,
                ctu->height >> 1,
                4,
                4,
                TRUE);

            Xin266ExtendSectionHor (
                reconCtu[PLANE_CHROMA_V] - 4,
                reconUvStride,
                4,
                ctu->height >> 1,
                4,
                4,
                TRUE);

        }

        if (ctu->ctuX + 1 == seqSet->frameWidthInCtu)
        {
            Xin266ExtendSectionHor (
                reconCtu[PLANE_LUMA] + ctu->width,
                reconYStride,
                4,
                ctu->height,
                4,
                4,
                TRUE);

            Xin266ExtendSectionHor (
                reconCtu[PLANE_CHROMA_U] + (ctu->width >> 1),
                reconUvStride,
                4,
                ctu->height >> 1,
                4,
                4,
                TRUE);

            Xin266ExtendSectionHor (
                reconCtu[PLANE_CHROMA_V] + (ctu->width >> 1),
                reconUvStride,
                4,
                ctu->height >> 1,
                4,
                4,
                TRUE);

        }

    }

    if (ctu->ctuY + 1 == frameHeightInCtu)
    {
        Xin266ExtendSectionHor (
            reconCtu[PLANE_LUMA],
            reconYStride,
            ctu->width,
            ctu->height,
            ctu->width,
            4,
            FALSE);

        Xin266ExtendSectionHor (
            reconCtu[PLANE_CHROMA_U],
            reconUvStride,
            ctu->width >> 1,
            ctu->height >> 1,
            ctu->width >> 1,
            4,
            FALSE);

        Xin266ExtendSectionHor (
            reconCtu[PLANE_CHROMA_V],
            reconUvStride,
            ctu->width >> 1,
            ctu->height >> 1,
            ctu->width >> 1,
            4,
            FALSE);


        if (ctu->ctuX == 0)
        {
            Xin266ExtendSectionHor (
                reconCtu[PLANE_LUMA] - 4,
                reconYStride,
                4,
                ctu->height,
                4,
                4,
                FALSE);

            Xin266ExtendSectionHor (
                reconCtu[PLANE_CHROMA_U] - 4,
                reconUvStride,
                4,
                ctu->height >> 1,
                4,
                4,
                FALSE);

            Xin266ExtendSectionHor (
                reconCtu[PLANE_CHROMA_V] - 4,
                reconUvStride,
                4,
                ctu->height >> 1,
                4,
                4,
                FALSE);

        }

        if (ctu->ctuX + 1 == frameWidthInCtu)
        {
            Xin266ExtendSectionHor (
                reconCtu[PLANE_LUMA] + ctu->width,
                reconYStride,
                4,
                ctu->height,
                4,
                4,
                FALSE);

            Xin266ExtendSectionHor (
                reconCtu[PLANE_CHROMA_U] + (ctu->width >> 1),
                reconUvStride,
                4,
                ctu->height >> 1,
                4,
                4,
                FALSE);

            Xin266ExtendSectionHor (
                reconCtu[PLANE_CHROMA_V] + (ctu->width >> 1),
                reconUvStride,
                4,
                ctu->height >> 1,
                4,
                4,
                FALSE);

        }

    }

}

