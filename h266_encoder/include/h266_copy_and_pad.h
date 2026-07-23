/***************************************************************************//**
 *
 * @file          h266_copy_and_pad.h
 * @brief         h266 copy and padding generic subroutines.
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
#ifndef _h266_copy_and_pad_h_
#define _h266_copy_and_pad_h_

void Xin266CopyAndPad (
    PIXEL      *ref,
    intptr_t   refStride,
    PIXEL      *pad,
    intptr_t   padStride,
    UINT32     width,
    UINT32     height);

void Xin266CopyAndPad_AVX2 (
    PIXEL      *ref,
    intptr_t   refStride,
    PIXEL      *pad,
    intptr_t   padStride,
    UINT32     width,
    UINT32     height);

void Xin266CopyAndPadUv (
    PIXEL      *refU,
    PIXEL      *refV,
    intptr_t   refStride,
    PIXEL      *padU,
    PIXEL      *padV,
    intptr_t   padStride,
    UINT32     width,
    UINT32     height);

void Xin266CopyAndPadUv_SSE4 (
    PIXEL      *refU,
    PIXEL      *refV,
    intptr_t   refStride,
    PIXEL      *padU,
    PIXEL      *padV,
    intptr_t   padStride,
    UINT32     width,
    UINT32     height);

#endif