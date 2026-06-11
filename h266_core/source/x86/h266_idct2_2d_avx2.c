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

static void TransposeS16Block_AVX2(
    SINT16*       src,
    intptr_t      srcStride,
    unsigned int  width,
    unsigned int  height,
    SINT16*       dst,
    intptr_t      dstStride)
{
    const intptr_t stride8 = srcStride * 8;
    const intptr_t rewind  = 8 - 7 * srcStride;

    for (unsigned int x = 0; x < width; x += 16)
    {
        SINT16* srcCol = src + x * srcStride;
        SINT16* dstCol = dst + x;

        for (unsigned int y = 0; y < height; y += 8)
        {
            // =========================
            // load 8 rows (each split low/high)
            // =========================
            __m256i r0 = _mm256_loadu2_m128i((__m128i*)(srcCol + stride8), (__m128i*)srcCol); srcCol += srcStride;
            __m256i r1 = _mm256_loadu2_m128i((__m128i*)(srcCol + stride8), (__m128i*)srcCol); srcCol += srcStride;
            __m256i r2 = _mm256_loadu2_m128i((__m128i*)(srcCol + stride8), (__m128i*)srcCol); srcCol += srcStride;
            __m256i r3 = _mm256_loadu2_m128i((__m128i*)(srcCol + stride8), (__m128i*)srcCol); srcCol += srcStride;
            __m256i r4 = _mm256_loadu2_m128i((__m128i*)(srcCol + stride8), (__m128i*)srcCol); srcCol += srcStride;
            __m256i r5 = _mm256_loadu2_m128i((__m128i*)(srcCol + stride8), (__m128i*)srcCol); srcCol += srcStride;
            __m256i r6 = _mm256_loadu2_m128i((__m128i*)(srcCol + stride8), (__m128i*)srcCol); srcCol += srcStride;
            __m256i r7 = _mm256_loadu2_m128i((__m128i*)(srcCol + stride8), (__m128i*)srcCol); srcCol += rewind;

            // =========================
            // stage 1: unpack 16
            // =========================
            __m256i t0 = _mm256_unpacklo_epi16(r0, r1);
            __m256i t1 = _mm256_unpackhi_epi16(r0, r1);
            __m256i t2 = _mm256_unpacklo_epi16(r2, r3);
            __m256i t3 = _mm256_unpackhi_epi16(r2, r3);
            __m256i t4 = _mm256_unpacklo_epi16(r4, r5);
            __m256i t5 = _mm256_unpackhi_epi16(r4, r5);
            __m256i t6 = _mm256_unpacklo_epi16(r6, r7);
            __m256i t7 = _mm256_unpackhi_epi16(r6, r7);

            // =========================
            // stage 2: unpack 32
            // =========================
            __m256i u0 = _mm256_unpacklo_epi32(t0, t2);
            __m256i u1 = _mm256_unpackhi_epi32(t0, t2);
            __m256i u2 = _mm256_unpacklo_epi32(t1, t3);
            __m256i u3 = _mm256_unpackhi_epi32(t1, t3);
            __m256i u4 = _mm256_unpacklo_epi32(t4, t6);
            __m256i u5 = _mm256_unpackhi_epi32(t4, t6);
            __m256i u6 = _mm256_unpacklo_epi32(t5, t7);
            __m256i u7 = _mm256_unpackhi_epi32(t5, t7);

            // =========================
            // stage 3: unpack 64 (final transpose)
            // =========================
            __m256i o0 = _mm256_unpacklo_epi64(u0, u4);
            __m256i o1 = _mm256_unpackhi_epi64(u0, u4);
            __m256i o2 = _mm256_unpacklo_epi64(u1, u5);
            __m256i o3 = _mm256_unpackhi_epi64(u1, u5);
            __m256i o4 = _mm256_unpacklo_epi64(u2, u6);
            __m256i o5 = _mm256_unpackhi_epi64(u2, u6);
            __m256i o6 = _mm256_unpacklo_epi64(u3, u7);
            __m256i o7 = _mm256_unpackhi_epi64(u3, u7);

            // =========================
            // store
            // =========================
            _mm256_storeu_si256((__m256i*)(dstCol + 0 * dstStride), o0);
            _mm256_storeu_si256((__m256i*)(dstCol + 1 * dstStride), o1);
            _mm256_storeu_si256((__m256i*)(dstCol + 2 * dstStride), o2);
            _mm256_storeu_si256((__m256i*)(dstCol + 3 * dstStride), o3);
            _mm256_storeu_si256((__m256i*)(dstCol + 4 * dstStride), o4);
            _mm256_storeu_si256((__m256i*)(dstCol + 5 * dstStride), o5);
            _mm256_storeu_si256((__m256i*)(dstCol + 6 * dstStride), o6);
            _mm256_storeu_si256((__m256i*)(dstCol + 7 * dstStride), o7);

            dstCol += 8 * dstStride;
        }
    }
}

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

    Xin26xIDct2VP32_AVX2 (
        input,
        inputStride,
        tempBuf,
        32,
        32,
        0,
        7);

    // Inverse transform horizontally.
    TransposeS16Block_AVX2 (
        tempBuf,
        32,
        32,
        32,
        output,
        outputStride);

    Xin26xIDct2VP32_AVX2 (
        output,
        outputStride,
        tempBuf,
        32,
        32,
        0,
        12);

    TransposeS16Block_AVX2 (
        tempBuf,
        32,
        32,
        32,
        output,
        outputStride);

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

    COEFF *tempBuf0;
    COEFF *tempBuf1;

    tempBuf0 = tempBuf;
    tempBuf1 = tempBuf + 16*32;

    Xin26xIDct2P16_AVX2 (
        input,
        inputStride,
        tempBuf0,
        16,
        32,
        0,
        7);

    Xin26xIDct2VP32_AVX2 (
        tempBuf0,
        16,
        tempBuf1,
        16,
        16,
        0,
        12);

    TransposeS16Block_AVX2 (
        tempBuf1,
        16,
        32,
        16,
        output,
        outputStride);

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

    COEFF *tempBuf0;
    COEFF *tempBuf1;

    tempBuf0 = tempBuf;
    tempBuf1 = tempBuf + 16*32;

    Xin26xIDct2VP32_AVX2 (
        input,
        inputStride,
        tempBuf0,
        16,
        16,
        0,
        7);

    TransposeS16Block_AVX2 (
        tempBuf0,
        16,
        32,
        16,
        tempBuf1,
        32);

    Xin26xIDct2P16_AVX2 (
        tempBuf1,
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
    COEFF *tempBuf0;
    COEFF *tempBuf1;
    
    (void)height;
    (void)width;

    tempBuf0 = tempBuf;
    tempBuf1 = tempBuf + 64*32;

    Xin26xIDct2VP32_AVX2 (
        input,
        inputStride,
        tempBuf0,
        64,
        64,
        32,
        7);

    TransposeS16Block_AVX2 (
        tempBuf0,
        64,
        32,
        64,
        tempBuf1,
        32);

    Xin26xIDct2P64_AVX2 (
        tempBuf1,
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
    COEFF *tempBuf0;
    COEFF *tempBuf1;
    
    (void)height;
    (void)width;

    tempBuf0 = tempBuf;
    tempBuf1 = tempBuf + 32*64;

    Xin26xIDct2P64_AVX2 (
        input,
        inputStride,
        tempBuf0,
        64,
        32,
        0,
        7);

    Xin26xIDct2VP32_AVX2 (
        tempBuf0,
        64,
        tempBuf1,
        64,
        64,
        0,
        12);

    TransposeS16Block_AVX2 (
        tempBuf1,
        64,
        32,
        64,
        output,
        outputStride);

}




