/***************************************************************************//**
 *
 * @file          h266_forward_trans.c
 * @brief         h266 forward transform subroutines.
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
#include "h266_cabac_struct.h"
#include "h26x_trans_context.h"
#include "h26x_forward_1d_trans.h"
#include "h266_section_struct.h"
#include "h266_common_data.h"
#include "h266_func_struct.h"

void Xin266FDctWxH (
    xin_sec_struct *secSet,
    COEFF          *input,
    intptr_t       inputStride,
    COEFF          *output,
    intptr_t       outputStride,
    UINT32         compId,
    UINT32         width,
    UINT32         height,
    UINT32         mtsIdx,
    BOOL           isIntra)
{
    UINT32          horTx;
    UINT32          verTx;
    UINT32          lgWidth;
    UINT32          lgHeight;
    SINT32          shift1st;
    SINT32          shift2nd;
    xin_func_struct *funcSet;
    xin_seq_struct  *seqSet;

    horTx    = mtsIdx2txType[mtsIdx][0];
    verTx    = mtsIdx2txType[mtsIdx][1];
    lgWidth  = calcLog2[width];
    lgHeight = calcLog2[height];
    funcSet  = secSet->funcSet;
    seqSet   = secSet->seqSet;
    shift1st = lgWidth + seqSet->config.internalBitDepth + XIN_TR_MATRIX_SHIFT - XIN_TR_MAX_LG_RANGE;
    shift2nd = lgHeight + XIN_TR_MATRIX_SHIFT;

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
        funcSet->pfXinFor2dDct2[lgWidth][lgHeight] (
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
        funcSet->pfXinFor2dSkip[lgWidth] (
            input,
            inputStride,
            output,
            outputStride,
            width,
            height,
            (COEFF *)secSet->tempBuffer);
    }
    else if ((lgWidth > 0) && (lgHeight > 0))
    {
        funcSet->pfXinFor1dDct[horTx][lgWidth] (
            input,
            inputStride,
            (COEFF *)secSet->tempBuffer,
            height,
            height,
            shift1st);

        funcSet->pfXinFor1dDct[verTx][lgHeight] (
            (COEFF *)secSet->tempBuffer,
            height,
            output,
            outputStride,
            width,
            shift2nd);

    }
    else if (lgWidth > 0)
    {
        funcSet->pfXinFor1dDct[horTx][lgWidth] (
            input,
            inputStride,
            output,
            outputStride,
            height,
            shift1st);
    }
    else
    {
        funcSet->pfXinFor1dDct[verTx][lgHeight] (
            input,
            inputStride,
            output,
            outputStride,
            width,
            shift2nd);
    }

}

