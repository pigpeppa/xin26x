/***************************************************************************//**
 *
 * @file          h26x_picture_copy.h
 * @brief         This file defines picture copy subroutines.
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
#ifndef _h26x_picture_copy_h_
#define _h26x_picture_copy_h_

void Xin26xPictureCopy (
    PIXEL    *src,
    intptr_t srcStride,
    PIXEL    *dst,
    intptr_t dstStride,
    UINT32   width,
    UINT32   height);

void Xin26xPictureScaleCopy (
    void     *src,
    intptr_t srcStride,
    void     *dst,
    intptr_t dstStride,
    UINT32   srcBitDepth,
    UINT32   dstBitDepth,
    UINT32   width,
    UINT32   height);

void Xin26xPictureCopy_SSE2 (
    UINT8    *src,
    intptr_t srcStride,
    UINT8    *dst,
    intptr_t dstStride,
    UINT32   width,
    UINT32   height);

void Xin26xPictureCopy_AVX2 (
    UINT8    *src,
    intptr_t srcStride,
    UINT8    *dst,
    intptr_t dstStride,
    UINT32   width,
    UINT32   height);



#endif

