/***************************************************************************//**
 *
 * @file          h26x_block_transpose.h
 * @brief         Declares block transpose subroutines.
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
#ifndef _h26x_block_transpose_h_
#define _h26x_block_transpose_h_

void Xin26xBlockTranspose (
    const PIXEL *input,
    intptr_t    inputStride,
    PIXEL       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height);

void Xin26xBlockTranspose_SSE2 (
    const PIXEL *input,
    intptr_t    inputStride,
    PIXEL       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height);

void Xin26xBlockTranspose_Neon (
    const PIXEL *input,
    intptr_t    inputStride,
    PIXEL       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height);

void Xin26xBlock16xNTransposeS16_Neon (
    SINT16          *input,
    intptr_t        inputStride,
    unsigned int    outputWidth,
    unsigned int    outputHeight,
    SINT16          *output,
    intptr_t        outputStride);

void Xin26xBlock8x8TransposeS16_Neon (
    SINT16          *input,
    intptr_t        inputStride,
    SINT16          *output,
    intptr_t        outputStride);

void Xin26xBlock16xHTransposeS16_AVX2 (
    SINT16          *input,
    intptr_t        inputStride,
    unsigned int    outputWidth,
    unsigned int    outputHeight,
    SINT16          *output,
    intptr_t        outputStride);

void Xin26xBlock8xHTransposeS16_AVX2 (
    SINT16          *input,
    intptr_t        inputStride,
    unsigned int    outputWidth,
    unsigned int    outputHeight,
    SINT16          *output,
    intptr_t        outputStride);

void Xin26xBlockTransposeS16 (
    SINT16          *input,
    intptr_t        inputStride,
    unsigned int    outputWidth,
    unsigned int    outputHeight,
    SINT16          *output,
    intptr_t        outputStride);


#endif

