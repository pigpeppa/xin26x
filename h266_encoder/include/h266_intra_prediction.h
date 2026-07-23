/***************************************************************************//**
 *
 * @file          h266_intra_prediction.h
 * @brief         This file declares h266 intra prediction subroutines.
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
#ifndef _h266_intra_prediction_h_
#define _h266_intra_prediction_h_

void Xin266GetBlockAvail (
    xin_sec_struct *secSet,
    xin_pu_struct  *pu);

void Xin266ExtractIntraNB (
    xin_sec_struct *secSet,
    xin_pu_struct  *pu,
    SINT32         multiRefIdx);

void Xin266GetIntraAvail (
    xin_sec_struct *secSet,
    xin_pu_struct  *pu,
    UINT32         lgBlockSize,
    intptr_t       blockStride);

void Xin266IntraPred (
    xin_sec_struct *secSet,
    PIXEL          *pred,
    intptr_t       predStride,
    SINT32         mode,
    SINT32         multiRefIdx,
    UINT32         lgWidth,
    UINT32         lgHeight);

void Xin266IntraPredChroma (
    xin_sec_struct *secSet,
    PIXEL           *pred,
    intptr_t        predStride,
    UINT32          compIdx,
    SINT32          mode,
    UINT32          lgWidth,
    UINT32          lgHeight);

void Xin266ExtractIntraNBChroma (
    xin_sec_struct *secSet,
    xin_pu_struct  *pu);

void Xin266LoadLMLumaRec (
    xin_sec_struct  *secSet,
    PIXEL           *dst,
    intptr_t        dstStride,
    SINT32          cclmMode);

void Xin266FilterIntraNB (
    PIXEL   *src,
    PIXEL   *dst,
    UINT32  width,
    UINT32  height,
    SINT32  multiRefIdx);

void Xin266FilterIntraNB_AVX2 (
    PIXEL   *src,
    PIXEL   *dst,
    UINT32  width,
    UINT32  height,
    SINT32  multiRefIdx);

void Xin266ApplyAngPDPC (
    xin_sec_struct  *secSet,
    xin_fast_md_buf *fastBuf,
    xin_cu_struct   *cu);

#endif

