/***************************************************************************//**
 *
 * @file          h265p_intra_prediction.h
 * @brief         This file declares av1 intra prediction subroutines.
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
#ifndef _h265p_intra_prediction_h_
#define _h265p_intra_prediction_h_

void Xin265pIntraPred (
    xin_sec_struct *secSet,
    UINT32         planeIdx,
    PIXEL          *pred,
    intptr_t       predStride,
    SINT32         mode,
    SINT32         deltaAngle);

BOOL Xin265pFilterIntraAllowed (
    BOOL    enableFilterIntra,
    UINT32  blockSize);

void Xin265pGetIntraAvail (
    xin_mb_struct  *mb,
    UINT32         lgMiSize,
    intptr_t       miStride);

void Xin265pExtractIntraNBChroma (
    xin_sec_struct  *secSet,
    xin_mb_struct   *mb);

void Xin265pExtractIntraNBLuma (
    xin_sec_struct  *secSet,
    xin_mb_struct   *mb);


#endif

