/***************************************************************************//**
 *
 * @file          h26x_me_func_struct.h
 * @brief         This file contains h26x motion estimation SIMD function definitions.
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
#ifndef _h26x_me_func_struct_h_
#define _h26x_me_func_struct_h_

typedef enum xin_me_size 
{
    XIN_ME_SIZE_1xH   = 0,
    XIN_ME_SIZE_2xH   = 1,
    XIN_ME_SIZE_4xH   = 2,
    XIN_ME_SIZE_8xH   = 3,
    XIN_ME_SIZE_16xH  = 4,
    XIN_ME_SIZE_32xH  = 5,
    XIN_ME_SIZE_64xH  = 6,
    XIN_ME_SIZE_128xH = 7,
    XIN_ME_SIZE_NUM   = 8,
}xin_me_size;

typedef void (*XinComputeDist) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

typedef void (*XinComputeAvgDist) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *refA,
    PIXEL    *refB,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

typedef void (*XinComputeSadx8) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref[8],
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

typedef void (*XinComputeSadx5) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref0,
    PIXEL    *ref1,
    PIXEL    *ref2,
    PIXEL    *ref3,
    PIXEL    *ref4,
    intptr_t refStride,
    UINT32   width,
    UINT32   height,
    UINT32   *sad);

typedef void (*XinComputeSadx3) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref0,
    PIXEL    *ref1,
    PIXEL    *ref2,
    intptr_t refStride,
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

typedef void (*Xin1DMotionSearch) (
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

typedef void (*XinDownscale2x2) (
    PIXEL       *pu8InputCorner,
    intptr_t    s32InputStride,
    BOOL        bHaveInputRowAbove,
    BOOL        bHaveInputRowBelow,
    PIXEL       *pu8OutputCorner,
    intptr_t    s32OutputStride,
    UINT32      u32OutputWidth,
    UINT32      u32OutputHeight);

typedef void (*XinHierMotionSearch) (
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

typedef void (*XinContructBiMeInput) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    PIXEL    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height);

typedef void (*XinInterpHalfPel) (
    PIXEL       *ref,
    intptr_t    refStride,
    UINT32      width,
    UINT32      height,
    PIXEL       *intergPel,
    PIXEL       *halfPelH,
    PIXEL       *halfPelV,
    PIXEL       *halfPelHv,
    intptr_t    interpStride,
    BOOL        interpAlt);

typedef void(*XinComputeVar) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *pred,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   *var);

typedef void(*XinComputeAvgVar) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *predA,
    PIXEL    *predB,
    intptr_t predStride,
    UINT32   width,
    UINT32   height,
    UINT32   *var);

typedef struct xin_me_func
{
    XinComputeDist       pfXinComputeSad[XIN_ME_SIZE_NUM];
    XinComputeAvgDist    pfXinComputeAvgSad[XIN_ME_SIZE_NUM];
    XinComputeSadx8      pfXinComputeSadx8[XIN_ME_SIZE_NUM];
    XinComputeSadx5      pfXinComputeSadx5[XIN_ME_SIZE_NUM];
    XinComputeSadx3      pfXinComputeSadx3[XIN_ME_SIZE_NUM];
    XinComputeAvgSadx8   pfXinComputeAvgSadx8[XIN_ME_SIZE_NUM];
    Xin1DMotionSearch    pfXinHorMotionSearch[XIN_ME_SIZE_NUM];
    Xin1DMotionSearch    pfXinVerMotionSearch[XIN_ME_SIZE_NUM];
    XinComputeVar        pfXinComputeVar[XIN_ME_SIZE_NUM];
    XinComputeAvgVar     pfXinComputeAvgVar[XIN_ME_SIZE_NUM];
    XinHierMotionSearch  pfXinMotionSearchHier2[XIN_ME_SIZE_NUM];
    XinHierMotionSearch  pfXinMotionSearchHier1[XIN_ME_SIZE_NUM];
    XinContructBiMeInput pfXinConstructBiMeInput[XIN_ME_SIZE_NUM];
    XinInterpHalfPel     pfXinInterpoHalfPel;
    XinDownscale2x2      pfXinDownscale2x2;
}xin_me_func;

#endif
