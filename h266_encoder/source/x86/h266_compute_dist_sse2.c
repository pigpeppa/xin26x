/***************************************************************************//**
 *
 * @file          h266_compute_dist_sse2.c
 * @brief         Compute SAD or SSE (SSE2).
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
#include "emmintrin.h"
#include "xin_typedef.h"
#include "xin_video_common.h"
#include "h26x_definition.h"
#include "h266_definition.h"
#include "h26x_compute_dist.h"

void Xin266ComputeSsdFd16xH_SSE2 (
    COEFF    *tCoef,
    intptr_t tCoefStride,
    COEFF    *rCoef,
    intptr_t rCoefStride,
    UINT32   tuLgSize,
    UINT32   width,
    UINT32   height,
    UINT64   *ssd)
{
    UINT64 localSsd[2];
    UINT32 shift;
    UINT64 offset;

    localSsd[0] = 0;
    localSsd[1] = 0;

    Xin26xComputeSsdFdGt8xH_SSE2 (
        tCoef,
        tCoefStride,
        rCoef,
        rCoefStride,
        width,
        height,
        localSsd);

    shift  = 14 - tuLgSize;
    offset = (UINT64)1 << (shift - 1);    
    ssd[0] = (localSsd[0] + offset) >> shift;
    ssd[1] = (localSsd[1] + offset) >> shift;

}

void Xin266ComputeSsdFd8xH_SSE2 (
    COEFF    *tCoef,
    intptr_t tCoefStride,
    COEFF    *rCoef,
    intptr_t rCoefStride,
    UINT32   tuLgSize,
    UINT32   width,
    UINT32   height,
    UINT64   *ssd)
{
    UINT64 localSsd[2];
    UINT32 shift;
    UINT64 offset;

    localSsd[0] = 0;
    localSsd[1] = 0;

    Xin26xComputeSsdFd8xH_SSE2 (
        tCoef,
        tCoefStride,
        rCoef,
        rCoefStride,
        width,
        height,
        localSsd);

    shift  = 14 - tuLgSize;
    offset = (UINT64)1 << (shift - 1);    
    ssd[0] = (localSsd[0] + offset) >> shift;
    ssd[1] = (localSsd[1] + offset) >> shift;

}

void Xin266ComputeSsdFd4xH_SSE2 (
    COEFF    *tCoef,
    intptr_t tCoefStride,
    COEFF    *rCoef,
    intptr_t rCoefStride,
    UINT32   tuLgSize,
    UINT32   width,
    UINT32   height,
    UINT64   *ssd)
{
    UINT64 localSsd[2];
    UINT32 shift;
    UINT64 offset;

    localSsd[0] = 0;
    localSsd[1] = 0;

    Xin26xComputeSsdFd4xH_SSE2 (
        tCoef,
        tCoefStride,
        rCoef,
        rCoefStride,
        width,
        height,
        localSsd);

    shift  = 14 - tuLgSize;
    offset = (UINT64)1 << (shift - 1);    
    ssd[0] = (localSsd[0] + offset) >> shift;
    ssd[1] = (localSsd[1] + offset) >> shift;

}



