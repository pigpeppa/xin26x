/***************************************************************************//**
 *
 * @file          h26x_extend_picture.h
 * @brief         Subroutines related to extending frame boundaries.
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
#ifndef _h26x_extend_picture_h_
#define _h26x_extend_picture_h_

void Xin26xExtendPicture (
    PIXEL    *buf,
    intptr_t bufStride,
    UINT32   pictureWidth,
    UINT32   pictureHeight,
    UINT32   paddingWidth,
    UINT32   paddingHeight);

void Xin26xExtendLowerFrame (
    PIXEL       *output,
    intptr_t    outputStride,
    UINT32      inputWidth,
    UINT32      inputHeight,
    UINT32      outputWidth,
    UINT32      outputHeight);

#endif