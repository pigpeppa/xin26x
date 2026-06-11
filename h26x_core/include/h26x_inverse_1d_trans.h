/***************************************************************************//**
 *
 * @file          h26x_inverse_1d_trans.h
 * @brief         Inverse transform subroutines declaration and definitions.
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
#ifndef _h26x_inverse_1d_trans_h_
#define _h26x_inverse_1d_trans_h_

typedef void (*Xin266Idct) (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      skipLine,
    SINT32      shift);

void Xin26xIDct2P2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      skipLine,
    SINT32      shift);

void Xin26xIDct2P4 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      skipLine,
    SINT32      shift);

void Xin26xIDct2P8 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      skipLine,
    SINT32      shift);

void Xin26xIDct2P8_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    SINT16      *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      skipLine,
    SINT32      shift);

void Xin26xIDct2P16 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      skipLine,
    SINT32      shift);

void Xin26xIDct2P16_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    SINT16      *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      skipLine,
    SINT32      shift);

void Xin26xIDct2P32 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      skipLine,
    SINT32      shift);

void Xin26xIDct2VP32_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    SINT16      *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      skipLine,
    SINT32      shift);

void Xin26xIDct2P64 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      skipLine,
    SINT32      shift);

void Xin26xIDct2P64_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    SINT16      *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      skipLine,
    SINT32      shift);

void Xin26xIDst7P4 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      skipLine,
    SINT32      shift);

void Xin26xIDst7P8 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      skipLine,
    SINT32      shift);

void Xin26xIDst7P8_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      skipLine,
    SINT32      shift);

void Xin26xIDst7P16 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      skipLine,
    SINT32      shift);

void Xin26xIDst7P16_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      skipLine,
    SINT32      shift);

void Xin26xIDst7P32 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      skipLine,
    SINT32      shift);

void Xin26xIDct8P4 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      skipLine,
    SINT32      shift);

void Xin26xIDct8P8 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      skipLine,
    SINT32      shift);

void Xin26xIDct8P16 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      skipLine,
    SINT32      shift);

void Xin26xIDct8P32 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    SINT32      line,
    SINT32      skipLine,
    SINT32      shift);

#endif

