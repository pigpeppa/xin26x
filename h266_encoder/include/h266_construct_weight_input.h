/***************************************************************************//**
 *
 * @file          h266_construct_weight_input.h
 * @brief         This file declares subroutines to construct weighted bi-directional ME input.
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
#ifndef _h266_construct_weight_input_h_
#define _h266_construct_weight_input_h_

void Xin266ConstructWeightBiMeInput (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    PIXEL    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height,
    SINT32   bcwWeight);

void Xin266ConstructWeightBiMeInputGt8xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    PIXEL    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height,
    SINT32   bcwWeight);

void Xin266ConstructWeightBiMeInputGt4xH_SSE4 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    PIXEL    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height,
    SINT32   bcwWeight);

#endif
