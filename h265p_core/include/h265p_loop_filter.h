/***************************************************************************//**
 *
 * @file          h265p_loop_filter.h
 * @brief         This file declares av1 loop filter subroutines.
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
#ifndef _h265p_loop_filter_h_
#define _h265p_loop_filter_h_

void Xin265pLpfVert4 (
    PIXEL    *src,
    intptr_t srcStride,
    UINT8    *blimit,
    UINT8    *limit,
    UINT8    *thresh);

void Xin265pLpfVert6 (
    PIXEL    *src,
    intptr_t srcStride,
    UINT8   *blimit,
    UINT8   *limit,
    UINT8   *thresh);

void Xin265pLpfVert8 (
    PIXEL    *src,
    intptr_t srcStride,
    UINT8    *blimit,
    UINT8    *limit,
    UINT8    *thresh);

void Xin265pLpfVert14 (
    PIXEL    *src,
    intptr_t srcStride,
    UINT8    *blimit,
    UINT8    *limit,
    UINT8    *thresh);

void Xin265pLpfHorz4 (
    PIXEL    *src,
    intptr_t srcStride,
    UINT8    *blimit,
    UINT8    *limit,
    UINT8    *thresh);

void Xin265pLpfHorz6 (
    PIXEL    *src,
    intptr_t srcStride,
    UINT8    *blimit,
    UINT8    *limit,
    UINT8    *thresh);

void Xin265pLpfHorz8 (
    PIXEL    *src,
    intptr_t srcStride,
    UINT8    *blimit,
    UINT8    *limit,
    UINT8    *thresh);

void Xin265pLpfHorz14 (
    PIXEL    *src,
    intptr_t srcStride,
    UINT8    *blimit,
    UINT8    *limit,
    UINT8    *thresh);

#endif

