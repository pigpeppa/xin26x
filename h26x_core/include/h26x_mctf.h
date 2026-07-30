/***************************************************************************//**
 *
 * @file          h26x_mctf.h
 * @brief         This file declares h26x temporal filter subroutines.
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
#ifndef _h26x_mctf_h_
#define _h26x_mctf_h_

#define XIN_MOTION_VECT_FACTOR   16
#define XIN_MOTION_VECT_MASK     15
#define XIN_MOTION_VECT_BITS     4

void Xin26xMctfMotionEstFrame (
    xin_mctf_struct *mctfSet,
    SINT32          refIdx,
    SINT32          searchLevel,
    SINT32          blockSize);

void Xin26xMctfSubSample (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *output,
    intptr_t outputStride,
    SINT32   frameWidth,
    SINT32   frameHeight,
    SINT32   marginX,
    SINT32   marginY,
    SINT32   factor);

void Xin26xMctfSubSample_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *output,
    intptr_t outputStride,
    SINT32   frameWidth,
    SINT32   frameHeight,
    SINT32   marginX,
    SINT32   marginY,
    SINT32   factor);

void Xin26xMctfBiFilterFrame (
    xin_mctf_struct *mctfSet);

void Xin26xMctfFuncInit (
    xin_mctf_struct *mctfSet,
    UINT32          cpuFeature);

void Xin26xMotionSsd8xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    SINT32   mvX,
    SINT32   mvY,
    SINT32   width,
    SINT32   height,
    SINT32   *outError);

void Xin26xMotionSsd16xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    SINT32   mvX,
    SINT32   mvY,
    SINT32   width,
    SINT32   height,
    SINT32   *outError);

void Xin26xMotionSad8xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    SINT32   mvX,
    SINT32   mvY,
    SINT32   width,
    SINT32   height,
    SINT32   *outError);

void Xin26xMotionSad16xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    SINT32   mvX,
    SINT32   mvY,
    SINT32   width,
    SINT32   height,
    SINT32   *outError);

void Xin26xMctfMotionComp16xH_AVX2 ( 
    PIXEL      *ref,
    intptr_t   refStride,
    PIXEL      *pred,
    intptr_t   predStride,
    xin_mv32_u *mv,
    SINT32     width,
    SINT32     height,
    BOOL       isChroma);

void Xin26xMctfMotionComp8xH_SSE2 (
    PIXEL      *ref,
    intptr_t   refStride,
    PIXEL      *pred,
    intptr_t   predStride,
    xin_mv32_u *mv,
    SINT32     width,
    SINT32     height,
    BOOL       isChroma);

void Xin26xMctfMotionComp (
    PIXEL      *ref,
    intptr_t   refStride,
    PIXEL      *pred,
    intptr_t   predStride,
    xin_mv32_u *mv,
    SINT32     width,
    SINT32     height,
    BOOL       isChroma);

void Xin26xMctfMotionComp4xH_SSE2 (
    PIXEL      *ref,
    intptr_t   refStride,
    PIXEL      *pred,
    intptr_t   predStride,
    xin_mv32_u *mv,
    SINT32     width,
    SINT32     height,
    BOOL       isChroma);

void Xin26xMctfFrame (
    xin_mctf_struct  *mctfSet);

void Xin26xMctfBlockWeightSum (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref[XIN_TF_MAX_REF_NUM],
    intptr_t refStride,
    UINT32   refNum,
    UINT16   weight[XIN_TF_MAX_REF_NUM + 1],
    SINT32   width,
    SINT32   height);

void Xin26xMctfBlockWeightSum4xH_SSE4 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref[XIN_TF_MAX_REF_NUM],
    intptr_t refStride,
    UINT32   refNum,
    UINT16   weight[XIN_TF_MAX_REF_NUM + 1],
    SINT32   width,
    SINT32   height);

void Xin26xMctfBlockWeightSum8xH_SSE4 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref[XIN_TF_MAX_REF_NUM],
    intptr_t refStride,
    UINT32   refNum,
    UINT16   weight[XIN_TF_MAX_REF_NUM + 1],
    SINT32   width,
    SINT32   height);

void Xin26xMctfBlockWeightSum16xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref[XIN_TF_MAX_REF_NUM],
    intptr_t refStride,
    UINT32   refNum,
    UINT16   weight[XIN_TF_MAX_REF_NUM + 1],
    SINT32   width,
    SINT32   height);

void Xin26xMotionSad4Tap8xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    SINT32   mvX,
    SINT32   mvY,
    SINT32   width,
    SINT32   height,
    SINT32   *outError);

void Xin26xMotionSad4Tap16xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    SINT32   mvX,
    SINT32   mvY,
    SINT32   width,
    SINT32   height,
    SINT32   *outError);

void Xin26xMotionSad4Tap32xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    SINT32   mvX,
    SINT32   mvY,
    SINT32   width,
    SINT32   height,
    SINT32   *outError);

void Xin26xMotionSad6Tap8xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    SINT32   mvX,
    SINT32   mvY,
    SINT32   width,
    SINT32   height,
    SINT32   *outError);


void Xin26xMotionSad6Tap16xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    SINT32   mvX,
    SINT32   mvY,
    SINT32   width,
    SINT32   height,
    SINT32   *outError);

void Xin26xMotionSsd6Tap8xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    SINT32   mvX,
    SINT32   mvY,
    SINT32   width,
    SINT32   height,
    SINT32   *outError);

void Xin26xMotionSsd4Tap8xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    SINT32   mvX,
    SINT32   mvY,
    SINT32   width,
    SINT32   height,
    SINT32   *outError);


void Xin26xMotionSsd6Tap16xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    SINT32   mvX,
    SINT32   mvY,
    SINT32   width,
    SINT32   height,
    SINT32   *outError);

void Xin26xMotionSsd4Tap16xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    SINT32   mvX,
    SINT32   mvY,
    SINT32   width,
    SINT32   height,
    SINT32   *outError);

void Xin26xMotionSsd4Tap32xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    SINT32   mvX,
    SINT32   mvY,
    SINT32   width,
    SINT32   height,
    SINT32   *outError);

void Xin26xMctfComputeBlockNoise (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    SINT32   width,
    SINT32   height,
    SINT32   *noise);

void Xin26xMctfComputeBlockNoise8xH_SSE4 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    SINT32   width,
    SINT32   height,
    SINT32   *noise);

void Xin26xMctfComputeBlockNoise16xH_AVX2 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    SINT32   width,
    SINT32   height,
    SINT32   *noise);

void Xin26xMctfComputeBlockNoise4xH_SSE4 (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *ref,
    intptr_t refStride,
    SINT32   width,
    SINT32   height,
    SINT32   *noise);

void Xin26xMctfComputeBlockVar (
    PIXEL    *input,
    intptr_t inputStride,
    SINT32   width,
    SINT32   height,
    SINT32   *outputVar);

void Xin26xMctfComputeBlockVar8xH_SSE4 (
    PIXEL    *input,
    intptr_t inputStride,
    SINT32   width,
    SINT32   height,
    SINT32   *outputVar);

void Xin26xMctfComputeBlockVar16xH_SSE4 (
    PIXEL    *input,
    intptr_t inputStride,
    SINT32   width,
    SINT32   height,
    SINT32   *outputVar);

#endif

