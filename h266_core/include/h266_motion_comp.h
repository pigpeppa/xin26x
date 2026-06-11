/***************************************************************************//**
 *
 * @file          h266_motion_comp.h
 * @brief         This file contains definitions related to motion compensation
 *                and motion compensation subroutine declarations.
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
#ifndef _h266_motion_comp_h_
#define _h266_motion_comp_h_

void Xin266LumaInterpHorVet (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266LumaInterpVet (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266LumaInterpHor (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266InterpCopy (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266InterpCopyU8S16 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void  Xin266LumaInterpVetU8S16 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266LumaInterpVet4xHU8S16_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266LumaInterpVet8xHU8S16_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266LumaInterpVet16xHU8S16_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266LumaInterpVetGt16xHU8S16_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266LumaInterpHorU8S16 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266LumaInterpHor4xHU8S16_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266LumaInterpHor8xHU8S16_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266LumaInterpHorGt8xHU8S16_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266LumaInterpHorGt8xHU8S16_AVX2 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void  Xin266LumaInterpHorVetU8S16 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266LumaInterpHorVet4xHU8S16_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266LumaInterpHorVet8xHU8S16_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266LumaInterpHorVetGt8xHU8S16_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266LumaInterpHorVetGt16xHU8S16_AVX2 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266ChromaInterpHorVet (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266ChromaInterpVet (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266ChromaInterpHor (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266ChromaInterpHorU8S16 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266ChromaInterpHor8xHU8S16_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266ChromaInterpHorGt8xHU8S16_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266ChromaInterpVetU8S16 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266ChromaInterpHorVetU8S16 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266ChromaInterpHorVet8xHU8S16_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266ChromaInterpHorVetGt8xHU8S16_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void  Xin266InterpAvgS16U8 (
    const SINT16 *src0,
    intptr_t     src0Stride,
    const SINT16 *src1,
    intptr_t     src1Stride,
    PIXEL        *dst,
    intptr_t     dstStride,
    UINT32       width,
    UINT32       height);

void Xin266InterpAvg8xHS16U8_SSSE3 (
    const SINT16 *src0,
    intptr_t     src0Stride,
    const SINT16 *src1,
    intptr_t     src1Stride,
    PIXEL        *dst,
    intptr_t     dstStride,
    UINT32       width,
    UINT32       height);

void Xin266InterpAvgGt8xHS16U8_SSSE3 (
    const SINT16 *src0,
    intptr_t     src0Stride,
    const SINT16 *src1,
    intptr_t     src1Stride,
    PIXEL        *dst,
    intptr_t     dstStride,
    UINT32       width,
    UINT32       height);

void Xin266InterpCopy4xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266InterpCopy8xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266InterpCopy16xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266InterpCopy32xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266InterpCopy64xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266InterpCopy32xH_AVX2 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266InterpCopy64xH_AVX2 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266InterpCopy128xH_AVX2 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266InterpCopy4xHU8S16_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266InterpCopy8xHU8S16_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266InterpCopyGt8xHU8S16_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266LumaInterpHor4xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266LumaInterpHor8xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266LumaInterpHor16xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266LumaInterpHorGt16xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266LumaInterpVet4xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266LumaInterpVet8xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266LumaInterpVet16xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266LumaInterpVetGt16xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266LumaInterpVetGt16xH_AVX2 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266LumaInterpHorVet4xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266LumaInterpHorVet8xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266LumaInterpHorVetGt8xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266LumaInterpVetS16U8_SSSE3 (
    const SINT16 *src,
    intptr_t     srcStride,
    PIXEL        *dst,
    intptr_t     dstStride,
    SINT32       frac,
    UINT32       width,
    UINT32       height,
    UINT32      filterIndex);

void Xin266LumaInterpHorVetGt8xH_AVX2 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266ChromaInterpHor8xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266ChromaInterpHorGt8xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266ChromaInterpVet8xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266ChromaInterpVetGt8xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266ChromaInterpHorVet4xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266ChromaInterpHorVet8xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266ChromaInterpHorVetGt8xH_SSSE3 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266LumaInterpHorGt16xH_AVX2 (
    const PIXEL *src,
    intptr_t    srcStride,
    PIXEL       *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height,
    UINT32      filterIndex);

void Xin266BiliInterpCopy (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height);

void Xin266BiliInterpCopy4xH_SSE4 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height);

void Xin266BiliInterpCopy8xH_SSE4 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height);

void Xin266BiliInterpCopy16xH_SSE4 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height);

void Xin266BiliInterpCopy32xH_AVX2 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height);

void Xin266BiliInterpCopy64xH_AVX2 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height);

void Xin266BiliInterpCopy128xH_AVX2 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height);

void Xin266BiliInterpHor (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height);

void Xin266BiliInterpHor4xH_SSE4 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height);

void Xin266BiliInterpHor8xH_SSE4 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height);

void Xin266BiliInterpHorGt8xH_AVX2 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height);

void Xin266BiliInterpVet (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height);

void Xin266BiliInterpVet4xH_SSE4 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height);

void Xin266BiliInterpVet8xH_SSE4 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height);

void Xin266BiliInterpVetGt8xH_AVX2 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height);

void  Xin266BiliInterpHorVet (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height);

void  Xin266BiliInterpHorVet4xH_SSE4 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height);

void  Xin266BiliInterpHorVet8xH_SSE4 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height);

void  Xin266BiliInterpHorVetGt8xH_AVX2 (
    const PIXEL *src,
    intptr_t    srcStride,
    SINT16      *dst,
    intptr_t    dstStride,
    SINT32      frac,
    UINT32      width,
    UINT32      height);

void  Xin266InterpWeightS16U8 (
    const SINT16 *src0,
    intptr_t     src0Stride,
    const SINT16 *src1,
    intptr_t     src1Stride,
    PIXEL        *dst,
    intptr_t     dstStride,
    UINT32       width,
    UINT32       height,
    SINT32       weightA,
    SINT32       weightB);

void  Xin266InterpWeightGt4xHS16U8_SSSE3 (
    const SINT16 *src0,
    intptr_t     src0Stride,
    const SINT16 *src1,
    intptr_t     src1Stride,
    PIXEL        *dst,
    intptr_t     dstStride,
    UINT32       width,
    UINT32       height,
    SINT32       weightA,
    SINT32       weightB);

void  Xin266InterpWeightGt8xHS16U8_AVX2 (
    const SINT16 *src0,
    intptr_t     src0Stride,
    const SINT16 *src1,
    intptr_t     src1Stride,
    PIXEL        *dst,
    intptr_t     dstStride,
    UINT32       width,
    UINT32       height,
    SINT32       weightA,
    SINT32       weightB);

#endif

