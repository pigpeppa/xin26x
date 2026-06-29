/***************************************************************************//**
 *
 * @file          h266_sao_ctu.c
 * @brief         h266 SAO application.
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
#include "stdio.h"
#include "xin_typedef.h"
#include "h266_constant.h"
#include "xin_video_common.h"
#include "h26x_definition.h"
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
#include "h26x_sao.h"
#include "memory.h"
#include "h266_alf_rdo.h"
#include "h266_func_struct.h"

void Xin266SaoCtuLuma (
    xin_pic_struct *picSet,
    xin_ctu_struct *ctu)
{
    xin_func_struct *funcSet;
    xin_seq_struct  *seqSet;
    xin_ref_picture *pictureWrite;
    UINT32          width;
    UINT32          height;
    UINT32          lftOffset;
    UINT32          topOffset;
    PIXEL           *reconY;
    intptr_t        reconYStride;

    pictureWrite = picSet->pictureWrite;
    reconYStride = pictureWrite->refStride[0];
    reconY       = pictureWrite->refBuf[PLANE_LUMA] + ctu->ctuPelY*reconYStride + ctu->ctuPelX;
    seqSet       = picSet->seqSet;
    width        = ctu->width;
    height       = ctu->height;
    funcSet      = seqSet->funcSet;
    lftOffset    = ctu->ctuPelX + ctu->ctuX;
    topOffset    = ctu->ctuY*seqSet->frameWidth + ctu->ctuPelX;

    switch (ctu->saoType[PLANE_LUMA])
    {

    case XIN_SAO_BO:

        Xin26xSaoBo (
            reconY,
            reconYStride,
            ctu->saoBandPos[PLANE_LUMA],
            ctu->saoOffset[PLANE_LUMA],
            width,
            height,
            seqSet->config.internalBitDepth);

        break;

    case XIN_SAO_EO_0:

        funcSet->pfXinSaoEo0[(width & 0x1F) == 0] (
            reconY,
            reconYStride,
            picSet->saoLftBuf[ctu->ctuY][PLANE_LUMA] + lftOffset,
            ctu->saoOffset[PLANE_LUMA],
            ctu->availField,
            width,
            height);

        break;

    case XIN_SAO_EO_1:

        funcSet->pfXinSaoEo90[(width & 0x1F) == 0] (
            reconY,
            reconYStride,
            picSet->saoTopBuf[PLANE_LUMA] + topOffset,
            ctu->saoOffset[PLANE_LUMA],
            ctu->availField,
            width,
            height);

        break;

    case XIN_SAO_EO_2:

        funcSet->pfXinSaoEo135[(width & 0x1F) == 0] (
            reconY,
            reconYStride,
            picSet->saoLftBuf[ctu->ctuY][PLANE_LUMA] + lftOffset,
            picSet->saoTopBuf[PLANE_LUMA] + topOffset,
            ctu->saoOffset[PLANE_LUMA],
            ctu->availField,
            width,
            height);

        break;

    case XIN_SAO_EO_3:

        funcSet->pfXinSaoEo45[(width & 0x1F) == 0] (
            reconY,
            reconYStride,
            picSet->saoLftBuf[ctu->ctuY][PLANE_LUMA] + lftOffset,
            picSet->saoTopBuf[PLANE_LUMA] + topOffset,
            ctu->saoOffset[PLANE_LUMA],
            ctu->availField,
            width,
            height);

        break;

    default:

        _XIN_LOGGER (XIN_LOGGER_ERROR, "Illegal sao type.\n");

        break;

    }

}

void Xin266SaoCtuChroma (
    xin_pic_struct *picSet,
    xin_ctu_struct *ctu)
{
    xin_func_struct *funcSet;
    xin_seq_struct  *seqSet;
    xin_ref_picture *pictureWrite;
    UINT32          width;
    UINT32          height;
    UINT32          lftOffset;
    UINT32          topOffset;
    PIXEL           *reconU;
    PIXEL           *reconV;
    intptr_t        reconUvStride;

    pictureWrite  = picSet->pictureWrite;
    reconUvStride = pictureWrite->refStride[1];
    reconU        = pictureWrite->refBuf[PLANE_CHROMA_U] + ((ctu->ctuPelY*reconUvStride + ctu->ctuPelX) >> 1);
    reconV        = pictureWrite->refBuf[PLANE_CHROMA_V] + ((ctu->ctuPelY*reconUvStride + ctu->ctuPelX) >> 1);
    seqSet        = picSet->seqSet;
    width         = ctu->width >> 1;
    height        = ctu->height >> 1;
    funcSet       = seqSet->funcSet;
    lftOffset     = (ctu->ctuPelX >> 1) + ctu->ctuX;
    topOffset     = (ctu->ctuY*seqSet->frameWidth + ctu->ctuPelX)>>1;

    switch (ctu->saoType[PLANE_CHROMA])
    {

    case XIN_SAO_BO:

        Xin26xSaoBo (
            reconU,
            reconUvStride,
            ctu->saoBandPos[PLANE_CHROMA_U],
            ctu->saoOffset[PLANE_CHROMA_U],
            width,
            height,
            seqSet->config.internalBitDepth);

        Xin26xSaoBo (
            reconV,
            reconUvStride,
            ctu->saoBandPos[PLANE_CHROMA_V],
            ctu->saoOffset[PLANE_CHROMA_V],
            width,
            height,
            seqSet->config.internalBitDepth);

        break;

    case XIN_SAO_EO_0:

        funcSet->pfXinSaoEo0[(width & 0x1F) == 0] (
            reconU,
            reconUvStride,
            picSet->saoLftBuf[ctu->ctuY][PLANE_CHROMA_U] + lftOffset,
            ctu->saoOffset[PLANE_CHROMA_U],
            ctu->availField,
            width,
            height);

        funcSet->pfXinSaoEo0[(width & 0x1F) == 0] (
            reconV,
            reconUvStride,
            picSet->saoLftBuf[ctu->ctuY][PLANE_CHROMA_V] + lftOffset,
            ctu->saoOffset[PLANE_CHROMA_V],
            ctu->availField,
            width,
            height);

        break;

    case XIN_SAO_EO_1:

        funcSet->pfXinSaoEo90[(width & 0x1F) == 0] (
            reconU,
            reconUvStride,
            picSet->saoTopBuf[PLANE_CHROMA_U] + topOffset,
            ctu->saoOffset[PLANE_CHROMA_U],
            ctu->availField,
            width,
            height);

        funcSet->pfXinSaoEo90[(width & 0x1F) == 0] (
            reconV,
            reconUvStride,
            picSet->saoTopBuf[PLANE_CHROMA_V] + topOffset,
            ctu->saoOffset[PLANE_CHROMA_V],
            ctu->availField,
            width,
            height);

        break;

    case XIN_SAO_EO_2:

        funcSet->pfXinSaoEo135[(width & 0x1F) == 0] (
            reconU,
            reconUvStride,
            picSet->saoLftBuf[ctu->ctuY][PLANE_CHROMA_U] + lftOffset,
            picSet->saoTopBuf[PLANE_CHROMA_U] + topOffset,
            ctu->saoOffset[PLANE_CHROMA_U],
            ctu->availField,
            width,
            height);

        funcSet->pfXinSaoEo135[(width & 0x1F) == 0] (
            reconV,
            reconUvStride,
            picSet->saoLftBuf[ctu->ctuY][PLANE_CHROMA_V] + lftOffset,
            picSet->saoTopBuf[PLANE_CHROMA_V] + topOffset,
            ctu->saoOffset[PLANE_CHROMA_V],
            ctu->availField,
            width,
            height);

        break;

    case XIN_SAO_EO_3:

        funcSet->pfXinSaoEo45[(width & 0x1F) == 0] (
            reconU,
            reconUvStride,
            picSet->saoLftBuf[ctu->ctuY][PLANE_CHROMA_U] + lftOffset,
            picSet->saoTopBuf[PLANE_CHROMA_U] + topOffset,
            ctu->saoOffset[PLANE_CHROMA_U],
            ctu->availField,
            width,
            height);

        funcSet->pfXinSaoEo45[(width & 0x1F) == 0] (
            reconV,
            reconUvStride,
            picSet->saoLftBuf[ctu->ctuY][PLANE_CHROMA_V] + lftOffset,
            picSet->saoTopBuf[PLANE_CHROMA_V] + topOffset,
            ctu->saoOffset[PLANE_CHROMA_V],
            ctu->availField,
            width,
            height);

        break;

    default:

        _XIN_LOGGER (XIN_LOGGER_ERROR, "Illegal sao type.\n");

        break;

    }

}

static void Xin266LftPelBackup (
    xin_pic_struct *picSet,
    xin_ctu_struct *ctu)
{
    PIXEL           *srcY;
    PIXEL           *dstY;
    PIXEL           *srcU;
    PIXEL           *dstU;
    PIXEL           *srcV;
    PIXEL           *dstV;
    UINT32          idx;
    UINT32          width;
    UINT32          height;
    xin_ref_picture *pictureWrite;
    intptr_t        reconYStride;
    intptr_t        reconUvStride;

    width         = ctu->width;
    height        = ctu->height;
    pictureWrite  = picSet->pictureWrite;
    reconYStride  = pictureWrite->refStride[0];
    reconUvStride = pictureWrite->refStride[1];

    if (ctu->availField & XIN_RGT_CTU_AVAIL)
    {
        dstY = picSet->saoLftBuf[ctu->ctuY][PLANE_LUMA] + (width+1)*(ctu->ctuX+1);
        srcY = pictureWrite->refBuf[PLANE_LUMA] + ctu->ctuPelY*reconYStride + ctu->ctuPelX + width - 1;

        for (idx = 0; idx < height+1; idx++)
        {
            dstY[idx] = srcY[idx*reconYStride];
        }

        dstU = picSet->saoLftBuf[ctu->ctuY][PLANE_CHROMA_U] + ((width>>1)+1)*(ctu->ctuX+1);
        srcU = pictureWrite->refBuf[PLANE_CHROMA_U] + ((ctu->ctuPelY*reconUvStride + ctu->ctuPelX) >> 1) + (width>>1) - 1;

        dstV = picSet->saoLftBuf[ctu->ctuY][PLANE_CHROMA_V] + ((width>>1)+1)*(ctu->ctuX+1);
        srcV = pictureWrite->refBuf[PLANE_CHROMA_V] + ((ctu->ctuPelY*reconUvStride + ctu->ctuPelX) >> 1) + (width>>1) - 1;

        for (idx = 0; idx < (height>>1)+1; idx++)
        {
            dstU[idx] = srcU[idx*reconUvStride];
            dstV[idx] = srcV[idx*reconUvStride];
        }

    }

}

static void Xin266TopPelBackup (
    xin_pic_struct *picSet,
    xin_ctu_struct *ctu)
{
    xin_seq_struct  *seqSet;
    xin_ref_picture *pictureWrite;
    intptr_t        reconYStride;
    intptr_t        reconUvStride;
    PIXEL           *srcY;
    PIXEL           *dstY;
    PIXEL           *srcU;
    PIXEL           *dstU;
    PIXEL           *srcV;
    PIXEL           *dstV;
    UINT32          ctuWidth;
    UINT32          frameWidth;
    UINT32          height;
    UINT32          ctuY;

    seqSet        = picSet->seqSet;
    pictureWrite  = picSet->pictureWrite;
    ctuWidth      = ctu->width;
    frameWidth    = seqSet->frameWidth;
    height        = ctu->height;
    ctuY          = ctu->ctuY + 1;
    reconYStride  = pictureWrite->refStride[0];
    reconUvStride = pictureWrite->refStride[1];

    if (ctu->availField & XIN_BOT_CTU_AVAIL)
    {
        dstY = picSet->saoTopBuf[PLANE_LUMA] + frameWidth*ctuY + ctu->ctuPelX;
        srcY = pictureWrite->refBuf[PLANE_LUMA] + ctu->ctuPelY*reconYStride + ctu->ctuPelX + (height - 1)*reconYStride;

        memcpy (dstY, srcY, sizeof(PIXEL)*ctuWidth);

        dstU = picSet->saoTopBuf[PLANE_CHROMA_U] + ((frameWidth*ctuY + ctu->ctuPelX) >> 1);
        srcU = pictureWrite->refBuf[PLANE_CHROMA_U] + ((ctu->ctuPelY*reconUvStride + ctu->ctuPelX) >> 1) + ((height>>1) - 1)*reconUvStride;

        dstV = picSet->saoTopBuf[PLANE_CHROMA_V] + ((frameWidth*ctuY + ctu->ctuPelX) >> 1);
        srcV = pictureWrite->refBuf[PLANE_CHROMA_V] + ((ctu->ctuPelY*reconUvStride + ctu->ctuPelX) >> 1) + ((height>>1) - 1)*reconUvStride;

        memcpy (dstU, srcU, sizeof(PIXEL)*(ctuWidth>>1));
        memcpy (dstV, srcV, sizeof(PIXEL)*(ctuWidth>>1));
    }

}

void Xin266SaoCtu (
    xin_ctu_struct *ctu)
{
    xin_pic_struct *picSet;
    xin_seq_struct *seqSet;

    picSet = ctu->picSet;
    seqSet = picSet->seqSet;

    if ((!seqSet->config.enableSao) || ((!picSet->pictureWrite->isReferenced) && (!seqSet->config.needRecon) && (!picSet->enableAlf)))
    {
        
    }
    else
    {
        Xin266TopPelBackup (
            picSet,
            ctu);

        Xin266LftPelBackup (
            picSet,
            ctu);

        if (ctu->saoType[PLANE_LUMA] >= 0)
        {
            Xin266SaoCtuLuma (
                picSet,
                ctu);
        }

        if (ctu->saoType[PLANE_CHROMA] >= 0)
        {
            Xin266SaoCtuChroma (
                picSet,
                ctu);
        }
        
    }

    if (picSet->enableAlf)
    {
        Xin266AlfPadPixel (
            ctu);
    }

}
