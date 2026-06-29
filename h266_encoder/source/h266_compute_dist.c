/***************************************************************************//**
 *
 * @file          h266_compute_dist.c
 * @brief         Compute SAD or SSE.
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
#include "xin_typedef.h"
#include "xin_video_common.h"
#include "h26x_definition.h"
#include "h266_definition.h"
#include "h26x_compute_dist.h"

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
    UINT64          *ssdUv)
{
    UINT64 ySsd[2];
    UINT64 uSsd[2];
    UINT64 vSsd[2];
    SINT32 shift;
    UINT64 offset;

    ssdY[0]  = 0;
    ssdY[1]  = 0;
    ssdUv[0] = 0;
    ssdUv[1] = 0;

    if (planeMask & XIN_LUMA_MASK)
    {
        Xin26xComputeSsdFd (
            tCoef[PLANE_LUMA],
            tCoefYStride,
            rCoef[PLANE_LUMA],
            rCoefYStride,
            width,
            height,
            ySsd);

#ifdef ENABLE_10BIT_ENCODER
        shift  = 10 - tuLgYSize;
#else
        shift  = 14 - tuLgYSize;
#endif
        if (shift > 0)
        {
            offset  = (UINT64)1 << (shift - 1);
            ySsd[0] = (ySsd[0] + offset) >> shift;
            ySsd[1] = (ySsd[1] + offset) >> shift;
        }
        else
        {
            ySsd[0] = ySsd[0] << -shift;
            ySsd[1] = ySsd[1] << -shift;
        }

        ssdY[0] = ySsd[0];
        ssdY[1] = ySsd[1];

    }

    if (planeMask & XIN_CHROMA_MASK)
    {
        Xin26xComputeSsdFd (
            tCoef[PLANE_CHROMA_U],
            tCoefUvStride,
            rCoef[PLANE_CHROMA_U],
            rCoefUvStride,
            width/2,
            height/2,
            uSsd);

        Xin26xComputeSsdFd (
            tCoef[PLANE_CHROMA_V],
            tCoefUvStride,
            rCoef[PLANE_CHROMA_V],
            rCoefUvStride,
            width/2,
            height/2,
            vSsd);

#ifdef ENABLE_10BIT_ENCODER
        shift  = 10 - tuLgUvSize;
#else
        shift  = 14 - tuLgUvSize;
#endif
        if (shift > 0)
        {
            offset  = (UINT64)1 << (shift - 1);
            uSsd[0] = (uSsd[0] + offset) >> shift;
            uSsd[1] = (uSsd[1] + offset) >> shift;
            vSsd[0] = (vSsd[0] + offset) >> shift;
            vSsd[1] = (vSsd[1] + offset) >> shift;
        }
        else
        {
            uSsd[0] = uSsd[0] << -shift;
            uSsd[1] = uSsd[1] << -shift;
            vSsd[0] = vSsd[0] << -shift;
            vSsd[1] = vSsd[1] << -shift;
        }

        ssdUv[0] = uSsd[0] + vSsd[0];
        ssdUv[1] = uSsd[1] + vSsd[1];

    }

}

void Xin266ComputeSsdFd (
    COEFF           *tCoef,
    intptr_t        tCoefStride,
    COEFF           *rCoef,
    intptr_t        rCoefStride,
    UINT32          tuLgSize,
    UINT32          width,
    UINT32          height,
    UINT64          *ssd)
{
    UINT64 localSsd[2];
    SINT32 shift;
    UINT64 offset;

    localSsd[0] = 0;
    localSsd[1] = 0;

    Xin26xComputeSsdFd (
        tCoef,
        tCoefStride,
        rCoef,
        rCoefStride,
        width,
        height,
        localSsd);

#ifdef ENABLE_10BIT_ENCODER
    shift  = 10 - tuLgSize;
#else
    shift  = 14 - tuLgSize;
#endif

    if (shift > 0)
    {
        offset = (UINT64)1 << (shift - 1);
        ssd[0] = (localSsd[0] + offset) >> shift;
        ssd[1] = (localSsd[1] + offset) >> shift;
    }
    else
    {
        ssd[0] = localSsd[0] << -shift;
        ssd[1] = localSsd[1] << -shift;
    }

}

