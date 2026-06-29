/***************************************************************************//**
 *
 * @file          h266_trans_quant.c
 * @brief         h266 forward transform and quantization for a TU or CU.
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
#include "assert.h"
#include "basic_macro.h"
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
#include "h266_sub_dct.h"
#include "h266_quant_inv_quant.h"
#include "h266_dep_quant.h"
#include "h266_func_struct.h"

static void Xin266ZeroOutCoeff (
    COEFF    *coeff,
    intptr_t coeffStride,
    SINT32   width,
    SINT32   height)
{
    SINT32  rowIdx;

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        memset (coeff, 0, sizeof(COEFF)*width);

        coeff += coeffStride;
    }
}

void Xin266Transform (
    xin_sec_struct  *secSet,
    xin_cu_struct   *cu,
    xin_fast_md_buf *fastBuf,
    UINT32          mtsIdx,
    UINT32          planeIdx)
{
    xin_tu_struct   *tu;
    xin_pic_struct  *picSet;
    xin_seq_struct  *seqSet;
    xin_func_struct *funcSet;
    xin_quant_param *qParam;
    UINT32          tuIdx;
    UINT32          compType;
    UINT32          offsetX;
    UINT32          offsetY;
    UINT32          tuOffX;
    UINT32          tuOffY;
    SINT32          width;
    SINT32          lgWidth;
    SINT32          lgSize;
    SINT32          height;
    UINT32          cgWidth;
    UINT32          cgHeight;
    UINT32          qp;
    UINT32          frameType;
    SINT32          qShift;
    SINT32          iqShift;
    SINT32          qAdd;
    SINT32          iqAdd;
    intptr_t        inputStride;
    intptr_t        coeffStride;
    intptr_t        predStride;
    intptr_t        coeffPos;
    BOOL            isAdjust;
    BOOL            isIntra;
    xin_full_md_buf *fullBuf;
    UINT32          lgTuSize;

    fullBuf     = fastBuf->fullBuf;
    picSet      = secSet->picSet;
    funcSet     = secSet->funcSet;
    seqSet      = secSet->seqSet;
    frameType   = picSet->pictureWrite->frameType;
    compType    = (planeIdx != PLANE_LUMA);
    qp          = (planeIdx == PLANE_LUMA) ? secSet->qp : secSet->uvQp;
    qp         += (XIN_INTERNAL_BIT_DEPTH == XIN_10_BIT_DEPTH) ? XIN_QP_SHIFT : 0;
    inputStride = (planeIdx == PLANE_LUMA) ? secSet->inputYStride : secSet->inputUvStride;
    coeffStride = fullBuf->coeffStride[compType];
    predStride  = (planeIdx == PLANE_LUMA) ? fastBuf->lumaStride : fastBuf->chromaStride;

    lgTuSize = cu->lgTuWidth[compType] + cu->lgTuHeight[compType];
    isAdjust = lgTuSize & 1;
    lgSize   = lgTuSize >> 1;
    isAdjust = isAdjust && (mtsIdx != XIN_MTS_SKIP);
    qParam   = seqSet->quantParam[isAdjust] + qp;
    qShift   = qParam->qShift - lgSize - isAdjust;
    iqShift  = qParam->iqShift + lgSize + isAdjust;
    qAdd     = (seqSet->config.enableRdoq && (mtsIdx != XIN_MTS_SKIP)) ? (1 << (qShift - 1)) : (((frameType >= XIN_I_FRAME) ? 171 : 85) << (qShift - 9));
    iqAdd    = (mtsIdx != XIN_MTS_SKIP) ? 1 << (iqShift - 1) : 1 << ((iqShift - lgSize ) + (15 - XIN_INTERNAL_BIT_DEPTH) - 1);
    isIntra  = fastBuf->type == XIN_INTRA_MODE;

    fullBuf->yuvCbf[mtsIdx][planeIdx] = 0;

    for (tuIdx = 0; tuIdx < fullBuf->tuNum; tuIdx++)
    {
        tu       = cu->tu[tuIdx];
        tuOffX   = tu->offsetX;
        tuOffY   = tu->offsetY;
        offsetX  = tuOffX + cu->offX;
        offsetY  = tuOffY + cu->offY;
        lgWidth  = tu->lgWidth[compType];
        width    = 1 << lgWidth;
        height   = 1 << tu->lgHeight[compType];
        cgWidth  = 1 << tu->lgCGWidth[compType];
        cgHeight = 1 << tu->lgCGHeight[compType];
        lgWidth  = XIN_MIN (lgWidth, 5);
        lgWidth  = ((cgWidth != 4) || (cgHeight != 4)) ? 1 : lgWidth;
        coeffPos = (offsetX + offsetY*coeffStride) >> compType;

        Xin266SubFDctWxH (
            secSet,
            secSet->reshapeCu[planeIdx] + ((tuOffX + tuOffY*inputStride) >> compType),
            inputStride,
            fastBuf->predBuf[planeIdx] + ((offsetX + offsetY*predStride) >> compType),
            predStride,
            fullBuf->tCoefBuf[mtsIdx][planeIdx] + coeffPos,
            coeffStride,
            planeIdx,
            mtsIdx,
            width,
            height,
            isIntra);

        if ((width > 32) && (height > 32))
        {
            assert (width == 64);
            assert (height == 64);

            Xin266ZeroOutCoeff (
                fullBuf->rCoefBuf[mtsIdx][planeIdx] + coeffPos + 32,
                coeffStride,
                32,
                32);

            Xin266ZeroOutCoeff (
                fullBuf->rCoefBuf[mtsIdx][planeIdx] + coeffPos + 32*coeffStride,
                coeffStride,
                64,
                32);

        }
        else if (width > 32)
        {
            Xin266ZeroOutCoeff (
                fullBuf->rCoefBuf[mtsIdx][planeIdx] + coeffPos + 32,
                coeffStride,
                width - 32,
                height);
        }
        else if (height > 32)
        {
            Xin266ZeroOutCoeff (
                fullBuf->rCoefBuf[mtsIdx][planeIdx] + coeffPos + 32*coeffStride,
                coeffStride,
                width,
                height - 32);
        }

        if ((seqSet->config.enableDepQuant) && (mtsIdx != XIN_MTS_SKIP))
        {
            funcSet->pfXinDepQuant (
                secSet->depQuant,
                fastBuf,
                tu,
                fullBuf->tCoefBuf[mtsIdx][planeIdx] + coeffPos,
                fullBuf->rCoefBuf[mtsIdx][planeIdx] + coeffPos,
                fullBuf->qCoefBuf[mtsIdx][planeIdx] + coeffPos,
                coeffStride,
                qp,
                width,
                height,
                fullBuf->nzCGMapRs[mtsIdx][tuIdx] + planeIdx,
                planeIdx);
        }
        else
        {
            funcSet->pfXinQuantInvQuant[lgWidth] (
                fullBuf->qCoefBuf[mtsIdx][planeIdx] + coeffPos,
                fullBuf->tCoefBuf[mtsIdx][planeIdx] + coeffPos,
                fullBuf->rCoefBuf[mtsIdx][planeIdx] + coeffPos,
                coeffStride,
                XIN_MIN (width, 32),
                XIN_MIN (height, 32),
                cgWidth,
                cgHeight,
                qParam->qMult,
                qAdd,
                qShift,
                qParam->iqMult,
                iqAdd,
                iqShift,
                fullBuf->nzCGMapRs[mtsIdx][tuIdx] + planeIdx);

            if ((fullBuf->nzCGMapRs[mtsIdx][tuIdx][planeIdx]) && (seqSet->config.rdoqThrVal) && (mtsIdx != XIN_MTS_SKIP))
            {
                funcSet->pfXinPreRdoq (
                    fullBuf->tCoefBuf[mtsIdx][planeIdx] + coeffPos,
                    fullBuf->rCoefBuf[mtsIdx][planeIdx] + coeffPos,
                    fullBuf->qCoefBuf[mtsIdx][planeIdx] + coeffPos,
                    coeffStride,
                    XIN_MIN(width, 32),
                    XIN_MIN(height, 32),
                    cgWidth,
                    cgHeight,
                    (seqSet->config.rdoqThrVal << (qShift - 1)) / (qParam->qMult << 2),
                    tu->scanOrderCG[compType],
                    fullBuf->nzCGMapRs[mtsIdx][tuIdx] + planeIdx);
            }
            
        }

        fullBuf->yuvCbf[mtsIdx][planeIdx] |= (fullBuf->nzCGMapRs[mtsIdx][tuIdx][planeIdx] != 0) << tuIdx;

    }

}

