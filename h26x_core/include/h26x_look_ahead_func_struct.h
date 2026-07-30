/***************************************************************************//**
 *
 * @file          h26x_look_ahead_func_struct.h
 * @brief         This file contains h26x look ahead SIMD function definitions.
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
#ifndef _h26x_look_ahead_func_struct_h_
#define _h26x_look_ahead_func_struct_h_

typedef void (*XinIntraLower) (
    PIXEL    *dst,
    intptr_t dstStride,
    PIXEL    *nBuf,
    UINT32   size);

typedef void (*XinComputeDist) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

typedef void (*XinComputeAvgSadx8) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA[8],
    PIXEL    *refB[8],
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

typedef void (*XinBlockAvg) (
    const PIXEL *src0,
    intptr_t    src0Stride,
    const PIXEL *src1,
    intptr_t    src1Stride,
    PIXEL       *dst,
    intptr_t    dstStride,
    UINT32      width,
    UINT32      height);

typedef void (*XinComputeAvgDist) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA,
    PIXEL    *refB,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

typedef struct xin_la_func
{
    XinIntraLower        pfXinIntraLower[XIN_LA_INTRA_NUM][XIN_BLOCK_NUM];
    XinComputeDist       pfXinComputeSad[XIN_BLOCK_NUM];
    XinComputeDist       pfXinComputeSatd[XIN_BLOCK_NUM];
    XinComputeDist       pfXinComputeDist[XIN_BLOCK_NUM];
    XinComputeAvgSadx8   pfXinComputeAvgSadx8[XIN_BLOCK_NUM];
    XinBlockAvg          pfXinBlockAvg[XIN_BLOCK_NUM];
    XinComputeAvgDist    pfXinComputeAvgSad[XIN_BLOCK_NUM];
    XinComputeAvgDist    pfXinComputeAvgSatd[XIN_BLOCK_NUM];
    XinComputeAvgDist    pfXinComputeAvgDist[XIN_BLOCK_NUM];
    XinDownscale2x2      pfXinDownscale2x2;
}xin_la_func;

#endif

