/***************************************************************************//**
 *
 * @file          h26x_construct_bi_me_input.h
 * @brief         This file declares subroutines to construct bi-directional ME input.
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
#ifndef _h26x_contruct_bi_me_input_h_
#define _h26x_contruct_bi_me_input_h_

void Xin26xConstructBiMeInput (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    PIXEL    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height);

void Xin26xConstructBiMeInput8xH_SSE2 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *pred,
    intptr_t predStride,
    UINT8    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height);

void Xin26xConstructBiMeInput16xH_SSE2 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *pred,
    intptr_t predStride,
    UINT8    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height);

void Xin26xConstructBiMeInputGt16xH_SSE2 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *pred,
    intptr_t predStride,
    UINT8    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height);

#endif
