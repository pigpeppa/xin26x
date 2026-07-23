/***************************************************************************//**
 *
 * @file          h266_compute_dist.h
 * @brief         This file declares SAD or SSE computation subroutines.
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
#ifndef _h266_compute_dist_h_
#define _h266_compute_dist_h_

void Xin266ComputeSsdFdYuv (
    COEFF           *tCoef[PLANE_NUM],
    intptr_t        tCoefYStride,
    intptr_t        tCoefUvStride,
    COEFF           *rCoef[PLANE_NUM],
    intptr_t        rCoefYStride,
    intptr_t        rCoefUvStride,
    UINT32          tuLgYSize,
    UINT32          tuLgUvSize,
    UINT32          width,
    UINT32          height,
    UINT32          planeMask,
    UINT64          *ssdY,
    UINT64          *ssdUv);

void Xin266ComputeSsdFdYuv8xH_SSE2 (
    COEFF           *tCoef[PLANE_NUM],
    intptr_t        tCoefYStride,
    intptr_t        tCoefUvStride,
    COEFF           *rCoef[PLANE_NUM],
    intptr_t        rCoefYStride,
    intptr_t        rCoefUvStride,
    UINT32          tuLgYSize,
    UINT32          tuLgUvSize,
    UINT32          width,
    UINT32          height,
    UINT32          planeMask,
    UINT64          *ssdY,
    UINT64          *ssdUv);

void Xin266ComputeSsdFdYuv16xH_SSE2 (
    COEFF           *tCoef[PLANE_NUM],
    intptr_t        tCoefYStride,
    intptr_t        tCoefUvStride,
    COEFF           *rCoef[PLANE_NUM],
    intptr_t        rCoefYStride,
    intptr_t        rCoefUvStride,
    UINT32          tuLgYSize,
    UINT32          tuLgUvSize,
    UINT32          width,
    UINT32          height,
    UINT32          planeMask,
    UINT64          *ssdY,
    UINT64          *ssdUv);

void Xin266ComputeSsdFdYuvGt16xH_SSE2 (
    COEFF           *tCoef[PLANE_NUM],
    intptr_t        tCoefYStride,
    intptr_t        tCoefUvStride,
    COEFF           *rCoef[PLANE_NUM],
    intptr_t        rCoefYStride,
    intptr_t        rCoefUvStride,
    UINT32          tuLgYSize,
    UINT32          tuLgUvSize,
    UINT32          width,
    UINT32          height,
    UINT32          planeMask,
    UINT64          *ssdY,
    UINT64          *ssdUv);

void Xin266ComputeSsdFd (
    COEFF           *tCoef,
    intptr_t        tCoefStride,
    COEFF           *rCoef,
    intptr_t        rCoefStride,
    UINT32          tuLgSize,
    UINT32          width,
    UINT32          height,
    UINT64          *ssd);

void Xin266ComputeSsdFd4xH_SSE2 (
    COEFF           *tCoef,
    intptr_t        tCoefStride,
    COEFF           *rCoef,
    intptr_t        rCoefStride,
    UINT32          tuLgSize,
    UINT32          width,
    UINT32          height,
    UINT64          *ssd);

void Xin266ComputeSsdFd8xH_SSE2 (
    COEFF           *tCoef,
    intptr_t        tCoefStride,
    COEFF           *rCoef,
    intptr_t        rCoefStride,
    UINT32          tuLgSize,
    UINT32          width,
    UINT32          height,
    UINT64          *ssd);

void Xin266ComputeSsdFd16xH_SSE2 (
    COEFF           *tCoef,
    intptr_t        tCoefStride,
    COEFF           *rCoef,
    intptr_t        rCoefStride,
    UINT32          tuLgSize,
    UINT32          width,
    UINT32          height,
    UINT64          *ssd);

void Xin266ComputeSsdFd32xH_AVX2 (
    COEFF           *tCoef,
    intptr_t        tCoefStride,
    COEFF           *rCoef,
    intptr_t        rCoefStride,
    UINT32          tuLgSize,
    UINT32          width,
    UINT32          height,
    UINT64          *ssd);

void Xin266ComputeSsdFd64xH_AVX2 (
    COEFF           *tCoef,
    intptr_t        tCoefStride,
    COEFF           *rCoef,
    intptr_t        rCoefStride,
    UINT32          tuLgSize,
    UINT32          width,
    UINT32          height,
    UINT64          *ssd);

void Xin266ComputeSsdFd128xH_AVX2 (
    COEFF           *tCoef,
    intptr_t        tCoefStride,
    COEFF           *rCoef,
    intptr_t        rCoefStride,
    UINT32          tuLgSize,
    UINT32          width,
    UINT32          height,
    UINT64          *ssd);

#endif

