/***************************************************************************//**
 *
 * @file          h265p_sub_dct.c
 * @brief         Subtract prediction and perform forward transform.
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
#include "h265p_trans_context.h"
#include "h265p_forward_trans.h"
#include "h26x_definition.h"
#include "h265p_definition.h"
#include "h265p_constant.h"
#include "xin_video_common.h"
#include "basic_macro.h"
#include "video_macro.h"
#include "h265p_bit_stream_struct.h"
#include "h265p_cabac_struct.h"
#include "h265p_cabac_context.h"
#include "h265p_trans_unit_struct.h"
#include "h265p_picture_struct.h"
#include "h265p_macro_block_struct.h"
#include "h265p_section_struct.h"
#include "h265p_sub_dct.h"
#include "h26x_common_data.h"

void Xin265pSubFDct (
    xin_sec_struct *secSet,
    PIXEL          *input,
    intptr_t       inputStride,
    PIXEL          *pred,
    intptr_t       predStride,
    SINT32         *output,
    intptr_t       outputStride,
    UINT32         tranType,
    UINT32         tranSize)
{
    SINT16          residual[XIN_MAX_TX_SIZE*XIN_MAX_TX_SIZE];
    UINT32          height;
    UINT32          width;
    UINT32          lgWidth;
    SINT32          *tempBuffer;
    xin_func_struct *funcSet;

    width      = txSize2TxDim[tranSize][0];
    height     = txSize2TxDim[tranSize][1];
    lgWidth    = calcLog2[width];
    tempBuffer = (SINT32 *)secSet->tempBuffer;
    funcSet    = secSet->funcSet;

    funcSet->pfXinBlockSub[lgWidth] (
        input,
        inputStride,
        pred,
        predStride,
        residual,
        XIN_MAX_TX_SIZE,
        width,
        height);
    
    funcSet->pfXinForTrans2d[tranSize] (
            residual,
            XIN_MAX_TX_SIZE,
            output,
            outputStride,
            tranType,
            tranSize,
            tempBuffer);

}

