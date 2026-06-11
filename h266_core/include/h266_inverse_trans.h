/***************************************************************************//**
 *
 * @file          h266_inverse_trans.h
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
#ifndef _h266_inverse_trans_h_
#define _h266_inverse_trans_h_

void Xin266IDct2WxH (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf,
    SINT32      shift1st,
    SINT32      shift2nd);

void Xin266IDct2W8H8_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf,
    SINT32      shift1st,
    SINT32      shift2nd);

void Xin266IDct2W16H16_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf,
    SINT32      shift1st,
    SINT32      shift2nd);

void Xin266IDct2W32H32_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf,
    SINT32      shift1st,
    SINT32      shift2nd);

void Xin266IDct2W16H8_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf,
    SINT32      shift1st,
    SINT32      shift2nd);

void Xin266IDct2W8H16_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf,
    SINT32      shift1st,
    SINT32      shift2nd);

void Xin266IDct2W32H16_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf,
    SINT32      shift1st,
    SINT32      shift2nd);

void Xin266IDct2W16H32_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf,
    SINT32      shift1st,
    SINT32      shift2nd);

void Xin266IDct2W64H64_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf,
    SINT32      shift1st,
    SINT32      shift2nd);

void Xin266IDct2W16H64_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf,
    SINT32      shift1st,
    SINT32      shift2nd);

void Xin266IDct2W64H16_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf,
    SINT32      shift1st,
    SINT32      shift2nd);

void Xin266IDct2W64H32_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf,
    SINT32      shift1st,
    SINT32      shift2nd);

void Xin266IDct2W32H64_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height,
    COEFF       *tempBuf,
    SINT32      shift1st,
    SINT32      shift2nd);

void Xin266IDctWxH (
    xin_sec_struct *secSet,
    COEFF          *input,
    intptr_t       inputStride,
    COEFF          *output,
    intptr_t       outputStride,
    UINT32         width,
    UINT32         height,
    BOOL           isIntra,
    UINT32         compId,
    UINT32         mtsIdx);

void Xin266ISkipWxH (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height);

void Xin266ISkip64xH_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height);

void Xin266ISkip32xH_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height);

void Xin266ISkip16xH_AVX2 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height);

void Xin266ISkip8xH_SSE4 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height);

void Xin266ISkip4xH_SSE4 (
    COEFF       *input,
    intptr_t    inputStride,
    COEFF       *output,
    intptr_t    outputStride,
    UINT32      width,
    UINT32      height);

#endif

