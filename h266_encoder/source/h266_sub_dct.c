/***************************************************************************//**
 *
 * @file          h266_sub_dct.c
 * @brief         h.266 subtract prediction and perform forward transform.
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
#include "h26x_common_data.h"
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
#include "h26x_trans_context.h"
#include "h26x_forward_1d_trans.h"
#include "h266_section_struct.h"
#include "h266_forward_trans.h"
#include "h26x_block_utility.h"
#include "h266_lmcs.h"
#include "h266_func_struct.h"

void Xin266SubFDctWxH (
    xin_sec_struct *secSet,
    PIXEL          *input,
    intptr_t       inputStride,
    PIXEL          *pred,
    intptr_t       predStride,
    COEFF          *output,
    intptr_t       outputStride,
    UINT32         compId,
    UINT32         mtsIdx,
    SINT32         width,
    SINT32         height,
    BOOL           isIntra)
{
    xin_func_struct *funcSet;
    xin_seq_struct  *seqSet;
    xin_pic_struct  *picSet;
    xin_lmcs_struct *lmcsSet;
    COEFF           resi[XIN_MAX_TU_SIZE*XIN_MAX_TU_SIZE];
    UINT32          lgWidth;

    funcSet = secSet->funcSet;
    lgWidth = calcLog2[width];
    picSet  = secSet->picSet;
    seqSet  = secSet->seqSet;
    lmcsSet = picSet->lmcsSet;

    funcSet->pfXinBlockSubForDct[lgWidth] (
        input,
        inputStride,
        pred,
        predStride,
        resi,
        width,
        width,
        height);

    if ((seqSet->config.enableLmcs) && (lmcsSet->lmcsParam.enableChromaAdj) && (compId != PLANE_LUMA))
    {
        Xin266FwdScaleSignal (
            resi,
            width,
            width,
            height,
            secSet->chromaResScaleInv);
    }

    Xin266FDctWxH (
        secSet,
        resi,
        width,
        output,
        outputStride,
        compId,
        width,
        height,
        mtsIdx,
        isIntra);

}

