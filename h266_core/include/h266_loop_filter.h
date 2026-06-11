/***************************************************************************//**
 *
 * @file          h266_loop_filter.h
 * @brief         This file declares h266 loop filter subroutines.
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
#ifndef _h266_loop_filter_h_
#define _h266_loop_filter_h_

void Xin266LumaLoopFilter (
    PIXEL    *src,
    intptr_t srcStride,
    SINT32   tc[2],
    SINT32   beta[2],
    BOOL     sidePisLarge,
    BOOL     sideQisLarge,
    SINT32   maxFilterLengthP,
    SINT32   maxFilterLengthQ,
    BOOL     isVert);

void Xin266LumaLoopFilter_SSE4 (
    PIXEL    *src,
    intptr_t srcStride,
    SINT32   tc[2],
    SINT32   beta[2],
    BOOL     sidePisLarge,
    BOOL     sideQisLarge,
    SINT32   maxFilterLengthP,
    SINT32   maxFilterLengthQ,
    BOOL     isVert);

void Xin266ChromaLoopFilter (
    PIXEL    *src,
    intptr_t srcStride,
    SINT32   tc[2],
    SINT32   beta[2],
    BOOL     largeBoundary,
    BOOL     isChromaHorCtuBoundary,
    BOOL     isVert);

#endif

