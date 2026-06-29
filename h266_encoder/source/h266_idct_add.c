/***************************************************************************//**
 *
 * @file          h266_idct_add.c
 * @brief         h266 inverse transform and add prediction.
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
#include "h266_lmcs.h"
#include "h266_func_struct.h"

void Xin266IDctAdd (
    xin_sec_struct *secSet,
    COEFF          *input,
    intptr_t       inputStride,
    PIXEL          *pred,
    intptr_t       predStride,
    PIXEL          *output,
    intptr_t       outputStride,
    BOOL           isIntra,
    UINT32         compId,
    UINT32         mtsIdx,
    UINT32         width,
    UINT32         height)
{
    xin_func_struct *funcSet;
    xin_seq_struct  *seqSet;
    xin_pic_struct  *picSet;
    xin_lmcs_struct *lmcsSet;
    COEFF           buffer[XIN_MAX_TU_SIZE*XIN_MAX_TU_SIZE];
    UINT32          lgWidth;
    
    funcSet = secSet->funcSet;
    lgWidth = calcLog2[width];
    picSet  = secSet->picSet;
    seqSet  = secSet->seqSet;
    lmcsSet = picSet->lmcsSet;

    Xin266IDctWxH (
        secSet,
        input,
        inputStride,
        buffer,
        XIN_MAX_TU_SIZE,
        width,
        height,
        isIntra,
        compId,
        mtsIdx);

    if ((seqSet->config.enableLmcs) && (lmcsSet->lmcsParam.enableChromaAdj) && (compId != PLANE_LUMA))
    {
        Xin266InvScaleSignal (
            buffer,
            XIN_MAX_TU_SIZE,
            width,
            height,
            secSet->chromaResScaleInv);
    }
    
    funcSet->pfXinBlockAddForDct[lgWidth] (
        buffer,
        XIN_MAX_TU_SIZE,
        pred,
        predStride,
        output,
        outputStride,
        width,
        height);
    
}

