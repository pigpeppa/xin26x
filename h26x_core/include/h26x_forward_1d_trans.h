/***************************************************************************//**
 *
 * @file          h26x_forward_1d_trans.h
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
#ifndef _h26x_forward_1d_trans_h_
#define _h26x_forward_1d_trans_h_

typedef void (*Xin26xFdct) (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      shift);

void Xin26xFDct2P2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      shift);

void Xin26xFDct2P4 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      shift);

void Xin26xFDct2P8 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      shift);

void Xin26xFDct2P16 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      shift);

void Xin26xFDct2P32 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      shift);

void Xin26xFDct2P64 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      shift);

void Xin26xFDst7P4 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      shift);

void Xin26xFDst7P8 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      shift);

void Xin26xFDst7P16 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      shift);

void Xin26xFDst7P32 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      shift);

void Xin26xFDct8P4 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      shift);

void Xin26xFDct8P8 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      shift);

void Xin26xFDct8P16 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      shift);

void Xin26xFDct8P32 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      shift);

void Xin26xFDct2P16_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      shift);

void Xin26xFDct2P8_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      shift);

void Xin26xFDst7P16_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      shift);

void Xin26xFDst7P8_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      shift);

void Xin26xFDst7P4_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      shift);

#endif

