/***************************************************************************//**
 *
 * @file          h266_alf_func_struct.h
 * @brief         This file contains h266 ALF SIMD function definitions.
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
#ifndef _h266_alf_func_struct_h_
#define _h266_alf_func_struct_h_

typedef void (*XinGetPreBlkStats) (
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

typedef FLOAT32 (*XinCalcCoeffError) (
    SINT32  *clip,
    SINT32  *coeff,
    FLOAT32 E[XIN_ALF_CLIP_NUM][XIN_ALF_CLIP_NUM][XIN_ALF_MAX_LUMA_COEF_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    FLOAT32 y[XIN_ALF_CLIP_NUM][XIN_ALF_MAX_LUMA_COEF_NUM],
    SINT32  numCoeff,
    SINT32  bitDepth);

typedef void (*XinAlfBlockLuma) (
    PIXEL         *src,
    intptr_t      srcStride,
    PIXEL         *dst,
    intptr_t      dstStride,
    SINT16        *filterSet,
    SINT16        *fClipSet,
    xin_alf_class *alfClass,
    intptr_t      alfClassStride,
    SINT32        width,
    SINT32        height,
    SINT32        vbPos,
    SINT32        vbCtuHeight);

typedef void (*XinAlfBlockChroma) (
    PIXEL         *src,
    intptr_t      srcStride,
    PIXEL         *dst,
    intptr_t      dstStride,
    SINT16        *filterSet,
    SINT16        *fClipSet,
    SINT32        width,
    SINT32        height,
    SINT32        vbPos,
    SINT32        vbCtuHeight);

typedef void (*XinAlfDeriveClass) (
    xin_alf_class *alfClass,
    intptr_t      classStride,
    PIXEL         *src,
    intptr_t      srcStride,
    SINT32        blockWidth,
    SINT32        blockHeight,
    SINT32        blockPosY,
    SINT32        vbCtuHeight,
    SINT32        vbPos,
    SINT32        shift);

typedef struct xin_alf_func
{
    XinGetPreBlkStats    pfXinGetPreBlkStats;
    XinCalcCoeffError    pfXinCalcCoeffError;
    XinAlfBlockLuma      pfXinAlfBlockLuma;
    XinAlfBlockChroma    pfXinAlfBlockChroma;
    XinAlfDeriveClass    pfXinAlfDeriveClass;
}xin_alf_func;

#endif
