/***************************************************************************//**
 *
 * @file          h266_copy_and_pad.c
 * @brief         h266 copy and padding generic subroutines.
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
#include "h266_constant.h"
#include "xin_video_common.h"
#include "h26x_definition.h"
#include "h266_definition.h"
#include "h266_inter_pred_context.h"
#include "basic_macro.h"
#include "h26x_block_utility.h"
#include "memory.h"
#include "h26x_compute_dist.h"
#include "h26x_common_data.h"

void Xin266CopyAndPad (
    PIXEL      *ref,
    intptr_t   refStride,
    PIXEL      *pad,
    intptr_t   padStride,
    UINT32     width,
    UINT32     height)
{
    SINT32   offset;

    width    += LUMA_INTERP_TAPS - 1;
    height   += LUMA_INTERP_TAPS - 1;
    offset    = (LUMA_INTERP_TAPS>>1) - 1;

    Xin26xBlockCopy (
        ref - offset*refStride - offset,
        refStride,
        pad - offset*padStride - offset,
        padStride,
        width,
        height);

    Xin26xBlockPad2 (
        pad - offset*padStride - offset,
        padStride,
        width,
        height,
        DMVR_NUM_ITERATION,
        DMVR_NUM_ITERATION);

}

void Xin266CopyAndPadUv (
    PIXEL      *refU,
    PIXEL      *refV,
    intptr_t   refStride,
    PIXEL      *padU,
    PIXEL      *padV,
    intptr_t   padStride,
    UINT32     width,
    UINT32     height)
{
    SINT32  offset;

    width  += CHROMA_INTERP_TAPS - 1;
    height += CHROMA_INTERP_TAPS - 1;
    offset  = (CHROMA_INTERP_TAPS>>1) - 1;

    Xin26xBlockCopy (
        refU - offset*refStride - offset,
        refStride,
        padU - offset*padStride - offset,
        padStride,
        width,
        height);

    Xin26xBlockCopy (
        refV - offset*refStride - offset,
        refStride,
        padV - offset*padStride - offset,
        padStride,
        width,
        height);

    Xin26xBlockPad1 (
        padU - offset*padStride - offset,
        padStride,
        width,
        height,
        DMVR_NUM_ITERATION>>1,
        DMVR_NUM_ITERATION>>1);

    Xin26xBlockPad1 (
        padV - offset*padStride - offset,
        padStride,
        width,
        height,
        DMVR_NUM_ITERATION>>1,
        DMVR_NUM_ITERATION>>1);

}
