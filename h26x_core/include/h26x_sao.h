/***************************************************************************//**
 *
 * @file          h26x_sao.h
 * @brief         This file declares SAO subroutines.
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
#ifndef _h26x_sao_h_
#define _h26x_sao_h_

void Xin26xSaoBo (
    PIXEL    *src,
    intptr_t srcStride,
    SINT32   saoBandPos,
    SINT8    *offset,
    SINT32   width,
    SINT32   height,
    UINT32   bitDepth);

void Xin26xSaoEo0 (
    PIXEL    *src,
    intptr_t srcStride,
    PIXEL    *lftBuf,
    SINT8    *offset,
    UINT32   ctuAvail,
    SINT32   width,
    SINT32   height);

void Xin26xSaoEo0_SSE4 (
    PIXEL    *src,
    intptr_t srcStride,
    PIXEL    *lftBuf,
    SINT8    *offset,
    UINT32   ctuAvail,
    SINT32   width,
    SINT32   height);

void Xin26xSaoEo0_Neon (
    PIXEL    *src,
    intptr_t srcStride,
    PIXEL    *lftBuf,
    SINT8    *offset,
    UINT32   ctuAvail,
    SINT32   width,
    SINT32   height);

void Xin26xSaoEo90 (
    PIXEL    *src,
    intptr_t srcStride,
    PIXEL    *topBuf,
    SINT8    *offset,
    UINT32   ctuAvail,
    SINT32   width,
    SINT32   height);

void Xin26xSaoEo90_SSE4 (
    PIXEL    *src,
    intptr_t srcStride,
    PIXEL    *topBuf,
    SINT8    *offset,
    UINT32   ctuAvail,
    SINT32   width,
    SINT32   height);

void Xin26xSaoEo90_Neon (
    PIXEL    *src,
    intptr_t srcStride,
    PIXEL    *topBuf,
    SINT8    *offset,
    UINT32   ctuAvail,
    SINT32   width,
    SINT32   height);

void Xin26xSaoEo45 (
    PIXEL    *src,
    intptr_t srcStride,
    PIXEL    *lftBuf,
    PIXEL    *topBuf,
    SINT8    *offset,
    UINT32   ctuAvail,
    SINT32   width,
    SINT32   height);

void Xin26xSaoEo45_SSE4 (
    PIXEL    *src,
    intptr_t srcStride,
    PIXEL    *lftBuf,
    PIXEL    *topBuf,
    SINT8    *offset,
    UINT32   ctuAvail,
    SINT32   width,
    SINT32   height);

void Xin26xSaoEo45_Neon (
    PIXEL    *src,
    intptr_t srcStride,
    PIXEL    *lftBuf,
    PIXEL    *topBuf,
    SINT8    *offset,
    UINT32   ctuAvail,
    SINT32   width,
    SINT32   height);

void Xin26xSaoEo135 (
    PIXEL    *src,
    intptr_t srcStride,
    PIXEL    *lftBuf,
    PIXEL    *topBuf,
    SINT8    *offset,
    UINT32   ctuAvail,
    SINT32   width,
    SINT32   height);

void Xin26xSaoEo135_SSE4 (
    PIXEL    *src,
    intptr_t srcStride,
    PIXEL    *lftBuf,
    PIXEL    *topBuf,
    SINT8    *offset,
    UINT32   ctuAvail,
    SINT32   width,
    SINT32   height);

void Xin26xSaoEo135_Neon (
    PIXEL    *src,
    intptr_t srcStride,
    PIXEL    *lftBuf,
    PIXEL    *topBuf,
    SINT8    *offset,
    UINT32   ctuAvail,
    SINT32   width,
    SINT32   height);

void Xin26xSaoStatEo (
    SINT16    *diff,
    intptr_t  diffStride,
    PIXEL     *recon,
    intptr_t  reconStride,
    UINT32    eoType,
    UINT32    width,
    UINT32    height,
    SINT32    *diffSum,
    UINT16    *count);

void Xin26xSaoStatBo (
    SINT16    *diff,
    intptr_t  diffStride,
    PIXEL     *recon,
    intptr_t  reconStride,
    UINT32    width,
    UINT32    height,
    UINT32    bitDepth,
    SINT32    *diffSum,
    UINT16    *count);

void Xin26xSaoStatEo128xH_AVX2 (
    SINT16    *diff,
    intptr_t  diffStride,
    PIXEL     *recon,
    intptr_t  reconStride,
    UINT32    eoType,
    UINT32    width,
    UINT32    height,
    SINT32    *diffSum,
    UINT16    *count);


void Xin26xSaoStatEo64xH_AVX2 (
    SINT16    *diff,
    intptr_t  diffStride,
    PIXEL     *recon,
    intptr_t  reconStride,
    UINT32    eoType,
    UINT32    width,
    UINT32    height,
    SINT32    *diffSum,
    UINT16    *count);

void Xin26xSaoStatEo32xH_AVX2 (
    SINT16    *diff,
    intptr_t  diffStride,
    PIXEL     *recon,
    intptr_t  reconStride,
    UINT32    eoType,
    UINT32    width,
    UINT32    height,
    SINT32    *diffSum,
    UINT16    *count);

#endif

