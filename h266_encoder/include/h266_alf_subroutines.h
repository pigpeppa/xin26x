/***************************************************************************//**
 *
 * @file          h266_alf_subroutines.h
 * @brief         This file declares ALF-related subroutines.
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
#ifndef _h266_alf_subroutines_h_
#define _h266_alf_subroutines_h_

#include "h266_alf_context.h"

void Xin266GetPreBlkStats (
    xin_alf_cov    *alfCov,
    BOOL           useNonLinearAlf,
    xin_alf_filter *alfFilter,
    xin_alf_class  *alfClass,
    intptr_t       alfClassStride,
    PIXEL          *input,
    intptr_t       inputStride,
    PIXEL          *recon,
    intptr_t       reconStride,
    UINT32         planeIdx,
    SINT32         width,
    SINT32         height,
    SINT16         alfClippingValues[PLANE_TYPE][XIN_ALF_CLIP_NUM],
    SINT32         vbCtuHeight,
    SINT32         vbPos);

void Xin266GetPreBlkStats_AVX2 (
    xin_alf_cov    *alfCov,
    BOOL           useNonLinearAlf,
    xin_alf_filter *alfFilter,
    xin_alf_class  *alfClass,
    intptr_t       alfClassStride,
    PIXEL          *input,
    intptr_t       inputStride,
    PIXEL          *recon,
    intptr_t       reconStride,
    UINT32         planeIdx,
    SINT32         width,
    SINT32         height,
    SINT16         alfClippingValues[PLANE_TYPE][XIN_ALF_CLIP_NUM],
    SINT32         vbCtuHeight,
    SINT32         vbPos);

FLOAT32 Xin266CalcCoeffErrorLin_AVX2 (
    SINT32  *clip,
    SINT32  *coeff,
    FLOAT32 E[XIN_ALF_CLIP_NUM][XIN_ALF_CLIP_NUM][XIN_ALF_MAX_LUMA_COEF_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    FLOAT32 y[XIN_ALF_CLIP_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    SINT32  numCoeff,
    SINT32  bitDepth);

FLOAT32 Xin266CalcCoeffError (
    SINT32  *clip,
    SINT32  *coeff,
    FLOAT32 E[XIN_ALF_CLIP_NUM][XIN_ALF_CLIP_NUM][XIN_ALF_MAX_LUMA_COEF_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    FLOAT32 y[XIN_ALF_CLIP_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    SINT32  numCoeff,
    SINT32  bitDepth);

#endif

