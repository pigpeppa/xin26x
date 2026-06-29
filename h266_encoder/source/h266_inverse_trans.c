/***************************************************************************//**
 *
 * @file          h266_inverse_trans.c
 * @brief         h266 inverse transform subroutines.
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
#include "basic_macro.h"
#include "h266_constant.h"
#include "h26x_trans_context.h"
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
#include "h26x_forward_1d_trans.h"
#include "h266_section_struct.h"
#include "h266_common_data.h"
#include "h266_inverse_trans.h"
#include "h266_idct_add.h"
#include "h266_func_struct.h"
#include "h26x_inverse_1d_trans.h"

static Xin266Idct xinIdct[XIN_TRANS_TYPE][7] =
{
    {NULL, Xin26xIDct2P2, Xin26xIDct2P4, Xin26xIDct2P8, Xin26xIDct2P16, Xin26xIDct2P32, Xin26xIDct2P64},
    {NULL, NULL,          Xin26xIDct8P4, Xin26xIDct8P8, Xin26xIDct8P16, Xin26xIDct8P32, NULL},
    {NULL, NULL,          Xin26xIDst7P4, Xin26xIDst7P8, Xin26xIDst7P16, Xin26xIDst7P32, NULL},
};

void Xin266IDct2WxH (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf,
    SINT32      shift1st,
    SINT32      shift2nd)
{
    UINT32  lgWidth;
    UINT32  lgHeight;
    SINT32  skipWidth;

    lgWidth   = calcLog2[width];
    lgHeight  = calcLog2[height];
    skipWidth = (width  > XIN_TR_COEFF_ZERO_OUT_TH) ? (width  - XIN_TR_COEFF_ZERO_OUT_TH) : 0;

    xinIdct[XIN_DCT2][lgHeight] (
        input,
        inputStride,
        tempBuf,
        height,
        width,
        skipWidth,
        shift1st);

    xinIdct[XIN_DCT2][lgWidth] (
        tempBuf,
        height,
        output,
        outputStride,
        height,
        0,
        shift2nd);

}

void Xin266ISkipWxH (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height)
{
    UINT32 colIdx, rowIdx;
    UINT32 shift;

    shift = XIN_TR_MAX_LG_RANGE - XIN_8_BIT_DEPTH - ((calcLog2[width] + calcLog2[height]) >> 1);

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        for (colIdx = 0; colIdx < width; colIdx++)
        {
            output[colIdx] = (SINT16)(input[colIdx] >> shift);
        }

        input  += inputStride;
        output += outputStride;
    }

}

void Xin266IDctWxH (
    xin_sec_struct *secSet,
    COEFF          *input,
    intptr_t       inputStride,
    COEFF          *output,
    intptr_t       outputStride,
    UINT32         width,
    UINT32         height,
    BOOL           isIntra,
    UINT32         compId,
    UINT32         mtsIdx)
{
    xin_func_struct *funcSet;
    xin_seq_struct  *seqSet;
    SINT32          shift1st;
    SINT32          shift2nd;
    SINT32          horTx;
    SINT32          verTx;
    UINT32          lgWidth;
    UINT32          lgHeight;
    SINT32          skipWidth;

    funcSet   = secSet->funcSet;
    seqSet    = secSet->seqSet;
    horTx     = mtsIdx2txType[mtsIdx][0];
    verTx     = mtsIdx2txType[mtsIdx][1];
    lgWidth   = calcLog2[width];
    lgHeight  = calcLog2[height];
    shift1st  = XIN_TR_MATRIX_SHIFT + 1;
    shift2nd  = XIN_TR_MATRIX_SHIFT + XIN_TR_MAX_LG_RANGE - 1 - seqSet->config.internalBitDepth;
    skipWidth = ((horTx != XIN_DCT2) && (width == 32)) ? 16 : (width  > XIN_TR_COEFF_ZERO_OUT_TH) ? (width  - XIN_TR_COEFF_ZERO_OUT_TH) : 0;

    if (isIntra && (compId == PLANE_LUMA) && (seqSet->config.enableMts) && (mtsIdx == XIN_MTS_DCT2_DCT2))
    {
        if (width >= 4 && width <= 16)
        {
            horTx = XIN_DST7;
        }
        if (height >= 4 && height <= 16)
        {
            verTx = XIN_DST7;
        }
    }

    if ((lgWidth > 0) && (lgHeight > 0) && (horTx == XIN_DCT2) && (verTx == XIN_DCT2))
    {
        funcSet->pfXinInv2dDct2[lgWidth][lgHeight] (
            input,
            inputStride,
            output,
            outputStride,
            width,
            height,
            (COEFF *)secSet->tempBuffer,
            shift1st,
            shift2nd);
    }
    else if (horTx == XIN_SKIP)
    {
        funcSet->pfXinInv2dSkip[lgWidth] (
            input,
            inputStride,
            output,
            outputStride,
            width,
            height);

    }
    else if ((lgWidth > 0) && (lgHeight > 0)) //2-D transform
    {
        funcSet->pfXinInv1dDct[verTx][lgHeight] (
            input,
            inputStride,
            (COEFF *)secSet->tempBuffer,
            height,
            width,
            skipWidth,
            shift1st);

        funcSet->pfXinInv1dDct[horTx][lgWidth] (
            (COEFF *)secSet->tempBuffer,
            height,
            output,
            outputStride,
            height,
            0,
            shift2nd);

    }
    else if (lgHeight > 0)
    {
        funcSet->pfXinInv1dDct[verTx][lgHeight] (
            input,
            inputStride,
            (COEFF *)secSet->tempBuffer,
            width,
            width,
            0,
            shift1st);
    }
    else
    {
        funcSet->pfXinInv1dDct[horTx][lgWidth] (
            input,
            inputStride,
            (COEFF *)secSet->tempBuffer,
            height,
            height,
            0,
            shift2nd);
    }

}

