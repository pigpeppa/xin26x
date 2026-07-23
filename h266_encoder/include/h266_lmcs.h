/***************************************************************************//**
 *
 * @file          h266_lmcs.h
 * @brief         This file declares functions related to LMCS.
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
#ifndef _h266_lmcs_h_
#define _h266_lmcs_h_

SINT32 Xin266CreateLmcs (
    xin_pic_struct *picSet);

void Xin266LmcsPreAnalyzer (
    xin_pic_struct *picSet,
    UINT32         signalType);

void Xin266ConstructReshaperLmcs (
    xin_lmcs_struct *lmcsSet);

void Xin266CalcChromaAdjVpdu (
    xin_sec_struct *secSet,
    SINT32         *ouputScale);

void Xin266ReshapeSignal (
    PIXEL    *input,
    intptr_t inputStride,
    PIXEL    *output,
    intptr_t outputStride,
    SINT32   width,
    SINT32   height,
    PIXEL    *fwdLUT);

void Xin266InvReshapeSignal (
    xin_pic_struct *picSet,
    xin_ctu_struct *ctu);

void Xin266FwdScaleSignal (
    SINT16   *residual,
    intptr_t residualStride,
    SINT32   width,
    SINT32   height,
    SINT32   chromaAdj);

void Xin266InvScaleSignal (
    SINT16   *residual,
    intptr_t residualStride,
    SINT32   width,
    SINT32   height,
    SINT32   chromaAdj);

#endif

