/***************************************************************************//**
 *
 * @file          h265p_intra_pred_filter.h
 * @brief         This file declares av1 intra prediction filter subroutines and weights.
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

#ifndef _h265p_intra_pred_filter_h_
#define _h265p_intra_pred_filter_h_

#define XIN_LOG_SM_WEIGHT_SCALE     8
extern const UINT8  intraSmWeightU8[2 * 64];
extern const UINT16 intraSmWeightU16[2 * 64];

void Xin265pIntraPredDrZ1 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf,
    BOOL     upSampleTop,
    SINT32   dx,
    SINT32   dy);

void Xin265pIntraPredDrZ2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf,
    BOOL     upSampleTop,
    BOOL     upSampleLft,
    SINT32   dx,
    SINT32   dy);

void Xin265pIntraPredDrZ3 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf,
    BOOL     upSampleLft,
    SINT32   dx,
    SINT32   dy);

void Xin265pIntraPredVer (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredHor (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredDc (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredSm (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredSmH (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredSmV (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredPaeth (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredHor4xH_SSE4 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredHor8xH_SSE4 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredHor16xH_SSE4 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredHor32xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredHor64xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredVer4xH_SSE4 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredVer8xH_SSE4 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredVer16xH_SSE4 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredVer32xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredVer64xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPred4xHDrZ2_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf,
    BOOL     upSampleTop,
    BOOL     upSampleLft,
    SINT32   dx,
    SINT32   dy);

void Xin265pIntraPred8xHDrZ2_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf,
    BOOL     upSampleTop,
    BOOL     upSampleLft,
    SINT32   dx,
    SINT32   dy);

void Xin265pIntraPredWxHDrZ2_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf,
    BOOL     upSampleTop,
    BOOL     upSampleLft,
    SINT32   dx,
    SINT32   dy);

void Xin265pIntraPred4xHDrZ1_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf,
    BOOL     upSampleTop,
    SINT32   dx,
    SINT32   dy);

void Xin265pIntraPred8xHDrZ1_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf,
    BOOL     upSampleTop,
    SINT32   dx,
    SINT32   dy);

void Xin265pIntraPred16xHDrZ1_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf,
    BOOL     upSampleTop,
    SINT32   dx,
    SINT32   dy);

void Xin265pIntraPred32xHDrZ1_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf,
    BOOL     upSampleTop,
    SINT32   dx,
    SINT32   dy);

void Xin265pIntraPred64xHDrZ1_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf,
    BOOL     upSampleTop,
    SINT32   dx,
    SINT32   dy);

void Xin265pIntraPredPaeth8xH_SSE4 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredPaeth16xH_SSE4 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredPaeth32xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredPaeth64xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredSmV8xH_SSE4 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredSmV16xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredSmV32xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredSmV64xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredSm8xH_SSE4 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredSm16xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredSm16xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredSm32xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredSm64xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredSmH8xH_SSE4 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredSmH16xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredSmH32xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

void Xin265pIntraPredSmH64xH_AVX2 (
    PIXEL    *dst,
    intptr_t dstStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *topBuf,
    PIXEL    *lftBuf);

#endif

