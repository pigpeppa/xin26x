/***************************************************************************//**
 *
 * @file          h266_alf.h
 * @brief         This file contains ALF-related definitions.
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
#ifndef _h266_alf_h_
#define _h266_alf_h_

void Xin266AlfDeriveClass (
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

void Xin266AlfDeriveClass_AVX2 (
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

void Xin266AlfBlockLuma (
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

void Xin266AlfBlockLuma_AVX2 (
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

void Xin266AlfBlockChroma (
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

void Xin266AlfBlockChroma_AVX2 (
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

void Xin266FilterBlockCcAlf (
    PIXEL         *luma,
    intptr_t      lumaStride,
    PIXEL         *chroma,
    intptr_t      chromaStride,
    SINT16        *filterSet,
    SINT32        width,
    SINT32        height,
    SINT32        vbPos,
    SINT32        vbCtuHeight);


#endif

