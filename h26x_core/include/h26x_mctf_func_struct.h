/***************************************************************************//**
 *
 * @file          h26x_mctf_func_struct.h
 * @brief         This file contains h26x MCTF SIMD function definitions.
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
#ifndef _h26x_mctf_func_struct_h_
#define _h26x_mctf_func_struct_h_

typedef enum xin_tf_size
{
    XIN_TF_SIZE_1xH   = 0,
    XIN_TF_SIZE_2xH   = 1,
    XIN_TF_SIZE_4xH   = 2,
    XIN_TF_SIZE_8xH   = 3,
    XIN_TF_SIZE_16xH  = 4,
    XIN_TF_SIZE_32xH  = 5,
    XIN_TF_SIZE_64xH  = 6,
    XIN_TF_SIZE_128xH = 7,
    XIN_TF_SIZE_NUM   = 8,
} xin_tf_size;

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


typedef void (*XinMctfSubSample) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *output,
    intptr_t outputStride,
    SINT32   frameWidth,
    SINT32   frameHeight,
    SINT32   marginX,
    SINT32   marginY,
    SINT32   factor);

typedef void (*XinMctfMotionSsd) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    SINT32   mvX,
    SINT32   mvY,
    SINT32   width,
    SINT32   height,
    SINT32   *outError);

typedef void (*XinMctfMotionComp) (
    PIXEL      *ref,
    intptr_t   refStride,
    PIXEL      *pred,
    intptr_t   predStride,
    xin_mv32_u *mv,
    SINT32     width,
    SINT32     height,
    BOOL       isChroma);

typedef void (*XinGenLowerFrame) (
    PIXEL       *input,
    intptr_t    inputStride,
    PIXEL       *output,
    PIXEL       *outputH,
    PIXEL       *outputV,
    PIXEL       *outputHv,
    intptr_t    outputStride,
    UINT32      outputWidth,
    UINT32      outputHeight);

typedef void (*XinComputeSsd) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *recon,
    intptr_t reconStride,
    UINT32   width,
    UINT32   height,
    UINT64   *ssd);

typedef void (*XinBlockWeightSum) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref[XIN_TF_MAX_REF_NUM],
    intptr_t refStride,
    UINT32   refNum,
    UINT16   weight[XIN_TF_MAX_REF_NUM + 1],
    SINT32   width,
    SINT32   height);

typedef void (*XinComputeBlockNoise) (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    SINT32   width,
    SINT32   height,
    SINT32   *noise);

typedef void (*XinComputeBlockVar) (
    PIXEL    *input,
    intptr_t inputStride,
    SINT32   width,
    SINT32   height,
    SINT32   *outputVar);

typedef struct xin_mctf_func
{
    XinMctfMotionSsd     pfXinMctfMotionDist[XIN_TF_SIZE_NUM];
    XinMctfSubSample     pfXinMctfSubSample;
    XinMctfMotionComp    pfXinMctfMotionComp[XIN_TF_SIZE_NUM];
    XinDownscale2x2      pfXinDownscale2x2;
    XinComputeSsd        pfXinComputeSsd[XIN_TF_SIZE_NUM];
    XinBlockWeightSum    pfXinBlockWeightSum[XIN_TF_SIZE_NUM];
    XinComputeBlockNoise pfXinComputeBlockNoise[XIN_TF_SIZE_NUM];
    XinComputeBlockVar   pfXinComputeBlockVar[XIN_TF_SIZE_NUM];
} xin_mctf_func;

#endif

