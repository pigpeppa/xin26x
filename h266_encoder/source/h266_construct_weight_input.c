/***************************************************************************//**
 *
 * @file          h266_construct_weight_Input.c
 * @brief         Construct input for weighted bi-directional motion estimation.
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
#include <xin_typedef.h>
#include <h266_constant.h>
#include "basic_macro.h"

void Xin266ConstructWeightBiMeInput (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    PIXEL    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height,
    SINT32   bcwWeight)
{
    SINT32  normalizer;
    SINT32  weight0;
    SINT32  weight1;
    UINT32  rowIdx;
    UINT32  colIdx;
    SINT32  unclipPel;
    
    normalizer = ((1 << 12) + (bcwWeight > 0 ? (bcwWeight >> 1) : -(bcwWeight >> 1))) / bcwWeight;
    weight0    = normalizer << XIN_BCW_LOG_WGT_BASE;
    weight1    = (XIN_BCW_WGT_BASE - bcwWeight)*normalizer;

    for (rowIdx = 0; rowIdx < height; rowIdx++)
    {
        for (colIdx = 0; colIdx < width; colIdx++)
        {
            unclipPel = (input[colIdx]*weight0 - pred[colIdx]*weight1 + (1 << 11)) >> 12;

            output[colIdx] = (PIXEL)XIN_CLIP (unclipPel, MIN_PIXEL_VALUE, MAX_PIXEL_VALUE);
        }

        input  += inputStride;
        pred   += predStride;
        output += outputStride;
    }

}
