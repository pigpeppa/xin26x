/***************************************************************************//**
 *
 * @file          h265p_compute_dist.c
 * @brief         Compute SAD or SSE in frequency domain.
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
#include "h26x_definition.h"
#include "h265p_definition.h"
#include "h265p_constant.h"
#include "h265p_trans_context.h"
#include "h265p_common_data.h"
#include "basic_macro.h"

void Xin265pComputeSsdFd (
    SINT32   *tCoeff,
    intptr_t tCoeffStride,
    SINT32   *rCoeff,
    intptr_t rCoeffStride,
    UINT32   width,
    UINT32   height,
    SINT32   txSize,
    UINT64   *ssd)
{
    UINT32  row;
    UINT32  col;
    UINT64  outSsd[2];
    SINT32  txShift;

    outSsd[0] = 0;
    outSsd[1] = 0;
    txShift   = (1 - txSize2LogScale[txSize])*2;

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col++)
        {
            outSsd[0] += (tCoeff[col] - rCoeff[col]) * (tCoeff[col] - rCoeff[col]);
            outSsd[1] += tCoeff[col] * tCoeff[col];
        }

        tCoeff += tCoeffStride;
        rCoeff += rCoeffStride;
    }

    ssd[0] = XIN_SIGNED_SHIFT(outSsd[0], txShift);
    ssd[1] = XIN_SIGNED_SHIFT(outSsd[1], txShift);

}

