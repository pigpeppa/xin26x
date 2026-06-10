/***************************************************************************//**
 *
 * @file          h265p_trans_quant.c
 * @brief         av1 forward transform and quantization.
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
#include "h265p_sub_dct.h"
#include "h265p_quant_inv_quant.h"
#include "h265p_common_data.h"

static void Xin265pZeroOutCoeff (
    SINT32   *coeff,
    intptr_t coeffStride,
    SINT32   width,
    SINT32   height)
{
    SINT32  rowIdx;

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        memset (coeff, 0, sizeof(SINT32)*width);

        coeff += coeffStride;
    }
}

void Xin265pTransformTx (
    xin_sec_struct  *secSet,
    xin_full_md_buf *fullBuf,
    xin_tu_struct   *tu,
    UINT32          planeIdx)
{
    xin_mb_struct   *mb;
    xin_pic_struct  *picSet;
    xin_quant_param *qParam;
    UINT32          planeType;
    UINT32          tuIdx;
    UINT32          offsetX;
    UINT32          offsetY;
    UINT32          tuOffX;
    UINT32          tuOffY;
    UINT32          qp;
    intptr_t        inputStride;
    intptr_t        coeffStride;
    intptr_t        predStride;
    UINT32          tranSize;
    UINT32          logScale;
    xin_fast_md_buf *fastBuf;
    xin_func_struct *funcSet;

    fastBuf   = fullBuf->fastBuf;
    funcSet   = secSet->funcSet;
    picSet    = secSet->picSet;
    planeType = planeIdx != PLANE_LUMA;
    mb        = secSet->mb;
    
    inputStride = planeType ? secSet->inputUvStride : secSet->inputYStride;
    coeffStride = fullBuf->coefStride[planeType];
    predStride  = fastBuf->predStride[planeType];
    qp          = planeType ? secSet->uvQp : secSet->qp;
    qParam      = picSet->quantParam + qp;
    tranSize    = fullBuf->tranSize[planeType];
    logScale    = txSize2LogScale[tranSize];

    for (tuIdx = 0; tuIdx < fullBuf->tuNum; tuIdx++)
    {
        tuOffX    = tu->offsetX;
        tuOffY    = tu->offsetY;
        offsetX   = tuOffX + mb->offX[planeType];
        offsetY   = tuOffY + mb->offY[planeType];
    
        Xin265pSubFDct (
            secSet,
            secSet->inputMb[planeIdx] + tuOffX + tuOffY*inputStride,
            inputStride,
            fastBuf->predBuf[planeIdx] + offsetX + offsetY*predStride,
            predStride,
            fullBuf->tCoefBuf[planeIdx] + offsetX + offsetY*coeffStride,
            coeffStride,
            fullBuf->tranType[planeType],
            tranSize);

        funcSet->pfXinQuantInvQuant[tu->lgWidth] (
            fullBuf->qCoefBuf[planeIdx] + offsetX + offsetY*coeffStride,
            fullBuf->tCoefBuf[planeIdx] + offsetX + offsetY*coeffStride,
            fullBuf->rCoefBuf[planeIdx] + offsetX + offsetY*coeffStride,
            coeffStride,
            XIN_MIN(tu->width, 32),
            XIN_MIN(tu->height, 32),
            logScale,
            qParam->round,
            qParam->quant,
            qParam->zBin,
            qParam->quantShift,
            qParam->dequant,
            &fullBuf->nzCount[tuIdx][planeIdx]);

        if ((tu->width > 32) && (tu->height > 32))
        {
            assert(tu->width == 64);
            assert(tu->height == 64);

            Xin265pZeroOutCoeff (
                fullBuf->rCoefBuf[planeIdx] + offsetX + offsetY*coeffStride + 32,
                coeffStride,
                32,
                32);

            Xin265pZeroOutCoeff (
                fullBuf->rCoefBuf[planeIdx] + offsetX + (offsetY + 32)*coeffStride,
                coeffStride,
                64,
                32);
        }
        else if (tu->width > 32)
        {
            Xin265pZeroOutCoeff (
                fullBuf->rCoefBuf[planeIdx] + offsetX + offsetY*coeffStride + 32,
                coeffStride,
                tu->width - 32,
                tu->height);
        }
        else if (tu->height > 32)
        {
            Xin265pZeroOutCoeff (
                fullBuf->rCoefBuf[planeIdx] + offsetX + (offsetY + 32)*coeffStride,
                coeffStride,
                tu->width,
                tu->height - 32);
        }
        
    }

}


