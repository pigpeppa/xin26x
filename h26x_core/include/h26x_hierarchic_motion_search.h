/***************************************************************************//**
 *
 * @file          h26x_hierarchic_motion_search.h
 * @brief         This file declares hierarchic motion search subroutines.
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
#ifndef _h26x_hierarchic_motion_search_h_
#define _h26x_hierarchic_motion_search_h_

//*************************************************************************
//                          Constants
//*************************************************************************
#define HME_LEVEL1_H_STEP       1
#define HME_LEVEL1_V_STEP       2

void Xin26xMotionSearchHier1 (
    PIXEL    *input1,
    intptr_t input1Stride,
    PIXEL    *ref1,
    intptr_t ref1Stride,
    SINT32   minMvX,
    SINT32   maxMvX,
    SINT32   minMvY,
    SINT32   maxMvY,
    UINT32   width,
    UINT32   height,
    xin_mv_u *bestMv);

void Xin26xMotionSearchHier2 (
    PIXEL    *input2,
    intptr_t input2Stride,
    PIXEL    *ref2,
    intptr_t ref2Stride,
    SINT32   minMvX,
    SINT32   maxMvX,
    SINT32   minMvY,
    SINT32   maxMvY,
    UINT32   width,
    UINT32   height,
    xin_mv_u *bestMv);

void Xin26xMotionSearchHier14xH_SSE4 (
    PIXEL    *input1,
    intptr_t input1Stride,
    PIXEL    *ref1,
    intptr_t ref1Stride,
    SINT32   minMvX,
    SINT32   maxMvX,
    SINT32   minMvY,
    SINT32   maxMvY,
    UINT32   width,
    UINT32   height,
    xin_mv_u *bestMv);

void Xin26xMotionSearchHier18xH_SSE4 (
    PIXEL    *input1,
    intptr_t input1Stride,
    PIXEL    *ref1,
    intptr_t ref1Stride,
    SINT32   minMvX,
    SINT32   maxMvX,
    SINT32   minMvY,
    SINT32   maxMvY,
    UINT32   width,
    UINT32   height,
    xin_mv_u *bestMv);

void Xin26xMotionSearchHier116xH_SSE4 (
    PIXEL    *input1,
    intptr_t input1Stride,
    PIXEL    *ref1,
    intptr_t ref1Stride,
    SINT32   minMvX,
    SINT32   maxMvX,
    SINT32   minMvY,
    SINT32   maxMvY,
    UINT32   width,
    UINT32   height,
    xin_mv_u *bestMv);

void Xin26xMotionSearchHier132xH_AVX2 (
    PIXEL    *input1,
    intptr_t input1Stride,
    PIXEL    *ref1,
    intptr_t ref1Stride,
    SINT32   minMvX,
    SINT32   maxMvX,
    SINT32   minMvY,
    SINT32   maxMvY,
    UINT32   width,
    UINT32   height,
    xin_mv_u *bestMv);

void Xin26xMotionSearchHier164xH_AVX2 (
    PIXEL    *input1,
    intptr_t input1Stride,
    PIXEL    *ref1,
    intptr_t ref1Stride,
    SINT32   minMvX,
    SINT32   maxMvX,
    SINT32   minMvY,
    SINT32   maxMvY,
    UINT32   width,
    UINT32   height,
    xin_mv_u *bestMv);

void Xin26xMotionSearchHier24xH_SSE4 (
    PIXEL    *input1,
    intptr_t input1Stride,
    PIXEL    *ref1,
    intptr_t ref1Stride,
    SINT32   minMvX,
    SINT32   maxMvX,
    SINT32   minMvY,
    SINT32   maxMvY,
    UINT32   width,
    UINT32   height,
    xin_mv_u *bestMv);

void Xin26xMotionSearchHier28xH_SSE4 (
    PIXEL    *input1,
    intptr_t input1Stride,
    PIXEL    *ref1,
    intptr_t ref1Stride,
    SINT32   minMvX,
    SINT32   maxMvX,
    SINT32   minMvY,
    SINT32   maxMvY,
    UINT32   width,
    UINT32   height,
    xin_mv_u *bestMv);

void Xin26xMotionSearchHier216xH_SSE4 (
    PIXEL    *input1,
    intptr_t input1Stride,
    PIXEL    *ref1,
    intptr_t ref1Stride,
    SINT32   minMvX,
    SINT32   maxMvX,
    SINT32   minMvY,
    SINT32   maxMvY,
    UINT32   width,
    UINT32   height,
    xin_mv_u *bestMv);

void Xin26xMotionSearchHier232xH_AVX2 (
    PIXEL    *input1,
    intptr_t input1Stride,
    PIXEL    *ref1,
    intptr_t ref1Stride,
    SINT32   minMvX,
    SINT32   maxMvX,
    SINT32   minMvY,
    SINT32   maxMvY,
    UINT32   width,
    UINT32   height,
    xin_mv_u *bestMv);

#endif