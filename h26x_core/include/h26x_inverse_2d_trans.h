/***************************************************************************//**
 *
 * @file          h26x_inverse_2d_trans.h
 * @brief         h.26x inverse transform subroutines.
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
#ifndef _h26x_inverse_2d_trans_h_
#define _h26x_inverse_2d_trans_h_

void Xin26xIDct2WxH (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf,
    SINT32      shift1st,
    SINT32      shift2nd);

void Xin26xIDct2W8H8_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf,
    SINT32      shift1st,
    SINT32      shift2nd);

void Xin26xIDct2W16H16_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf,
    SINT32      shift1st,
    SINT32      shift2nd);

void Xin26xIDct2W32H32_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf,
    SINT32      shift1st,
    SINT32      shift2nd);

void Xin26xIDct2W64H64_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf,
    SINT32      shift1st,
    SINT32      shift2nd);

void Xin26xIDct2W16H8_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf,
    SINT32      shift1st,
    SINT32      shift2nd);

void Xin26xIDct2W8H16_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf,
    SINT32      shift1st,
    SINT32      shift2nd);

void Xin26xIDct2W32H16_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf,
    SINT32      shift1st,
    SINT32      shift2nd);

void Xin26xIDct2W16H32_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf,
    SINT32      shift1st,
    SINT32      shift2nd);

void Xin26xIDct2W16H64_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf,
    SINT32      shift1st,
    SINT32      shift2nd);

void Xin26xIDct2W64H16_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf,
    SINT32      shift1st,
    SINT32      shift2nd);

void Xin26xIDct2W64H32_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf,
    SINT32      shift1st,
    SINT32      shift2nd);

void Xin26xIDct2W32H64_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf,
    SINT32      shift1st,
    SINT32      shift2nd);

void Xin26xISkipWxH (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height);

#endif
