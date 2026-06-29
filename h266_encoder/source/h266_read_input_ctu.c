/***************************************************************************//**
 *
 * @file          h266_read_input_ctu.c
 * @brief         Read an input CTU from input frame buffer.
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
#include "h26x_block_utility.h"
#include "h266_lmcs.h"
#include "h266_func_struct.h"

void Xin266ReadInputCtu (
    xin_sec_struct *secSet,
    xin_ctu_struct *ctu)
{
    UINT32   width;
    UINT32   height;
    UINT32   restWidth;
    UINT32   restHeight;
    intptr_t srcYStride;
    intptr_t srcUvStride;
    intptr_t inputYStride;
    intptr_t inputUvStride;
    PIXEL    *srcY;
    PIXEL    *srcU;
    PIXEL    *srcV;
    PIXEL    *inputY;
    PIXEL    *inputU;
    PIXEL    *inputV;
    PIXEL    *reshapeY;
    UINT32   rowIdx;
    UINT32   funcIdx;

    xin_pic_struct  *picSet;
    xin_ref_picture *pictureWrite;
    xin_seq_struct  *seqSet;
    xin_func_struct *funcSet;
    xin_lmcs_struct *lmcsSet;

    picSet  = secSet->picSet;
    lmcsSet = picSet->lmcsSet;
    seqSet  = secSet->seqSet;
    width   = ctu->width;
    height  = ctu->height;
    funcSet = secSet->funcSet;

    pictureWrite  = picSet->pictureWrite;
    srcYStride    = pictureWrite->inputStride[PLANE_LUMA];
    srcUvStride   = pictureWrite->inputStride[PLANE_CHROMA];
    srcY          = pictureWrite->inputBuf[PLANE_LUMA] + ctu->ctuPelY*srcYStride + ctu->ctuPelX;
    srcU          = pictureWrite->inputBuf[PLANE_CHROMA_U] + ((ctu->ctuPelY*srcUvStride + ctu->ctuPelX) >> 1);
    srcV          = pictureWrite->inputBuf[PLANE_CHROMA_V] + ((ctu->ctuPelY*srcUvStride + ctu->ctuPelX) >> 1);
    inputYStride  = secSet->inputYStride;
    inputUvStride = secSet->inputUvStride;
    inputY        = secSet->inputCtu[PLANE_LUMA];
    inputU        = secSet->inputCtu[PLANE_CHROMA_U];
    inputV        = secSet->inputCtu[PLANE_CHROMA_V];
    funcIdx       = (width == seqSet->ctuSize) ? ctu->lgWidth : 1;
    restWidth     = seqSet->config.inputWidth - ctu->ctuPelX;
    restHeight    = seqSet->config.inputHeight - ctu->ctuPelY;

    if ((restWidth >= width) && (restHeight >= height))
    {
        funcSet->pfXinBlockCopy[funcIdx](
            srcY,
            srcYStride,
            inputY,
            inputYStride,
            width,
            height);

        funcSet->pfXinBlockCopy[funcIdx - 1](
            srcU,
            srcUvStride,
            inputU,
            inputUvStride,
            width/2,
            height/2);

        funcSet->pfXinBlockCopy[funcIdx - 1](
            srcV,
            srcUvStride,
            inputV,
            inputUvStride,
            width/2,
            height/2);

    }
    else
    {
        width   = (restWidth < width) ? restWidth : width;
        height  = (restHeight < height) ? restHeight: height;
        funcIdx = (width == seqSet->ctuSize) ? ctu->lgWidth : 1;

        funcSet->pfXinBlockCopy[funcIdx](
            srcY,
            srcYStride,
            inputY,
            inputYStride,
            width,
            height);

        funcSet->pfXinBlockCopy[funcIdx - 1](
            srcU,
            srcUvStride,
            inputU,
            inputUvStride,
            width/2,
            height/2);

        funcSet->pfXinBlockCopy[funcIdx - 1](
            srcV,
            srcUvStride,
            inputV,
            inputUvStride,
            width/2,
            height/2);

        if (width < ctu->width)
        {
            for (rowIdx = 0; rowIdx < height; rowIdx++)
            {
                memset (inputY + width, *(inputY + width - 1), ctu->width - width);
                inputY += inputYStride;
            }

            for (rowIdx = 0; rowIdx < height/2; rowIdx++)
            {
                memset (inputU + width/2, *(inputU + width/2 - 1), (ctu->width - width)/2);
                memset (inputV + width/2, *(inputV + width/2 - 1), (ctu->width - width)/2);
                inputU += inputUvStride;
                inputV += inputUvStride;
            }
        }

        if (height < ctu->height)
        {
            inputY = secSet->inputCtu[PLANE_LUMA] + height*inputYStride;
            inputU = secSet->inputCtu[PLANE_CHROMA_U] + height*inputUvStride/2;
            inputV = secSet->inputCtu[PLANE_CHROMA_V] + height*inputUvStride/2;

            for (rowIdx = height; rowIdx < ctu->height; rowIdx++)
            {
                memcpy (inputY, inputY - inputYStride, ctu->width);
                inputY += inputYStride;
            }

            for (rowIdx = height/2; rowIdx < ctu->height/2; rowIdx++)
            {
                memcpy (inputU, inputU - inputUvStride, ctu->width/2);
                memcpy (inputV, inputV - inputUvStride, ctu->width/2);
                inputU += inputUvStride;
                inputV += inputUvStride;
            }
        }

    }

    if (seqSet->config.enableLmcs && lmcsSet->lmcsParam.sliceReshaperEnabled)
    {
        inputY   = secSet->inputCtu[PLANE_LUMA];
        reshapeY = secSet->reshapeCtuY;

        Xin266ReshapeSignal (
            inputY,
            inputYStride,
            reshapeY,
            inputYStride,
            ctu->width,
            ctu->height,
            lmcsSet->fwdLUT);
    }

}

