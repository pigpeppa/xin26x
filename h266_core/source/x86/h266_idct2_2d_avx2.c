/***************************************************************************//**
 *
 * @file          h266_idct2_2d_avx2.c
 * @brief         h.266 inverse transform (AVX2).
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
#include "immintrin.h"
#include "xin_typedef.h"
#include "h26x_trans_context.h"
#include "basic_macro.h"
#include "memory.h"
#include "h26x_inverse_1d_trans.h"
#ifdef __linux__
#include "xin_avx2_linux_patch.h"
#endif

#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif

void Xin266IDct2W8H8_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf)
{
    (void)height;
    (void)width;

    Xin26xIDct2P8_AVX2 (
        input,
        inputStride,
        tempBuf,
        8,
        8,
        0,
        7);

    Xin26xIDct2P8_AVX2 (
        tempBuf,
        8,
        output,
        outputStride,
        8,
        0,
        12);

}

void Xin266IDct2W16H16_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf)
{
    (void)height;
    (void)width;

    Xin26xIDct2P16_AVX2 (
        input,
        inputStride,
        tempBuf,
        16,
        16,
        0,
        7);

    Xin26xIDct2P16_AVX2 (
        tempBuf,
        16,
        output,
        outputStride,
        16,
        0,
        12);

}

void Xin266IDct2W32H32_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf)
{

    (void)height;
    (void)width;

    Xin26xIDct2P32_AVX2 (
        input,
        inputStride,
        tempBuf,
        32,
        32,
        0,
        7);

    Xin26xIDct2P32_AVX2 (
        tempBuf,
        32,
        output,
        outputStride,
        32,
        0,
        12);

}

void Xin266IDct2W16H8_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf)
{
    (void)height;
    (void)width;

    Xin26xIDct2P8_AVX2 (
        input,
        inputStride,
        tempBuf,
        8,
        16,
        0,
        7);

    Xin26xIDct2P16_AVX2 (
        tempBuf,
        8,
        output,
        outputStride,
        16,
        0,
        12);

}

void Xin266IDct2W8H16_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf)
{
    (void)height;
    (void)width;

    Xin26xIDct2P16_AVX2 (
        input,
        inputStride,
        tempBuf,
        16,
        8,
        0,
        7);

    Xin26xIDct2P8_AVX2 (
        tempBuf,
        16,
        output,
        outputStride,
        16,
        0,
        12);

}

void Xin266IDct2W32H16_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf)
{
    (void)height;
    (void)width;

    Xin26xIDct2P16_AVX2 (
        input,
        inputStride,
        tempBuf,
        16,
        32,
        0,
        7);

    Xin26xIDct2P32_AVX2 (
        tempBuf,
        16,
        output,
        outputStride,
        16,
        0,
        12);

}

void Xin266IDct2W16H32_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf)
{
    (void)height;
    (void)width;

    Xin26xIDct2P32_AVX2 (
        input,
        inputStride,
        tempBuf,
        32,
        16,
        0,
        7);

    Xin26xIDct2P16_AVX2 (
        tempBuf,
        32,
        output,
        outputStride,
        32,
        0,
        12);

}

void Xin266IDct2W64H64_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf)
{
    (void)height;
    (void)width;

    Xin26xIDct2P64_AVX2 (
        input,
        inputStride,
        tempBuf,
        64,
        64,
        32,
        7);

    Xin26xIDct2P64_AVX2 (
        tempBuf,
        64,
        output,
        outputStride,
        64,
        0,
        12);

}

void Xin266IDct2W16H64_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf)
{
    (void)height;
    (void)width;

    Xin26xIDct2P64_AVX2 (
        input,
        inputStride,
        tempBuf,
        64,
        16,
        0,
        7);

    Xin26xIDct2P16_AVX2 (
        tempBuf,
        64,
        output,
        outputStride,
        64,
        0,
        12);

}

void Xin266IDct2W64H16_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf)
{
    (void)height;
    (void)width;

    Xin26xIDct2P16_AVX2 (
        input,
        inputStride,
        tempBuf,
        16,
        64,
        32,
        7);

    Xin26xIDct2P64_AVX2 (
        tempBuf,
        16,
        output,
        outputStride,
        16,
        0,
        12);

}

void Xin266IDct2W64H32_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf)
{    
    (void)height;
    (void)width;

    Xin26xIDct2P32_AVX2 (
        input,
        inputStride,
        tempBuf,
        32,
        64,
        32,
        7);

    Xin26xIDct2P64_AVX2 (
        tempBuf,
        32,
        output,
        outputStride,
        32,
        0,
        12);

}

void Xin266IDct2W32H64_AVX2 (
    SINT16      *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf)
{
    Xin26xIDct2P64_AVX2 (
        input,
        inputStride,
        tempBuf,
        64,
        32,
        0,
        7);

    Xin26xIDct2P32_AVX2 (
        tempBuf,
        64,
        output,
        outputStride,
        64,
        0,
        12);

}




