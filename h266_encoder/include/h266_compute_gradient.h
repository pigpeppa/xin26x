/***************************************************************************//**
 *
 * @file          h266_compute_gradient.h
 * @brief         This file declares gradient computation subroutines.
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
#ifndef _h266_compute_gradient_h_
#define _h266_compute_gradient_h_

void Xin266ComputeGradient (
    PIXEL    *input,
    intptr_t inputStride,
    SINT32   *cuGrad,
    SINT32   width,
    SINT32   height);

void Xin266ComputeGradient32xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    SINT32   *cuGrad,
    SINT32   width,
    SINT32   height);

void Xin266ComputeGradient64xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    SINT32   *cuGrad,
    SINT32   width,
    SINT32   height);

#endif

