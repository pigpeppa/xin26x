/***************************************************************************//**
 *
 * @file          h26x_one_dim_motion_search.h
 * @brief         Declares one-dimension motion search subroutines.
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
#ifndef _h26x_one_dim_motion_search_h_
#define _h26x_one_dim_motion_search_h_

//*************************************************************************
//                          Constants
//*************************************************************************
#define XIN_VER_SEARCH_STEP     4
#define XIN_VER_SEARCH_SIZE     512
#define XIN_HOR_SEARCH_SIZE     256

void Xin26xHorMotionSearch (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    SINT32   minMv,
    SINT32   maxMv,
    UINT32   width,
    UINT32   height,
    SINT32   *outputMv,
    UINT32   *outputDist);

void Xin26xVerMotionSearch (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    SINT32   minMv,
    SINT32   maxMv,
    UINT32   width,
    UINT32   height,
    SINT32   *outputMv,
    UINT32   *outputDist);

void Xin26xHorMotionSearch8xH_SSE4 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref,
    intptr_t refStride,
    SINT32   minMv,
    SINT32   maxMv,
    UINT32   width,
    UINT32   height,
    SINT32   *outputMv,
    UINT32   *outputDist);

void Xin26xHorMotionSearch16xH_SSE4 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref,
    intptr_t refStride,
    SINT32   minMv,
    SINT32   maxMv,
    UINT32   width,
    UINT32   height,
    SINT32   *outputMv,
    UINT32   *outputDist);

void Xin26xHorMotionSearch32xH_SSE4 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref,
    intptr_t refStride,
    SINT32   minMv,
    SINT32   maxMv,
    UINT32   width,
    UINT32   height,
    SINT32   *outputMv,
    UINT32   *outputDist);

void Xin26xHorMotionSearch64xH_SSE4 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref,
    intptr_t refStride,
    SINT32   minMv,
    SINT32   maxMv,
    UINT32   width,
    UINT32   height,
    SINT32   *outputMv,
    UINT32   *outputDist);

void Xin26xHorMotionSearch32xH_AVX2 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref,
    intptr_t refStride,
    SINT32   minMv,
    SINT32   maxMv,
    UINT32   width,
    UINT32   height,
    SINT32   *outputMv,
    UINT32   *outputDist);

void Xin26xHorMotionSearch64xH_AVX2 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref,
    intptr_t refStride,
    SINT32   minMv,
    SINT32   maxMv,
    UINT32   width,
    UINT32   height,
    SINT32   *outputMv,
    UINT32   *outputDist);

void Xin26xVerMotionSearch8xH_SSE4 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref,
    intptr_t refStride,
    SINT32   minMv,
    SINT32   maxMv,
    UINT32   width,
    UINT32   height,
    SINT32   *outputMv,
    UINT32   *outputDist);

void Xin26xVerMotionSearch16xH_SSE4 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref,
    intptr_t refStride,
    SINT32   minMv,
    SINT32   maxMv,
    UINT32   width,
    UINT32   height,
    SINT32   *outputMv,
    UINT32   *outputDist);

void Xin26xVerMotionSearch32xH_SSE4 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref,
    intptr_t refStride,
    SINT32   minMv,
    SINT32   maxMv,
    UINT32   width,
    UINT32   height,
    SINT32   *outputMv,
    UINT32   *outputDist);

void Xin26xVerMotionSearch64xH_SSE4 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref,
    intptr_t refStride,
    SINT32   minMv,
    SINT32   maxMv,
    UINT32   width,
    UINT32   height,
    SINT32   *outputMv,
    UINT32   *outputDist);

void Xin26xVerMotionSearch32xH_AVX2 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref,
    intptr_t refStride,
    SINT32   minMv,
    SINT32   maxMv,
    UINT32   width,
    UINT32   height,
    SINT32   *outputMv,
    UINT32   *outputDist);

void Xin26xVerMotionSearch64xH_AVX2 (
    UINT8    *input,
    intptr_t inputStride,
    UINT8    *ref,
    intptr_t refStride,
    SINT32   minMv,
    SINT32   maxMv,
    UINT32   width,
    UINT32   height,
    SINT32   *outputMv,
    UINT32   *outputDist);

#endif

