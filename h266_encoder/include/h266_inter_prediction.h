/***************************************************************************//**
 *
 * @file          h266_inter_prediction.h
 * @brief         This file declares h.266 motion compensation subroutines.
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
#ifndef _h266_inter_prediction_h_
#define _h266_inter_prediction_h_

void Xin266LumaInterPred (
    xin_sec_struct *secSet,
    xin_pu_struct  *pu,
    PIXEL          *pred,
    intptr_t       predStride,
    xin_mv32_u     *mv,
    SINT32         refIdx,
    SINT32         listIdx,
    BOOL           useAltHpelIf);

void Xin266ChromaInterPred (
    xin_sec_struct *secSet,
    xin_pu_struct  *pu,
    PIXEL          *predU,
    PIXEL          *predV,
    intptr_t       predStride,
    xin_mv32_u     *mv,
    SINT32         refIdx,
    SINT32         listIdx);

void Xin266LumaInterpolation (
    xin_func_struct *funcSet,
    const PIXEL     *src,
    intptr_t        srcStride,
    PIXEL           *dst,
    intptr_t        dstStride,
    SINT32          frac,
    UINT32          lgWidth,
    UINT32          height);

void Xin266LumaMotionComp (
    xin_sec_struct *secSet,
    SINT32         width,
    SINT32         height,
    PIXEL          *pred,
    intptr_t       predStride,
    xin_mv32_u     *mv,
    SINT8          *refIdx,
    UINT8          bcwIdx,
    UINT32         filterIndex);

void Xin266ChromaMotionComp (
    xin_sec_struct *secSet,
    SINT32         width,
    SINT32         height,
    PIXEL          *predU,
    PIXEL          *predV,
    intptr_t       predStride,
    xin_mv32_u     *mv,
    SINT8          *refIdx,
    UINT8          bcwIdx);

void Xin266LumaMotionCompDmvr (
    xin_sec_struct *secSet,
    xin_pu_struct  *pu,
    PIXEL          *pred,
    intptr_t       predStride,
    xin_mv32_u     *mv,
    SINT8          *refIdx,
    xin_mv_s       *mvdL0SubPu,
    BOOL           useAltHpelIf);

void Xin266ChromaMotionCompDmvr (
    xin_sec_struct *secSet,
    xin_pu_struct  *pu,
    PIXEL          *predU,
    PIXEL          *predV,
    intptr_t       predStride,
    xin_mv32_u     *mv,
    SINT8          *refIdx,
    xin_mv_s       *mvdL0SubPu);

void Xin266LumaMotionCompSubBlock (
    xin_sec_struct   *secSet,
    xin_pu_struct    *pu,
    PIXEL            *pred,
    intptr_t         predStride,
    xin_mv32_u       subBlockMv[XIN_MAX_SUB_MV_SIZE][XIN_LIST_NUM],
    SINT8            refIdx[XIN_MAX_SUB_MV_SIZE][XIN_LIST_NUM],
    UINT8            bcwIdx);

void Xin266LumaMotionCompAffine (
    xin_sec_struct   *secSet,
    xin_pu_struct    *pu,
    PIXEL            *pred,
    intptr_t         predStride,
    SINT8            refIdx[XIN_LIST_NUM],
    xin_mv32_u       *affineMv[XIN_LIST_NUM],
    UINT8            bcwIdx);

void Xin266ChromaMotionCompSubBlock (
    xin_sec_struct   *secSet,
    xin_pu_struct    *pu,
    PIXEL            *predU,
    PIXEL            *predV,
    intptr_t         predStride,
    xin_mv32_u       subBlockMv[XIN_MAX_SUB_MV_SIZE][XIN_LIST_NUM],
    SINT8            refIdx[XIN_MAX_SUB_MV_SIZE][XIN_LIST_NUM],
    UINT8            bcwIdx);

void Xin266ChromaMotionCompAffine (
    xin_sec_struct   *secSet,
    xin_pu_struct    *pu,
    PIXEL            *predU,
    PIXEL            *predV,
    intptr_t         predStride,
    SINT8            refIdx[XIN_LIST_NUM],
    xin_mv32_u       *affineMv[XIN_LIST_NUM],
    UINT8            bcwIdx);


#endif

