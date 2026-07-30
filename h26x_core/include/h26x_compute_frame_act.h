/***************************************************************************//**
 *
 * @file          h26x_compute_frame_act.h
 * @brief         This file computes frame activity level.
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
#ifndef _h26x_compute_frame_act_h_
#define _h26x_compute_frame_act_h_

void Xin26xComputeFrameAct (
    PIXEL       *input,
    intptr_t    inputStride,
    PIXEL       *ref,
    intptr_t    refStride,
    SINT32      width,
    SINT32      height,
    double      *outputAct);

void Xin26xComputeFrameAct_AVX2 (
    PIXEL       *input,
    intptr_t    inputStride,
    PIXEL       *ref,
    intptr_t    refStride,
    SINT32      width,
    SINT32      height,
    double      *outputAct);

#endif

