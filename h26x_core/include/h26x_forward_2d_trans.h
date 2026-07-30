/***************************************************************************//**
 *
 * @file          h26x_forward_2d_trans.h
 * @brief         h.26x forward transform subroutines.
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
#ifndef _h26x_forward_2d_trans_h_
#define _h26x_forward_2d_trans_h_

void Xin26xFDct2WxH (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf,
    SINT32      shift1st,
    SINT32      shift2nd);

void Xin26xFDct2W4H4_SSE4 (
    COEFF    *input,
    intptr_t inputStride,
    COEFF    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height,
    COEFF    *tempBuf,
    SINT32   shift1st,
    SINT32   shift2nd);

void Xin26xFDstW4H4_SSE4 (
    SINT16   *input,
    intptr_t inputStride,
    COEFF    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height,
    COEFF    *tempBuf,
    SINT32   shift1st,
    SINT32   shift2nd);

void Xin26xFDct2W8H8_AVX2 (
    COEFF    *input,
    intptr_t inputStride,
    COEFF    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height,
    COEFF    *tempBuf,
    SINT32   shift1st,
    SINT32   shift2nd);

void Xin26xFDct2W16H16_AVX2 (
    COEFF    *input,
    intptr_t inputStride,
    COEFF    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height,
    COEFF    *tempBuf,
    SINT32   shift1st,
    SINT32   shift2nd);

void Xin26xFDct2W32H32_AVX2 (
    COEFF    *input,
    intptr_t inputStride,
    COEFF    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height,
    COEFF    *tempBuf,
    SINT32   shift1st,
    SINT32   shift2nd);

void Xin26xFDct2W8H16_AVX2 (
    COEFF    *input,
    intptr_t inputStride,
    COEFF    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height,
    COEFF    *tempBuf,
    SINT32   shift1st,
    SINT32   shift2nd);

void Xin26xFDct2W16H8_AVX2 (
    COEFF    *input,
    intptr_t inputStride,
    COEFF    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height,
    COEFF    *tempBuf,
    SINT32   shift1st,
    SINT32   shift2nd);

void Xin26xFDct2W32H16_AVX2 (
    COEFF    *input,
    intptr_t inputStride,
    COEFF    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height,
    COEFF    *tempBuf,
    SINT32   shift1st,
    SINT32   shift2nd);

void Xin26xFDct2W16H32_AVX2 (
    COEFF    *input,
    intptr_t inputStride,
    COEFF    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height,
    COEFF    *tempBuf,
    SINT32   shift1st,
    SINT32   shift2nd);

void Xin26xFDct2W32H8_AVX2 (
    COEFF    *input,
    intptr_t inputStride,
    COEFF    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height,
    COEFF    *tempBuf,
    SINT32   shift1st,
    SINT32   shift2nd);

void Xin26xFDct2W8H32_AVX2 (
    COEFF    *input,
    intptr_t inputStride,
    COEFF    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height,
    COEFF    *tempBuf,
    SINT32   shift1st,
    SINT32   shift2nd);

void Xin26xFDct2W64H64_AVX2 (
    COEFF    *input,
    intptr_t inputStride,
    COEFF    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height,
    COEFF    *tempBuf,
    SINT32   shift1st,
    SINT32   shift2nd);

void Xin26xFDct2W32H64_AVX2 (
    COEFF    *input,
    intptr_t inputStride,
    COEFF    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height,
    COEFF    *tempBuf,
    SINT32   shift1st,
    SINT32   shift2nd);

void Xin26xFDct2W64H32_AVX2 (
    COEFF    *input,
    intptr_t inputStride,
    COEFF    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height,
    COEFF    *tempBuf,
    SINT32   shift1st,
    SINT32   shift2nd);

void Xin26xFDct2W16H64_AVX2 (
    COEFF    *input,
    intptr_t inputStride,
    COEFF    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height,
    COEFF    *tempBuf,
    SINT32   shift1st,
    SINT32   shift2nd);

void Xin26xFDct2W64H16_AVX2 (
    COEFF    *input,
    intptr_t inputStride,
    COEFF    *output,
    intptr_t outputStride,
    UINT32   width,
    UINT32   height,
    COEFF    *tempBuf,
    SINT32   shift1st,
    SINT32   shift2nd);

void Xin26xFSkipWxH (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf);

void Xin26xFSkip64xH_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf);

void Xin26xFSkip32xH_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf);

void Xin26xFSkip16xH_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf);

void Xin26xFSkip8xH_SSE2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf);

void Xin26xFSkip4xH_SSE2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf);

#endif

