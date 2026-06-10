/***************************************************************************//**
 *
 * @file          h265p_idct_add.c
 * @brief         av1 inverse transform and add prediction.
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
#include "h265p_inverse_trans.h"
#include "h26x_definition.h"
#include "h265p_definition.h"
#include "h265p_constant.h"
#include "xin_video_common.h"
#include "video_macro.h"
#include "h265p_trans_context.h"
#include "h265p_bit_stream_struct.h"
#include "h265p_cabac_struct.h"
#include "h265p_cabac_context.h"
#include "h265p_trans_unit_struct.h"
#include "h265p_picture_struct.h"
#include "h265p_macro_block_struct.h"
#include "h265p_section_struct.h"
#include "h265p_idct_add.h"
#include "h26x_common_data.h"

void Xin265pIDctAdd (
    xin_sec_struct *secSet,
    SINT32         *input,
    intptr_t       inputStride,
    PIXEL          *pred,
    intptr_t       predStride,
    PIXEL          *output,
    intptr_t       outputStride,
    UINT32         tranType,
    UINT32         tranSize)
{
    SINT16          buffer[XIN_MAX_TX_SIZE*XIN_MAX_TX_SIZE];
    SINT32          width;
    SINT32          lgWidth;
    SINT32          height;
    SINT32          *tempBuffer;
    xin_func_struct *funcSet;

    tempBuffer = (SINT32 *)secSet->tempBuffer;
    funcSet    = secSet->funcSet;
    
    funcSet->pfXinInvTrans2d[tranSize] (
        input,
        inputStride,
        buffer,
        XIN_MAX_TX_SIZE,
        tranType,
        tranSize,
        tempBuffer);

    width   = txSize2TxDim[tranSize][0];
    height  = txSize2TxDim[tranSize][1];
    lgWidth = calcLog2[width];
    
    funcSet->pfXinBlockRecon[lgWidth] (
        buffer,
        XIN_MAX_TX_SIZE,
        pred,
        predStride,
        output,
        outputStride,
        width,
        height);

}

